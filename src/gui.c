/*gui.c*/

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

//TODO check that signal connection is done correctly and doesn't leave any leaks and that return values of callbacks are ok
//TODO check if the model needs some kind of unref or free (memory leaks?)

#include "gui.h"
#include "gui_interaction.h"
#include "logger.h"
#include "user.h"
#include <stdlib.h>
#include <string.h>

GtkWidget *window = NULL,
                *pane = NULL,
                *lpane = NULL,
                *history_scrollbox = NULL,
                *history_view = NULL,
                *input_scrollbox = NULL,
                *input_view = NULL,
                *rgrid = NULL,
                *button_contacts = NULL, *button_chats = NULL, *button_settings = NULL,
                *list = NULL,
                *label = NULL,
                *button_add_contact = NULL;
GtkWidget *dialog_add_contact = NULL,
                *dialog_login = NULL,
				*dialog_register = NULL,
                *dialog_connect = NULL;

gboolean input_view_enabled = FALSE;
gulong input_view_key_press_handler_id = 0;

//variables for toggling between label and list
gboolean togglestatus = FALSE;
gboolean list_was_visible = FALSE;
gboolean label_was_visible = FALSE;

unsigned long addcontactbuttonsignalid;

//TODO the destroy signal needs to call a quit function, the quit function needs to shutdown the GIOChannel and call gtk_main_quit
extern void create_window(void)
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //signals associated with the main window
    g_signal_connect( window, "destroy", G_CALLBACK(gtk_main_quit), NULL );

    //graphical window properties
    gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_CENTER );
    gtk_window_set_default_size( GTK_WINDOW(window), 600, 600 );
    gtk_window_set_title( GTK_WINDOW(window), "Chatnut" );
    gtk_container_set_border_width( GTK_CONTAINER(window), 10 );	//this only has an effect if there are widgets shown inside the given widget (window in this case)

    gtk_widget_show(window);

    return;
}

extern void update_window_title(void)
{
	char *username = get_username();
	char *title = NULL;

	if(username)
	{
		title = calloc(strlen(username) + 3 + strlen("Chatnut") + 1, sizeof(char) );
		strncpy(title, username, strlen(username)+1);
		strncat(title, " - ", 3);
		strncat(title, "Chatnut", 7);
	}
	else
	{
		title = calloc(strlen("Chatnut") + 1, sizeof(char) );
		strncpy(title, "Chatnut", 8);
	}

	gtk_window_set_title(GTK_WINDOW(window), title);

	return;
}

extern void create_main_pane(void)
{
	pane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_position( GTK_PANED(pane), 350 );

	gtk_widget_show(pane);

	return;
}

extern void create_left_pane(void)
{
	lpane = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_paned_set_position( GTK_PANED(lpane), 450 );

	gtk_widget_show(lpane);

	return;
}

extern void create_history_scrollbox(void)
{
	history_scrollbox = gtk_scrolled_window_new( NULL, NULL );

	gtk_widget_set_hexpand( history_scrollbox, TRUE );
	gtk_widget_set_vexpand( history_scrollbox, TRUE );

	gtk_widget_show(history_scrollbox);

	return;
}

extern void create_history_view(void)
{
    history_view = gtk_text_view_new();

    gtk_widget_set_size_request( history_view, -1, -1 );

    gtk_text_view_set_editable( GTK_TEXT_VIEW(history_view), FALSE );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(history_view), FALSE );
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(history_view), GTK_WRAP_WORD_CHAR );

    gtk_widget_show(history_view);

    return;
}

