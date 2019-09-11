/* Header file for server side, assignment 3
 *
 */

#ifndef prog3_server_h
#define prog3_server_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/types.h> 
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef
struct node_struct {
   struct node_struct* next;
   char* name;
   int par_sd;
   int obs_sd;
}Node;

typedef Node* list;

/* Format Message
 *
 * void
 *
 * fm    message to be formated
 * name  name of recipient
 * pm    indicates if it is a private message
 *
 * Formats the message to be sent to observer(s)
 *
 */
void format_msg(char* fm, char* name, int pm);


/* Send Broadcast
 *
 * void
 *
 * head     head of the linked list storing participants
 * curr     sending node
 * message  message to be sent 
 * type     0 for general msg, 1 for user leaving, 2 for user entering
 *
 * Sends a broadcast message to all active participants
 *
 *
 */
void send_broadcast(list head, list curr, char* message, int type);

/* Send Private Message
 *
 * void
 *
 * head     head of linked list 
 * curr     sending node
 * message  message to be sent
 *
 * Sends a private message to a user
 *
 *
 */
void send_pm(list head, list curr, char* message); 

/* Is Valid
 *
 * char  returns Y if valid name, I if invalid
 *
 * name  name to be checked
 *
 * Checks if the users name fulfills criteria: 10 or less letters, alphabetical,
 * numerical or an '_'
 *
 */
char is_val(char* name);

/* Append
 *
 * void
 *
 * head        node to append
 * sd          socket descriptor
 * observer    type of node to append
 *
 * Appends to linked list. If it is the first node in the list, the node will be
 * considered the head node of the list. If observer is set, will append to
 * observer socket descriptor instead of participant socket descriptor
 *
 */
void append(list* head, int sd, int observer);

/* Remove Node
 *
 * void
 *
 * head        head of linked list
 * sd          socket descriptor
 * observer    type of node to remove
 *
 * Removes node from linked list. If observer is set, removes observer socket
 * descriptor
 *
 */
void remove_node(list* head, int sd, int observer);

/* Find Name
 *
 * list
 *
 * head  head of linked list
 * name  name to be searched for
 *
 * Returns node containing the name
 *
 */
list find_name(list head, char* name);

/* Find Socket
 *
 * list
 *
 * head  head of linked list
 * sd    socket descriptor to be searched for
 *
 * Returns node contained socket descriptor given (participant)
 *
 */
list find_socket(list head, int sd); 

/* Free Linked List
 *
 * void
 *
 * n  list to be freed
 *
 * Frees all allocated memory for the linked list
 *
 */
void free_linked_list(list n);

/* Attach Name
 *
 * void
 *
 * head  head of linked list
 * name  name to be attached to node
 * sd    socket descriptor
 *
 * Attaches a name to a node 
 *
 */
void attach_name(list head, char* name, int sd);

/* Attach Observer
 *
 * int returns 0 if node containing name isn't found
 *
 * head     head of linked list
 * name     name to be found
 * obs_sd   observer socket descriptor
 *
 * Attaches an observer to a participant
 *
 */
int attach_observer(list head, char* name, int obs_sd);

/* Address Set Up
 *
 * void
 *
 * port  port number
 * sad   sockaddr structure
 *
 * Sets up ports
 *
 */
void addr_setup(int port, struct sockaddr_in* sad); 

/* Socket Set Up
 *
 * void
 *
 * sd       socket descriptor
 * ptrp     protocal
 * optval   boolean value
 * sad      sockaddr structure
 *
 * Sets up socket descriptors
 *
 */
void socket_setup(int sd, struct protoent* ptrp, int optval, struct sockaddr_in* sad);

/* Print List
 *
 * void
 *
 * n  list to be printed
 *
 * Prints linked list
 *
 * Used for debugging purposes
 *
 */
void printList(Node* n);
#endif
