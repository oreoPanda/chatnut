/*connection.h*/

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
extern void set_connection_data(char *addr, unsigned short p);
extern gboolean watch_connection(gpointer data);
extern void cleanup_connection_data(void);

#endif /* CONNECTION_H_ */