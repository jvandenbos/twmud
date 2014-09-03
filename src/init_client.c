 /*
  * init_client(struct client_info *) initializes its client argument to
  * suitable default values. All members are initialized to zeros, except
  * for: hostname and address (which become "unknown"), and the remote
  * username (which becomes a zero-length string).
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */


/* Local stuff. */

#include "log_tcp.h"

/* init_client - initialize client info to suitable default values */

void    init_client(client)
struct client_info *client;
{
    static struct client_info default_info = {
	FROM_UNKNOWN,
	FROM_UNKNOWN,
	"",
    };

    *client = default_info;
}
