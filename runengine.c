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
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mtime.h>
#include <hemul.h>
#include <regex.h>
#include <mqueue.h>
#include <sys/param.h>
#include <unistd.h>

#include "assert_np.h"
#include "local.h"
/* More than 3 should really not be needed, adding a few just in case */
#define MAX_SUBEXP 5

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/* This should be declared in time.h but isn't. Declared temporary here (FIX
 * TBD)*/
char *strptime(const char *buf, const char *format,
       struct tm *tm);

enum msg_type {time_event, char_chunk};
struct qmsg {
	enum msg_type t;
	union {
		struct {
			int len;
			char *s;
		}line;
	}d;
};
#define MSGSIZE sizeof(struct qmsg)

/*
 * Timer event generator. Principal idea is to keep an extremely tight loop
 * and to trigger the dumper periodically.
 */
void *time_eventgen_thread(void* inarg) {
	int usec_sleep = *(int*)inarg;
	mqd_t q;
	struct qmsg m = {
		.t = time_event,
	};
	int rc;

	DBG_INF(3,("Thread [%s] starts... \n", __FUNCTION__));

	assert_ext(usec_sleep>0);
	assert_ext((q = mq_open( QNAME , O_WRONLY, NULL, 0 )) != (mqd_t)-1);

	while (1) {
		//assert_ext(pthread_mutex_unlock(&samplermod_data.master_event) == 0);
		usleep(usec_sleep);
		assert_ign((errno=pthread_mutex_lock(&mod_hemul.mx_userio)) == 0);
		assert_ign((errno=pthread_mutex_unlock(&mod_hemul.mx_userio)) == 0);
		if (hemul_args.debuglevel>=3) {
		DBG_INF(4,("Timer delivers event\n"));
		}
		rc = mq_send(q, (char*)&m, MSGSIZE, 1);
	}
}

static void dump_and_flush() {
	if (mod_hemul.curr_sz>0) {
		write(mod_hemul.fdout, mod_hemul.obuff, mod_hemul.curr_sz);
		mod_hemul.curr_sz=0;
		if (hemul_args.debuglevel>=0)
			memset(mod_hemul.obuff, 0, hemul_args.buffer_size+3);
	}
}

/* Recurse until ibuf is completely either output or transfered to obuf*/
static int swallow(char *ibuf, int len) {
	int obuf_left = hemul_args.buffer_size - mod_hemul.curr_sz;
	int ibuf_left = len;
	int olen = 0;
	int trans_sz=MIN(obuf_left, len);

	if (len==0)
		return 0;

	if (len+mod_hemul.curr_sz < hemul_args.buffer_size) {
		memcpy(&mod_hemul.obuff[mod_hemul.curr_sz], ibuf, len);
		mod_hemul.curr_sz += len;
	} else {
		/* Will fill buffer.
		 * 1) Stuff as much as possible.
		 * 2) output
		 * 3) Stuff the rest (recurse)*/

		assert_ext((mod_hemul.curr_sz+trans_sz)<= hemul_args.buffer_size);
		memcpy(&mod_hemul.obuff[mod_hemul.curr_sz], ibuf, trans_sz);
		mod_hemul.curr_sz += trans_sz;
		dump_and_flush();
		ibuf_left = len - trans_sz;
		if (ibuf_left>0)
			olen=swallow(&ibuf[trans_sz],ibuf_left);
		assert_ext(olen==ibuf_left);
	}
	return len;
}

/* This thread output and flushes what it's got in buffer when buff-length is
 * reached. */
void *buff_dumper(void* inarg) {
	mqd_t q;
	int rc,len;
	struct qmsg m;

	DBG_INF(3,("Thread [%s] starts... \n", __FUNCTION__));
	assert_ext((q = mq_open( QNAME , O_RDONLY, NULL, 0 )) != (mqd_t)-1);
	while (1) {

		rc = mq_receive(q, (char*)&m, MSGSIZE, NULL);
		if ( m.t == char_chunk ) {
			DBG_INF(4,("R: %s\n", m.d.line.s));
			len=swallow(m.d.line.s, m.d.line.len);
			assert_ext(len==m.d.line.len);
			free(m.d.line.s);
		} else if (	m.t == time_event ) {
			DBG_INF(3,("Flushing buffer\n"));
			dump_and_flush();
		}
	}
}

static void outputs(int lineN, const char *sin, mqd_t q) {
	struct qmsg m = {
		.t = char_chunk,
	};
	int rc;
	char s[LINE_MAX];

	assert_ign((errno=pthread_mutex_lock(&mod_hemul.mx_userio)) == 0);
	assert_ign((errno=pthread_mutex_unlock(&mod_hemul.mx_userio)) == 0);

	if (hemul_args.linenumb){
		rc=sprintf(s,"%d%s", lineN, hemul_args.linenumb);
		strncpy(&s[rc], sin, LINE_MAX-10);
	} else {
		strncpy(s, sin, LINE_MAX);
	}

	if (!mod_hemul.buff_mode) {
		/*Simple case, just get rid of the data to wherever it's supposed to
		 * go. Make sure output is not buffered. */
		write(mod_hemul.fdout, s, strnlen(s,LINE_MAX));
	} else {
		m.d.line.len = strlen(s);
		m.d.line.s = strdup(s);
		DBG_INF(4,("S: %s\n",m.d.line.s));
		rc = mq_send(q, (char*)&m, MSGSIZE, 1);
	}
}

