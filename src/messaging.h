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

#define HELP_WANTED 1		// /help
#define LIST 2				// /list
#define NAME_IS_SET 3		// /name
#define BUDDY_IS_SET 4		// /who
#define BUDDY_NOT_EXIST	5	// /who if buddy doesn't exist
#define NOARG 6			// valid command but no arguments (only used with commands that need arguments, not with /help or /list
#define ERROR 7				// invalid command
