 /*
 *  File name: stream.c
 *  Author: Fiko Febriyan 34348474
 *          
 *  Date: 28/7/2022
 *
 *  Description: This file is used for implement the function which declared in 
 *               stream.h.
 * 
 */
#include "stream.h"
#include <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>  // errno
#include <stdlib.h> // free()
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>


void run(int socketID) {
  int bytesRead, check, numFiles, i, openFile, bytesRemain;
  int twoBytesMsg;
  long fourBytesMsg;
  char command, tempChar, input[BLOCKMAXSIZE], file[BLOCKMAXSIZE], str[BUF_SIZE];
  struct dirent **listOfFile;
  struct stat fileDetails;

  while (1) {
    // Read first opcode
    bytesRead = read(socketID, &command, sizeof(char));

    // quit loop if connection got issue
    if (bytesRead <= 0) {
      break;
    }

    // cd
    if (command == 'A') 
    {
      /*
      * set tempchar as opcode A
      * read in directory name length
      * convert back to host byte order
      * read in directory name
      */ 
      tempChar = 'A';
      bytesRead = read(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
      twoBytesMsg = ntohs(twoBytesMsg);
      bytesRead = read(socketID, input, (sizeof(char) * twoBytesMsg));

      /* 
      *  try to change the directory, 
      *  return 0 = success, -1 = fails
      *  if fail check the reason
      */
      check = chdir(input);
      if (check == -1) 
      {
          // Dir does not exist
          if (errno == ENOENT) 
          {
              tempChar = '1';
          }

          // Access denied
          else if (errno == EACCES) 
          {
              tempChar = '2';
          }

          // Other errors
          else 
          {
              tempChar = '3';
          }
      }
      else {
          tempChar = '0';
      }

      /*
      * send opcode back to client
      * send back status of directory change
      */
      write(socketID, &tempChar, sizeof(char));
      write(socketID, &tempChar, sizeof(tempChar));
    } 
    // dir
    else if (command == 'B') // dir
    {
      // set tempchar as opcode A and send back to client
      tempChar = 'B';
      write(socketID, &tempChar, sizeof(char));

      // Scan directory for list of files
      numFiles = scandir(".", &listOfFile, NULL, alphasort);
      if (numFiles == 0) { perror("myftpd:scandir");}

      // Send number of files and folders
      numFiles = htons(numFiles);
      write(socketID, &numFiles, sizeof(numFiles));
      
      /*
      * Loop through the file
      * Identify their file type
      */
      numFiles = ntohs(numFiles);
      for (i = 0; i < numFiles; ++i) {
        if (listOfFile[i]->d_type == DT_REG) {
          tempChar = 'r';
        } else if (listOfFile[i]->d_type == DT_DIR) {
          tempChar = 'd';
        } else {
          tempChar = 'o';
        }
       
       // Get filename length
       twoBytesMsg = strlen(listOfFile[i]->d_name) + 1;
       twoBytesMsg = htons(twoBytesMsg);

       // Send file type, filename length, and filename
       write(socketID, &tempChar, sizeof(char));
       write(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
       write(socketID, listOfFile[i]->d_name, ntohs(twoBytesMsg));
       free(listOfFile[i]);
      }
      free(listOfFile);
    } 
    // get
    else if (command == 'C')
    {
      //set tempchar as opcode C and send
      tempChar = 'C';
      write(socketID, &tempChar, sizeof(tempChar)); 
      
      /*
      * set tempchar as opcode C
      * Get filename length
      * Get filename
      */
      read(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
      read(socketID, input, (sizeof(char) * ntohs(twoBytesMsg)));

      /* 
      * Open file
      * return -1 if fail opening the file
      * check what is the reason that cause the issue
      * send back the number which represent the issue
      */
      openFile = open(input, O_RDONLY);
      if (openFile == -1) {
        if (errno == ENOENT) {
          tempChar = '1';
        } else if (errno == EACCES){
          tempChar = '2';
        } else {
          tempChar = '3';
        }
        write(socketID, &tempChar, sizeof(tempChar)); 
        continue;
      }

      // If no error, send success opcode
      tempChar = '0';
      write(socketID, &tempChar, sizeof(tempChar));

      // Read file info and send the size of file
      stat(input, &fileDetails);
      fourBytesMsg = fileDetails.st_size;
      fourBytesMsg = htonl(fourBytesMsg);
      write(socketID, &fourBytesMsg, sizeof(fourBytesMsg));

      fourBytesMsg = ntohl(fourBytesMsg);
      bytesRemain = fourBytesMsg;
      // Loop through and send file
      while (bytesRemain) {
        if (bytesRemain < BLOCKMAXSIZE) {
          bytesRead = read(openFile, file, (sizeof(char) * bytesRemain));
        } else {
          bytesRead = read(openFile, file, (sizeof(char) * BLOCKMAXSIZE));
        }
        write(socketID, file, bytesRead);
        bytesRemain = bytesRemain - bytesRead;
      }
      // Close the opened file
      close(openFile);
    } 

    // put
    else if (command == 'D')
    {
      //set the tempchar opcode to D
      tempChar = 'D';

      /*
      * Get filename length
      * converts the unsigned short integer hostshort from 
      *     host byte order to network byte order.
      * 
      * Get filename
      */
      read(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
      twoBytesMsg = ntohs(twoBytesMsg);
      read(socketID, input, (sizeof(char) * twoBytesMsg));

      // send opcode back to client
      write(socketID, &tempChar, sizeof(tempChar));

      // Open local file for writing
      openFile = open(input, O_WRONLY | O_CREAT | O_APPEND | O_EXCL, 0644);
      if (openFile == -1) {
        if (errno == EEXIST) {
          tempChar = '1';
        } else if (errno == EACCES) {
          tempChar = '2';
        } else {
          tempChar = '3';
        }

        // Send status code
        write(socketID, &tempChar,sizeof(tempChar));
        continue;
      }

      // If no issue, set tempchar as 0 and send back
      tempChar = '0';
      write(socketID, &tempChar, sizeof(tempChar));
      
      /*
      * Receive next opcode
      * Receive file size from client
      */
      read(socketID, &tempChar, sizeof(tempChar));
      read(socketID, &fourBytesMsg, sizeof(fourBytesMsg));

      fourBytesMsg = ntohl(fourBytesMsg);
      bytesRemain = fourBytesMsg;

      // Receive file from client
      while (bytesRemain) {
        if (bytesRemain < BLOCKMAXSIZE) {
          bytesRead = read(socketID, file, (sizeof(char) * bytesRemain));
        } else {
          bytesRead =
              read(socketID, file, (sizeof(char) * BLOCKMAXSIZE));
        }
        write(openFile, file, bytesRead);
        bytesRemain = bytesRemain - bytesRead;
      }
      // Close open file
      close(openFile);
    } 
    // pwd
    else if (command == 'E') 
    {
      // set the tempchar opcode to E
      tempChar = 'E';

      /*
      * Get pwd and its length
      */
      getcwd(str, BUF_SIZE);
      twoBytesMsg = strlen(str) + 1;
      twoBytesMsg = htons(twoBytesMsg);

      /*
      * Send opcode
      * Send pwd and its length
      */
      write(socketID, &tempChar, sizeof(tempChar));
      write(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
      write(socketID, str, ntohs(twoBytesMsg));
    } 
    
    else {}
  }

} // end

