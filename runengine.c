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
#include "assert_np.h"
#include "hemul.h"
#include <regex.h>
#include <mqueue.h>

/* More than 3 should really not be needed, adding a few just in case */
#define MAX_SUBEXP 5

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/* This should be declared in time.h but isn't. Declared temporary here (FIX TBD)*/
char *strptime(const char *buf, const char *format,
       struct tm *tm);

/* This thread takes what it's got in buffer when buf-len is reached. */
void *output_pumper(void* inarg) {
	mqd_t q;
	int s,rc;
	char msg_buf[1024];


	assert_ext((q = mq_open( Q_OUTPUT_NAME , O_RDONLY, NULL, 0 )) > 0);
	while (1) {

		rc = mq_receive(q, msg_buf, 1024/*MYMSGSIZE*/, NULL);
		//fputs(stdout,"hello\n");

	}
}

int hemul_run() {
	char line[LINE_MAX];

	if (arguments.ptime >= 0) {
		while (fgets(line, LINE_MAX, mod_hemul.in) != NULL) {
			fputs(line, mod_hemul.out);
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

		while (fgets(line, LINE_MAX, mod_hemul.in) != NULL) {
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
			fputs(line, mod_hemul.out);
		}
	}
}
