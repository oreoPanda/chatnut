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

#include "connection.h"
#include "connection_backend.h"

GIOChannel *channel = NULL;
gboolean connected = FALSE;		//only used by watch_connection and channel_in_handle

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

static void create_channel(int fd)
{
    GIOStatus status;
    GError *error = NULL;

    channel = g_io_channel_unix_new(fd);
    if( !channel )
    {
    	//TODO use different error mechanism for this
        fprintf( stderr, "Could not create GIOChannel.\n" );
        return;
    }
    while( (status = g_io_channel_set_encoding( channel, NULL, &error )) == G_IO_STATUS_AGAIN )
    {
        fprintf( stderr, "Trying again to create GIOChannel.\n" );
    }
    switch(status)
    {
        case G_IO_STATUS_ERROR:
        {
            //TODO handle this error
            fprintf( stderr, "Cannot set encoding on GIOChannel.\n" );
            break;
        }
        default:
        {
            break;
        }
    }
    if(error)
    {
        fprintf( stderr, "%s\n", error->message );
    }

    printf( "[Connection] Created a GIOChannel\n" );

    return;
}

extern gboolean read_line_from_channel(char **line)
{
	/*read from the channel*/
	GIOStatus status;
	gsize length = 0;	//the length of read data will go here
	gsize line_terminator_pos = 0;	//position of '\n' will go here (both gsizes will be NULL if no data read)
	GError *error = NULL;
	gboolean return_value = SUCCESS;

	while( (status = g_io_channel_read_line( channel, line, &length, &line_terminator_pos, &error )) == G_IO_STATUS_AGAIN )
	{
		printf("Trying again.\n");
	}
	switch(status)
	{
		case G_IO_STATUS_ERROR:
		{
			fprintf( stderr, "Error reading from GIOChannel.\n" );
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		case G_IO_STATUS_EOF:
		{
			fprintf( stderr, "Reached end of file while reading from GIOChannel.\n" );
			return_value = FAILURE;
			break;
		}
		default:
		{
			fprintf( stderr, "[Connection] Unknown return value while reading from GIOChannel\n" );
			break;
		}
	}
	if(error)
	{
		fprintf( stderr, "%s", error->message );
		error = NULL;
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
	GError *error = NULL;

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
	while( (status = g_io_channel_write_chars( channel, data, length_of_data, &bytes_written, &error )) == G_IO_STATUS_AGAIN );

	switch(status)
	{
		case G_IO_STATUS_ERROR:
		{
			fprintf( stderr, "Error writing to a GIOChannel.\n" );
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		case G_IO_STATUS_EOF:
		{
			fprintf( stderr, "Reached end of file while writing to a GIOChannel.\n" );
			break;
		}
		default:
		{
			fprintf( stderr, "Unknown return value while writing to a GIOChannel\n" );
			break;
		}
	}
	if( bytes_written < (gsize)length_of_data )
	{
		fprintf( stderr, "[Connection] Not all bytes were written to GIOChannel\n" );
	}
	if(error)
	{
		fprintf( stderr, "%s", error->message );
		error = NULL;
	}

	/*free data since it is no longer needed*/
	g_free(data);

	/*as a convenience, let it point at NULL now that it's freed*/
	data = NULL;

	/*flush the channel*/
	while( (status = g_io_channel_flush( channel, &error )) == G_IO_STATUS_AGAIN );
	switch(status)
	{
		case G_IO_STATUS_ERROR:
		{
			fprintf( stderr, "(write_to_channel)Error flushing GIOChannel.\n" );
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		default:
		{
			fprintf( stderr, "Unknown return value flushing GIOChannel\n" );
			break;
		}
	}
	if(error)
	{
		fprintf( stderr, "%s", error->message );
		error = NULL;
	}

	return;
}

static void shutdown_channel(void)
{
	GIOStatus status;
	gboolean flush = FALSE;
	GError *error = NULL;

	while( (status = g_io_channel_shutdown( channel, flush, &error )) == G_IO_STATUS_AGAIN );
	//printf( "channel->ref_count after shutdown = %i\n", channel->ref_count );
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
		{
			printf( "[Connection] Shutdown of GIOChannel successful.\n" );
			break;
		}
		case G_IO_STATUS_EOF:
		{
			print_error("Reached end of file while shutting down the GIOChannel", errno);
			break;
		}
		case G_IO_STATUS_ERROR:
		{
			print_error("Unable to shut down the GIOChannel", errno);
			break;
		}
		default:
		{
			print_error("Unknown status of g_io_channel_shutdown", errno);
			break;
		}
	}

	if(error)
	{
		print_error(error->message, errno);
		g_error_free(error);
	}

	return;
}

static gboolean channel_in_handle( GIOChannel *source, GIOCondition condition, gpointer eval_func )
{
    int read_status = FAILURE;
    char *buffer = NULL;
    void (*evaluate)(const char *) = eval_func;

    if( source == channel )
    {
        switch(condition)
        {
            case G_IO_IN:
            {
                read_status = read_line_from_channel(&buffer);  //returns FAILURE when EOF
                break;
            }
            default:
            {
                printf("Unknown GIOCondition %d.\n", condition);	//TODO use error function
                break;
            }
        }

        if( read_status == SUCCESS )
        {
            evaluate( (const char *)buffer );
            g_free(buffer);
            return G_SOURCE_CONTINUE;
        }
        else
        {
            g_free(buffer);
            connected = FALSE;
            return G_SOURCE_REMOVE;
        }
    }
    else
    {
        fprintf( stderr, "[Connection] Warning The passed channel doesn't match the global channel." );
        return G_SOURCE_REMOVE;
    }
}

extern gboolean watch_connection(gpointer eval_func)
{
    if(!connected)  //if not connected, has to be connected
    {
        if(channel)
        {
            shutdown_channel();
            g_io_channel_unref(channel);
            channel = NULL;
        }

        /*this will only be done if connection is lost and channel is NULL*/
        socket_t sock = create_socket();
        if( sock != (unsigned int)SOCKET_ERROR )
        {
            connected = connect_socket( &sock, "192.168.178.20", 1234 );
            if(connected)
            {
                create_channel(sock);
                g_io_add_watch( channel, G_IO_IN, channel_in_handle, eval_func );
            }
            else
            {
                close_socket(&sock);
            }
        }
    }

    return G_SOURCE_CONTINUE;   //this callback should not be removed <-- wrong comment TODO
}