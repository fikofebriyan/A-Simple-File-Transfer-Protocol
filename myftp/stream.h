/*
*  File name: stream.h
*  Author: Fiko Febriyan 
*  Date: 28/7/2022
*
*  Description: This header file is used for declaring the function
*				in stream.c
*/
#define maxBuffSize 512
#define tokenStr "\n"

/*
* Function name: run
* Return: void
* Parameter: socketID(int)
*
* Description:
*   The run function is to communicate with the server
*   by used opcode
*/
void run(int socketID);
