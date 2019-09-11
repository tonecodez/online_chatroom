
/* Assignment 3
 * Computer Networks, Winter 2019
 * Tanzima Islam
 *
 * Tony Dinh, Anders Bergquist
 *
 */

#include "prog3_participant.h"

/*------------------------------------------------------------------------
* Program: prog3_participant
*
* Purpose: Allows for client to send messages to chatroom 
*
* Syntax: ./prog3_participant server_address server_port
*
* server_address - name of a computer on which server is executing
* server_port    - protocol port number server is using
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0 && port > 1023 && port < 65535) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	if (connect(sd, (struct sockaddr *) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

   int ready_reading;
   fd_set input_set;
   struct timeval timeout;
   FD_ZERO(&input_set);
   FD_SET(0, &input_set);
   uint8_t word_len;
   uint8_t n;
   char name[255];
   char val_name;
   int prompt = 1; 
   int to_reset = 1;
   
   char message[1000];
   uint16_t mes_len;
      
   char successful_connect;
   int sc = 0;
   recv(sd, &successful_connect, sizeof(char), 0);

   if(successful_connect == 'Y') {
      sc = 1;
   }

   else {
      printf("Server is full!\n");
   }
   
   while(sc) {
       
      memset(message, 0, 1000);
      mes_len = 0;
       
      while(prompt) {
         
         ready_reading = 0;
         n = 0;
         memset(name, 0, 255);

         if(to_reset) {
            reset_timer(&timeout);
         }

         printf("Enter your name (1-10 Characters): ");
         fflush(stdout);
      
         ready_reading = select(1, &input_set, NULL, NULL, &timeout);

         if(ready_reading == -1) {
            perror("select");
            return -1;
         }
            
		   if(ready_reading) {
            n = read(0, name, 256);
         }
            
         word_len = n - 1;

         if(n < 1) {           
            printf("Timed out!\n");
            exit(EXIT_SUCCESS);
         }

         else if(word_len > 10) {
            printf("Word too long, try again.\n");
         }

         else if(word_len < 1) {
            printf("Word too short, try again.\n");
         }

         else {
            send(sd, &word_len, sizeof(uint8_t), 0);
            send(sd, name, word_len, 0);
            
            recv(sd, &val_name, sizeof(char), 0);

            if(val_name == 'Y') {
               prompt = 0;
            }

            if(val_name == 'I') {
               to_reset = 0;
               printf("Invalid name, try again.\n");
            }

            if(val_name == 'T') {
               to_reset = 1;
               printf("Name taken, try again\n");
            }


         }
      }
      
      FD_ZERO(&input_set);
      FD_SET(0, &input_set);
      
      printf("Enter message: ");
      fflush(stdout);
      
      ready_reading = select(1, &input_set, NULL, NULL, NULL);
      
	   if(ready_reading == -1) {
         perror("select");
         return -1;
      }
            
		if(ready_reading) {
         n = read(0, message, 1000);
      }
            
      mes_len = n - 1;
		 
      send(sd, &mes_len, sizeof(uint16_t), 0);
      send(sd, &message, mes_len, 0);
      
   }
   
   close(sd);
   exit(EXIT_SUCCESS);
}

/* Reset Timer
 *
 * void
 *
 * timeout  struct timeval
 *
 * resets timer to 60
 *
 */
void reset_timer(struct timeval* timeout) {
   timeout->tv_sec = 60;
   timeout->tv_usec = 0;
}

