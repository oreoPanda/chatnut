/*file_operations.h*/

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

#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <gtk/gtk.h>

extern GtkListStore *create_contact_list_model(void);
extern gboolean add_contact_to_list(const char *contact);
extern gboolean load_file( const char *dir, const char *name, char **to );
extern void append_to_history( const char *message, const char *buddyname, gboolean received );
extern int init_chatnut_directory(void);
extern int init_user_directory(void);

#endif /* FILE_OPERATIONS_H */

