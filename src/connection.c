/*socketlayer.c - for UNIX*/

#include "connection.h"

/*void reset_timeout(struct timeval *timeout)
{
	timeout->tv_sec = 0;
	timeout->tv_usec = 10,000; //microseconds: 1,000,000 of them each second, here process will sleep for one 100th of a second
}*/

/*exit with custom error message*/
void error_exit(char *error_message)
{
	fprintf( stderr, "%s: %s\n", error_message, strerror(errno) );

	exit(EXIT_FAILURE);
}

/*create a socket*/
int create_socket(int af, int type, int protocol)
{
	socket_t sock;
	const int y = 1;

	sock = socket( af, type, protocol );
	if( sock < 0 )		//check if it worked
	{
		error_exit("Error creating socket!");
	}
	
	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int) );//SO_REUSEADDR makes the socket not be unusable for two minutes after server closes

	return sock;
}

void bind_socket(socket_t *sock, unsigned long address, unsigned short port)
{
	struct sockaddr_in server;

	memset( &server, 0, sizeof(server) );	//fill structure with zeroes to erase false data

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(address);

	if( bind( *sock, (struct sockaddr*)&server, sizeof(server) ) < 0 )
	{
		error_exit("Error binding socket to port!" );
	}
}

void listen_socket(socket_t *sock)
{
	if( listen(*sock, CLIENT_MAX) == -1)		//will set up queue with length CLIENT_MAX, kernel has a longer queue so more than CLIENT_MAX unconnected clients might be connect()ed successfully
	{
		error_exit("Error listening for connections!");
	}
}

void connect_socket(socket_t *sock, char *server_addr, unsigned short port)
{
	struct sockaddr_in server;
	struct hostent *host_info;
	unsigned long ip_addr;

	memset( &server, 0, sizeof(server) );	//fill structure with zeroes to erase false data

	/*copy the ip address into the server address structure*/
	ip_addr = inet_addr(server_addr);	//turn ip into unsigned long
	if( ip_addr != INADDR_NONE )	//if it is a numeric ip
	{
		memcpy( (char*)&server.sin_addr, &ip_addr, sizeof(ip_addr) );//copy the unsigned long into the structure
	}
	else		//if it is a string address that needs to be resolved (like "localhost")
	{
		host_info = gethostbyname( server_addr );
		if( host_info == NULL )		//if resolving didn't quite work
		{
			error_exit("Error resolving server address (unknown server)!");
		}
		else
		{
			#define h_addr h_addr_list[0]
			memcpy( (char*)&server.sin_addr, host_info->h_addr, host_info->h_length );//copy the resolved address into structure
		}
	}

	/*copy the protocol family and the port number into the structure*/
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	/*connect to server and check if connecting worked*/
	if( connect( *sock, (struct sockaddr*)&server, sizeof(server) ) < 0)
	{
		error_exit("Error connecting to server!");
	}
	else
	{
		//say who the client is connected to
		printf( "Connected to server with address %s\n", inet_ntoa(server.sin_addr) );
	}
}

void my_select( int numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout )
{
	if( select(numfds, readfds, writefds, exceptfds, timeout) == -1)//+1 was already done
	{
		error_exit("Error select()ing socket!");
	}
}


void accept_socket( socket_t *sock, socket_t *new_sock )
{
	struct sockaddr_in client;
	unsigned int len;

	len = sizeof(client);
	*new_sock = accept( *sock, (struct sockaddr*)&client, &len );
	if( *new_sock == -1 )	//if none found
	{
		error_exit("Error accepting connection!");
	}
	else
	{
		/*say who the server is connected to*/
		printf( "Connected to client with address %s\n", inet_ntoa(client.sin_addr) );

		/*char *msg = "hi\0";
		int len = 3;
		TCP_send( new_sock, msg, len, 0 );*/
	}
}

/*receive incoming data, this function is called when the GIOChannel is ready for reading
 *these should be considered as GIOFuncs
 *GLib Reference Manual says: "the function should return FALSE if the event source should be removed"*/
