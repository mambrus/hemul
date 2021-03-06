/***************************************************************************
 *   Copyright (C) 2013 by Michael Ambrus                                  *
 *   ambrmi09@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <mtime.h>
#include <regex.h>
#include <pthread.h>
#include <semaphore.h>

#include <hemul.h>
#include "assert_np.h"
#include "local.h"

/* More than 3 should really not be needed, adding a few just in case */
#define MAX_SUBEXP 5

struct mod_hemul mod_hemul = {
	.pipe_created = 0,
	.buff_mode = 0,
	.curr_sz = 0,
	.obuff = NULL,
	.fdout = -1,
	.fout = NULL,
	.fdin = -1,
	.fin = NULL,
	.ts_regex = &hemul_args.ts_regex,
	.fdin_user = 0,
	.fdout_user = 1,
	.fin_user = NULL,
	.fout_user = NULL,
	.echo=0,
};

int hemul_init() {
	int rc;
	int lerrno, create_file=0;
	struct stat isbuf,osbuf;
	char err_str[80];

	assert_ext((errno = pthread_mutex_init(&mod_hemul.mx_send, 0)) == 0);
	assert_ext((errno = pthread_mutex_init(&mod_hemul.mx_userio, 0)) == 0);
	assert_ign(!pthread_create(
		&mod_hemul.th_userio,
		NULL,
		userio_thread,
		NULL
	));
	mod_hemul.fin_user=stdin;
	mod_hemul.fout_user=stdout;

	if (hemul_args.ofilename != NULL) {
		rc = stat(hemul_args.ofilename, &osbuf);
		lerrno = errno;
		if ( rc==-1 ) {
			if (lerrno==ENOENT )
				create_file = 1;
			else
				assert_ret("stat() failed unexpectedly" == NULL);
		}
	}
	if (hemul_args.ifilename != NULL) {
		rc = stat(hemul_args.ifilename, &isbuf);
		lerrno = errno;
		if ( rc==-1 )
			assert_ret("stat() failed unexpectedly" == NULL);
	}

	if (hemul_args.piped_output) {
		assert_ret(hemul_args.ofilename != NULL);
		/* Check if exists, & is a pipe, create if not */
		if (create_file) {
			INFO(("Creating named pipe %s\n",hemul_args.ofilename));
			assert_ret(mkfifo(hemul_args.ofilename,
				S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)==0);
			mod_hemul.pipe_created = 1;
		}
	}

	if (mod_hemul.ts_regex->str) {
		INFO(("Compileing regexp %s for index %d\n",
			mod_hemul.ts_regex->str,
			mod_hemul.ts_regex->idx));
		rc=regcomp(
			&(mod_hemul.ts_regex->rgx),
			mod_hemul.ts_regex->str,
			REG_EXTENDED
		);
		if (rc) {
			regerror(rc, &(mod_hemul.ts_regex->rgx), err_str, 80);
			fprintf(stderr, "Regcomp faliure: %s\n", err_str);
			return(rc);
		}
	} else if (hemul_args.ptime < 0) {
		fprintf(stderr, "ERROR: Must define either regex or period time.\n");
		return(1);
	}

	if (hemul_args.ofilename){
		assert_ret((mod_hemul.fdout=open(hemul_args.ofilename,
			O_WRONLY | O_CREAT, OPEN_MODE_REGULAR_FILE)) != -1);
		assert_ret((mod_hemul.fout=fdopen(mod_hemul.fdout,"a")) != NULL);
	} else {
		mod_hemul.fout=stdout;
		mod_hemul.fdout=1;
	}
	if (hemul_args.ifilename){
		assert_ret((mod_hemul.fdin=open(hemul_args.ifilename,
			O_RDONLY)) != -1);
		assert_ret((mod_hemul.fin=fdopen(mod_hemul.fdin,"r")) != NULL);
	} else {
		mod_hemul.fin=stdin;
		mod_hemul.fdin=0;
	}

	if (hemul_args.buffer_size > 0) {
		mod_hemul.buff_mode = 1;
		assert_ret((mod_hemul.obuff = malloc(hemul_args.buffer_size+3)) != NULL);
	}
	mod_hemul.echo = hemul_args.echo;
	mod_hemul.running = 1;

	return 0;
}

int hemul_fini() {
	int cancel_val[4];

	if (mod_hemul.buff_mode){
		assert_ign((errno=pthread_mutex_lock(&mod_hemul.mx_send)) == 0);
		mod_hemul.running = 0;
		/* at this point we know that "th_timer" does not and will never call "mq_send()" */
		assert_ign((errno=pthread_mutex_unlock(&mod_hemul.mx_send)) == 0);
		hemul_quit_dumper_thread();

		if (hemul_args.buffer_timeout>0) {
			pthread_join(mod_hemul.th_out,NULL);
			pthread_join(mod_hemul.th_timer,NULL);
		} else {
			/* Will never timeout. Kill thread, it's cancellations function
			 * will flush buffer */
			pthread_join(mod_hemul.th_out,NULL);
		}
	}
	if (hemul_args.ofilename){
		assert_ret(fclose(mod_hemul.fout)==0);
	}
	if (hemul_args.ifilename){
		assert_ret(fclose(mod_hemul.fin)==0);
	}

	if (mod_hemul.pipe_created)
		assert_ret(remove(hemul_args.ofilename)==0);

	return 0;
}

