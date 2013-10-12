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
#include <semaphore.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define QNAME "/das_Q"

#define VERBOSE_TO( F, FNKN, S ) \
	{ \
		if (hemul_args.verbose) { \
			FNKN S; \
			fflush( F ); \
		} \
	}

#define DBG_TO( F, FNKN, S, L ) \
	{ \
		if (L <= hemul_args.debuglevel) { \
			FNKN S; \
			fflush( F ); \
		} \
	}

#if ( VERBOSE_TYPE == 0 )
#  define INFO( S ) ((void)0)
#  define DBG_INF( L, S ) DBG_TO( stdout, printf, S, L )
#elif ( VERBOSE_TYPE == 1 )
#  define INFO( S ) VERBOSE_TO( stdout, printf, S )
#  define DBG_INF( L, S ) DBG_TO( stdout, printf, S, L )
#elif ( VERBOSE_TYPE == 2 )
#  define INFO( S ) VERBOSE_TO( stdout, printf, S )
#  define DBG_INF( L, S ) DBG_TO( stdout, printf, S , L )
#elif ( VERBOSE_TYPE == 3 )
#  define eprintf(...) fprintf (stderr, __VA_ARGS__)
#  define INFO( S ) VERBOSE_TO( stderr, eprintf, S )
#  define DBG_INF( L, S ) DBG_TO( stderr, eprintf, S , L )
#else
#  error bad value of VERBOSE_TYPE
#endif

/* OPEN_MODE_REGULAR_FILE is 0666, open() will apply umask on top of it */
#define OPEN_MODE_REGULAR_FILE \
	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/* Globals */
struct mod_hemul
{
	pthread_t th_out;
	pthread_t th_userio;
	pthread_mutex_t mx_userio;
	pthread_t th_timer;
	int pipe_created;
	struct ts_regex *ts_regex;
	FILE *fin;
	FILE *fout;
	int fdin;
	int fdout;
	int buff_mode;
	char *obuff;
	int curr_sz;
	FILE *fin_user;
	FILE *fout_user;
	int fdin_user;
	int fdout_user;
	struct termios orig_termio_mode;
	void (*orig_exit)(int status);
	pthread_mutex_t mx_send;
	int running;
	int echo;
};
extern struct mod_hemul mod_hemul;

/* mod-global threads */
void *userio_thread(void* inarg);

void hemul_quit_dumper_thread();

#endif /* local_h */

