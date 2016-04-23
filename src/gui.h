/*gui.h*/

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

#ifndef GUI_H_
#define GUI_H_

#include <gtk/gtk.h>

extern void create_window(void);
extern void update_window_title(void);
extern void create_main_pane(void);
extern void create_left_pane(void);
extern void create_history_scrollbox(void);
extern void create_history_view(void);
extern void create_input_scrollbox(void);
extern void create_input_view(void);
extern gboolean input_view_get_enabled(void);
extern void enable_input_view(void);
extern void disable_input_view(void);
extern void clear_input_view(void);
extern void show_message_history( const char *history );
extern void append_to_history_view( const char *buffer, const char *sender );
extern void create_right_grid(void);
extern void init_list_view(void);
extern void show_list_view(GtkListStore *model);
extern void add_contact_to_list_view(const char *contact);
extern void create_label(const gchar *message);
extern void destroy_label(void);
extern void destroy_list(void);
extern void create_buttons(void);

extern void populate_window(void);
extern void populate_window_with_list(void);
extern void populate_window_with_label(void);
extern void enable_add_contact_button(void);

extern gboolean window_contains_label(void);
extern gboolean window_contains_list(void);

extern gboolean popup_login(gpointer data);
extern gboolean popup_add_contact( GtkButton *button, gpointer data );

#endif /* GUI_H_ */
