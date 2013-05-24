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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <assert_np.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <mtime.h>
#include "local.h"

/* Module initializers. Not much to do here for init, but fini might have
 * clean-up to do, iterate through the list and:
 * * kill threads
 * * Free allocated memory
 */
void __init __mtime_init(void) {
#ifdef INITFINI_SHOW
	fprintf(stderr,">>> Running module _init in ["__FILE__"]\n"
			">>> using CTORS/DTORS mechanism ====\n");
#endif
	assert_ext(!mtimemod_data.isinit);
	if (mtimemod_settings.clock_type == AUTODETECT) {
		struct timespec tv;
#ifdef CLOCK_MONOTONIC_RAW
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &tv) == 0)
			mtimemod_settings.clock_type = KERNEL_CLOCK;
#else
#  warning CLOCK_MONOTONIC_RAW undefined.
#  warning Best aproximation of kernel-time will be using CLOCK_MONOTONIC
		if (clock_gettime(CLOCK_MONOTONIC, &tv) == 0)
			mtimemod_settings.clock_type = KERNEL_CLOCK;
#endif
		else
			mtimemod_settings.clock_type = CALENDER_CLOCK;
	}
	mtimemod_data.isinit=1;
	/* Note: mtimemod_settings.isinit=1 is not set here but by module
	   aggregate */
}

void __fini __mtime_fini(void) {
#ifdef INITFINI_SHOW
	fprintf(stderr,">>> Running module _fini in ["__FILE__"]\n"
			">>> using CTORS/DTORS mechanism\n");
#endif
	if (!mtimemod_data.isinit)
		/* Someone already did this in a more controlled way. Nothing to do
		 * here, return */
		 return;
	mtimemod_data.isinit=0;
	mtimemod_settings.isinit=0;
}
