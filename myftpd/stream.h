 /*
 *  File name: stream.c
 *  Author: Fiko Febriyan 34348474
 *          
 *  Date: 28/7/2022
 *
 *  Description: This header file is used for declaring the function
 *				 in stream.c. The main idea of this stream file is to
 *               create a socket for server to communicate with client.
 *
 */

#define BUF_SIZE 512
#define BLOCKMAXSIZE (1024*5)    /* maximum size of any piece of */
                                   /* data that can be sent by client */

/*
* Function name: run
* Return: void
* Parameter: socketID(int)
*
* Description:
*   The run function is to communicate with the client 
*   by used opcode
*/
void run(int socketID);             
                      

