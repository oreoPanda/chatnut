/*connection_backend.h*/

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

#ifndef CONNECTION_BACKEND_H_
#define	CONNECTION_BACKEND_H_

#include <errno.h>
//#include <fcntl.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
//#include <sys/select.h>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <arpa/inet.h>
//#include <netdb.h>
#include <winsock.h>
#include <io.h>

#define socket_t int


extern int winsock_init(void);
extern socket_t create_socket(void);
extern int connect_socket( socket_t *sock, char *server_addr, unsigned short port );
extern void close_socket( socket_t *sock );

#endif	/* CONNECTION_BACKEND_H_ */
