
#include <string.h> 
#include <unistd.h> 
#include <errno.h>      // errno
#include <stdlib.h>     // free()
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h> // struct sockaddr_in, htons(), htonl()
#include <dirent.h>     // dirent structure("ino_t d_ino: file serial number
                        // , char d_name[] = name of entry")
#include "stream.h"


void run(int socketID) {
    char input[maxBuffSize], str[maxBuffSize], charValue,file[maxBuffSize];
    char* token;
    int twoBytesMsg, fd, condition, fnl, i,fileNumber;
    long fourBytesMsg, bytesRead, remainingBytes;
    struct stat fileDetails;
    struct dirent** listOfFile;

    while (1)
    {
        printf(">");

        /*read user input into input buff*/
        fgets(input, maxBuffSize, stdin);

        /*
        * Test user input
        * if input got data then continue
        * else if input got nothing then request for input again
        */
        if (strlen(input) == 1)
        {
            continue;
        }
        else if ((input[strlen(input) - 1]) == '\n')
        {
            input[strlen(input) - 1] = '\0';
        }

        /*
        * analyse input content
        *
        * if the input content is "quit"
        * then terminate program
        */
        if (strcmp("quit", input) == 0) {
            printf("Bye~\n");
            exit(1);
        }

        /*
        * else analyse the input
        */
        else
        {
            // Get the first parameter by tokenizing the input
            token = strtok(input, tokenStr);

            /*
            * cd - change the current directory of the server
            *      that is serving the client
            *
            * example - cd directory_pathname
            */
            if (strcmp("cd", token) == 0) {

                // Set opcode in charValue
                charValue = 'A';
                token = strtok(NULL, tokenStr);
                if (token == NULL) {
                    printf("No directory specified\n");
                    continue;
                }

                /*
                * convert the short integer from network byte order
                * to local host byte order
                */
                twoBytesMsg = strlen(token) + 1;
                twoBytesMsg = htons(twoBytesMsg);

                /*
                *  Send opcode
                *  Send length of dir name
                *  Send dir name
                */
                write(socketID, &charValue, sizeof(charValue));
                write(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
                write(socketID, token, (sizeof(char) * (ntohs(twoBytesMsg))));

                /*
                * Get back command response
                * Get back status of dir change
                */
                read(socketID, &charValue, sizeof(char));
                read(socketID, &charValue, sizeof(char));


                if (charValue == '1') {
                    printf("Directory '%s' does not exist\n", token);
                }
                else if (charValue == '2') {
                    printf("Access has been denied\n");
                }
                else if (charValue == '3') {
                    printf("Change directory fail\n");
                }

            }

            /*
            * dir - display the file names under the current
            *       directory of the server that is serving the client
            */
            else if (strcmp("dir", token) == 0) {
                // Set opcode in tempChar
                charValue = 'B';

                //Send opcode
                write(socketID, &charValue, sizeof(charValue));

                /*
                *  Get back command response
                *  Get back number of files and folders
                */
                read(socketID, &charValue, sizeof(charValue));
                read(socketID, &twoBytesMsg, sizeof(twoBytesMsg));

                /*
                * convert the short integer from network byte order
                * to local host byte order
                */
                twoBytesMsg = ntohs(twoBytesMsg);

                // Get back file info and print
                for (i = 0; i < twoBytesMsg; ++i) {
                    // Print the file type of every file
                    read(socketID, &charValue, sizeof(charValue));
                    printf("%c ", charValue);

                    // Get filename length and filename then print
                    read(socketID, &fnl, sizeof(fnl));
                    fnl = ntohs(fnl);
                    read(socketID, str, (sizeof(char) * fnl));
                    printf("%s\n", str);
                }
            }

            /*
            * get - to download the named file from the current
            *       directory of the remote server and save it in the
            *       current directory of the client
            *
            * example - get filename
            */
            else if (strcmp("get", token) == 0) {
                // Set opcode 'C' in tempChar
                charValue = 'C';

                // Get the filename
                token = strtok(NULL, tokenStr);
                if (token == NULL) {
                    printf("No file name specified\n");
                    continue;
                }

                /*
                 * convert the short integer from network byte order
                 * to local host byte order
                 */
                twoBytesMsg = strlen(token) + 1;
                twoBytesMsg = htons(twoBytesMsg);

                // get file descriptor
                fd = open(token, O_APPEND | O_EXCL | O_WRONLY | O_CREAT, 0644);

                if (fd == -1) {
                    if (errno == EEXIST) {
                        printf("File already exists in client folder\n");
                    }
                    else {
                        printf("Could not create file on client\n");
                    }
                    continue;
                }
                /*
                * Send opcode
                * Send filename length
                * Send filename
                */
                write(socketID, &charValue, sizeof(charValue));
                write(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
                write(socketID, token, (sizeof(char) * ntohs(twoBytesMsg)));

                /*
                * Get back opcode
                * Get back status code
                */
                read(socketID, &charValue, sizeof(charValue));
                read(socketID, &charValue, sizeof(charValue));

                // if file can send
                if (charValue == '0')
                {
                    // Get file size
                    read(socketID, &fourBytesMsg, sizeof(fourBytesMsg));

                    /*
                     * convert the long integer from network byte order
                     * to local host byte order
                     */
                    fourBytesMsg = ntohl(fourBytesMsg);
                    remainingBytes = fourBytesMsg;

                    // Loop through and get file
                    while (remainingBytes) {
                        if (remainingBytes < maxBuffSize) {
                            bytesRead = read(socketID, file, (sizeof(char) * remainingBytes));
                        }
                        else {
                            bytesRead = read(socketID, file, (sizeof(char) * maxBuffSize));
                        }

                        write(fd, file, bytesRead);
                        remainingBytes = remainingBytes - bytesRead;
                    }
                }
                else if (charValue == '1') {
                    printf("File not found\n");
                }
                else if (charValue == '2') {
                    printf("Access denied\n");
                }
                //other issue
                else if (charValue == '3') {
                    printf("Could not get file\n");
                }
                else {
                    printf("Something went wrong...\n");
                }
                // Close the opened file
                close(fd);
            }

            /*
            * put - to upload the named file from the current
            *       directory of the client to the current
            *       directory of the remove server.
            *
            * example - put filename
            */
            else if (strcmp("put", token) == 0) {
                // Set opcode in tempChar
                charValue = 'D';

                // Get the filename
                token = strtok(NULL, tokenStr);
                if (token == NULL) {
                    printf("No file name specified\n");
                    continue;
                }

                /*
                 * convert an IP port number in host byte order
                 * to the IP port number in network byte order.
                 */
                twoBytesMsg = strlen(token) + 1;
                twoBytesMsg = htons(twoBytesMsg);

                // Open file for sending
                fd = open(token, O_RDONLY);
                if (fd == -1) {
                    if (errno == ENOENT) {
                        printf("Could not find file to send\n");
                    }
                    else if (errno == EACCES) {
                        printf("Permission denied\n");
                    }
                    else {
                        printf("Could not open file to send\n");
                    }
                    continue;
                }

                /*
                * Send opcode
                * Send filename length
                * Send filename
                */
                write(socketID, &charValue, sizeof(charValue));
                write(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
                write(socketID, token, (sizeof(char) * ntohs(twoBytesMsg)));

                /*
                * Get back opcode
                * Get back status code
                */
                read(socketID, &charValue, sizeof(charValue));
                read(socketID, &charValue, sizeof(charValue));

                if (charValue == '0') // Can send file
                {
                    // Set next opcode and send
                    charValue = 'E';
                    write(socketID, &charValue, sizeof(charValue));

                    // Get file info
                    stat(token, &fileDetails);
                    fourBytesMsg = fileDetails.st_size;

                    // Send file size
                    fourBytesMsg = htonl(fourBytesMsg);

                    write(socketID, &fourBytesMsg, sizeof(fourBytesMsg));
                    fourBytesMsg = ntohl(fourBytesMsg);
                    remainingBytes = fourBytesMsg;
                    // Loop through and send file
                    while (remainingBytes) {
                        if (remainingBytes < maxBuffSize) {
                            bytesRead =
                                read(fd, file, (sizeof(char) * remainingBytes));
                        }
                        else {
                            bytesRead =
                                read(fd, file, (sizeof(char) * maxBuffSize));
                        }
                        write(socketID, file, bytesRead);
                        remainingBytes = remainingBytes - bytesRead;
                    }
                    // Close the open file
                    close(fd);
                }
                else if (charValue == '1') // Filename clash
                {
                    printf("A file of that name already "
                        "exists on the server\n");
                }
                else if (charValue == '2') // Create error
                {
                    printf("Server could not create file "
                        "for saving\n");
                }
                else if (charValue == '3') // Other error
                {
                    printf("The server incountered an "
                        "unspecified error\n");
                }
                else // Just in case
                {
                    printf("Something went wrong...\n");
                }
            }

            /*
            * pwd - to display the current directory
            *       of the server that is serving the client
            */
            else if (strcmp("pwd", token) == 0) {
                // Set opcode in tempChar and send
                charValue = 'E';
                write(socketID, &charValue, sizeof(charValue));

                /*
                * Get back opcode response
                * Get back length of pwd
                * Get pwd
                */
                read(socketID, &charValue, sizeof(charValue));
                read(socketID, &twoBytesMsg, sizeof(twoBytesMsg));
                read(socketID, str, (sizeof(char) * ntohs(twoBytesMsg)));

                // Print out pwd
                printf("%s\n", str);
            }

            /*
            * lpwd - to display the current directory of the client;
            */
            else if (strcmp("lpwd", token) == 0) {
                getcwd(str, maxBuffSize);
                printf("%s\n", str);
            }

            /*
            * lcd - to change the current directory of the client;
            *       Must support "." and ".." notations.
            *
            * example - lcd directory_pathname
            */
            else if (strcmp("lcd", token) == 0) {
                token = strtok(NULL, tokenStr);
                condition = chdir(token);
                if (condition == -1) // If dir change fails
                {
                    if (errno == ENOENT) {
                        printf("Directory not exist\n");
                    }
                    else if (errno == EACCES) {
                        printf("Permission denied\n");
                    }
                    else {
                        printf("Unspecified error. Directory not changed\n");
                    }
                }
            }

            /*
            * ldir - to display the file names under the
            *        current directory of the client;
            */
            else if (strcmp("ldir", token) == 0) {
                fileNumber = scandir(".", &listOfFile, NULL, alphasort);
                if (fileNumber == 0) {
                    perror("myftp:scandir");

                }
                else {

                    for (i = 0; i < fileNumber; ++i) {
                        if (listOfFile[i]->d_type == DT_REG) {
                            printf("f ");

                        }
                        else if (listOfFile[i]->d_type == DT_DIR) {
                            printf("d ");

                        }
                        else {
                            printf("o ");

                        }
                        printf("%s\n", listOfFile[i]->d_name);
                        free(listOfFile[i]);

                    }
                    free(listOfFile);

                }

            }
            else {
                printf("Invalid command\n");
            }

        }

    }
}
