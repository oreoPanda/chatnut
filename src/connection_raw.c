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
#include "logger.h"

#define TRUE 1
#define FALSE 0

/*create a socket*/
extern int create_socket(void)
{
	socket_t sock;
	const int y = 1;

	errno = 0;
	sock = socket( PF_INET, SOCK_STREAM, 0 );
	if( sock < 0 )		//check if it worked
	{
		error("Connection Backend", "Unable to create socket");
	}

	//SO_REUSEADDR makes the socket not be unusable for two minutes after server closes
	if( (setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int) ) ) != 0)
	{
		error("Connection Backend", "Unable to set socket reusable");
		return -1;
	}

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
			errno = 0;	//gethostbyname does not set errno, set to 0 to avoid invalid error message
			error("Connection Backend", "Error resolving server address (unknown server)");
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

	errno = 0;
	/*connect to server and check if connecting worked*/
	if( connect( *sock, (struct sockaddr*)&server, sizeof(server) ) < 0)
	{
		error("Connection Backend", "Unable to connect to server");
		return FALSE;
	}
	else
	{
		//say who the client is connected to
		logg("Connection Backend", "Connected to server");
		return TRUE;
	}
}

extern void close_socket( socket_t *sock )
{
	logg("Connection Backend", "Closing socket");
	errno = 0;
	if( close(*sock) < 0 )
	{
		error("Connection Backend", "Unable to close socket");
	}
}
