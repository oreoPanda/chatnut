/*connection_raw.h*/

#ifndef CONNECTION_RAW_H_
#define	CONNECTION_RAW_H_

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define socket_t int

extern int create_socket(void);
extern int connect_socket( socket_t *sock, char *server_addr, unsigned short port );
extern void close_socket( socket_t *sock );
extern void print_error(char *message);

#endif	/* CONNECTION_RAW_H_ */
