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
extern gboolean input_view_is_enabled(void);
extern void enable_input_view(void);
extern void disable_input_view(void);
extern void clear_input_view(void);
extern void show_message_history( const char *history );
extern void append_to_history_view( const char *buffer, const char *sender );
extern void create_right_grid(void);
extern void create_list_view(void);
extern void create_label(const gchar *message);
extern void toggle_list_view(gboolean toggleon, GtkListStore *model);
extern void add_contact_to_list_view(const char *contact);
extern void edit_label(const gchar *text);
extern void destroy_list_or_label(void);
extern void create_buttons(void);

extern void populate_window(void);
extern void disable_add_contact_button(void);
extern void enable_add_contact_button(void);

enum DialogResponseType
{
	RESPONSE_REGISTER
};

extern void popup_connect(const char *msg);
extern void popup_login(const char *msg);
extern void popup_register(void);
extern void popup_add_contact(void);

#endif /* GUI_H_ */