extern void show_message_history(const char *history)
{
	GtkTextBuffer *historybuffer = NULL;
	GtkTextIter end;
	GtkTextMark *mark;

	historybuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(history_view) );

	if(history)
	{
		//TODO not checking if historybuffer is non-NULL because I think gtktextview always has a gtktextbuffer
		gtk_text_buffer_set_text( historybuffer, history, -1 );	//-1 cuz text is terminated

		/*scroll down*/
		gtk_text_buffer_get_end_iter( historybuffer, &end );
		mark = gtk_text_mark_new( NULL, FALSE );
		gtk_text_buffer_add_mark( historybuffer, mark, &end );
		gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW(history_view), mark, 0, FALSE, 1.0, 1.0 );
	}
	else
	{
		gtk_text_buffer_set_text( historybuffer, "<No recent chats found>", -1 );	//-1 cuz text is terminated
	}

	return;
}

extern void append_to_history_view( const char *buffer, const char *sender )
{
	GtkTextBuffer *historybuffer = NULL;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextMark *mark;

	char *history = NULL;
	unsigned int char_count;
	char *to_append = NULL;
	gboolean need_newline = TRUE;

	historybuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(history_view));

	/*check for "<No recent chats found>" at the top of historybuffer and remove it*/
	char_count = gtk_text_buffer_get_char_count(historybuffer);
	if( char_count == strlen("<No recent chats found>") )
	{
		gtk_text_buffer_get_bounds( historybuffer, &start, &end );
		//TODO check if history needs to be free()d (look at source of gtk_text_buffer)
		history = gtk_text_buffer_get_text( historybuffer, &start, &end, FALSE );
		if( strcmp( history, "<No recent chats found>" ) == 0 )
		{
			gtk_text_buffer_delete( historybuffer, &start, &end );
			need_newline = FALSE;
		}
		free(history);
	}
	/*"<No recent chats found>" is not or no longer at the top of historybuffer*/
	/*prepend a '\n' to buffer and insert into historybuffer*/
	if(need_newline)
	{
		to_append = calloc( 1 + strlen(sender) + 2 + strlen(buffer) + 1, sizeof(char) );
		*to_append = '\n';
		strncpy( to_append+1, sender, strlen(sender)+1 );
		strncat( to_append, ": ", strlen(": ") );
		strncat( to_append, buffer, strlen(buffer) );
	}
	else
	{
		to_append = calloc( strlen(sender) + 2 + strlen(buffer) + 1, sizeof(char) );
		strncpy( to_append, sender, strlen(sender)+1 );//TODO there was no +1 at first, that's not good or? (~10 lines above there was +1)
		strncat( to_append, ": ", strlen(": ") );
		strncat( to_append, buffer, strlen(buffer) );
	}
	gtk_text_buffer_get_end_iter( historybuffer, &end );
	gtk_text_buffer_insert( historybuffer, &end, to_append, -1 );

	/*scroll down*/
	gtk_text_buffer_get_end_iter( historybuffer, &end );
	mark = gtk_text_mark_new( NULL, FALSE );
	gtk_text_buffer_add_mark( historybuffer, mark, &end );
	gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW(history_view), mark, 0, FALSE, 1.0, 1.0 );

	return;
}

extern void create_input_scrollbox(void)
{
    input_scrollbox = gtk_scrolled_window_new( NULL, NULL );

    //gtk_widget_set_size_request( input_scrollbox, 100, 50 );
    gtk_widget_set_hexpand( input_scrollbox, TRUE );
    gtk_widget_set_vexpand( input_scrollbox, TRUE );

    gtk_widget_show(input_scrollbox);

    return;
}

extern void create_input_view(void)
{
    input_view = gtk_text_view_new();

    gtk_widget_set_size_request( input_view, -1, -1 );

    gtk_text_view_set_editable( GTK_TEXT_VIEW(input_view), FALSE );
    gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(input_view), FALSE );
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(input_view), GTK_WRAP_WORD_CHAR );
    input_view_enabled = FALSE;

    //connect signal to handler but block for now
    input_view_key_press_handler_id = g_signal_connect(input_view, "key-press-event", G_CALLBACK(input_view_key_pressed_cb), NULL);
    disable_input_view();

    //visible
    gtk_widget_show(input_view);

    return;
}

extern gboolean input_view_is_enabled(void)
{
	return input_view_enabled;
}

