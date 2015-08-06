/*socketprx.h - for UNIX*/

#include <errno.h>		//for errno used in error_exit()
#include <fcntl.h>		//for nonblocking sockets
#include <netinet/in.h>		//for struct sockaddr_in used in connect()
#include <stdio.h>		//standard I/O
#include <stdlib.h>		//for exit(EXIT_FAILURE) used in error_exit()
#include <sys/select.h>		//for select() in connect()
#include <sys/socket.h>		//for sockets
#include <string.h>		//for memset zero-filling and memcpy
#include <unistd.h>		//for close()
#include <arpa/inet.h>		//for network byte order conversions
#include <netdb.h>		//for gethostbyname()
	//TODO do I need sys/types.h?

#include <glib.h>

#define socket_t int
#define SUCCESS 0
#define FAILURE 1

//void reset_timeout(struct timeval *timeout);
void error_exit( char *error_message );
int create_socket( int af, int type, int protocol );
void bind_socket( socket_t *sock, unsigned long address, unsigned short port );
void listen_socket( socket_t *sock );
void connect_socket( socket_t *sock, char *server_addr, unsigned short port );
void my_select( int numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout );
void accept_socket( socket_t *sock, socket_t *new_sock );
void cleanup(void);
void close_socket( socket_t *sock );

static gboolean channel_data_in_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data );
static gboolean channel_data_out_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data );
static gboolean channel_error_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data );
static gboolean channel_hungup_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data );
void receive_incoming( GIOChannel *channel );
void send_outgoing( GIOChannel *channel, char *data );

static void evaluate_incoming( char *message );

void print_error( char *message );