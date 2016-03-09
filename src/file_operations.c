/*file_operations.c*/

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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>	//for mkdir()

#include "file_operations.h"
#include "user.h"

static size_t separate_lines( const char *raw, char ***lines );

/*loads contacts from file into a GtkListStore*/
extern GtkListStore *create_contact_list_model(void)
{
    GtkListStore *store = NULL;
    GtkTreeIter iter;

    char *raw_list = NULL;
    char **contacts = NULL;
    int count = 0;

    /*load contact list from file*/
    if( load_file( NULL, "contactlist", &raw_list ) )
    {
    	count = separate_lines( raw_list, &contacts );

        /*create contact list model with name and online status for each contact*/
        store = gtk_list_store_new( 1, G_TYPE_STRING );

		/*add contacts to the list store*/
        for( int i = 0; i < count; i++ )
        {
        	gtk_list_store_append( store, &iter );
        	gtk_list_store_set( store, &iter, 0, *(contacts+i), -1 );
        }

        return store;
    }
    else
    {
    	return NULL;
    }
}

/*separates raw contents read from a file into lines*/
static size_t separate_lines( const char *raw, char ***lines )
{
	/*separate contents by line*/
	int line_count = 0;
	unsigned int position = 0;
	int line_start = 0;
	int line_len = 0;

	if(raw)
	{
		while( position < strlen(raw)+1 )
		{
			if( raw[position] == '\n' || raw[position] =='\0' )
			{
				line_count++;
				line_len = position-line_start;		//length of line excluding '\n'

				/*reserve enough space for another line*/
				if( !( *lines = realloc(*lines, line_count*sizeof(char *)) ) )
				{
					line_count--;
					break;
				}
				/*reserve enough space in the new line*/
				if( !( *( *(lines)+(line_count-1) ) = calloc( line_len+1, sizeof(char)) ) )	//one extra for '\0'
				{
					line_count--;
					break;
				}
				strncpy( *( *lines+(line_count-1) ), raw+line_start, line_len );
				*( *( *lines+(line_count-1) ) + line_len ) = '\0';

				line_start = position+1;
			}

			position++;
		}

	}

	return line_count;
}

/*add a contact to the contactlist if it doesn't already exsit*/
extern gboolean add_contact_to_list(const char *contact)
{
	char *content = NULL;
	char **contacts = NULL;
	size_t line_count = 0;
	gboolean contact_exists = FALSE;

	/*load raw contents of the contact file into memory*/
	if( load_file( NULL, "contactlist", &content ) )
	{
		/*separate the raw contents by line*/
		line_count = separate_lines( content, &contacts );
		free(content);

		/*check if the contact already exists in file*/
		for( unsigned int i = 0; i < line_count; i++ )
		{
			if( strcmp( *(contacts+i), contact ) == 0 )
			{
				contact_exists = TRUE;
				break;
			}
		}

		/*free the separated contents*/
		for( unsigned int i = 0; i < line_count; i++ )
		{
			free( *(contacts+i) );
		}
		free(contacts);
	}

	/*if contact doesn't exist yet (or if file doesn't exist), open file and write to it*/
	if( !contact_exists )
	{
		FILE *contactfile = NULL;
		contactfile = fopen( "contactlist", "a" );
		if(contactfile)
		{
			fprintf( contactfile, "%s\n", contact );
			fclose(contactfile);
			return TRUE;
		}
		else
		{
			fprintf( stderr, "Can't append contact to contactlist in the users .chatnut subdirectory at $HOME/.chatnut/[user]: %s\n", strerror(errno) );
			return FALSE;
		}
	}
	else
	{
		fprintf( stderr, "[Contacts] Contact exists!\n" );
		return FALSE;
	}
}

