/*gui.h*/

#ifndef GUI_H_
#define GUI_H_

#include <gtk/gtk.h>

extern void create_window(void);
extern void create_main_pane(void);
extern void create_left_pane(void);
extern void create_history_scrollbox(void);
extern void create_history_view(void);
extern void create_input_scrollbox(void);
extern void create_input_view(void);
extern void enable_input_view( gboolean (*key_pressed_cb)( GtkWidget *, GdkEvent *, gpointer ) );
extern void show_message_history( const char *history );
extern void append_to_history_view( const char *buffer, const char *sender );
extern void create_right_grid(void);
extern void create_list_view(GtkListStore *model);
extern void functionalize_list_view( void (*contact_selected_func)( GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, gpointer ) );
extern void create_label(const gchar *message);
extern void destroy_label(void);
extern void create_buttons(void);

extern void populate_window(void);
extern void populate_window_with_list_or_label(void);
extern void functionalize_window( gboolean (*add_contact_callback_func)( GtkButton *, gpointer ) );

extern void popup_add_contact( gboolean (*response_handle)( GtkDialog *, gint, gpointer ) );
extern gboolean popup_login(gpointer login_func);

#endif /* GUI_H_ */
