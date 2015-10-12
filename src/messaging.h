/*commands.h*/

/*  used by eval_cmd to tell cmd_reply what command was used	*
 *  and by the client to tell if name and buddies are set	    */

/* commands are:
 * /help
 * /list
 * /name
 * /who
 */
#include <stdlib.h>
#define CONN_MSG_LEN 10
const char conn_msg[CONN_MSG_LEN] = "Connected";

#define MSG_LEN 2000
#define CMD_LEN 120
#define NAMELEN 21			//last byte is null-pointer, so effective namelength is 20 bytes/letters

enum commands
{
    HELP_WANTED = 1,		// /help
    LIST,			// /list
    NAME_IS_SET,		// /name
    BUDDY_IS_SET,		// /who
    BUDDY_NOT_EXIST,            // /who if buddy doesn't exist
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
    REGISTRATION_SUCCESS,
    REGISTRATION_FAILURE,
    NOARG,			// valid command but no arguments (only used with commands that need arguments, not with /help or /list
    ERROR				// invalid command
};
