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

#include "gui_interaction.h"
#include "gui.h"
#include "connection.h"
#include "file_operations.h"
#include "user.h"
#include <string.h>
#include <stdlib.h>

//ask the server if the requested contact exists, gets called by the Add Contact dialog
extern gboolean add_contact( GtkDialog *dialog, gint response_id, gpointer data )
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

            /*ask server if the given contact exists*/
            commandlen = strlen("/lookup ") + strlen(contact) + 1;	//strlen + space for NULL
            command = calloc( commandlen, sizeof(char) );
            strncpy( command, "/lookup ", 9 );	//"/who " + NULL
            strncat( command, contact, strlen(contact) );
            //TODO check is over-engineering or? actually no, since server may leave before user clicks "Add Contact" button
            if( channel_not_null() )
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
            fprintf( stderr, "Unhandled response id for GtkDialog.\n" );

            break;
        }
    }
    return G_SOURCE_REMOVE;
}

/*tell the server our login data, gets called by the login dialog*/
extern gboolean login( GtkDialog *dialog, gint response_id, gpointer data )
{
    int commandlen = 0;
    char *command = NULL;
    GtkWidget *content_area = NULL;
    GList *list_of_elements = NULL;
    GtkWidget *username_entry = NULL;
    GtkWidget *password_entry = NULL;
    GtkEntryBuffer *usernamebuffer = NULL;
    GtkEntryBuffer *passwordbuffer = NULL;
    
    if( data )
    {
        fprintf( stderr, "(login) Ignoring data passed as gpointer.\n" );
    }

    if( response_id == GTK_RESPONSE_OK )
    {
        /*get elements inside the widgets content area, which is a GtkBox, which is a GtkContainer*/
        content_area = gtk_dialog_get_content_area(dialog);
        list_of_elements = gtk_container_get_children(GTK_CONTAINER(content_area));
        GList *l = list_of_elements;
        username_entry = l->data;
        l = l->next;
        password_entry = l->data;
        l = l->next;
        
        /*get buffers from the two GtkEntries*/
        usernamebuffer = gtk_entry_get_buffer(GTK_ENTRY(username_entry));
        passwordbuffer = gtk_entry_get_buffer(GTK_ENTRY(password_entry));
        
        /*get username and password, const, so should not be free()d*/
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

        free(command);

        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
    else
    {
        fprintf( stdout, "Warning, The login dialog sent response_id not equal to GTK_RESPONSE_OK\n" );
    }

    return G_SOURCE_REMOVE;
}

//TODO error when /who fails
extern void contact_selection_handler( GtkTreeView *treeview, GtkTreePath *treepath, GtkTreeViewColumn *column, gpointer data )
{
	GtkTreeSelection *selection = NULL;
	GtkTreeIter iter;		//will be set to the selected row
	GtkTreeModel *model = NULL;
	char *contact_name = NULL;
	char *history = NULL;

	if( data != NULL )
	{
		fprintf( stderr, "(contact_selection_handler): gpointer data is non-NULL but not used.\n" );
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

extern gboolean input_view_key_pressed_cb( GtkWidget *inputview, GdkEvent *event, gpointer data )
{
	guint keyval;
	GtkTextBuffer *inputbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(inputview));
	GtkTextIter start;
	GtkTextIter end;
	gchar *text = NULL;

	/*check that a key-press-event is connected*/
	if(event->type != GDK_KEY_PRESS)
	{
		fprintf(stderr, "[Key press handle] Error: The connected event is not of type GDK_KEY_PRESS.\n");
	}

	if(data)
	{
		fprintf(stderr, "[Key press handle] Warning: Data set but not used.\n");
	}

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
