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

/* This file contains various helper functions to keep track of, and handle,
 * time arithmetics */

/* NOTE: !!! These functions are in need of thorough testing !!! (TBD) */

#include <stdio.h>
#include <assert_np.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <stdint.h>
#include <string.h>
#include <mtime.h>

/* Include module common stuff */
#include "local.h"

/* Return "time-now" in best available format according to modality.
 * Currently using either:
 *  int clock_gettime(clockid_t clock_id, struct timespec *tp);
 *  int gettimeofday(struct timeval *restrict tp, void *restrict tzp);
 *
 * Both represent time in some "absolute" form. What's most interesting is a
 * time-representation as close to kernel-time as possible, in which case
 * clock_gettime(CLOCK_MONOTONIC_RAW) would be preferred. This however
 * either isn't available at all systems or requires root-privileges.
 * Therefore as first fall-back: CLOCK_MONOTONIC is used and as second
 * fall-back: calender-time in form of clock_gettime is used.
 *
 * Note however that all SW clocks, even global ones, are drift compensated
 * which is a disadvantage if a sample is to be compared with a kernel
 * log-entry event (except CLOCK_MONOTONIC_RAW i.e. which is a drift
 * uncompensated clock) */
int time_now(struct timespec *tp) {
	int rc;
	switch (mtimemod_settings.clock_type) {
		case KERNEL_CLOCK:
#ifdef CLOCK_MONOTONIC_RAW
			rc=clock_gettime(CLOCK_MONOTONIC_RAW, tp);
#else
			rc=clock_gettime(CLOCK_MONOTONIC, tp);
#endif
			break;
		case CALENDER_CLOCK:
			rc=clock_gettime(CLOCK_REALTIME, tp);
			//Alternatively clock_gettimeofday can be used but with lower
			//resolution.
			break;
		case AUTODETECT:
		default:
			fprintf(stderr,"clock-type not supported\n");
			rc=-1;
	}
	return rc;
}

int tv_sleep(struct timeval *s) {
	sleep(s->tv_sec);
	usleep(s->tv_usec);
	return 0;
}
