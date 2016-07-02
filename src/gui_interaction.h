/*gui_interaction.h*/

/*Copyright (C) 2016 Jonas Fuglsang-Petersen*/

/*This file is part of chatnut.*/

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

#ifndef GUI_INTERACTION_H
#define GUI_INTERACTION_H

#include <gtk/gtk.h>

extern void connect_cb(GtkDialog *dialog, gint response_id, gpointer data);
extern void add_contact_cb(GtkDialog *dialog, gint response_id, gpointer data);
extern void login_cb(GtkDialog *dialog, gint response_id, gpointer data);
extern void Register_cb(GtkDialog *dialog, gint response_id, gpointer data);

extern void contact_selection_cb( GtkTreeView *treeview, GtkTreePath *treepath, GtkTreeViewColumn *column, gpointer data );
extern gboolean input_view_key_pressed_cb( GtkWidget *inputview, GdkEvent *event, gpointer data );
extern void add_contact_button_press_cb(GtkButton *button, gpointer data);

#endif /* GUI_INTERACTION_H */
