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

/* Local interface & stuff for the mtime library/module  */

#ifndef local_h
#define local_h
#include "include/mtime.h"

/* Mod-global helper macros */

#define SEC( TV ) ((int)TV.tv_sec)
#define USEC( TV ) ((int)TV.tv_usec)
#define xstr(s) str(s)
#define str(s) #s
#define __init __attribute__((constructor))
#define __fini __attribute__((destructor))

#ifndef VERBOSE_TYPE
#define VERBOSE_TYPE 3
#endif

#ifndef TESTSPEED
#define TESTSPEED 3
#endif

#define INFO_TO( F, FNKN, S ) \
	{ \
		if (mtimemod_settings.verbose) { \
			FNKN S; \
			fflush( F ); \
		} \
	}

#if ( VERBOSE_TYPE == 0 )
#  define INFO( S ) ((void)0)
#  define DUSLEEP( T ) ((void)0)
#elif ( VERBOSE_TYPE == 1 )
#  define INFO( S ) INFO_TO( stdout, printf, S )
#  define DUSLEEP( U ) usleep( U )
#elif ( VERBOSE_TYPE == 2 )
#  define INFO( S ) INFO_TO( stdout, printf, S )
#  define DUSLEEP( U ) usleep( U )
#elif ( VERBOSE_TYPE == 3 )
#  define eprintf(...) fprintf (stderr, __VA_ARGS__)
#  define INFO( S ) INFO_TO( stderr, eprintf, S )
#  define DUSLEEP( U ) usleep( U )
#else
#error bad value of VERBOSE_TYPE
#endif

#if ( TESTSPEED == 5 )
#  define SMALL      0
#  define MEDIUM     0
#  define LONG       0
#  undef  DUSLEEP
#  define DUSLEEP( T ) ((void)0)
#elif ( TESTSPEED == 4 )
#  define SMALL      1
#  define MEDIUM     10
#  define LONG       50
#elif ( TESTSPEED == 2 )
#  define SMALL      10
#  define MEDIUM     100
#  define LONG       500
#elif ( TESTSPEED == 3 )
#  define SMALL      100
#  define MEDIUM     1000
#  define LONG       5000
#elif ( TESTSPEED == 2 )
#  define SMALL      1000
#  define MEDIUM     10000
#  define LONG       50000
#elif ( TESTSPEED == 1 )
#  define SMALL      10000
#  define MEDIUM     100000
#  define LONG       500000
#elif ( TESTSPEED == 0 )
#  define SMALL      100000
#  define MEDIUM     1000000
#  define LONG       5000000
#else
#error bad value of TESTSPEED
#endif

/* End of Mod-global helper macros */

/* States of internal mod-global data (empty in most modules).
 *
 */
struct mtimemod_data {
	int isinit;
};

/* Non-public (i.e. externally hidden), module global API go here */

/* End non-public */

extern struct mtimemod_settings mtimemod_settings;
extern struct mtimemod_data mtimemod_data;


#endif /* local_h */

