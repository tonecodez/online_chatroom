/* Assignment 3
 * Computer Networks, Winter 2019
 * Tanzima Islam
 *
 * Tony Dinh, Anders Bergquist
 *
 */

#include "prog3_server.h"
#include <errno.h>

#define QLEN 6 /* size of request queue */
#define MAX_LOBBY 255

/*
* Program: prog3_server
* 
* Purpose: Connects participants and observers and is responsible for formatting
* and sending messages to all observers
*
* Syntax: ./prog3_server participant_port observer_port
*
* participant_port - protocol port number for participant
* 
* observer_port - protocal port number for observer
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	
   struct sockaddr_in psad; /* structure to hold server's address */
	struct sockaddr_in osad; /* structure to hold server's address */
	
   struct sockaddr_in cad; /* structure to hold client's address */
	
   int psd, osd; /* socket descriptors */
	int p_port, o_port; /* protocol port numbers */
   
   socklen_t alen; /* length of address */
   int optval = 1; /* boolean value when we set socket option */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server participant_port observer_port\n");
		exit(EXIT_FAILURE);
	}

	p_port = atoi(argv[1]); /* convert argument to binary */
   o_port = atoi(argv[2]); 

   addr_setup(p_port, &psad);
   addr_setup(o_port, &osad);

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number\n");
		exit(EXIT_FAILURE);
	}

   psd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
   socket_setup(psd, ptrp, optval, &psad);
   
   osd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	socket_setup(osd, ptrp, optval, &osad);

   char name[255];
   uint8_t word_len;
   char user_check;

   char message[1000];
   uint16_t mes_len;

   list head = NULL;
   list observer_head = NULL;
   Node* curr;
   Node* obs_curr;
   int curr_sd;
   int search_pp, search_ob;
   
   fd_set readfds;
   int bytes_recv;
   int status;

   int maxsd = osd;
   
   Node* affiliate;
   char* successful_con = "A new observer has joined.";
   uint16_t sc_size = strlen(successful_con);

   int num_participants = 0;
   int num_observers = 0;
   char sc = 'Y';
   char fs = 'N';
	
   while (1) {
	   
      FD_ZERO(&readfds);
      FD_SET(psd, &readfds);
      FD_SET(osd, &readfds);

      search_pp = 1;
      search_ob = 1;
      bytes_recv = 0;
      curr = head;
      obs_curr = observer_head;

      while(curr != NULL) {
         
         FD_SET(curr->par_sd, &readfds);
         
         if(curr->obs_sd != 0) {
            FD_SET(curr->obs_sd, &readfds);
         }

         if(maxsd < curr->par_sd) {
            maxsd = curr->par_sd;
         }
         curr = curr->next;
      }
      
      while(obs_curr != NULL) {
         
         FD_SET(obs_curr->obs_sd, &readfds);
         
         if(maxsd < obs_curr->obs_sd) {
            maxsd = obs_curr->obs_sd;
         }
         obs_curr = obs_curr->next;
      }

      status = select(maxsd + 1, &readfds, NULL, NULL, NULL);
      if(status < 0) {
         fprintf(stderr, "select() error\n");
         exit(EXIT_FAILURE);
      }

      else if(FD_ISSET(psd, &readfds)) {
         
         
         alen = sizeof(cad);

		   if ( (curr_sd = accept(psd, (struct sockaddr *)&cad, &alen)) < 0) {
			   fprintf(stderr, "Error: Accept failed\n");
			   exit(EXIT_FAILURE);
		   }
        
         if(num_participants + 1 <= MAX_LOBBY) {
            append(&head, curr_sd, 0);
            num_participants++;
            send(curr_sd, &sc, sizeof(char), 0);
         }

         else {
            send(curr_sd, &fs, sizeof(char), 0);
            close(curr_sd);
         }

      }

      else if(FD_ISSET(osd, &readfds)) {
         
         alen = sizeof(cad);
		   if ( (curr_sd = accept(osd, (struct sockaddr *)&cad, &alen)) < 0) {
			   fprintf(stderr, "Error: Accept failed\n");
			   exit(EXIT_FAILURE);
		   }

         if(num_observers + 1 <= MAX_LOBBY) {
            append(&observer_head, curr_sd, 1);
            num_observers++;
            send(curr_sd, &sc, sizeof(char), 0);
         }

         else {
            send(curr_sd, &fs, sizeof(char), 0);
            close(curr_sd);
         }

      }

      else {
         
         curr = head;
         
         while(curr != NULL && search_pp) {
            
            if(FD_ISSET(curr->par_sd, &readfds)) {

               search_ob = 0;
               
               if(curr->name == NULL) {
                
                  bytes_recv = recv(curr->par_sd, &word_len, sizeof(uint8_t), 0);

                  if(bytes_recv == 0) {

                     close(curr->par_sd);
                     remove_node(&head, curr->par_sd, 0);
                     num_participants--;
                  }
                  
                  else {
                     recv(curr->par_sd, &name, word_len, 0);
                  
                     name[word_len] = '\0';
                  
                     if(find_name(head, name) == NULL) {
                        user_check = is_val(name);
                        send(curr->par_sd, &user_check, sizeof(char), 0);
                     
                        if(user_check == 'Y') {
                           curr->name = strdup(name);
                           send_broadcast(head, curr, NULL, 2);
                        }
                     }

                     else {
                        user_check = 'T';
                        send(curr->par_sd, &user_check, sizeof(char), 0); 
                     }
                  }
               }

               else {
                  
                  bytes_recv = recv(curr->par_sd, &mes_len, sizeof(uint16_t), 0);
                  
                  if(bytes_recv == 0) {
                     close(curr->par_sd);
                     
                     if(curr->obs_sd != 0) { 
                        close(curr->obs_sd);
                        send_broadcast(head, curr, message, 1);
                        num_observers--;
                     }

                     remove_node(&head, curr->par_sd, 0);
                     num_participants--;
                     
                  }        

                  else {
                     recv(curr->par_sd, &message, mes_len, 0);
                  
                     if(mes_len > 1000) {
                        close(curr->par_sd);
                        remove_node(&head, curr->par_sd, 0);
                        num_participants--;
                        num_observers--;
                     }

                     else {

                        message[mes_len] = '\0';
                  
                        if(message[0] == '@'){
					            send_pm(head, curr, message);
                        }
                  
                        else {
                           send_broadcast(head, curr, message, 0);
                        }
                     }
                  }
               }
               search_pp = 0;
            }

            if(search_pp != 0) {
               curr = curr->next;
            }
         }
         
         if(observer_head != NULL && search_ob) {
            obs_curr = observer_head;
         }
         
         while(obs_curr != NULL && search_ob) {
            
            if(FD_ISSET(obs_curr->obs_sd, &readfds)) {
               
               bytes_recv = recv(obs_curr->obs_sd, &word_len, sizeof(uint8_t), 0);

               if(bytes_recv == 0) {
                  close(obs_curr->obs_sd);
                  remove_node(&observer_head, obs_curr->obs_sd, 1);
                  num_observers--;
               }
                  
               else {
                  recv(obs_curr->obs_sd, &name, word_len, 0);
                  
                  name[word_len] = '\0';

                  if((affiliate = find_name(head, name)) != NULL) {
                     
                     if(affiliate->obs_sd == 0) {
                     
                        user_check = 'Y';
                     
                        send(obs_curr->obs_sd, &user_check, sizeof(char), 0);
                     
                        affiliate->obs_sd = obs_curr->obs_sd;
                        remove_node(&observer_head, obs_curr->obs_sd, 1);
                        
                        curr = head;

                        while(curr != NULL) {
                           if(curr->obs_sd != 0) {
                              send(curr->obs_sd, &sc_size, sizeof(uint16_t), 0);
                              send(curr->obs_sd, successful_con, sc_size, 0);
                           }
                           curr = curr->next;
                        }

                     }

                     else {
                        user_check = 'T';
                        send(obs_curr->obs_sd, &user_check, sizeof(char), 0);
                     }
                  }

                  else { 
                     user_check = 'N';
                     send(obs_curr->obs_sd, &user_check, sizeof(char), 0);
                     close(obs_curr->obs_sd);
                    
                     remove_node(&observer_head, obs_curr->obs_sd, 1);
                     num_observers--;
                  }
                  
               }
               search_ob = 0;
            }
            
            if(search_ob != 0) {
               obs_curr = obs_curr->next;
            }
         }

         curr = head;
         while(curr != NULL && search_ob) {
            
            if(FD_ISSET(curr->obs_sd, &readfds)) { 
               close(curr->obs_sd);
               curr->obs_sd = 0;
               search_ob = 0;
               num_observers--;
            }

            if(search_ob != 0) {
               curr = curr->next;
            }
         }
      }
   }
}

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
void format_msg(char* fm, char* name, int pm) {
   
   int ws_len = 11 - strlen(name);

   int i = 0;

   if(pm) {
      fm[i++] = '-';
   }

   else {
      fm[i++] = '>';
   }

   while(i < ws_len) {
      fm[i++] = ' ';
   }

   int j = 0;
   while(j < strlen(name)) {
      fm[i++] = name[j++];
   }

   fm[i++] = ':';
   fm[i++] = ' ';
   fm[i] = '\0';

}

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
void send_broadcast(list head, list curr, char* message, int type) {

   if(type == 1) {
      char* p1 = "User ";
      char* p2 = " has left";
      char* final_msg = malloc(strlen(p1) + strlen(curr->name) + strlen(p2) + 1);
   
      strcpy(final_msg, p1);
      strcat(final_msg, curr->name);
      strcat(final_msg, p2);
      uint16_t mes_len = strlen(final_msg);

      curr = head;
      while(curr != NULL) {
         if(curr->obs_sd != 0) {
            send(curr->obs_sd, &mes_len, sizeof(uint16_t), 0);
            send(curr->obs_sd, final_msg, mes_len, 0);
         }
         curr = curr->next;
      }
   }

   else if(type == 2) {
      char* p1 = "User ";
      char* p2 = " has joined";
      char* final_msg = malloc(strlen(p1) + strlen(curr->name) + strlen(p2) + 1);
   
      strcpy(final_msg, p1);
      strcat(final_msg, curr->name);
      strcat(final_msg, p2);
      uint16_t mes_len = strlen(final_msg);

      curr = head;
      while(curr != NULL) {
         if(curr->obs_sd != 0) {
            send(curr->obs_sd, &mes_len, sizeof(uint16_t), 0);
            send(curr->obs_sd, final_msg, mes_len, 0);
         }
         curr = curr->next;
      }
   }

   else {
      char fm[15];
      format_msg(fm, curr->name, 0);
      char* final_msg = malloc(strlen(fm) + strlen(message) + 1);
   
      strcpy(final_msg, fm);
      strcat(final_msg, message);
      uint16_t mes_len = strlen(final_msg);

      curr = head;
      while(curr != NULL) {
         if(curr->obs_sd != 0) {
            send(curr->obs_sd, &mes_len, sizeof(uint16_t), 0);
            send(curr->obs_sd, final_msg, mes_len, 0);
         }
         curr = curr->next;
      }
   }
}

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
void send_pm(list head, list curr, char* message) {
	
	int i = 1;
	char* name = strdup(&message[i]);
	while(!isspace(message[i]) && message[i] != '\0') {
      i++;
	}

	name[i - 1] = '\0';
		
	i++;

   if(strlen(name) == strlen(message) - 1) {
      message = " ";
   }

   else {
	   message = strdup(&message[i]);
      message[strlen(message)] = '\0';  
   }
   char* err_p1 = "Warning: user ";
   char* err_p2 = " does not exist...";
   
   char* err_msg = malloc(strlen(err_p1) + strlen(name) + strlen(err_p2) + 1);
   
   if(err_msg == NULL) {
      fprintf(stderr, "malloc");
      exit(EXIT_FAILURE);
   }

   strcpy(err_msg, err_p1);
   strcat(err_msg, name);
   strcat(err_msg, err_p2);

   Node* n = find_name(head, name);
   
   char fm[15];

   format_msg(fm, curr->name, 1); 
   char* final_msg = malloc(strlen(fm) + strlen(message) + 1);
   
   strcpy(final_msg, fm);
   strcat(final_msg, message);

   uint16_t sz_err_msg = strlen(err_msg);
   uint16_t sz_msg = strlen(final_msg);

   if(n == NULL) {
      send(curr->obs_sd, &sz_err_msg, sizeof(uint16_t), 0);
      send(curr->obs_sd, err_msg, sz_err_msg, 0);  
   }
   
   else {

      if(n->obs_sd == 0) {
         send(curr->obs_sd, &sz_err_msg, sizeof(uint16_t), 0);
         send(curr->obs_sd, err_msg, sz_err_msg, 0); 
      }

      else {
         send(curr->obs_sd, &sz_msg, sizeof(uint16_t), 0);
         send(curr->obs_sd, final_msg, sz_msg, 0);

         send(n->obs_sd, &sz_msg, sizeof(uint16_t), 0);
         send(n->obs_sd, final_msg, sz_msg, 0);
	   }
   }

   free(err_msg);
}

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
char is_val(char* name) {

   char val = 'Y';
   if(strlen(name) > 10) {
      val = 'I';
   }
   
   else {

      int i = 0;
      while(i < strlen(name) && val != 'I') {
         
         if( !isalpha(name[i]) && !isdigit(name[i]) && name[i] != '_') {
            val = 'I';
         }     
         i++;
      }
   }
   return val;
}

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
void append(list* head, int sd, int observer) {

    Node* newNode = (Node*) malloc(sizeof(Node));     
    Node* lastNode = *head;
      
    if(observer) {
      newNode->obs_sd = sd;
      newNode->par_sd = 0;
    }

    else {
      newNode->par_sd = sd;
      newNode->obs_sd = 0;
    }
    newNode->name = NULL;
    newNode->next = NULL;     
    
    if(*head == NULL) {         
       *head = newNode;
    }
    
    else {         
       while(lastNode->next != NULL) {             
          lastNode = lastNode->next;  
       }
       lastNode->next = newNode;     
    }
}

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
void remove_node(list* head, int sd, int observer) {

   Node* temp = *head, *prev;
   
   if(observer) {
      if(temp != NULL && temp->obs_sd == sd) {
         *head = temp->next;
         free(temp);
         return;
      }

      while(temp != NULL && temp->obs_sd != sd) {
         prev = temp;
         temp = temp->next;
      }
   }

   else {
      if(temp != NULL && temp->par_sd == sd) {
         *head = temp->next;
         free(temp);
         return;
      }

      while(temp != NULL && temp->par_sd != sd) {
         prev = temp;
         temp = temp->next;
      }
   }


   if(temp == NULL) {
      return;
   }

   prev->next = temp->next;

   free(temp);
}

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
list find_name(list head, char* name) {
    
   Node* curr = head;
   int found = 0;
   while(curr != NULL && !found) {
      
      if(curr->name != NULL) {
         if(strcmp(curr->name, name) == 0) {
            found = 1;
         }
      }
      
      if(found == 0) {
         curr = curr->next;
      }
   }

  return curr;
}

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
list find_socket(list head, int sd) {

   Node* curr = head;

   while(curr != NULL && curr->par_sd != sd) {
      curr = curr->next;
   }

   return curr;

}

