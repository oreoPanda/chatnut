/*gui.c*/

#include "gui.h"
#include "gui_interaction.h"
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
                *dialog_login = NULL;

gboolean input_view_enabled = FALSE;
gboolean contains_label = FALSE;

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

    //gtk_widget_set_size_request( history_scrollbox, 300, 100 );
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

    /* Change default color throughout the widget */
    GdkRGBA rgba;
    gdk_rgba_parse( &rgba, "green" );
    gtk_widget_override_color( history_view, GTK_STATE_FLAG_NORMAL, &rgba );

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
		strncpy( to_append, sender, strlen(sender) );
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

    //visible
    gtk_widget_show(input_view);

    return;
}

extern gboolean input_view_get_enabled(void)
{
	return input_view_enabled;
}

extern void enable_input_view(void)
{
        gtk_text_view_set_editable( GTK_TEXT_VIEW(input_view), TRUE );
        gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(input_view), TRUE );

        //TODO make sure this is disconnected when input view is disabled
        g_signal_connect( input_view, "key-press-event", G_CALLBACK(input_view_key_pressed_cb), NULL );
        g_signal_connect( input_view, "key-release-event", G_CALLBACK(input_view_key_released_cb), NULL );

        input_view_enabled = TRUE;

        return;
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
extern void init_list_view(void)
{
	GtkTreeViewColumn *column = NULL;
	GtkCellRenderer *renderer = NULL;

	list = gtk_tree_view_new();

	gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(list), FALSE );
	gtk_widget_set_hexpand( list, TRUE );
	gtk_widget_set_vexpand( list, TRUE );
	gtk_tree_view_set_activate_on_single_click( GTK_TREE_VIEW(list), FALSE );

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes( "Name", renderer, "text", 0, NULL );//"text" is an attribute (property) of a GtkCellRendererText
	gtk_tree_view_append_column( GTK_TREE_VIEW(list), column );

	g_signal_connect( list, "row-activated", G_CALLBACK(contact_selection_handler), NULL );


	return;
}

extern void show_list_view(GtkListStore *store)
{
	/*add the model to the treeview*/
	gtk_tree_view_set_model( GTK_TREE_VIEW(list), GTK_TREE_MODEL(store) );

	gtk_widget_show(list);

	return;
}

extern void add_contact_to_list_view(const char *contact)
{
    GtkListStore *store = NULL;
    GtkTreeIter iter;
    
    store = GTK_LIST_STORE( gtk_tree_view_get_model(GTK_TREE_VIEW(list)) );
    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0, contact, -1 );
    
    return;
}

extern void create_label(const gchar *message)
{
        label = gtk_label_new(message);
        gtk_widget_set_hexpand( label, TRUE );
        gtk_widget_set_vexpand( label, TRUE );

        gtk_widget_show(label);

        return;
}

extern void destroy_label(void)
{
	gtk_widget_destroy(label);

	return;
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
	contains_label = TRUE;
	gtk_grid_attach( GTK_GRID(rgrid), button_add_contact, 0, 2, 3, 1 );

	return;
}

extern void populate_window_with_list(void)
{
	gtk_grid_attach( GTK_GRID(rgrid), list, 0, 1, 3, 1 );
	contains_label = FALSE;
    return;
}

extern void populate_window_with_label(void)
{
	gtk_grid_attach( GTK_GRID(rgrid), label, 0, 1, 3, 1 );
	contains_label = TRUE;
	return;
}

/*TODO this call can be put into the function for the button...  ...no?, enabled after login!*/
extern void enable_add_contact_button(void)
{
    g_signal_connect( button_add_contact, "clicked", G_CALLBACK(popup_add_contact), NULL );

    return;
}

extern gboolean window_contains_label(void)
{
	return contains_label;
}

extern gboolean popup_login(gpointer data)
{
    GtkWidget *dialog_content_area = NULL,
                *username_entry_field = NULL,
                *password_entry_field = NULL;
    GtkEntryBuffer *username_buffer = NULL,
                    *password_buffer = NULL,
                    **bufferlist = NULL;

    if( data )
    {
        fprintf( stderr, "Data passed to the login popup will not be used.\n" );
    }
    
    /*dialog*/
    dialog_login = gtk_dialog_new();
    dialog_content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_login));
    gtk_widget_show(dialog_login);

    /*username entry*/
    username_entry_field = gtk_entry_new();
    username_buffer = gtk_entry_buffer_new( NULL, 0 );
    gtk_entry_set_buffer( GTK_ENTRY(username_entry_field), GTK_ENTRY_BUFFER(username_buffer) );
    gtk_widget_show(username_entry_field);

    /*password entry*/
    password_entry_field = gtk_entry_new();
    password_buffer = gtk_entry_buffer_new( NULL, 0 );
    gtk_entry_set_buffer( GTK_ENTRY(password_entry_field), GTK_ENTRY_BUFFER(password_buffer) );
    gtk_widget_show(password_entry_field);

    /*pack*/
    gtk_box_pack_start( GTK_BOX(dialog_content_area), username_entry_field, FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(dialog_content_area), password_entry_field, FALSE, FALSE, 0 );
    gtk_dialog_add_button( GTK_DIALOG(dialog_login),  "OK/Login", GTK_RESPONSE_OK );

    bufferlist = calloc( 2, sizeof(GtkEntryBuffer *) );
    *bufferlist = username_buffer;
    *(bufferlist+1) = password_buffer;

    /*connect the "response" signal*/
    g_signal_connect( GTK_DIALOG(dialog_login), "response", G_CALLBACK(login), NULL );

    return G_SOURCE_REMOVE;
}

//TODO do I need to free() or g_free() (?) dialog_content_area, field_buffer and so on?
extern gboolean popup_add_contact( GtkButton *button, gpointer data )
{
    GtkWidget *dialog_content_area = NULL,
                *contact_entry_field = NULL;
    GtkEntryBuffer *field_buffer = NULL;
    
    if( data )
    {
        fprintf( stderr, "Data passed to the add contact popup will not be used.\n" );
    }

    if( button )
    {
        /*dialog*/
        dialog_add_contact = gtk_dialog_new();
        dialog_content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_add_contact));

        /*entry*/
        contact_entry_field = gtk_entry_new();
        field_buffer = gtk_entry_buffer_new( NULL, 0 );
        gtk_entry_set_buffer( GTK_ENTRY(contact_entry_field), GTK_ENTRY_BUFFER(field_buffer) );

        /*start packing (buttons emit the "response" signal when clicked since they are in the action area)*/
        gtk_box_pack_start( GTK_BOX(dialog_content_area), contact_entry_field, FALSE, FALSE, 0 );
        gtk_dialog_add_button( GTK_DIALOG(dialog_add_contact), "OK/Add...", GTK_RESPONSE_OK );
        gtk_dialog_add_button( GTK_DIALOG(dialog_add_contact), "Cancel", GTK_RESPONSE_CANCEL );

        /*connect the "response" signal*/
        g_signal_connect( GTK_DIALOG(dialog_add_contact), "response", G_CALLBACK(add_contact), field_buffer );

        /*I forgot it again... show the widgets*/
        gtk_widget_show(dialog_add_contact);
        gtk_widget_show(contact_entry_field);
    }

    //TODO return value doesn't seem to matter?
    return G_SOURCE_REMOVE;
}