extern void enable_input_view(void)
{
	gtk_text_view_set_editable( GTK_TEXT_VIEW(input_view), TRUE );
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(input_view), TRUE );

	/*unblock the signal handler of signal "::key-press-event"*/
	g_signal_handler_unblock(input_view, input_view_key_press_handler_id);

	input_view_enabled = TRUE;

	return;
}

extern void disable_input_view(void)
{
	/*set it so that user can't type into it*/
	gtk_text_view_set_editable( GTK_TEXT_VIEW(input_view), FALSE );
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(input_view), FALSE );

	/*block handler for "::key-press-event" signal*/
	g_signal_handler_block(input_view, input_view_key_press_handler_id);

	input_view_enabled = FALSE;
}

extern void clear_input_view(void)
{
    gtk_text_buffer_set_text( gtk_text_view_get_buffer(GTK_TEXT_VIEW(input_view)), "", 0 );

    return;
}

extern void create_right_grid(void)
{
    rgrid = gtk_grid_new();

    gtk_widget_show(rgrid);

    return;
}

/*create a GtkTreeView with one column that has a GtkCellRendererText*/
extern void create_list_view(void)
{
	GtkTreeViewColumn *column = NULL;
	GtkCellRenderer *renderer = NULL;

	list = gtk_tree_view_new();

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);
	gtk_widget_set_hexpand(list, TRUE);
	gtk_widget_set_vexpand(list, TRUE);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(list), TRUE);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes( "Name", renderer, "text", 0, NULL );//"text" is an attribute (property) of a GtkCellRendererText
	gtk_tree_view_append_column( GTK_TREE_VIEW(list), column );

	g_signal_connect(list, "row-activated", G_CALLBACK(contact_selection_cb), NULL);

	return;
}

extern void create_label(const gchar *message)
{
	label = gtk_label_new(message);
	gtk_widget_set_hexpand( label, TRUE );
	gtk_widget_set_vexpand( label, TRUE );

	return;
}

extern void toggle_list_view(gboolean toggleon, GtkListStore *store)
{
	if(toggleon)
	{
		if(togglestatus == FALSE)
		{
			/*hide label*/
			gtk_widget_hide(label);

			/*show list*/
			//add the model to the treeview
			gtk_tree_view_set_model( GTK_TREE_VIEW(list), GTK_TREE_MODEL(store) );

			gtk_widget_show(list);
			togglestatus = TRUE;
			list_was_visible = TRUE;
		}
		else
		{
			warn("GUI", "List view is already toggled on");
		}
	}
	//toggle off and show label
	else
	{
		if(togglestatus == TRUE)
		{
			/*hide list*/
			gtk_widget_hide(list);
			
			/*show label*/
			gtk_widget_show(label);
			togglestatus = FALSE;
			label_was_visible = TRUE;
		}
		else
		{
			warn("GUI", "List view is already toggled off");
		}
	}
	
	return;
}

extern void add_contact_to_list_view(const char *contact)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	store = GTK_LIST_STORE( gtk_tree_view_get_model(GTK_TREE_VIEW(list)) );
	if(!store)
	{
		store = gtk_list_store_new(1, G_TYPE_STRING);
		toggle_list_view(TRUE, store);
	}
	gtk_list_store_append( store, &iter );
	gtk_list_store_set( store, &iter, 0, contact, -1 );

	return;
}

extern void edit_label(const gchar *text)
{
	gtk_label_set_text(GTK_LABEL(label), text);
}

extern void create_buttons(void)
{
	button_contacts = gtk_button_new_with_label("Contacts");
	gtk_widget_show(button_contacts);

	button_chats = gtk_button_new_with_label("Chats");
	gtk_widget_show(button_chats);

	button_settings = gtk_button_new_with_label("Settings");
	gtk_widget_show(button_settings);

	button_add_contact = gtk_button_new_with_label("Add Contact");
	addcontactbuttonsignalid = g_signal_connect(button_add_contact, "clicked", G_CALLBACK(add_contact_button_press_cb), NULL);
	disable_add_contact_button();
	gtk_widget_show(button_add_contact);

	return;
}

