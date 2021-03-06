/*chatnut.c*/

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

/*TODO FIXME TODO FIXME logo: squirrel that cracks a nut, a speech bubble pops out of the nut. Drawback: no chestnut... (name change??)*/
//or 2 squirrels with one chestnut each, they throw them which messages written on it    //conflict, not open-source like...
//or they pull on two sides of one chestnut
//maybe something not scene-like

#include <stdlib.h>
#include <string.h>

#include "connection.h"
#include "gui.h"
#include "gui_interaction.h"
#include "logger.h"
#include "response_handlers.h"
#include "file_operations.h"
#include "user.h"

typedef enum reply commandreply;

enum reply
{
    CONNECTED = 32,
    HELP,
    BUDDY_IS_SET,
    BUDDY_IS_UNSET,
    BUDDY_NOT_EXIST,
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    REGISTRATION_SUCCESS,
    REGISTRATION_FAILURE,
    LOOKUP_SUCCESS,
    LOOKUP_FAILURE,
    MESSAGE,
    NOARG,
    NOMEM,
    ERROR
};

/*returns a pointer to the part of message where the actual message begins, this pointer should not be freed separately from message*/
const char *strip_buddyname(const char *message, char **buddyname)
{
    int namelength = -1;

    /*calculate length of buddyname*/
    while( *(message + ++namelength) != ' ' );

    /*copy buddyname from message*/
    *buddyname = calloc( namelength+1, sizeof(char) );
    strncpy( *buddyname, message, namelength+1 );//copies a space to last index
    *( *(buddyname)+namelength ) = '\0';

    /*message_only should point to the start of the actual message*/
    const char *message_only = message+namelength+1;

    return message_only;
}

static void create_gui(void)
{
	create_window();
	create_main_pane();
	create_left_pane();
	create_history_scrollbox();
	create_history_view();
	create_input_scrollbox();
	create_input_view();
	create_right_grid();
	create_list_view();
	create_label("You are not logged in yet.");
	create_buttons();

	populate_window();

	return;
}
//TODO handle a registration failure (pop up the dialog again)
static void evaluate_incoming(const char *data)
{
	commandreply indicator = *data;
	const char *message = data+1;

	switch(indicator)
	{
		case CONNECTED:
		{
			logg("Server Reply", "Connected");

			popup_login("Enter your login data here");
			break;
		}
		case LOGIN_FAILURE:
		{
			logg("Server Reply", "Login failure");

			popup_login("Login attempt failed");
			break;
		}
		case LOGIN_SUCCESS:
		{
			logg("Server reply", "Login success");

			/*get buddyname string*/
			char *buddyname = NULL;
			strip_buddyname( message, &buddyname );//return value (which should net be free()d) ignored, username should be free()d
			handle_login_success(buddyname);
			free(buddyname);

			break;
		}
		case REGISTRATION_SUCCESS:
		{
			logg("Server reply", "Registration success");
			popup_login("Enter your login data here");

			break;
		}
		case BUDDY_IS_SET:
		{
			logg("Server Reply", "Buddy is now set");
			handle_buddy_is_set();

			break;
		}
		case BUDDY_IS_UNSET:
		{
			logg("Server Reply", "Buddy is now unset");
			handle_buddy_is_unset();

			break;
		}
		case BUDDY_NOT_EXIST:		//TODO could maybe take out this one
		{
			logg("Server Reply", "Buddy does not exist");

			break;
		}
		case LOOKUP_FAILURE:
		{
			logg("Server Reply", "User does not exist");
			break;
		}
		/*process of choosing buddy works, code analysis needs to be done TODO*/
		case LOOKUP_SUCCESS:
		{
			logg("Server Reply", "User exists");
			/*get buddyname string*/
			char *buddyname = NULL;
			strip_buddyname( message, &buddyname );//return value (which should net be free()d) ignored, buddyname should be free()d
			handle_lookup_success( buddyname );
			free(buddyname);

			break;
		}
		case MESSAGE:
		{
			/*get buddyname string and a pointer to the actual message part of message*/
			char *buddy_username = NULL;
			const char *raw_message = strip_buddyname( message, &buddy_username );//raw_message should not be freed, username should be

			//TODO I got up to here checking the process of an incoming message, continue checking below this line
			logg("Server Reply", "New message");

			/*append the actual message to history*/
			append_to_history( raw_message, buddy_username, TRUE );
			if( get_buddy() )
			{
				if( strcmp( get_buddy(), buddy_username ) == 0 )
				{
					append_to_history_view( raw_message, buddy_username );
				}
				//somehow notify the user about the new message
			}

			free(buddy_username);

			break;
		}
		default:
		{
			warn("Server Reply", "Server sent an unknown reply");
			break;
		}
	}
}

//TODO set_username() should be called once the server accepted the login request

/*alright let's not forget the main function :D*/
int main( int argc, char *argv[] )
{
    gtk_init( &argc, &argv );
    logger_init();
    g_timeout_add_seconds( 1, watch_connection, evaluate_incoming );
    if(enter_chatnut_directory() != 0 )
    {
        return EXIT_FAILURE;
    }
    create_gui();
    gtk_main();

    cleanup_connection_data();
    exit_chatnut_directory();
    shutdown_logger();
    return EXIT_SUCCESS;
}
