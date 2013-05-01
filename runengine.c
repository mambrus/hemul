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

	INFO(("Thread [%s] starts... \n", __FUNCTION__));

	assert_ext(usec_sleep>0);
	assert_ext((q = mq_open( QNAME , O_WRONLY, NULL, 0 )) != (mqd_t)-1);

	while (1) {
		//assert_ext(sem_post(&samplermod_data.master_event) == 0);
		usleep(usec_sleep);
		if (arguments.debuglevel>=3) {
			INFO(("Timer delivers event\n"));
		}
		rc = mq_send(q, (char*)&m, MSGSIZE, 1);
	}
}

/* This thread output and flushes what it's got in buffer when buff-length is
 * reached. */
void *buff_dumper(void* inarg) {
	mqd_t q;
	int s,rc;
	struct qmsg m;

	INFO(("Thread [%s] starts... \n", __FUNCTION__));
	assert_ext((q = mq_open( QNAME , O_RDONLY, NULL, 0 )) != (mqd_t)-1);
	while (1) {

		rc = mq_receive(q, (char*)&m, MSGSIZE, NULL);
		if ( m.t == char_chunk ) {
			if (arguments.debuglevel>=3) {
				INFO(("R: %s\n", m.d.line.s));
			}
			fputs(m.d.line.s, mod_hemul.fout);
			//TBD Should be this line really TBD
			//fputs(mod_hemul.obuff, mod_hemul.fout);
			free(m.d.line.s);
		} else if (	m.t == time_event ) {
			if (arguments.debuglevel>=3) {
				INFO(("Flushing buffer\n"));
			}
		}
	}
}

static int outputs(const char *s, mqd_t q) {
	struct qmsg m = {
		.t = char_chunk,
	};

	int rc;
	if (!mod_hemul.buff_mode) {
		/*Simple case, just get rid of the data to wherever it's supposed to
		 * go */
		fputs(s, mod_hemul.fout);
	} else {
		m.d.line.len = strlen(s);
		m.d.line.s = strdup(s);
		INFO(("S: %s\n",m.d.line.s));
		rc = mq_send(q, (char*)&m, MSGSIZE, 1);
	}
}

int hemul_run() {
	char line[LINE_MAX];
	mqd_t q;


	if (arguments.buffer_timeout > 0) {
		struct mq_attr qattr;

		INFO(("Unlinking old queue name (if used). \n"));
		mq_unlink(QNAME);  //Don't assert - "failure" is normal here

		qattr.mq_maxmsg = 3;
		qattr.mq_msgsize = sizeof(struct qmsg);
		assert_ext((q = mq_open( QNAME, O_CREAT|O_WRONLY, 0666, &qattr))
			!= (mqd_t)-1);

		assert_ret(pthread_create(
				&mod_hemul.th_timer,
				NULL,
				time_eventgen_thread,
				&arguments.buffer_timeout
			) == 0 );
		assert_ret(pthread_create(
				&mod_hemul.th_out,
				NULL,
				buff_dumper,
				NULL
			) == 0 );
	}

	if (arguments.ptime >= 0) {
		while (fgets(line, LINE_MAX, mod_hemul.fin) != NULL) {
			fputs(line, mod_hemul.fout);
			usleep(arguments.ptime);
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
		int usec; /*Extra tempoary usec*/
		time_t sepoch;

		while (fgets(line, LINE_MAX, mod_hemul.fin) != NULL) {
			/* Find out how long time to usleep */
			rc=regexec(&mod_hemul.ts_regex->rgx, line, MAX_SUBEXP+1,
				mtch_idxs, 0); if (rc) {
				regerror(rc, &mod_hemul.ts_regex->rgx, err_str, 80);
				fprintf(stderr, "Regexec faliure: %s\n", err_str);
				return(rc);
			} else {
				/*Note: The correct idx to match against is in arguments*/
				int idx = mod_hemul.ts_regex->idx;
				strncpy(
					time_str,
					&line[mtch_idxs[idx].rm_so],
					mtch_idxs[idx].rm_eo - mtch_idxs[idx].rm_so);
				if (!arguments.ts_format) {
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
						arguments.ts_format, &tm )) != NULL );
					if (lstr[0]='.') {
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
			outputs(line, q);
		}
	}
}