//important for all of them: data is the data passed via g_io_add_watch(), not the data waiting in socket)
static gboolean channel_data_in_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
    receive_incoming( sourcechannel, data );
            
    return FALSE;
}

static gboolean channel_data_out_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
    send_outgoing( sourcechannel, data );
    g_io_channel_unref(sourcechannel);  //decrement reference count, which should disconnect this callback
    
    return FALSE;               //signal handled, remove event source
}

static gboolean channel_error_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
    printf( "GIOChannel reported error.\n" );
    
    return TRUE;       //signal not finally handled, pass it on
}

static gboolean channel_hungup_handle( GIOChannel *sourcechannel, GIOCondition condition, gpointer data )
{
    printf( "GIOChannel was hung up.\n" );
    
    return TRUE;        //signal not finally handled, pass it on
}

//NOTE: the send and receive funtions in here only check if the messages were sent and received correctly.
//          Whether the messages are empty or not is up to funtions such as evaluate_incoming())

//receive incoming data
//TODO: FINISH
void receive_incoming( GIOChannel *channel, char *data )
{
    /*contents shall be similar to send_outgoing*/
    char *str = NULL;
    gsize *length = NULL;
    gsize *terminator_pos = NULL;
    GIOStatus status;
    GError *error = NULL;
    
    //try reading until function returns something other than GIO_STATUS_AGAIN
    while( (status = g_io_channel_read_line( channel,  )) == G_IO_STATUS_AGAIN );
    
    //evaluate returned status
    switch(status)
    {
        case G_IO_STATUS_ERROR:
        {
            print_error( "Unable to read data: " ); //error message from the GError will probably come after here
        }
        case G_IO_STATUS_NORMAL:
        {
            break;
        }
        case G_IO_STATUS_EOF:
        {
            print_error( "Reached end of file while sending data.\n" );
        }
        default:
        {
            print_error("Encountered unknown return value of g_io_channel_write_chars().\n");
        }
    }
    
    //check error variable
    if( error != NULL )
    {
        print_error( error->message );
        print_error("\n");
        g_error_free(error);
    }
    
    evaluate_incoming(str);
    
    return;
}

//send outgoing data, data has to be nul-terminated
void send_outgoing( GIOChannel *channel, char *data )
{
    int len = strlen(data);
    int size = len+1;
    gsize written_size = 0;
    GIOStatus status;
    GError *error = NULL;
    
    //try sending until a value different from GIO_STATUS_AGAIN is returned
    while( (status = g_io_channel_write_chars( channel, data, size, &written_size, &error )) == G_IO_STATUS_AGAIN );
    
    //evaluate return value
    switch(status)
    {
        case G_IO_STATUS_ERROR:
        {
            print_error( "Unable to send data: " );//I'm assuming error will be set so its message will come after here
        }
        case G_IO_STATUS_NORMAL:
        {
            break;
        }
        case G_IO_STATUS_EOF:
        {
            print_error( "Reached end of file while sending data.\n" );
        }
        default:
        {
            print_error("Encountered unknown return value of g_io_channel_write_chars().\n");
        }
    }
    
    //check if all characters were written
    if(written_size != size)
    {
        print_error("Not all characters were sent.\n");
    }
    
    //check error variable
    if( error != NULL )
    {
        print_error( error->message );
        print_error("\n");
        g_error_free(error);
    }
    return;
}

int TCP_recv( socket_t *sock, char *data, int len, int flags )
{	
	int return_val;
	return_val = recv( *sock, data, len, flags );
	if( return_val == -1 )
	{
		fprintf( stderr, "Error receiving data!: %s\n", strerror(errno) );
	}
	else if( return_val == 0 )
	{
		fprintf( stderr, "Looks like the other end is gone!: %s\n", strerror(errno) );
	}

	return return_val;
}

void cleanup(void)
{
	printf("Done cleaning up\n");
	return;
}

void close_socket( socket_t *sock )
{
	printf( "Trying to close socket %d\n", *sock );
	if( close(*sock) < 0 )
	{
		fprintf( stderr, "Error closing socket!: %s\n", strerror(errno) );
	}
}

void print_error( char *message )
{
    fprintf( stderr, "ERROR: %s", message );
    return;
}