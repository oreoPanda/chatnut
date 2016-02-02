/*response_handlers.c*/

#include "user.h"
#include "file_operations.h"
#include "gui.h"
#include <string.h>
#include <errno.h>

extern void handle_buddy_is_set(char *username)
{
    set_buddy(username);

    if( input_view_get_enabled() == FALSE )
    {
    	enable_input_view();
    }

    return;
}

extern void handle_lookup_success(const char *contact)
{
    if( add_contact_to_list(contact) )
    {
    	if( window_contains_label() )
    	{
    		destroy_label();
    		GtkListStore *model = create_contact_list_model();
    		show_list_view(model);
    		populate_window_with_list();
    	}
    	else
    	{
    		add_contact_to_list_view(contact);
    	}
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

    /*load and show contacts*/
	destroy_label();
	GtkListStore *model = create_contact_list_model();
	if(model)
	{
		show_list_view(model);
		populate_window_with_list();
	}
	else
	{
		create_label("You don't have any contacts yet.");
		populate_window_with_label();
	}

    return;
}
