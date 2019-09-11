/* Header file for observer side, assignment 3
 *
 */

#ifndef prog3_observer_h
#define prog3_observer_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h> 
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Reset Timer
 *
 * void
 *
 * timeout  struct timeval
 *
 * resets timer to 60
 *
 */
void reset_timer(struct timeval* timeout);

#endif
