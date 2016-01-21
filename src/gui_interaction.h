/*gui_interaction.h*/

#ifndef GUI_INTERACTION_H
#define GUI_INTERACTION_H

#include <gtk/gtk.h>

extern gboolean add_contact( GtkDialog *dialog, gint response_id, gpointer data );
extern gboolean login( GtkDialog *dialog, gint response_id, gpointer data );
extern void contact_selection_handler( GtkTreeView *treeview, GtkTreePath *treepath, GtkTreeViewColumn *column, gpointer data );
extern gboolean input_view_key_pressed_cb( GtkWidget *inputview, GdkEvent *event, gpointer data );
extern gboolean input_view_key_released_cb( GtkWidget *inputview, GdkEvent *event, gpointer data );

#endif /* GUI_INTERACTION_H */

