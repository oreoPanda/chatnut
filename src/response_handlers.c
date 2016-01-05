/*response_handlers.c*/

#include "user.h"
#include "file_operations.h"
#include <string.h>
#include <errno.h>

extern void handle_buddy_is_set(char *username)
{
    set_buddy(username);    

    return;
}

extern void handle_lookup_success(char *contact)
{
    FILE *contactfile = NULL;
    char *filename = "contactlist";	//is this on heap or stack? no malloc(), so no free() or?

    /*open file and write to it*/
    contactfile = fopen( filename, "a" );		//NULL check if file not found (errno)
    if(contactfile)
    {
        fprintf( contactfile, "%s\n", contact );
        fclose(contactfile);
    }
    else
    {
        fprintf( stderr, "Can't append contact to contactlist in the users chatnut directory at $HOME/.chatnut/[user]: %s\n", strerror(errno) );
    }

    return;
}

extern void handle_login_success(const char *username)
{
    /*connect the "Add Contact" button to its handler*/
    enable_add_contact_button();
    
    /*set the username that user logged in with*/
    set_username(username);

    if( init_user_directory() != 0 )
    {
        fprintf( stderr, "Error initializing the users directory and files in $HOME/.chatnut. Did the permissions change?\n" );
    }

    return;
}