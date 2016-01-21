/*connection.h*/

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 1
#define FAILURE 0

extern gboolean channel_not_null(void);
extern gboolean read_line_from_channel(char **line);
extern void write_to_channel( const gchar *buf, const char *username );
extern gboolean watch_connection(gpointer data);

#endif /* CONNECTION_H_ */
