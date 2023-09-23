/*
*  File name: myftp.c
*  Author: Fiko Febriyan 
*          
*  Date: 28/7/2022
*
*  Description: This file is the implementation of the client. 
*               It will try to connect to server at first,
*               and able to give command after its connected with server.
*
*/

#include  <stdlib.h>
#include  <stdio.h>
#include  <sys/types.h>        
#include  <sys/socket.h>
#include  <netinet/in.h>       /* struct sockaddr_in, htons, htonl */
#include  <netdb.h>            /* struct hostent, gethostbyname() */ 
#include  <string.h>
#include  "stream.h"           /* MAX_BLOCK_SIZE, readn(), writen() */

#define   SERV_TCP_PORT  40233 /* our server listening port */


int main(int argc, char *argv[])
{
    int sd, n, nr, nw, i = 0;
    char hoster[60];
    unsigned short porter;
    struct sockaddr_in address; // structure for handling internet addresses
    struct hostent *hp;


    /*
    * = 1,  using localhost
    * = 2,  using given hostname
    * = 3,  use given host and port for server and port 
    *       must be between 1024 and 65536
    */
     if (argc==1) {  
          strcpy(hoster, "localhost");
          porter = SERV_TCP_PORT;
     } else if (argc == 2) {  
          strcpy(hoster, argv[1]);
          porter = SERV_TCP_PORT;
     } else if (argc == 3) { 
         strcpy(hoster, argv[1]);
          int n = atoi(argv[2]);
          if (n >= 1024 && n < 65536) 
              porter = n;
          else {
              printf("Error: server port number must be between 1024 and 65535\n");
              exit(1);
          }
     } else { 
         printf("Usage: %s [ <server host name> [ <server listening port> ] ]\n", argv[0]); 
         exit(1); 
     }

     // create client internet socket 
     bzero((char *) &address, sizeof(address));
     address.sin_family = AF_INET;
     address.sin_port = htons(porter);
     
     if ((hp = gethostbyname(hoster)) == NULL){
           printf(" - host %s not found\n", hoster); 
           exit(1);   
     }
     printf(" + Host name: %s found\n", hoster);
     
     address.sin_addr.s_addr = * (u_long *) hp->h_addr;

     //create TCP socket 
     sd = socket(PF_INET, SOCK_STREAM, 0);
     
     if ( sd  == -1 ){
        printf(" - Cannot open stream socket\n");
     } else {
         printf(" + Socket created\n");
     }
     
     //connect socket to server address 
     if (connect(sd, (struct sockaddr *) &address, sizeof(address))<0) { 
          perror(" - client cannot connect\n");exit(1);
     }
     printf(" + Client connected\n");
     
     run(sd);

     exit(0);
}
