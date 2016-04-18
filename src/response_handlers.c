/*response_handlers.c*/

/*Copyright (C) 2016 Jonas Fuglsang-Petersen*/

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

#include "user.h"
#include "file_operations.h"
#include "gui.h"
#include <string.h>
#include <errno.h>

gboolean was_logged_in = FALSE;

extern void handle_buddy_is_set(void)
{
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

    /*check if somebody was logged in previously*/
    if(was_logged_in)
    {
	chdir("../");
    }
    /*initialize the users directory TODO this function needs to exit if init_user_dir errors*/
    if( init_user_directory() != 0 )
    {
        fprintf( stderr, "Error initializing the users directory and files in $HOME/.chatnut. Did the permissions change?\n" );
    }

	//set was_logged_in
	was_logged_in = TRUE;

    /*update window title*/
    update_window_title();

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