extern void populate_window(void)
{
	gtk_container_add( GTK_CONTAINER(window), pane );
	gtk_paned_add1( GTK_PANED(pane), lpane );
	gtk_paned_add2( GTK_PANED(pane), rgrid );

	gtk_paned_add1( GTK_PANED(lpane), history_scrollbox );
	gtk_paned_add2( GTK_PANED(lpane), input_scrollbox );
	gtk_container_add( GTK_CONTAINER(history_scrollbox), history_view );
	gtk_container_add( GTK_CONTAINER(input_scrollbox), input_view );

	gtk_grid_attach( GTK_GRID(rgrid), button_contacts, 0, 0, 1, 1 );
	gtk_grid_attach( GTK_GRID(rgrid), button_chats, 1, 0, 1, 1 );
	gtk_grid_attach( GTK_GRID(rgrid), button_settings, 2, 0, 1, 1 );

	gtk_grid_attach( GTK_GRID(rgrid), label, 0, 1, 3, 1 );
	gtk_grid_attach( GTK_GRID(rgrid), list, 0, 1, 3, 1 );
	gtk_widget_show(label);
	togglestatus = FALSE;
	label_was_visible = TRUE;
	gtk_grid_attach( GTK_GRID(rgrid), button_add_contact, 0, 2, 3, 1 );

	return;
}

extern void disable_add_contact_button(void)
{
	g_signal_handler_block(button_add_contact, addcontactbuttonsignalid);
	return;
}

extern void enable_add_contact_button(void)
{
	g_signal_handler_unblock(button_add_contact, addcontactbuttonsignalid);
	return;
}

//login popup, no cancel button
extern void popup_login(const char *msg)
{
	GtkWidget *dialog_content_area = NULL,
			*msg_label = NULL,
			*username_grid = NULL,
			*username_label = NULL,
			*username_entry_field = NULL,
			*password_grid = NULL,
			*password_label = NULL,
			*password_entry_field = NULL;

	/*dialog*/
	dialog_login = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog_login), "Login");

	gtk_window_set_transient_for(GTK_WINDOW(dialog_login), GTK_WINDOW(window) );
	dialog_content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_login));

	/*message label*/
	msg_label = gtk_label_new(msg);

	/*username grid*/
	username_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(username_grid), 10);

	/*username label*/
	username_label = gtk_label_new("Username:");
	gtk_widget_set_hexpand(username_label, TRUE);
	gtk_widget_set_halign(username_label, GTK_ALIGN_START);

	/*username entry*/
	username_entry_field = gtk_entry_new();

	/*password grid*/
	password_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(password_grid), 10);

	/*password label*/
	password_label = gtk_label_new("Password:");
	gtk_widget_set_hexpand(password_label, TRUE);
	gtk_widget_set_halign(password_label, GTK_ALIGN_START);

	/*password entry*/
	password_entry_field = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password_entry_field), FALSE);

	/*pack*/
	gtk_box_pack_start( GTK_BOX(dialog_content_area), msg_label, FALSE, FALSE, 0 );
	gtk_grid_attach(GTK_GRID(username_grid), username_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(username_grid), username_entry_field, 1, 0, 1, 1);
	gtk_box_pack_start( GTK_BOX(dialog_content_area), username_grid, FALSE, FALSE, 0 );
	gtk_grid_attach(GTK_GRID(password_grid), password_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(password_grid), password_entry_field, 1, 0, 1, 1);
	gtk_box_pack_start( GTK_BOX(dialog_content_area), password_grid, FALSE, FALSE, 0 );
	gtk_dialog_add_button( GTK_DIALOG(dialog_login), "Register now", RESPONSE_REGISTER );
	gtk_dialog_add_button( GTK_DIALOG(dialog_login),  "Login", GTK_RESPONSE_OK );

	/*connect the "response" signal*/
	g_signal_connect( GTK_DIALOG(dialog_login), "response", G_CALLBACK(login_cb), NULL );
	g_signal_connect_swapped(GTK_DIALOG(dialog_login), "response", G_CALLBACK(gtk_widget_destroy), dialog_login);

	gtk_widget_show(msg_label);
	gtk_widget_show(username_grid);
	gtk_widget_show(username_label);
	gtk_widget_show(username_entry_field);
	gtk_widget_show(password_grid);
	gtk_widget_show(password_label);
	gtk_widget_show(password_entry_field);
	gtk_widget_show(dialog_login);

	return;
}

