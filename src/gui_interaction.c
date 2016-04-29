/*gui_interaction.c*/

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

//TODO in callbacks that send something to server, check that no memory leaks occur by checking/not checking if connected to server

#include "gui_interaction.h"
#include "gui.h"
#include "connection.h"
#include "file_operations.h"
#include "logger.h"
#include "user.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

//get data from the Connect dialog and set it so that connecting is possible
extern gboolean connect_callback(GtkDialog *dialog, gint response_id, gpointer data)
{
	if(!dialog || response_id != GTK_RESPONSE_OK || data)
	{
		warn("GUI Callback", "Unexpected argument while processing server data");
	}

	//get entries from dialog
	GtkWidget *content_area = gtk_dialog_get_content_area(dialog);
	GList *elements = gtk_container_get_children(GTK_CONTAINER(content_area));
	GtkEntry *address_entry = elements->data;
	elements = elements->next;
	GtkEntry *port_entry = elements->data;

	//get text from entries
	const char *buf = gtk_entry_get_text(port_entry);
	char *end = NULL;
	errno = 0;
	long port = strtol(buf, &end, 10);

	//check for valid port, if it is valid store address and port
	if(end != buf && *end == '\0' && errno != ERANGE && (port >= 0 && port <= USHRT_MAX) )
	{
		set_connection_data(gtk_entry_get_text(address_entry), (unsigned short)port);
	}
	//in case of an invalid port, pop up the connect dialog again
	else
	{
		popup_connect();
	}

	return G_SOURCE_CONTINUE;
}

//ask the server if the requested contact exists, gets called by the Add Contact dialog
extern gboolean add_contact( GtkDialog *dialog, gint response_id, gpointer data )
{
	if(!dialog || !(response_id == GTK_RESPONSE_OK || response_id == GTK_RESPONSE_CANCEL) || data)
	{
		warn("GUI Callback", "Unexpected argument while processing new contact data");
	}

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

            /*ask server if the given contact exists*/
            commandlen = strlen("/lookup ") + strlen(contact) + 1;	//strlen + space for NULL
            command = calloc( commandlen, sizeof(char) );
            strncpy( command, "/lookup ", 9 );	//"/lookup " + NULL
            strncat( command, contact, strlen(contact) );
            if( channel_not_null() )
            {
                write_to_channel( command, NULL );
            }
            free(command);

            break;
        }
        case GTK_RESPONSE_CANCEL:
        {
            break;
        }
    }

    return G_SOURCE_CONTINUE;
}

/*tell the server our login data, gets called by the login dialog*/
extern gboolean login( GtkDialog *dialog, gint response_id, gpointer data )
{
    int commandlen = 0;
    char *command = NULL;
    
    if(!dialog || response_id != GTK_RESPONSE_OK || data)
    {
    	warn("GUI Callback", "Unexpected argument while processing login data");
    }

	/*get elements inside the widgets content area, which is a GtkBox, which is a GtkContainer*/
	GtkWidget *content_area = gtk_dialog_get_content_area(dialog);
	GList *elements = gtk_container_get_children(GTK_CONTAINER(content_area));

	GtkEntry *username_entry = elements->data;
	elements = elements->next;
	GtkEntry *password_entry = elements->data;

	//get text from entries
	const char *username = gtk_entry_get_text(username_entry);
	const char *password = gtk_entry_get_text(password_entry);

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

	free(command);

    return G_SOURCE_CONTINUE;
}

//TODO error when /who fails
extern void contact_selection_handler( GtkTreeView *treeview, GtkTreePath *treepath, GtkTreeViewColumn *column, gpointer data )
{
	GtkTreeSelection *selection = NULL;
	GtkTreeIter iter;		//will be set to the selected row
	GtkTreeModel *model = NULL;
	char *contact_name = NULL;
	char *history = NULL;

	if(!treeview || treepath || column || data)
	{
		warn("GUI Callback", "Unexpected argument while processing contact selection");
	}

	selection = gtk_tree_view_get_selection(treeview);

	if( gtk_tree_selection_get_selected( selection, &model, &iter ) )	//set model and iter
	{
		/*get the selected contact's name*/
		gtk_tree_model_get( model, &iter, 0, &contact_name, -1 );

		/*save as buddy's name*/
		set_buddy(contact_name);

		/*load it's chat history into the message_view*/
		load_file( "history", contact_name, &history );
		show_message_history(history);	//will clear historyview if history is NULL
		if(history)
		{
			free(history);
		}

		/*if connected, send the server a /unwho command and a /who command to specify who we're talking to*/
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

			g_free(contact_name);
			free(command);
		}
	}

	return;
}

//TODO isn't there some kind of signal that just gets emitted on press of Return key?
extern gboolean input_view_key_pressed_cb( GtkWidget *inputview, GdkEvent *event, gpointer data )
{
	if(!inputview || event->type != GDK_KEY_PRESS || data)
	{
		warn("GUI Callback", "Unexpected argument while handling keypress in the message input field");
	}
	
	guint keyval;
	GtkTextBuffer *inputbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(inputview));
	GtkTextIter start;
	GtkTextIter end;
	gchar *text = NULL;
	
	gdk_event_get_keyval( event, &keyval );		//keyvals from <gdk/gdkkeysyms.h>

	switch(keyval)
	{
		case GDK_KEY_Return:
		{
			/*get text from buffer*/
			gtk_text_buffer_get_bounds( inputbuffer, &start, &end );
			text = gtk_text_buffer_get_text( inputbuffer, &start, &end, FALSE );//TODO with hidden chars FALSE, embedded images won't copy

			/*send textstring to server*/
			write_to_channel( text, get_username() );

			/*append textstring to history and maybe to historyview*/
			append_to_history( text, get_buddy(), FALSE );
			append_to_history_view( text, get_username() );

			/*free text, since it is a non-const string returned from a gtk function*/
			g_free(text);

			/*clear inputview*/
			clear_input_view();

			return TRUE;    //do not propagate event further (otherwise newline would show up in input view)
		}
		default:
		{
			return FALSE;	//propagate event further, so that characters show up in input view
		}
	}
}

extern gboolean add_contact_button_press(GtkButton *button, gpointer data)
{
	if(!button || data)
	{
		warn("GUI Callback", "Unexpected argument while processing a button-press on the \"Add Contact\" button");
	}
	
	popup_add_contact();

	return G_SOURCE_REMOVE;	//TODO check
}
