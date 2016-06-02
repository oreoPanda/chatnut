/*user.c*/

/*Copyright (C) 2016 Jonas Fuglsang-Petersen*/

/*This file is part of chatnut.*/

/*chatnut is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

chatnut is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with chatnut.  If not, see <http://www.gnu.org/licenses/>.*/

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "user.h"

char *username = NULL;
char *buddy = NULL;
gboolean buddy_online = FALSE;		//for later use

/*returns a pointer to username, this pointer should not be free()d*/
extern char *get_username(void)
{
    return username;
}

/*set username by copying name*/
extern void set_username(const char *name)
{
	if(name)
	{
		//TODO missing a free?
		username = calloc( strlen(name)+1, sizeof(char) );
		strncpy( username, name, strlen(name)+1 );
	}
	else
	{
		username = NULL;
	}

    return;
}

/*returns a pointer to buddy, this pointer should not be free()d*/
extern char *get_buddy(void)
{
    return buddy;
}

/*set buddy by copying name*/
extern void set_buddy(char *name)
{
	if(buddy)
	{
		free(buddy);
	}
    buddy = calloc( strlen(name)+1, sizeof(char) );
    strncpy( buddy, name, strlen(name)+1 );

    return;
}
