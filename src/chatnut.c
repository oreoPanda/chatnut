/*chatnut.c*/

#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>	//for mkdir()

#include "connection.h"
#include "connection_raw.h"
#include "gui.h"

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

gboolean login_status = FALSE;
char *user = NULL;
char *currentbuddy = NULL;

struct actiondata
{
    char *name;
}pending_action_data;

//TODO check if strncpy and strncat return values matter for moved strings

/*stuff that interacts with the graphic*/

static GtkListStore *create_contact_list_model()
{
    GtkListStore *store = NULL;
    GtkTreeIter iter;

    FILE *contactlist = NULL;
    char *filename = "contactlist";
    int raw_list_len;
    char *raw_list = NULL;
    char *name = NULL;

    /*open contact list file and load its contents into memory*/
    contactlist = fopen( filename, "r" );
    if( contactlist == NULL )
    {
        fprintf( stdout, "Warning, no contactlist found for user." );
        return store;
    }
    else
    {
        /*create contact list model with name and online status for each contact*/
        store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_BOOLEAN );

        //get file length
        fseek( contactlist, 0, SEEK_END );
        raw_list_len = ftell(contactlist);
        rewind(contactlist);
        //set up data buffer
        raw_list = calloc( raw_list_len+1, sizeof(char) );	//one extra for '\0'
        //read from file
        fread( raw_list, sizeof(char), raw_list_len, contactlist );
        raw_list[raw_list_len] = 0;
        fclose(contactlist);

        //crunch data
        int i = 0;
        while( raw_list[i] != 0 )
        {
            //get one name at a time
            int name_pos = i;
            while( raw_list[i] != '\n' )
            {
                i++;
            }
            int nameLen = i-name_pos;
            name = calloc( nameLen+1, sizeof(char) );	//name + '\0'
            strncpy( name, raw_list+name_pos, nameLen );	//right after raw_list[nameLen] is a '\n', so strncpy wouldn't put a '\0'
            name[i] = '\0';		//adding terminator here cuz strncpy wouldn't do it in this case
            i++;//skip past '\n'

            //add the name to the contact list model
            gtk_list_store_append( store, &iter );
            gtk_list_store_set( store, &iter, 0, name, 1, FALSE, -1 );

            free(name);
        }

        free(raw_list);

        return store;
    }
}

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

//stores history of contact in to
void load_history( const char *contact, char **to )
{
    FILE *history_file = NULL;
    int file_len = 0;
    const char *filename = contact;
    char *path = "history";

    /*switch to directory*/
    if( chdir(path) != 0 )
    {
        fprintf( stderr, "Unable to switch into user's history directory in $HOME/.chatnut/[user]", strerror(errno) );
        return;
    }

    /*open file and read from it*/
    history_file = fopen( filename, "r" );		//NULL check if file not found (errno)
    if( history_file != NULL )
    {
        fseek( history_file, 0, SEEK_END );
        //TODO check these comments, they seem confusing...
        file_len = ftell(history_file);		//all characters + one additional '\n'
        rewind(history_file);
        *to = calloc( file_len, sizeof(char) );	//all characters (including additional '\n') + '\0' (TODO check: the '\n' is part of all characters)
        fread( *to, sizeof(char), file_len, history_file );
        *( *to+(file_len) ) = '\0';
    }
    else
    {
        *to = NULL;		//set the history storage buffer to NULL
        //TODO log error or simply no history there yet...
    }
    
    /*switch back to parent directory*/
    chdir("../");

    return;
}

void append_to_history( const char *message, const char *username )
{
    FILE *historyfile = NULL;
    char *historypath = NULL;
    const char *historyfilename = username;

    /*generate path to historyfile*/
    historypath = calloc( strlen(getenv("HOME")) + 1
                                            + strlen(".chatnut") + 1
                                            + strlen(user) + 1
                                            + strlen("history") + 1,
                                            sizeof(char) );
    strncpy( historypath, getenv("HOME"), strlen(getenv("HOME"))+1 );
    strncat( historypath, "/", 1 );
    strncat( historypath, ".chatnut", strlen(".chatnut") );
    strncat( historypath, "/", 1 );
    strncat( historypath, user, strlen(user) );
    strncat( historypath, "/", 1 );
    strncat( historypath, "history", strlen("history") );

    /*switch to directory*/
    if( chdir(historypath) != 0 )
    {
        mkdir( historypath, 0755 );
        chdir(historypath);			//TODO error?
    }
    free(historypath);

    /*open file and write to it*/
    historyfile = fopen( historyfilename, "a" );		//NULL check if file not found (errno)
    if(historyfile)
    {
        fprintf( historyfile, "%s\n", message );
        fclose(historyfile);	//TODO log an error if there is one
    }
    else
    {
        //TODO log error
    }

    return;
}

