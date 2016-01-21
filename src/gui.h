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
extern void enable_input_view( gboolean (*key_pressed_cb)( GtkWidget *, GdkEvent *, gpointer ),
                                gboolean (*key_released_cb)( GtkWidget *, GdkEvent *, gpointer) );
extern void clear_input_view(void);
extern void show_message_history( const char *history );
extern void append_to_history_view( const char *buffer, const char *sender );
extern void create_right_grid(void);
extern void create_list_view(GtkListStore *model);
extern void add_contact_to_list_view(const char *contact);
extern void create_label(const gchar *message);
extern void destroy_label(void);
extern void create_buttons(void);

extern void populate_window(void);
extern void populate_window_with_list_or_label(void);
extern void enable_add_contact_button(void);

extern gboolean popup_login(gpointer data);
extern gboolean popup_add_contact( GtkButton *button, gpointer data );

#endif /* GUI_H_ */
