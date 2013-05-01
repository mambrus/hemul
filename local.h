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

/* Local interface & stuff for HEMUL  */

#ifndef local_h
#define local_h
#include <pthread.h>

#define QNAME "/das_Q"

#define INFO_TO( F, FNKN, S ) \
	{ \
		if (arguments.verbose) { \
			FNKN S; \
			fflush( F ); \
		} \
	}

#if ( DBGLVL == 0 )
#  define INFO( S ) ((void)0)
#elif ( DBGLVL == 1 )
#  define INFO( S ) INFO_TO( stdout, printf, S )
#elif ( DBGLVL == 2 )
#  define INFO( S ) INFO_TO( stdout, printf, S )
#elif ( DBGLVL == 3 )
#  define eprintf(...) fprintf (stderr, __VA_ARGS__)
#  define INFO( S ) INFO_TO( stderr, eprintf, S )
#else
#  error bad value of DBGLVL
#endif

/* Globals */
struct mod_hemul
{
	pthread_t th_out;
	pthread_t th_timer;
	int pipe_created;
	struct ts_regex *ts_regex;
	FILE *fin;
	FILE *fout;
	int fdin;
	int fdout;
	int buff_mode;
	char *obuff;
};
extern struct mod_hemul mod_hemul;

#endif /* local_h */