extern void popup_register()
{
	GtkWidget *dialog_content_area = NULL,
			*username_grid = NULL,
			*username_label = NULL,
			*username_entry_field = NULL,
			*password_grid = NULL,
			*password_label = NULL,
			*password_entry_field = NULL;

	/*dialog*/
	dialog_register = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog_register), "Register");

	gtk_window_set_transient_for(GTK_WINDOW(dialog_register), GTK_WINDOW(window) );
	dialog_content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_register));

	/*username grid*/
	username_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(username_grid), 10);

	/*username label*/
	username_label = gtk_label_new("Username:");
	gtk_widget_set_hexpand(username_label, TRUE);
	gtk_widget_set_halign(username_label, GTK_ALIGN_START);

	/*username entry*/
	username_entry_field = gtk_entry_new();

	/*password grid*/
	password_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(password_grid), 10);

	/*password label*/
	password_label = gtk_label_new("Password:");
	gtk_widget_set_hexpand(password_label, TRUE);
	gtk_widget_set_halign(password_label, GTK_ALIGN_START);

	/*password entry*/
	password_entry_field = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password_entry_field), FALSE);

	/*pack*/
	gtk_grid_attach(GTK_GRID(username_grid), username_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(username_grid), username_entry_field, 1, 0, 1, 1);
	gtk_box_pack_start( GTK_BOX(dialog_content_area), username_grid, FALSE, FALSE, 0 );
	gtk_grid_attach(GTK_GRID(password_grid), password_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(password_grid), password_entry_field, 1, 0, 1, 1);
	gtk_box_pack_start( GTK_BOX(dialog_content_area), password_grid, FALSE, FALSE, 0 );
	gtk_dialog_add_button( GTK_DIALOG(dialog_register), "Cancel", GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG(dialog_register),  "Register", GTK_RESPONSE_OK );

	/*connect the "response" signal*/
	g_signal_connect( GTK_DIALOG(dialog_register), "response", G_CALLBACK(Register_cb), NULL );
	g_signal_connect_swapped( GTK_DIALOG(dialog_register), "response", G_CALLBACK(gtk_widget_destroy), dialog_register);

	gtk_widget_show(username_grid);
	gtk_widget_show(username_label);
	gtk_widget_show(username_entry_field);
	gtk_widget_show(password_grid);
	gtk_widget_show(password_label);
	gtk_widget_show(password_entry_field);
	gtk_widget_show(dialog_register);

	return;
}

