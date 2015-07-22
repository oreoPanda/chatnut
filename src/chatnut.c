/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * message_client.c
 * Copyright (C) 2014 
 * 
 * message is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * message is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//TODO receive server's connected message so this client knows it didn't just try to connect but it actually connected
//TODO have terminal default be no echo so there are no "floating" q's and m's and /'s on the screen?
//NOTE: 3-way handshake seems to be done by connect(), accept() comes later. That's why I'm using conn_msg as a connected status message.

#include <gtk/gtk.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "getch.h"
#include "messaging.h"
#include "socketprx.h"

GtkWidget *window, *grid, *textoutput, *textinput, *list;
int connected = FALSE;
int receiver_set = FALSE;
int name_set = FALSE;

enum
{
    NAME_COLUMN,
    ONLINE_COLUMN,
    N_COLUMNS
};

gchar *load_message_history( gchar *contact, int max_messages )
{
    FILE *message_log;
    char *messages;
    char *path = NULL;

    //create path
    path = calloc( 5+strlen(contact)+4+1, sizeof(char) );	//"data/" + contact ".txt" + "\0"
    strncpy( path, "data/", 5 );
    strncpy( path+5, contact, strlen(contact) );
    strncpy( path+5+strlen(contact), ".txt", 4 );
    path[5+strlen(contact)+4] = '\0';

    message_log = fopen( path, "r" );
    if( message_log == NULL )
    {
        //TODO
        return NULL;
    }

    fseek( message_log, 0, SEEK_END );
    int file_len = ftell(message_log);		//all characters + one additional '\n'
    rewind(message_log);
    messages = calloc( file_len+1, sizeof(char) );	//all characters + '\n' + '\0'
    fread( messages, sizeof(char), file_len, message_log );
    return messages;
}

static void contact_selection_handler( GtkTreeSelection *selection, GtkTextView *message_view )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *contact_name;
    GtkTextBuffer *messageview_buffer;

    if( gtk_tree_selection_get_selected( selection, &model, &iter ) )
    {
        //get the selected contact's name and load it's chat history into the message_view
        gtk_tree_model_get( model, &iter, NAME_COLUMN, &contact_name, -1 );
        gchar *history = load_message_history( contact_name, 30 );
        g_free(contact_name);
        messageview_buffer = gtk_text_view_get_buffer( message_view );
        if( history )
        {
            gtk_text_buffer_set_text( messageview_buffer, history, -1 );	//-1 cuz text is terminated
        }
        else
        {
            gtk_text_buffer_set_text( messageview_buffer, "<No recent chats found>", -1 );	//-1 cuz text is terminated
        }
        free(history);
    }
}

static void backspace_handler( GtkTextView *textView, gpointer data )
{
    printf( "Backspace received!\n" );
}

gboolean key_pressed( GtkWidget *textview, GdkEventKey *event, GtkWidget *textlog )	//textview is the input one, textlog the output
{
    if( (event->state & GDK_SHIFT_MASK) )	//if shift is held down, exit right away (shift-enter shall be newline, like skype or pidgin do, too)
    {
            return FALSE;
    }

    GtkTextBuffer *sourceBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));	//get buffer of input textview
    GtkTextBuffer *destinationBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textlog));	//get buffer of output textview

    switch( event->keyval )
    {
        case GDK_KEY_Return:
        {
            gchar *text = NULL;
            GtkTextIter start;
            GtkTextIter end;

            //get text from sourceBuffer if it is not empty
            if( gtk_text_buffer_get_char_count(sourceBuffer) > 0 )
            {
                gtk_text_buffer_get_bounds( sourceBuffer, &start, &end );
                text = gtk_text_buffer_get_text( sourceBuffer, &start, &end, FALSE );
            }
            else return TRUE;	//event was handled, '\n' will not be printed (would be if FALSE is returned)

            //let the text start in a new line in the destination buffer (by adding a '\n' at index 0)
            if( gtk_text_buffer_get_char_count(destinationBuffer) > 0 )
            {
                int text_len = strlen(text);
                char temp[text_len+1];
                strncpy(temp, text, text_len);
                temp[text_len] = '\0';
                text = realloc( text, sizeof(gchar) * (text_len+2) );	//+2: \n...\0
                strncpy( text+1, temp, text_len+1 );
                text[0] = '\n';
                text[text_len+1] = '\0';
            }
            gtk_text_buffer_set_text( sourceBuffer, "", -1 );//remove text from typing window

            //append text to destinationBuffer
            gtk_text_buffer_get_end_iter( destinationBuffer, &end );
            gtk_text_buffer_insert( destinationBuffer, &end, text, -1 );
            return TRUE;	//so the newline from enter doesn't make it into the buffer again
            break;
        }
        default:
        {
                break;
        }
    }

    return FALSE;	//if this returned TRUE, the GtkTextView would never show the pressed key because the event won't be handled further after this function (so typing in a message won't ever make the message appear in the GtkTextView or even in the GtkTextBuffer)
}

