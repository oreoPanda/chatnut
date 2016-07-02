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
#include "logger.h"
#include <string.h>
#include <errno.h>

extern void handle_buddy_is_set(void)
{
    if(!input_view_is_enabled() )
    {
    	enable_input_view();
    }

    return;
}

extern void handle_buddy_is_unset(void)
{
    if(input_view_is_enabled() )
    {
    	disable_input_view();
    }

    return;
}

extern void handle_lookup_success(const char *contact)
{
	if(add_contact_to_list(contact) )
	{
		add_contact_to_list_view(contact);
	}

    return;
}

extern void handle_login_success(const char *username)
{
    /*connect the "Add Contact" button to its handler*/
    enable_add_contact_button();
    
    /*set the username that user logged in with*/
    set_username(username);

    /*initialize the users directory*/
    if(init_user_directory() != 0 )
    {
    	//TODO don't change the GUI to logged in if this step doesn't work...
    }

    /*update window title*/
    update_window_title();

	/*load and show contacts TODO free model? or unref it?*/
	GtkListStore *model = create_contact_list_model();
	if(model)
	{
		toggle_list_view(TRUE, model);
	}
	else
	{
		edit_label("You don't have any contacts yet.");
		toggle_list_view(FALSE, NULL);
	}

    return;
}