gboolean input_view_key_pressed_cb( GtkWidget *inputview, GdkEvent *event, gpointer data )
{
    GdkModifierType state;
    guint keyval;
    GtkTextBuffer *inputbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(inputview));
    GtkTextIter start;
    GtkTextIter end;
    gchar *text = NULL;

    gdk_event_get_state( event, &state );
    gdk_event_get_keyval( event, &keyval );		//keyvals from <gdk/gdkkeysyms.h>

    switch(state)
    {
        case GDK_SHIFT_MASK:
        {
            return FALSE;
        }
        default:
        {
            break;
        }
    }
    switch(keyval)
    {
        case GDK_KEY_Return:
        {
            /*get text from buffer*/
            gtk_text_buffer_get_bounds( inputbuffer, &start, &end );
            text = gtk_text_buffer_get_text( inputbuffer, &start, &end, FALSE );//TODO with hidden chars FALSE, embedded images won't copy

            /*send textstring to server*/
            write_to_channel( text, user );

            /*append textstring to history and maybe to historyview*/
            append_to_history( text, currentbuddy );
            append_to_history_view( text, user );

            /*clear inputview*/
            clear_input_view();

            /*free text, since it is a non-const string returned from a gtk function*/
            g_free(text);
        }
    }

    return FALSE;
}

//TODO error when /who fails
static void contact_selection_handler( GtkTreeView *treeview, GtkTreePath *treepath, GtkTreeViewColumn *column, gpointer data )
{
    GtkTreeSelection *selection = NULL;
    GtkTreeIter iter;		//will be set to the selected row
    GtkTreeModel *model;
    char *contact_name;
    char *history;

    selection = gtk_tree_view_get_selection(treeview);

    if( gtk_tree_selection_get_selected( selection, &model, &iter ) )	//set model and iter
    {
        //get the selected contact's name and load it's chat history into the message_view
        gtk_tree_model_get( model, &iter, 0, &contact_name, -1 );
        load_history( contact_name, &history );
        show_message_history( history );
        free(history);

        /*if connected, send the server a /unwho command and a /who command to specify who we're talking to*/
        // /unwho
        if( channel_not_null() )
        {
            char *command = NULL;

            // /unwho
            command = calloc( strlen("/unwho") + 1, sizeof(char) );
            strncpy( command, "/unwho", 7 );
            write_to_channel( command, NULL );

            // /who [contact_name]
            command = realloc( command, sizeof(char) * ( strlen("/who ") + strlen(contact_name) + 1 ) );
            strncpy( command, "/who ", 6 );
            strncat( command, contact_name, strlen(contact_name) );
            write_to_channel( command, NULL );

            enable_input_view(input_view_key_pressed_cb);

            //TODO g_free() or free(), difference??
            free(command);
        }

        //write name into pending_action_data for BUDDY_IS_SET handler
        pending_action_data.name = calloc( strlen(contact_name) + 1, sizeof(char) );
        strncpy( pending_action_data.name, contact_name, strlen(contact_name) + 1 );
    }

    return;
}

//the BUDDY_IS_SET handler
static void contact_selected_finish()
{
    if(currentbuddy)		//TODO check for smarter way to do this
    {
        free(currentbuddy);
    }
    currentbuddy = calloc( strlen(pending_action_data.name) + 1, sizeof(char) );
    strncpy( currentbuddy, pending_action_data.name, strlen(pending_action_data.name) + 1 );
    free(pending_action_data.name);
    pending_action_data.name = NULL;

    return;
}

int init_chatnut_directory(void)
{
    if( chdir(getenv("HOME")) != 0 )
    {
        fprintf( stderr, "Error switching into your home directory: %s\n", strerror(errno) );
        return 1;
    }
    if( mkdir( ".chatnut", 0755 ) != 0 )
    {
        fprintf( stderr, "Error creating directory .chatnut in your home directory: %s\n", strerror(errno) );
        return 1;
    }
    if( chdir(".chatnut") != 0 )
    {
        fprintf( stderr, "Error switching to .chatnut in your home directory: %s\n", strerror(errno) );
        return 1;
    }
    return 0;
}

int init_user_directory(void)
{
    /*directories*/
    if( mkdir( user, 0755 ) != 0 )
    {
        fprintf( stderr, "Error creating users directory in $HOME/.chatnut: %s\n", strerror(errno) );
        return 1;
    }
    if( chdir(user) != 0 )
    {
        fprintf( stderr, "Error switching into user's directory in $HOME/.chatnut: %s\n", strerror(errno) );
        return 1;
    }
    if( mkdir( "history", 0755 ) != 0 )
    {
        fprintf( stderr, "Error creating history directory for user in $HOME/.chatnut/[user]: %s\n", strerror(errno) );
        return 1;
    }
    
    return 0;
}

static void add_contact_finish()
{
    FILE *contactfile = NULL;
    char *filename = "contactlist";	//is this on heap or stack? no malloc(), so no free() or?

    char *contact = calloc( strlen(pending_action_data.name) + 1, sizeof(char) );
    strncpy( contact, pending_action_data.name, strlen(pending_action_data.name) + 1 );
    free(pending_action_data.name);
    pending_action_data.name = NULL;

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

    free(contact);

    return;
}