//TODO the destroy signal needs to call a quit function, the quit function needs to shutdown the GIOChannel and call gtk_main_quit
GtkWidget *set_window()
{
    GtkWidget *window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //signals associated with the main window
    g_signal_connect( window, "destroy", G_CALLBACK(gtk_main_quit), NULL );

    //graphical window properties
    gtk_window_set_title( GTK_WINDOW(window), "Hello" );
    gtk_container_set_border_width( GTK_CONTAINER(window), 10 );	//this only has an effect if there are widgets shown inside the given widget (window in this case)

    return window;
}

GtkWidget *set_grid()
{
    GtkWidget *grid;
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing( GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing( GTK_GRID(grid), 5);

    return grid;
}

GtkWidget *set_messaging_view()
{
	GtkWidget *textviewer;
	textviewer = gtk_text_view_new();
	gtk_text_view_set_right_margin( GTK_TEXT_VIEW(textviewer), 30 );
	gtk_text_view_set_editable( GTK_TEXT_VIEW(textviewer), FALSE );
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(textviewer), FALSE );

	return textviewer;
}

GtkWidget *set_typing_area()
{
	GtkWidget *text_input_field;
	text_input_field = gtk_text_view_new();
	//signals
	g_signal_connect( text_input_field, "backspace", G_CALLBACK(backspace_handler), NULL );
	g_signal_connect( text_input_field, "key-press-event", G_CALLBACK(key_pressed), textoutput );

	return text_input_field;
}

static void evaluate_incoming( char *message )
{
	int indicator = message[0];		//indicates whether it is a message or a command reply

	switch(indicator)
	{
		case NAME_IS_SET:
		{
			name_set = TRUE;
			break;
		}
		case BUDDY_IS_SET:
		{
			receiver_set = TRUE;
			break;
		}
		default:
		{
			GtkTextBuffer *message_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textoutput));
			GtkTextIter start, end;
			gtk_text_buffer_get_bounds( message_buffer, &start, &end );
			gtk_text_buffer_insert( message_buffer, &end, message, -1 );
			break;
		}
	}
}

//receive incoming data, this function is called when the GIOChannel is ready for reading
static gboolean receive_incoming( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
	socket_t sock;
	char *message = malloc( MSG_LEN*sizeof(char) );

	g_print( "Received incoming.\n" );

	sock = g_io_channel_unix_get_fd(sourcechannel);
	TCP_recv( &sock, message, MSG_LEN, 0 );

	if( strncmp( message, conn_msg, CONN_MSG_LEN-1 ) == 0 )
	{
		printf( "Server accepted connection.\n" );
		evaluate_incoming(message);
		connected = TRUE;
	}
	else
	{
		evaluate_incoming(message);
	}
	return FALSE;	//event handled, no need for further handling
}

static gboolean connection_error( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
    printf( "GIOChannel reported error.\n" );
    return TRUE;       //signal not finally handled, pass it on
}

static gboolean connection_hungup( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
    printf( "GIOChannel was hung up.\n" );
    return TRUE;        //signal not finally handled, pass it on
}

void construct_main_window()
{
	gtk_container_add( GTK_CONTAINER(window), grid );

	gtk_grid_attach( GTK_GRID(grid), textoutput, 0, 1, 2, 1 );
	gtk_grid_attach( GTK_GRID(grid), textinput, 0, 2, 2, 1 );
	gtk_grid_attach( GTK_GRID(grid), list, 2, 0, 1, 3 );

	gtk_widget_show(textoutput);
	gtk_widget_show(textinput);
	gtk_widget_show(list);

	/*show*/
	gtk_widget_show(grid);
	gtk_widget_show(window);
}

