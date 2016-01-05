/*chatnut.c*/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>	//for mkdir()

#include "connection.h"
#include "gui.h"
#include "gui_interaction.h"
#include "response_handlers.h"
#include "file_operations.h"
#include "user.h"

typedef enum reply commandreply;

enum reply
{
    CONNECTED = 48,
    HELP,
    LIST,
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

//TODO check if strncpy and strncat return values matter for moved strings

/*returns a pointer to the part of message where the actual message begins, this pointer should not be freed separately from message*/
const char *strip_username( const char *message, char **username )
{
    int usernamelength = -1;

    /*calculate length of username*/
    while( *(message + ++usernamelength) != ' ' );

    /*copy username from message*/
    *username = calloc( usernamelength+1, sizeof(char) );
    strncpy( *username, message, usernamelength+1 );//copies a space to last index
    *( *(username)+usernamelength ) = '\0';

    /*message_only should point to the start of the actual message*/
    const char *message_only = message+usernamelength+1;

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
    create_label("You are not logged in yet.");
    create_buttons();

    populate_window();

    return;
}

static void evaluate_incoming(const char *data)
{
    commandreply indicator = *data;
    const char *message = data+1;

    g_print("(evaluate_incoming)Evaluating incoming message.\n");

    switch(indicator)
    {
        case CONNECTED:
        {
            printf( "(evaluate_incoming)Connected\n");

            g_idle_add( popup_login, NULL );
            break;
        }
        case LOGIN_FAILURE:
        {
            printf( "(evaluate_incoming)Login failure\n");

            g_idle_add( popup_login, NULL );
            break;
        }
        case LOGIN_SUCCESS:
        {
            printf( "(evaluate_incoming)Login success\n");

            /*get username string*/
            char *username = NULL;
            strip_username( message, &username );//return value (which should net be free()d) ignored, username should be free()d
            handle_login_success(username);
            free(username);

            /*load and show contacts*/
            destroy_label();
            GtkListStore *model = create_contact_list_model();
            if(model)
            {
                create_list_view(model);
                functionalize_list_view(contact_selection_handler);
            }
            else
            {
                create_label("You don't have any contacts yet.");
            }
            populate_window_with_list_or_label();
            break;
        }
        case BUDDY_IS_SET:
        {
            printf( "(evaluate_incoming)BUDDY_IS_SET\n");
            /*get username string*/
            char *username = NULL;
            strip_username( message, &username );//return value (which should net be free()d) ignored, username should be free()d
            handle_buddy_is_set(username);
            free(username);

            break;
        }
        case BUDDY_IS_UNSET:
        {
            printf( "(evaluate_incoming)BUDDY_IS_UNSET\n");

            break;
        }
        case BUDDY_NOT_EXIST:
        {
            printf( "(evaluate_incoming)BUDDY_NOT_EXIST\n");

            break;
        }
        case LOOKUP_FAILURE:
        {
            printf( "(evaluate_incoming)Lookup failure\n");
            break;
        }
        /*process of choosing buddy works, code analysis needs to be done TODO*/
        case LOOKUP_SUCCESS:
        {
            printf( "(evaluate_incoming)Lookup success\n");
            /*get username string*/
            char *username = NULL;
            strip_username( message, &username );//return value (which should net be free()d) ignored, username should be free()d
            handle_lookup_success(username);
            free(username);
            
            break;
        }
        case MESSAGE:
        {
            /*get username string and a pointer to the actual message part of message*/
            char *username = NULL;
            const char *raw_message = strip_username( message, &username );//raw_message should not be freed, username should be

            //TODO I got up to here checking the process of an incoming message, continue checking below this line

            /*append the actual message to history*/
            append_to_history( raw_message, username );
            if( strcmp( get_buddy(), username ) == 0 )
            {
                append_to_history_view( raw_message, username );
            }

            free(username);

            break;
        }
        default:
        {
            g_print( "(evaluate_incoming)Unknown command reply %d\n", indicator );
            g_print( "(evaluate_incoming)Might be message %s\n", message );
            break;
        }
    }
}

/*alright let's not forget the main function :D*/

int main( int argc, char *argv[] )
{
    gtk_init( &argc, &argv );
    g_idle_add( watch_connection, evaluate_incoming );
    if( init_chatnut_directory() != 0 )
    {
        fprintf( stderr, "Error initializing directory .chatnut in $HOME. Make sure this process is allowed to create directories in $HOME.\n" );
        return EXIT_FAILURE;
    }
    create_gui();
    gtk_main();
    return EXIT_SUCCESS;
}