/*load a file with name name into *to, dir specifies a directory or NULL*/
extern gboolean load_file( const char *dir, const char *name, char **to )
{
	FILE *file = NULL;
	int length = 0;
	gboolean loaded = FALSE;

	/*switch to directory*/
	if( dir )
	{
		if( chdir(dir) != 0 )
		{
			fprintf( stderr, "Unable to switch into user's history directory in $HOME/.chatnut/[user]: %s\n", strerror(errno) );
			return FALSE;
		}
	}

	/*open file and read from it*/
	file = fopen( name, "r" );
	if(!file)
	{
		/*no error message since file may just not have been created yet*/
		*to = NULL;		//set the history storage string to NULL
		loaded = FALSE;
	}
	else
	{
		fseek( file, 0, SEEK_END );
		length = ftell(file);		//number of characters including the terminating '\n' - or 0 when empty
		rewind(file);
		if( length > 0 )
		{
			if( ( *to = calloc(length, sizeof(char)) ) )	//all characters (including '\n')
			{
				fread( *to, sizeof(char), length, file );
				*( *to+(length-1) ) = '\0';		//substitute a '\0' for the '\n'
				loaded = TRUE;
			}
			else
			{
				loaded = FALSE;
			}
		}
		else
		{
			*to = NULL;		//set the history storage string to NULL
			loaded = FALSE;
		}
	}

	/*switch back to parent directory*/
	if( dir )
	{
		chdir("../");
	}

	return loaded;
}

/*received should be TRUE when this is a received message, FALSE if user sent it*/
extern void append_to_history( const char *message, const char *buddyname, gboolean received )//rename buddyname to filename
{
    FILE *historyfile = NULL;
    const char *filename = buddyname;
    const char *path = "history";
    
    /*switch to directory*/
    if( chdir(path) != 0 )
    {
        fprintf( stderr, "Unable to switch into user's history directory in $HOME/.chatnut/[user]: %s\n", strerror(errno) );
        return;
    }

    /*open file and write to it*/
    historyfile = fopen( filename, "a" );		//NULL check if file not found (errno)
    if(historyfile)
    {
    	if( received == TRUE )
    	{
    		fprintf( historyfile, "%s: ", buddyname );
    	}
    	else
    	{
    		fprintf( historyfile, "%s: ", get_username() );
    	}
        fprintf( historyfile, "%s\n", message );
        fclose(historyfile);	//TODO log an error if there is one
    }
    else
    {
        fprintf( stderr, "Unable to open user's chat history for contact in $HOME/.chatnut/[user]/history: %s\n", strerror(errno) );
    }
    
    /*switch back to parent directory*/
    chdir("../");

    return;
}

extern int init_chatnut_directory(void)
{
    if( chdir(getenv("HOME")) != 0 )
    {
        fprintf( stderr, "Error switching into your home directory: %s\n", strerror(errno) );
        return 1;
    }
    if( mkdir( ".chatnut", 0755 ) != 0 )
    {
        if( errno != EEXIST )
        {
            fprintf( stderr, "Error creating directory .chatnut in your home directory: %s\n", strerror(errno) );
            return 1;
        }
    }
    if( chdir(".chatnut") != 0 )
    {
        fprintf( stderr, "Error switching to .chatnut in your home directory: %s\n", strerror(errno) );
        return 1;
    }
    return 0;
}

extern int init_user_directory(void)
{
    /*directories*/
    if( mkdir( get_username(), 0755 ) != 0 )
    {
        if( errno != EEXIST )
        {
            fprintf( stderr, "Error creating users directory in $HOME/.chatnut: %s\n", strerror(errno) );
            return 1;
        }
    }
    if( chdir(get_username()) != 0 )
    {
        fprintf( stderr, "Error switching into user's directory in $HOME/.chatnut: %s\n", strerror(errno) );
        return 1;
    }
    if( mkdir( "history", 0755 ) != 0 )
    {
        if( errno != EEXIST )
        {
            fprintf( stderr, "Error creating history directory for user in $HOME/.chatnut/[user]: %s\n", strerror(errno) );
            return 1;
        }
    }
    
    return 0;
}