GtkListStore *create_contact_list_model()
{
	GtkListStore *store;
	GtkTreeIter iter;
	FILE *contactlist = NULL;

	/*create contact list model*/
	store = gtk_list_store_new( N_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN );

	contactlist = fopen( "data/contacts.txt", "r" );
	if( contactlist == NULL )
	{
		return store;
	}
	else
	{
		//get file length
        fseek( contactlist, 0, SEEK_END );
		int listlen = ftell(contactlist);
		rewind(contactlist);
		//set up data buffer
		char *raw_list = calloc( listlen+1, sizeof(char) );	//one extra for '\0'
		//read from file
		fread( raw_list, sizeof(char), listlen, contactlist );
		raw_list[listlen] = 0;
		fclose(contactlist);

		//crunch data
		int i = 0;
		int nameLen = 0;
		char *name = NULL;
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
			strncpy( name, raw_list+name_pos, nameLen );
			name[nameLen] = 0;
			i++;//skip past '\n'

			//add the name to the contact list model
			gtk_list_store_append( store, &iter );
			gtk_list_store_set( store, &iter, NAME_COLUMN, name, ONLINE_COLUMN, FALSE, -1 );
		}

		return store;
	}
}

int main(int argc, char *argv[])
{
	/*set up networking*/
	socket_t sock;
	GIOChannel *connection_point;

	if( argc != 3 )
	{
		printf( "Usage: %s [server address] [port]\n", *argv );
		return 1;
	}

	/*create TCP socket and connect to server*/
	sock = create_socket( PF_INET, SOCK_STREAM, 0 );
	connect_socket( &sock, argv[1], atoi(argv[2]) );
	printf( "Waiting for server...\n" );
	//done

	/*set up a GIOChannel*/
	connection_point = g_io_channel_unix_new(sock);
	g_io_add_watch( connection_point, G_IO_IN, receive_incoming, NULL );
        g_io_add_watch( connection_point, G_IO_ERR, connection_error, NULL );
        g_io_add_watch( connection_point, G_IO_HUP, connection_hungup, NULL );
	//done
	//done setting up networking

	gtk_init( &argc, &argv );

	/*set up the window and its components (two text areas and a contact list)*/
	window = set_window();
	grid = set_grid();
	textoutput = set_messaging_view();	//text viewer
	textinput = set_typing_area();


	//done setting up
	GtkListStore *store = create_contact_list_model();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;


	//graphical, create a GtkTreeView
	list = gtk_tree_view_new_with_model( GTK_TREE_MODEL(store) );
	gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(list), FALSE );

	//name column
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new();
	column = gtk_tree_view_column_new_with_attributes( "Name", renderer, "text", NAME_COLUMN, NULL );//"text" is an attribute (property) of a GtkCellRendererText
	gtk_tree_view_append_column( GTK_TREE_VIEW(list), column );

	//online column
	renderer = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio( GTK_CELL_RENDERER_TOGGLE(renderer), TRUE );
	gtk_cell_renderer_toggle_set_activatable( GTK_CELL_RENDERER_TOGGLE(renderer), TRUE );
	column = gtk_tree_view_column_new();
	column = gtk_tree_view_column_new_with_attributes( "Online", renderer, "active", ONLINE_COLUMN, NULL );//"active" is an attribute (property) of a GtkCellRendererToggle, this function takes all values out of that column in the GtkListStore and puts them into cells and renders them.
	gtk_tree_view_append_column( GTK_TREE_VIEW(list), column );

	//selection handle
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));	//not the actual selection, just an object that will be associated with the "changed" event/signal
	gtk_tree_selection_set_mode( selection, GTK_SELECTION_SINGLE );
	g_signal_connect( G_OBJECT(selection), "changed", G_CALLBACK(contact_selection_handler), textoutput );


	construct_main_window();
	gtk_main();

	close_socket(&sock);
	cleanup();

	return EXIT_SUCCESS;
}
