/*file_operations.c*/

#include "file_operations.h"
#include <errno.h>

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
        store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_BOOLEAN );

        //get file length
        fseek( contactlist, 0, SEEK_END );
        raw_list_len = ftell(contactlist);
        rewind(contactlist);
        //set up data buffer
        raw_list = calloc( raw_list_len+1, sizeof(char) );	//one extra for '\0'
        //read from file
        fread( raw_list, sizeof(char), raw_list_len, contactlist );
        raw_list[raw_list_len] = 0;
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
            strncpy( name, raw_list+name_pos, nameLen );	//right after raw_list[nameLen] is a '\n', so strncpy wouldn't put a '\0'
            name[i] = '\0';		//adding terminator here cuz strncpy wouldn't do it in this case
            i++;//skip past '\n'

            //add the name to the contact list model
            gtk_list_store_append( store, &iter );
            gtk_list_store_set( store, &iter, 0, name, 1, FALSE, -1 );

            free(name);
        }

        free(raw_list);

        return store;
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
    history_file = fopen( filename, "r" );		//NULL check if file not found (errno)
    if( history_file != NULL )
    {
        fseek( history_file, 0, SEEK_END );
        //TODO check these comments, they seem confusing...
        file_len = ftell(history_file);		//all characters + one additional '\n'
        rewind(history_file);
        *to = calloc( file_len, sizeof(char) );	//all characters (including additional '\n') + '\0' (TODO check: the '\n' is part of all characters)
        fread( *to, sizeof(char), file_len, history_file );
        *( *to+(file_len) ) = '\0';
    }
    else
    {
        *to = NULL;		//set the history storage buffer to NULL
        fprintf( stderr, "Unable to open user's chat history for contact in $HOME/.chatnut/[user]/history: %s\n", strerror(errno) );
    }
    
    /*switch back to parent directory*/
    chdir("../");

    return;
}

extern void append_to_history( const char *message, const char *username )
{
    FILE *historyfile = NULL;
    const char *filename = username;
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