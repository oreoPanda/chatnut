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

//TODO this is part of chatnut

#include "connection.h"
#include "connection_backend.h"
#include "gui.h"
#include "logger.h"

GIOChannel *channel = NULL;
char *address = NULL;
unsigned short port = 0;//TODO ok? what happens on port 0?
gboolean connected = FALSE;		//only used by watch_connection and channel_in_handle
gboolean waiting_for_data = FALSE;

extern gboolean channel_not_null(void)
{
	if(channel)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static void create_channel(int socket)
{
    GIOStatus status;
    GError *err = NULL;

    channel = g_io_channel_win32_new_socket(socket);
    if( !channel )
    {
    	//TODO ERRNO?
        error("Connection", "Unable to create GIOChannel");
        return;
    }
    while( (status = g_io_channel_set_encoding( channel, NULL, &err )) == G_IO_STATUS_AGAIN )
    {
        warn("Connection", "Trying again to set encoding on GIOChannel");
    }
    switch(status)
    {
        case G_IO_STATUS_ERROR:
        {
            //TODO handle this error
            break;
        }
        default:
        {
            break;
        }
    }
    if(err)
    {
        error("Connection", err->message);
    }

    logg("Connection", "Created a GIOChannel");

    return;
}

extern gboolean read_line_from_channel(char **line)
{
	/*read from the channel*/
	GIOStatus status;
	gsize length = NULL;	//the length of read data will go here
	gsize line_terminator_pos = NULL;	//position of '\n' will go here (both gsizes will be NULL if no data read)
	GError *err = NULL;
	gboolean return_value = SUCCESS;

	while( (status = g_io_channel_read_line( channel, line, &length, &line_terminator_pos, &err )) == G_IO_STATUS_AGAIN )
	{
		warn("Connection", "Trying again to read line from GIOChannel");
	}
	switch(status)
	{
		case G_IO_STATUS_ERROR:
		{
			error("Connection", "Unable to read from GIOChannel");
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		case G_IO_STATUS_EOF:
		{
			warn("Connection", "Reached end of file while reading from GIOChannel");
			return_value = FAILURE;
			break;
		}
		default:
		{
			warn("Connection", "Unknown return value while reading from GIOChannel");
			break;
		}
	}
	if(err)
	{
		error("Connection", err->message);
		err = NULL;//free()? TODO
	}

	/*replace line terminator with NULL*/
	if(*line)
	{
		*( *line + line_terminator_pos ) = '\0';
	}

	return return_value;
}

/*username should not be given when sending commands*/
extern void write_to_channel( const char *message, const char *username )
{
	/*write to the channel*/
	GIOStatus status;
	gchar *data = NULL;
	gssize length;
	gssize length_of_data;
	gchar length_str[4];
	gsize bytes_written;
	GError *err = NULL;

	/*calculate length of data that is to be sent TODO note somewhere that the word length always excludes \0*/
	if(username)
	{
		length = strlen(username) + strlen(" ") + strlen(message);
	}
	else
	{
		length = strlen(message);
	}

	/*length of data has to be length plus the length of length_str, which is 4*/
	length_of_data = length + 4;

	/*convert count to string*/
	for( int i = 3; i >= 0; i-- )
	{
		length_str[i] = 48 + length % 10;
		length /= 10;
	}

	/*put the length as string (if needed the username and a space) and the message into data.*/
	data = calloc( length_of_data + 1 , sizeof(char) );
	if(username)
	{
		strncpy( data, length_str, 4 );
		strncpy( data+4, username, strlen(username) + 1 );
		strncat( data, " ", 1 );
		strncat( data, message, strlen(message) );
	}
	else
	{
		strncpy( data, length_str, 4 );
		strncpy( data+4, message, strlen(message) + 1 );
	}

	/*send the completed data*/
	while( (status = g_io_channel_write_chars( channel, data, length_of_data, &bytes_written, &err )) == G_IO_STATUS_AGAIN )
	{
		warn("Connection", "Trying again to write to GIOChannel");
	}

	switch(status)
	{
		case G_IO_STATUS_ERROR:
		{

			error("Connection", "Unable to write to GIOChannel");
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		case G_IO_STATUS_EOF:
		{
			warn("Connection", "Reached end of file while writing to GIOChannel");
			break;
		}
		default:
		{
			warn("Connection", "Unknown return value while writing to GIOChannel");
			break;
		}
	}
	if( bytes_written < (gsize)length_of_data )
	{
		warn("Connection", "Not all bytes were written to GIOChannel");
	}
	if(err)
	{
		error("Connection", err->message);
		err = NULL;	//TODO free?
	}

	/*free data since it is no longer needed*/
	g_free(data);

	/*as a convenience, let it point at NULL now that it's freed*/
	data = NULL;

	/*flush the channel*/
	while( (status = g_io_channel_flush(channel, &err) ) == G_IO_STATUS_AGAIN )
	{
		warn("Connection", "Trying again to flush GIOChannel");
	}
	switch(status)
	{
		case G_IO_STATUS_ERROR:
		{
			error("Connection", "Unable to flush GIOChannel");
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		default:
		{
			warn("Connection", "Unknown return value flushing GIOChannel");
			break;
		}
	}
	if(err)
	{
		error("Connection", err->message);
		err = NULL;//TODO free?
	}

	return;
}

/*TODO check if errno works and then decide to use this way to print errors or the one used in write_to_channel()*/
static void shutdown_channel(void)
{
	GIOStatus status;
	gboolean flush = FALSE;
	GError *err = NULL;

	while( (status = g_io_channel_shutdown(channel, flush, &err) ) == G_IO_STATUS_AGAIN )
	{
		warn("Connection", "Trying again to shut down GIOChannel");
	}

	switch(status)
	{
		case G_IO_STATUS_NORMAL:
		{
			logg("Connection", "Successfully shut down GIOChannel");
			break;
		}
		case G_IO_STATUS_EOF:
		{
			warn("Connection", "Reached end of file while shutting down GIOChannel");
			break;
		}
		case G_IO_STATUS_ERROR:
		{
			error("Connection", "Unable to shut down GIOChannel");
			break;
		}
		default:
		{
			warn("Connection", "Unknown return value while shutting down GIOChannel");
			break;
		}
	}

	if(err)
	{
		error("Connection", err->message);
		g_error_free(err);//TODO?
	}

	return;
}

//check that this callback isn't reconnected indefinitely
static gboolean channel_in_handle( GIOChannel *source, GIOCondition condition, gpointer eval_func )
{
		if(source != channel || condition != G_IO_IN || !eval_func)
		{
			warn("Connection", "Unexpected argument while handling incoming data");
			return G_SOURCE_REMOVE;
		}
		
    int read_status = FAILURE;
    char *buffer = NULL;
    void (*evaluate)(const char *) = eval_func;
    
    read_status = read_line_from_channel(&buffer);  //returns FAILURE when EOF

        if(read_status == SUCCESS)
        {
            evaluate( (const char *)buffer );
            g_free(buffer);
            return G_SOURCE_CONTINUE;
        }
        else
        {
            g_free(buffer);
            connected = FALSE;
            disable_add_contact_button();
            return G_SOURCE_REMOVE;
        }
}

/*sets the variables address and port*/
//TODO check, strncpy
extern void set_connection_data(const char *addr, unsigned short p)
{
	free(address);  //only frees if non-null
	address = calloc(strlen(addr)+1, sizeof(char));
	strncpy(address, addr, strlen(addr)+1);
	port = p;
	
	return;
}

extern gboolean watch_connection(gpointer eval_func)
{
	/*only do something if chatnut isn´t connected yet*/
	if(!connected)
	{
		/*ask for server connection data if it was not set yet*/
		if(!address)
		{
			if(!waiting_for_data)
			{
				popup_connect();
				waiting_for_data = TRUE;
			}
		}
		else
		{
			if(channel)
			{
				shutdown_channel();
				g_io_channel_unref(channel);
				channel = NULL;
			}

			/*this will only be done if connection is lost and channel is NULL*/
			socket_t sock = create_socket();
			if( sock > 0 )
			{
				connected = connect_socket(&sock, address, port);
				if(connected)	//chatnut is now connected to a server
				{
					create_channel(sock);
					g_io_add_watch( channel, G_IO_IN, channel_in_handle, eval_func );
				}
				else		//connection failed, ask for data again
				{
					free(address);
					address = NULL;
					port = 0;
					waiting_for_data = FALSE;
					close_socket(&sock);
				}
			}
		}
	}

	return G_SOURCE_CONTINUE;	//don't remove this event's source (for now). If you do, this function isn't called
}

/*free and reset connection data*/
extern void cleanup_connection_data(void)
{
	free(address);
	address = NULL;
	port = 0;
}