//Connect popup, no cancel button and not always in front
extern void popup_connect(const char *msg)
{
	GtkWidget *dialog_content_area = NULL,
			*msg_label = NULL,
			*addr_grid = NULL,
			*addr_label = NULL,
			*address_entry_field = NULL,
			*port_grid = NULL,
			*port_label = NULL,
			*port_entry_field = NULL;

	/*dialog*/
	dialog_connect = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog_connect), "Connect to server");

	//set it to transient (belongs to window below, may not outlast it and is always on top)
	gtk_window_set_transient_for(GTK_WINDOW(dialog_connect), GTK_WINDOW(window) );
	dialog_content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_connect));

	/*message label*/
	msg_label = gtk_label_new(msg);

	/*address grid*/
	addr_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(addr_grid), 10);

	/*address label*/
	addr_label = gtk_label_new("Address:");
	gtk_widget_set_hexpand(addr_label, TRUE);
	gtk_widget_set_halign(addr_label, GTK_ALIGN_START);

	/*address entry*/
	address_entry_field = gtk_entry_new();

	/*port grid*/
	port_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(port_grid), 10);

	/*port label*/
	port_label = gtk_label_new("Port:");
	gtk_widget_set_hexpand(port_label, TRUE);
	gtk_widget_set_halign(port_label, GTK_ALIGN_START);

	/*port entry*/
	port_entry_field = gtk_entry_new();
	gtk_entry_set_input_purpose(GTK_ENTRY(port_entry_field), GTK_INPUT_PURPOSE_DIGITS);	//help the onscreen keyboards

	/*pack message and address and port inputs*/
	gtk_box_pack_start( GTK_BOX(dialog_content_area), msg_label, FALSE, FALSE, 0 );
	gtk_grid_attach(GTK_GRID(addr_grid), addr_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(addr_grid), address_entry_field, 1, 0, 1, 1);
	gtk_box_pack_start( GTK_BOX(dialog_content_area), addr_grid, FALSE, FALSE, 0 );
	gtk_grid_attach(GTK_GRID(port_grid), port_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(port_grid), port_entry_field, 1, 0, 1, 1);
	gtk_box_pack_start( GTK_BOX(dialog_content_area), port_grid, FALSE, FALSE, 0 );
	gtk_dialog_add_button( GTK_DIALOG(dialog_connect),  "Connect", GTK_RESPONSE_OK );

	/*connect the "response" signal*/
	g_signal_connect( GTK_DIALOG(dialog_connect), "response", G_CALLBACK(connect_cb), NULL);
	g_signal_connect_swapped( GTK_DIALOG(dialog_connect), "response", G_CALLBACK(gtk_widget_destroy), dialog_connect);

	//show
	gtk_widget_show(msg_label);
	gtk_widget_show(addr_grid);
	gtk_widget_show(addr_label);
	gtk_widget_show(address_entry_field);
	gtk_widget_show(port_grid);
	gtk_widget_show(port_label);
	gtk_widget_show(port_entry_field);
	gtk_widget_show(dialog_connect);

	//we no longer need the pointer to the dialog, the callbacks have it if they need it
	dialog_connect = NULL;

	return;
}

/*open up a dialog for adding a contact.*/
extern void popup_add_contact(void)
{
    GtkWidget *dialog_content_area = NULL,
                *contact_entry_field = NULL;
    GtkEntryBuffer *field_buffer = NULL;
    
	/*dialog*/
	dialog_add_contact = gtk_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog_add_contact), GTK_WINDOW(window) );
	dialog_content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_add_contact));

	/*entry*/
	contact_entry_field = gtk_entry_new();
	field_buffer = gtk_entry_buffer_new( NULL, 0 );
	gtk_entry_set_buffer( GTK_ENTRY(contact_entry_field), GTK_ENTRY_BUFFER(field_buffer) );

	/*start packing (buttons emit the "response" signal when clicked since they are in the action area)*/
	gtk_box_pack_start( GTK_BOX(dialog_content_area), contact_entry_field, FALSE, FALSE, 0 );
	gtk_dialog_add_button( GTK_DIALOG(dialog_add_contact), "Cancel", GTK_RESPONSE_CANCEL );
	gtk_dialog_add_button( GTK_DIALOG(dialog_add_contact), "OK/Add...", GTK_RESPONSE_OK );

	/*connect the "response" signal*/
	g_signal_connect(GTK_DIALOG(dialog_add_contact), "response", G_CALLBACK(add_contact_cb), NULL);
	g_signal_connect_swapped(GTK_DIALOG(dialog_add_contact), "response", G_CALLBACK(gtk_widget_destroy), dialog_add_contact);

	/*I forgot it again... show the widgets*/
	gtk_widget_show(dialog_add_contact);
	gtk_widget_show(contact_entry_field);

    return;
}
