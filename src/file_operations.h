/*file_operations.h*/

#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <gtk/gtk.h>

extern GtkListStore *create_contact_list_model(void);
extern void add_contact_to_list(const char *contact);
extern void load_history( const char *contact, char **to );
extern void append_to_history( const char *message, const char *username );
extern int init_chatnut_directory(void);
extern int init_user_directory(void);

#endif /* FILE_OPERATIONS_H */

