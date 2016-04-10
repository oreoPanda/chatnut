/*connection.c*/

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

#include "connection_raw.h"

#define TRUE 1
#define FALSE 0

/*initialize winsock*/
extern int init_winsock(void)
{
	WORD version;
	WSADATA winsockdata;

	version = MAKEWORD(1, 1);
	if(WSAStartup(version, &data) != 0)
	{
		print_error("Error initializing winsock");	//TODO winsock error message/function
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*create a socket*/
extern int create_socket(void)
{
	socket_t sock;
	const int y = 1;

	sock = socket( PF_INET, SOCK_STREAM, 0 );
	if( sock < 0 )		//check if it worked
	{
		print_error("Error creating socket");
	}

	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int) );//SO_REUSEADDR makes the socket not be unusable for two minutes after server closes

	return sock;
}

extern int connect_socket(socket_t *sock, char *server_addr, unsigned short port)
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
			print_error("Error resolving server address (unknown server)");
                        return FALSE;
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
		print_error("Error connecting to server");
		return FALSE;
	}
	else
	{
		//say who the client is connected to
		printf( "[Connection] Connected to server with address %s\n", inet_ntoa(server.sin_addr) );
		return TRUE;
	}
}

extern void close_socket( socket_t *sock )
{
	printf( "[Connection] Closing socket %d\n", *sock );
	if( close(*sock) < 0 )
	{
		fprintf( stderr, "Error closing socket!: %s\n", strerror(errno) );
	}
}

extern void print_error( char *message )
{
    fprintf( stderr, "ERROR: %s: %s\n", message, strerror(errno) );

    return;
}
