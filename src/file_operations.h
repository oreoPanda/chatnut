/*file_operations.h*/

#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <gtk/gtk.h>

extern GtkListStore *create_contact_list_model(void);
static size_t separate_lines( const char *raw, char ***lines );
extern gboolean add_contact_to_list(const char *contact);
extern gboolean load_file( const char *dir, const char *name, char **to );
extern void append_to_history( const char *message, const char *buddyname, gboolean received );
extern int init_chatnut_directory(void);
extern int init_user_directory(void);

#endif /* FILE_OPERATIONS_H */

