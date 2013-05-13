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
#include <termios.h>
#include <unistd.h>

#include "assert_np.h"
#include "local.h"
/* This file manages interactive use */


/* Overloaded assert_exit. Restores termio. */
static void hemul_exit(int status) {
	DBG_INF(0,("HEMUL exits via: [%s].\n",__FUNCTION__));
	tcsetattr (mod_hemul.fdin_user, TCSANOW, &mod_hemul.orig_termio_mode);
	mod_hemul.orig_exit(status);
}

/* This thread manages interactivity */
void *userio_thread(void* inarg) {
	int quit=0,rc;
	char ch;
	int running=1, cmd_mode=0; /* Technically you can be in both, therefor two
								  variables */
	struct termios new_termio_mode;

	/* Save orig termio before changing it */
	tcgetattr (mod_hemul.fdin_user, &mod_hemul.orig_termio_mode);

	/* Install new assert_np exit handler to restore as part of dieing */
	mod_hemul.orig_exit = get_exit();
	set_exit(hemul_exit);

	/* Prepare termio for raw unbuffered mode */
	memcpy (&new_termio_mode, &mod_hemul.orig_termio_mode,
		sizeof (struct termios));
	new_termio_mode.c_lflag &= ~(ICANON | ECHO);
	new_termio_mode.c_cc[VTIME] = 0;
	new_termio_mode.c_cc[VMIN] = 1;

	tcsetattr (mod_hemul.fdin_user, TCSANOW, &new_termio_mode);

	INFO(("User interaction thread starting...\n"));

	while (!quit) {
		assert_ext(read(mod_hemul.fdin_user, &ch, 1) == 1);
		DBG_INF(6,("Key pressed: [0x%02X:'%c']\n",ch,ch));
		if (ch==' ') {
			if (running) {
				/*Pausing data-output*/
				assert_ign((
					errno=pthread_mutex_lock(&mod_hemul.mx_userio)
				) == 0 );
				/*
				tcsetattr (mod_hemul.fdin_user, TCSANOW,
					&mod_hemul.orig_termio_mode);
				fprintf(mod_hemul.fout_user,"HEMUL> ");
				fflush(mod_hemul.fout_user);
				cmd_mode = 1;
				*/
				running = 0;
				DBG_INF(4,("HEMUL transfer engine paused. "
					"Entering command mode.\n"));
			} else {
				/*Resuming data-output*/
				assert_ign((
					errno=pthread_mutex_unlock(&mod_hemul.mx_userio)
				) == 0 );
				/*
				tcsetattr (mod_hemul.fdin_user, TCSANOW, &new_termio_mode);
				cmd_mode = 0;
				*/
				running = 1;
				DBG_INF(4,("HEMUL transfer engine resumed. "
					"Command mode exit.\n"));
			}
		}

		//quit = command();
	}
	tcsetattr (mod_hemul.fdin_user, TCSANOW, &mod_hemul.orig_termio_mode);
}