/* Free Linked List
 *
 * void
 *
 * n  list to be freed
 *
 * Frees all allocated memory for the linked list
 *
 */
void free_linked_list(list n) {
      
    int count = 0;

    while(n != NULL) {
        count++;
        n = n->next;
    }
}

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
void attach_name(list head, char* name, int sd) {
   
   Node* curr = head;
   
   while(curr != NULL && curr->par_sd != sd) {
      curr = curr->next;
   }

   curr->name = name;
}

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
int attach_observer(list head, char* name, int obs_sd) {

   int found = 0;
   Node* curr;
   if( (curr = find_name(head, name)) != NULL) {
      curr->obs_sd = obs_sd;
      found = 1;
   }

   return found;

}

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
void addr_setup(int port, struct sockaddr_in* sad) {

	memset((char *)sad,0,sizeof(*sad)); /* clear sockaddr structure */
        
   sad->sin_family = AF_INET;
        
	sad->sin_addr.s_addr = INADDR_ANY;
   
   if (port > 1023 && port < 65535) { /* test for illegal value */
                sad->sin_port = htons(port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %d\n", port);
		exit(EXIT_FAILURE);
	}

}

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
void socket_setup(int sd, struct protoent* ptrp, int optval, struct sockaddr_in* sad) {
  
   
   if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	if (bind(sd, (struct sockaddr *) sad, sizeof(*sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
      printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

}

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
void printList(Node *n) {
   while (n != NULL) {      
      printf("name: %s, psd : %d osd: %d\n", n->name, n->par_sd, n->obs_sd);
      n = n->next;
      printf("\n");
   }
}
