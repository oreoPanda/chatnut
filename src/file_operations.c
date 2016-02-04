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

extern GtkListStore *create_contact_list_model(void)
{
    GtkListStore *store = NULL;
    GtkTreeIter iter;

    FILE *contactlist = NULL;
    char *filename = "contactlist";
    int raw_list_len;
    char *raw_list = NULL;
    char *name = NULL;

    /*open contact list file and load its contents into memory*/
    contactlist = fopen( filename, "r" );
    if( contactlist == NULL )
    {
        fprintf( stdout, "Warning, no contactlist found for user." );
        return store;
    }
    else
    {
        /*create contact list model with name and online status for each contact*/
        store = gtk_list_store_new( 1, G_TYPE_STRING );

        //get file length
        fseek( contactlist, 0, SEEK_END );
        raw_list_len = ftell(contactlist);
        rewind(contactlist);
        //set up data buffer
        raw_list = calloc( raw_list_len+1, sizeof(char) );	//one extra for '\0'
        //read from file
        fread( raw_list, sizeof(char), raw_list_len, contactlist );
        raw_list[raw_list_len] = 0;
        
        //close file
        fclose(contactlist);

        //crunch data
        int i = 0;
        while( raw_list[i] != 0 )
        {
            //get one name at a time
            int name_pos = i;
            while( raw_list[i] != '\n' )
            {
                i++;
            }
            int nameLen = i-name_pos;
            name = calloc( nameLen+1, sizeof(char) );	//name + '\0'
            strncpy( name, raw_list+name_pos, nameLen );//right after raw_list[nameLen] is a '\n', so strncpy wouldn't put a '\0'
            name[i] = '\0';		//adding terminator here cuz strncpy wouldn't do it in this case
            i++;//skip past '\n'

            //add the name to the contact list model
            gtk_list_store_append( store, &iter );
            gtk_list_store_set( store, &iter, 0, name, -1 );

            free(name);
        }

        free(raw_list);

        return store;
    }
}

extern void add_contact_to_list(const char *contact)
{
    FILE *contactfile = NULL;
    char *filename = "contactlist";	//is this on heap or stack? no malloc(), so no free() or?

    /*open file and write to it*/
    contactfile = fopen( filename, "a" );		//NULL check if file not found (errno)
    if(contactfile)
    {
        fprintf( contactfile, "%s\n", contact );
        fclose(contactfile);
    }
    else
    {
        fprintf( stderr, "Can't append contact to contactlist in the users chatnut directory at $HOME/.chatnut/[user]: %s\n", strerror(errno) );
    }
}

//stores history of contact in to
extern void load_history( const char *contact, char **to )
{
    FILE *history_file = NULL;
    int file_len = 0;
    const char *filename = contact;
    const char *path = "history";

    /*switch to directory*/
    if( chdir(path) != 0 )
    {
        fprintf( stderr, "Unable to switch into user's history directory in $HOME/.chatnut/[user]: %s\n", strerror(errno) );
        return;
    }

    /*open file and read from it*/
    history_file = fopen( filename, "r" );
    if( history_file != NULL )
    {
        fseek( history_file, 0, SEEK_END );
        file_len = ftell(history_file);		//number of characters including the terminating '\n' - or 0 when empty
        rewind(history_file);
        if( file_len > 0 )
        {
			*to = calloc( file_len, sizeof(char) );	//all characters (including '\n')
			fread( *to, sizeof(char), file_len, history_file );
			*( *to+(file_len-1) ) = '\0';		//substitute a '\0' for the '\n'
        }
        else
        {
        	*to = NULL;		//set the history storage string to NULL
        }
    }
    else
    {
        *to = NULL;		//set the history storage string to NULL
        fprintf( stderr, "Unable to open user's chat history for contact in $HOME/.chatnut/[user]/history: %s\n", strerror(errno) );
    }
    
    /*switch back to parent directory*/
    chdir("../");

    return;
}

/*received should be TRUE when this is a received message, FALSE if user sent it*/
extern void append_to_history( const char *message, const char *buddyname, gboolean received )
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
