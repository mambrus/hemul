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

#include <hemul.h>
#include "assert_np.h"
#include "local.h"

/* More than 3 should really not be needed, adding a few just in case */
#define MAX_SUBEXP 5

struct mod_hemul mod_hemul = {
	.pipe_created = 0,
	.buff_mode = 0,
	.ts_regex = &arguments.ts_regex,
};

int hemul_init() {
	int rc;
	int lerrno, create_file=0;
	struct stat isbuf,osbuf;
	char err_str[80];

	if (arguments.ofilename != NULL) {
		rc = stat(arguments.ofilename, &osbuf);
		lerrno = errno;
		if ( rc==-1 )
			if (lerrno==ENOENT )
				create_file = 1;
			else
				assert_ret("stat() failed unexpectedly" == NULL);
	}
	if (arguments.ifilename != NULL) {
		rc = stat(arguments.ifilename, &isbuf);
		lerrno = errno;
		if ( rc==-1 )
			assert_ret("stat() failed unexpectedly" == NULL);
	}

	if (arguments.piped_output) {
		assert_ret(arguments.ofilename != NULL);
		/* Check if exists, & is a pipe, create if not */
		if (create_file) {
			INFO(("Creating named pipe %s\n",arguments.ofilename));
			assert_ret(mkfifo(arguments.ofilename,
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
	} else if (arguments.ptime < 0) {
		fprintf(stderr, "ERROR: Must define either regex or period time.\n");
		return(1);
	}

	if (arguments.ofilename){
		assert_ret((mod_hemul.fdout=open(arguments.ofilename,
			O_WRONLY | O_APPEND | O_CREAT)) != -1);
		assert_ret((mod_hemul.fout=fdopen(mod_hemul.fdout,"a")) != NULL);
	} else {
		mod_hemul.fout=stdout;
		mod_hemul.fdout=1;
	}
	if (arguments.ifilename){
		assert_ret((mod_hemul.fdin=open(arguments.ifilename,
			O_RDONLY)) != -1);
		assert_ret((mod_hemul.fin=fdopen(mod_hemul.fdin,"r")) != NULL);
	} else {
		mod_hemul.fin=stdin;
		mod_hemul.fdin=0;
	}

	if (arguments.buffer_size > 0) {
		mod_hemul.buff_mode = 1;
		assert_ret((mod_hemul.obuff = malloc(arguments.buffer_size+3)) != NULL);
	}

	return 0;
}

int hemul_fini() {
	int cancel_val[4];

	if (mod_hemul.buff_mode){
		if (arguments.buffer_timeout>0) {
			pthread_join(mod_hemul.th_out,NULL/*&cancel_val*/);
			pthread_cancel(mod_hemul.th_timer);
		} else {
			/* Will never timeout. Kill thread, it's cancellations function
			 * will flush buffer */
			pthread_cancel(mod_hemul.th_out);
		}
	}
	if (arguments.ofilename){
		assert_ret(fclose(mod_hemul.fout));
	}
	if (arguments.ifilename){
		assert_ret(fclose(mod_hemul.fin));
	}

	if (mod_hemul.pipe_created)
		assert_ret(remove(arguments.ofilename));

	return 0;
}

