/*
*  File name: myftpd.c
*  Author: Fiko Febriyan 34348474
*          
*  Date: 28/7/2022
*
*  Description: This file is the implementation of the server.
*               It will listen to the request of the clients and 
*               respond to it.
*
*/

#include  <unistd.h>
#include  <sys/stat.h>
#include  <stdlib.h>     /* strlen(), strcmp() etc */
#include  <stdio.h>      /* printf()  */
#include  <string.h>     /* strlen(), strcmp() etc */
#include  <errno.h>      /* extern int errno, EINTR, perror() */
#include  <signal.h>     /* SIGCHLD, sigaction() */
#include  <syslog.h>
#include  <sys/types.h>  /* pid_t, u_long, u_short */
#include  <sys/socket.h> /* struct sockaddr, socket(), etc */
#include  <sys/wait.h>   /* waitpid(), WNOHAND */
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include  <arpa/inet.h>
#include  <netdb.h>
                        /* and INADDR_ANY */
#include  "stream.h"     /* MAX_BLOCK_SIZE, readn(), writen() */



#define   SERV_TCP_PORT   40233   /* our server listening port */


void claim_children()
{
     pid_t pid=1;
     
     // claim zombies as many as possible
     while (pid>0) { 
         pid = waitpid(0, (int *)0, WNOHANG); 
     } 
}
void daemonFunction(void)
{       
     pid_t   pid;
     struct sigaction act;

     if ( (pid = fork()) < 0) {
          perror("fork"); exit(1); 
     } else if (pid > 0) {
          printf("This is my PID: %d\n", pid);
          exit(0);                  
     }

     // child continues 
     setsid();                      // become session leader 
     chdir("/");                    // change working directory 
     umask(0);                      // clear file mode creation mask 

     /* catch SIGCHLD to remove zombies from system */
     act.sa_handler = claim_children; // use reliable signal 
     sigemptyset(&act.sa_mask);       // not to block other signals 
     act.sa_flags   = SA_NOCLDSTOP;   // not catch stopped children 
     sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);

}

int main(int argc, char *argv[])
{
     int sd, nsd, n;  
     pid_t pid;
     unsigned short porter;   
     socklen_t cli_addrlen;  
     struct sockaddr_in serAddr, cliAddr; 
     
    /*
    * = 1,  using localhost
    * = 2,  using given hostname
    */
     if (argc == 1) {
          porter = SERV_TCP_PORT;
          
     } else if (argc == 2) {
          int n = atoi(argv[1]);   
          if (n >= 1024 && n < 65536) 
              porter = n;
          else {
              printf("Error: port number must be between 1024 and 65535\n");
              exit(1);
          }
     } else {
          printf("Usage: %s [ server listening port ]\n", argv[0]);     
          exit(1);
     }

     // make the program into a daemon 
     daemonFunction(); 
     

     /*
     * create listening socket sd 
     * if error exit(1)
     * else print output
     */ 
     if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
           perror("\n - server:socket error "); exit(1);
     } 
     printf("\n + server socket created\n");


     // create server internet socket 
     bzero((char*)&serAddr, sizeof(serAddr));
     serAddr.sin_family = AF_INET;
     serAddr.sin_port = htons(porter);
     serAddr.sin_addr.s_addr = htonl(INADDR_ANY);


     /*
     * bind server address with socket sd
     * if error exit(1)
     * else print output
     */ 
     if (bind(sd, (struct sockaddr *) &serAddr,sizeof(serAddr))<0){
           perror("server bind\n"); exit(1);
     }
     printf(" + Bind to port: %d\n", htons(porter));
     printf(" + Bind address: %d\n", htonl(INADDR_ANY) );


     // listening socket
     if ( listen(sd, 5) == 0 ){
        printf(" + Listening .. \n");
     } else {
        printf(" - Error listening \n");
     }
     

     while (1) {

         /*
         * waiting client for requesting connection
         * accept() if success return non-negative number
         * Otherwise, return -1 
         */
          cli_addrlen = sizeof(cliAddr);
          nsd = accept(sd, (struct sockaddr *) &cliAddr, &cli_addrlen);
          if (nsd < 0) {
              //if interrupted by SIGCHLD
               if (errno == EINTR){ 
                    continue;
               }
               perror("server:accept"); 
               exit(1);
          }

          //print address and port
          printf(" Connection accepted from %d:%d\n", htonl(INADDR_ANY), htons(porter) );

          //Handle this client by creating a child process
          if ((pid=fork()) <0) {
              perror("fork"); 
              exit(1);
              
          } else if (pid > 0) { 
              close(nsd);
              // parent wait for next client
              continue; 
          }
          close(sd); 
          
          run(nsd);
          exit(0);
     }
     
}