int hemul_run() {
	char line[LINE_MAX];
	char oline[LINE_MAX];
	mqd_t q;
	int lineN=0;


	if (hemul_args.buffer_size > 0) {
		struct mq_attr qattr;

		DBG_INF(4,("Unlinking old queue name (if used). \n"));
		mq_unlink(QNAME);  //Don't assert - "failure" is normal here

		qattr.mq_maxmsg = 3;
		qattr.mq_msgsize = sizeof(struct qmsg);
		assert_ext((q = mq_open( QNAME, O_CREAT|O_WRONLY, OPEN_MODE_REGULAR_FILE, &qattr))
			!= (mqd_t)-1);

		assert_ret(pthread_create(
				&mod_hemul.th_out,
				NULL,
				buff_dumper,
				NULL
			) == 0 );
		if (hemul_args.buffer_timeout > 0)
			assert_ret(pthread_create(
					&mod_hemul.th_timer,
					NULL,
					time_eventgen_thread,
					&hemul_args.buffer_timeout
				) == 0 );
	}

	if (hemul_args.ptime >= 0) {
		while (fgets(line, LINE_MAX, mod_hemul.fin) != NULL) {
			assert_ign((errno=pthread_mutex_lock(&mod_hemul.mx_userio)) == 0);
			assert_ign((errno=pthread_mutex_unlock(&mod_hemul.mx_userio)) == 0);
			outputs(++lineN,line, q);
			usleep(hemul_args.ptime);
		}
	} else {
		int rc;
		regmatch_t mtch_idxs[MAX_SUBEXP+1];
		char err_str[80];
		char time_str[80];
		struct timeval last_time, curr_time, diff_time;
		int i,first_round = 1;
		struct tm tm;
		char *lstr;
		int usec; /*Extra temporary usec*/
		time_t sepoch;

		while (fgets(line, LINE_MAX, mod_hemul.fin) != NULL) {
			assert_ign((errno=pthread_mutex_lock(&mod_hemul.mx_userio)) == 0);
			assert_ign((errno=pthread_mutex_unlock(&mod_hemul.mx_userio)) == 0);
			/* Find out how long time to usleep */
			rc=regexec(&mod_hemul.ts_regex->rgx, line, MAX_SUBEXP+1,
				mtch_idxs, 0); if (rc) {
				regerror(rc, &mod_hemul.ts_regex->rgx, err_str, 80);
				fprintf(stderr, "Regexec faliure: %s\n", err_str);
				return(rc);
			} else {
				/*Note: The correct idx to match against is in hemul_args*/
				int idx = mod_hemul.ts_regex->idx;
				strncpy(
					time_str,
					&line[mtch_idxs[idx].rm_so],
					mtch_idxs[idx].rm_eo - mtch_idxs[idx].rm_so);
				if (!hemul_args.ts_format) {
					/* No format given, assume numerical (e.g. kernel-time
					 * or numerical epoc time). */
					sscanf(time_str,"%d.%d",
						(int*)&curr_time.tv_sec,
						(int*)&curr_time.tv_usec);
				} else {
					/* Format is given. I.e. time stamp-needs to be parsed
					 * to be understood */

					strptime("0","%s",&tm);
					assert_ext( (lstr=strptime( time_str,
						hemul_args.ts_format, &tm )) != NULL );
					if (lstr[0]=='.') {
						/*
						* Permit reading fraction of a second. Assume this to
						* be microsecond (6 decimal). Adjust if not. Note, this
						* is a hack-on to strptime wih wount parse fractions
						* of a second (because struct tm has no field for
						* it) and works only on time-stamps where decimal
						* seconds are the last to be parsed.
						*/
						lstr++;
						usec=atoi(lstr);
						assert_ext(usec<1000000);
						for (i=strnlen(lstr,6); i<6; i++ )
							usec *= 10;
					}
					/* Parsing is now ready. Transfer ts to curr_time*/
					curr_time.tv_usec = usec;
					sepoch = timegm(&tm);
					assert_ext(sepoch != (time_t)(-1));
					curr_time.tv_sec = sepoch;
				}
				if (first_round) {
					first_round = 0;
					memcpy(&last_time, &curr_time,
						sizeof(struct timeval));
				}
				diff_time = tv_diff(&last_time,&curr_time);

				assert_ext(diff_time.tv_sec>=0);

				tv_sleep(&diff_time);
				memcpy(&last_time, &curr_time,
					sizeof(struct timeval));
			}
			/* Output the line */
			outputs(++lineN,line, q);
		}
	}
	return 0;
}
