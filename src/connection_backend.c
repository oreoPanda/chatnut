/*connection_backend.c*/

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

#include "connection_backend.h"

#define FALSE 0
#define TRUE 1

/*initialize winsock*/
extern int winsock_init(void)
{
	WORD version;
	WSADATA winsock_data;

	version = MAKEWORD(1, 1);
	if( WSAStartup(version, &winsock_data) != 0 )
	{
		print_error("Unable to initialize Winsock 1.1", WSAGetLastError() );
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*create a socket
	returns the socket on success or SOCKET_ERROR (-1) on failure*/
extern int create_socket(void)
{
	socket_t sock;

	sock = socket( PF_INET, SOCK_STREAM, 0 );
	if( sock == (unsigned int)SOCKET_ERROR )
	{
		print_error("Unable to create socket", errno);
	}

	//set to non-blocking TODO uncomment anc check/test
	//unsigned long mode = 1;
	//ioctlsocket(sock, FIONBIO, &mode);

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
			print_error("Unable to resolve server address (unknown server)", errno);
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
	if( connect( *sock, (struct sockaddr*)&server, sizeof(server) ) == 0 )
	{
		//say who the client is connected to
		printf( "[Connection] Connected to server with address %s\n", inet_ntoa(server.sin_addr) );
		return TRUE;
	}
	else
	{
		int error = WSAGetLastError();
		if(error != WSAEWOULDBLOCK)
		{
			print_error("Unable to connect to server", error);
		}
		return FALSE;
	}
	
}

extern void close_socket( socket_t *sock )
{
	if( closesocket(*sock) < 0 )
	{
		print_error("Unable to close socket", errno);
	}
}

extern void print_error(char *message, int errnum)
{
    fprintf( stderr, "[Connection backend error] %s: %s (%d)\n", message, strerror(errnum), errnum );

    return;
}
