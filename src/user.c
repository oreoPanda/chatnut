/*user.c*/

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
    username = calloc( strlen(name)+1, sizeof(char) );
    strncpy( username, name, strlen(name)+1 );

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