//TODO this is static, but called from other file. This may work cuz it is opened thru pointer passed from this file
static gboolean add_contact_response_handle( GtkDialog *dialog, gint response_id, gpointer data )
{
	switch(response_id)
	{
		case GTK_RESPONSE_OK:
		{
			int commandlen = 0;
			char *command = NULL;
			GtkEntryBuffer *buffer = NULL;

			/*read contact name*/
			buffer = data;
			const gchar *contact = gtk_entry_buffer_get_text(buffer);

			pending_action_data.name = (char *)contact;

			/*ask server if the given contact exists*/
			commandlen = strlen("/lookup ") + strlen(contact) + 1;	//strlen + space for NULL
			command = calloc( commandlen, sizeof(char) );
			strncpy( command, "/lookup ", 9 );	//"/who " + NULL
			strncat( command, contact, strlen(contact) );
			//TODO check is over-engineering or? actually no, since server may leave before user clicks "Add Contact" button
			if( channel_not_null() && login_status == TRUE )
			{
				write_to_channel( command, NULL );
			}
			free(command);

			gtk_widget_destroy(GTK_WIDGET(dialog));

			break;
		}
		case GTK_RESPONSE_CANCEL:
		{
			gtk_widget_destroy(GTK_WIDGET(dialog));

			break;
		}
		default:
		{
			//TODO use a different error report for this, print_error is for connection_raw.c and should not be in connection_raw.h
			print_error("Unhandled response id for GtkDialog");

			break;
		}
	}
	return G_SOURCE_REMOVE;
}

//TODO same as above
static gboolean add_contact_button_clicked( GtkButton *button, gpointer data )
{
    if(button)
    {
        printf("Adding contact...\n");

        /*open a new window where the user enters the contacts name*/
        popup_add_contact(add_contact_response_handle);
    }

    //FIXME return value doesn't seem to matter
    return G_SOURCE_CONTINUE;
}

static void login_finish(void)
{
    login_status = TRUE;
    //TODO user needs to be free()d at some point in (run)time
    user = calloc( strlen(pending_action_data.name) + 1, sizeof(char) );
    strncpy( user, pending_action_data.name, strlen(pending_action_data.name)+1 );
    free(pending_action_data.name);
    pending_action_data.name = NULL;

    if( init_user_directory() != 0 )
    {
        fprintf( stderr, "Error initializing the users directory and files in $HOME/.chatnut. Did the permissions change?\n" );
    }

    return;
}

/*TODO would make more sense if login isn't possible if not connected*/
static gboolean login( GtkDialog *dialog, gint response_id, gpointer data )
{
    int commandlen = 0;
    char *command = NULL;
    GtkEntryBuffer **bufferlist = data;
    GtkEntryBuffer *usernamebuffer = *bufferlist;
    GtkEntryBuffer *passwordbuffer = *(bufferlist+1);

    /*get username and password, const, so should not be free()ds*/
    const char *username = gtk_entry_buffer_get_text(usernamebuffer);
    const char *password = gtk_entry_buffer_get_text(passwordbuffer);

    /*create and send login command to server*/
    commandlen = strlen("/login ")
                                    + strlen(username)
                                    + 1			//space
                                    + strlen(password)
                                    + 1;		//NULL
    command = calloc( commandlen, sizeof(char) );
    strncpy( command, "/login ", 8 );	//"/login " + NULL
    strncat( command, username, strlen(username) );
    strncat( command, " ", 1 );
    strncat( command, password, strlen(password) );

    //TODO test what happens here when server leaves while user types in username and password
            //thought: server leaves, this command is written, G_IO_CHANNEL_HUNGUP gets emitted but not handled
                    //next thing that happens is a read with 0 (EOF), and client should work normally
    //TODO for over-engineering, see function above
    if( channel_not_null() )
    {
        write_to_channel( command, NULL );
    }

    //copy username for later use, don't free username or passwords
    pending_action_data.name = calloc( strlen(username) + 1, sizeof(char) );
    strncpy( pending_action_data.name, username, strlen(username)+1 );

    free(command);

    gtk_widget_destroy(GTK_WIDGET(dialog));

    return G_SOURCE_REMOVE;
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
    functionalize_window(add_contact_button_clicked);

    return;
}

/*here we go with the non-graphical stuff*/

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

            g_idle_add( popup_login, login );
            break;
        }
        case LOGIN_FAILURE:
        {
            printf( "(evaluate_incoming)Login failure\n");

            g_idle_add( popup_login, login );
            break;
        }
        case LOGIN_SUCCESS:
        {
            printf( "(evaluate_incoming)Login success\n");

            login_finish();

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

            contact_selected_finish();

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

            free(pending_action_data.name);
            pending_action_data.name = NULL;

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
            add_contact_finish();
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
            if( strcmp( currentbuddy, username ) == 0 )
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
    g_timeout_add_seconds( 1, watch_connection, evaluate_incoming );
    if( init_chatnut_directory() != 0 )
    {
        fprintf( stderr, "Error initializing directory .chatnut in $HOME. Make sure this process is allowed to create directories in $HOME.\n" );
        return EXIT_FAILURE;
    }
    create_gui();
    gtk_main();
    return EXIT_SUCCESS;
}
