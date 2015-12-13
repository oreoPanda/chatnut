/*connection.c*/

#include "connection.h"
#include "connection_raw.h"
#include "jstring.h"

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

    printf( "(create_channel)Created a GIOChannel\n" );

    return;
}

extern gboolean read_line_from_channel(char **line)
{
	/*read from the channel*/
	GIOStatus status;
	gsize length = NULL;	//the length of read data will go here
	gsize line_terminator_pos = NULL;	//position of '\n' will go here (both gsizes will be NULL if no data read)
	GError *error = NULL;
	gboolean return_value = SUCCESS;

	printf("(read_line_from_channel)will read line\n");

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
			printf( "(read_line_from_channel)Line was read: %s\n", *line );
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
			fprintf( stderr, "Unknown return value\n" );
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
			fprintf( stderr, "Error writing to stdout via a GIOChannel.\n" );
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			printf( "(write_to_channel)Normal return value writing \"%s\" to channel.\n", data );
			break;
		}
		case G_IO_STATUS_EOF:
		{
			fprintf( stderr, "Reached end of file while writing to stdout via a GIOChannel.\n" );
			break;
		}
		default:
		{
			fprintf( stderr, "Unknown return value\n" );
			break;
		}
	}
	if( bytes_written < (gsize)length_of_data )
	{
		fprintf( stderr, "(write_to_channel)Not all bytes were written.\n" );
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
			fprintf( stderr, "(write_to_channel)Error flushing stdout via a GIOChannel.\n" );
			break;
		}
		case G_IO_STATUS_NORMAL:
		{
			break;
		}
		default:
		{
			fprintf( stderr, "Unknown return value\n" );
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
	printf( "channel->ref_count after shutdown = %i\n", channel->ref_count );
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
		{
			printf( "Shutdown of GIOChannel successful.\n" );
			break;
		}
		case G_IO_STATUS_EOF:
		{
			print_error("Reached end of file while shutting down the GIOChannel");
			break;
		}
		case G_IO_STATUS_ERROR:
		{
			print_error("Error shutting down the GIOChannel");
			break;
		}
		default:
		{
			print_error("Unknown status of g_io_channel_shutdown");
			break;
		}
	}

	if(error)
	{
		print_error(error->message);
		g_error_free(error);
	}

	return;
}

static gboolean channel_in_handle( GIOChannel *source, GIOCondition condition, gpointer eval_func )
{
	int read_status;
	char *buffer = NULL;
	void (*evaluate)(const char *) = eval_func;

	switch(condition)
	{
		case G_IO_IN:
		{
			read_status = read_line_from_channel(&buffer);
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
		int sock = create_socket();
		if( sock > 0 )
		{
			connected = connect_socket( &sock, "localhost", 1234 );
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

	return G_SOURCE_CONTINUE;   //this callback should not be removed
}
