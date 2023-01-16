/*Filename: fl_trans.c
*
 * 
 *
 *File Description: The current file handles sending and recieving of files from clients to server.
 *
 */

#include "../common/common.h"


/*
    // Sending file
    Input:       a socket file descriptor
    Output:      void function
    Description: it serves the recieving option in the menu, 
                 it sends file data and takes the file choice from client and 
                 provide the file to client
*/
void send_file(int sockfd)
{
    int n;
    int flag=0;
    char fileName[32];
    char data[1024] = {0};
    FILE *fp;

    printf("\n=== FILE SENDING STARTED ===\n");
    printf("Enter complete file path and name: ");
    scanf(" %[^\n]s",fileName);
    fp = fopen(fileName, "r");
    if(fp == NULL)
    {
        send(sockfd, &flag, 8, 0);
        printf("File not found\nExiting...\n");
        exit(1);
    }

    flag = 1;
    send(sockfd, &flag, 8, 0);

    while(fgets(data, 1024, fp) != NULL) 
    {
        if (send(sockfd, data, sizeof(data), 0) == -1) 
        {
            printf("Error in sending file.");
            exit(1);
        }
        bzero(data, 1024);
    }

    printf("File transferred successfully\n\n");
    close(sockfd);
    exit(1);
}


/*
    // Receiving file
    Input:       a socket file descriptor
    Output:      void function
    Description: it serves the sending option in the menu, 
                 it opens a file and writes the data coming from client
*/
void receive_file(int sockfd)
{
    int n;
    int flag = 0;
    FILE *fp;
    char *filename = "Files/send.txt";
    char recvfile[32];
    char buffer[1024];

    printf("\n=== FILE RECEIVING STARTED ===\n");
    recv(sockfd, buffer, 1024, 0);
    printf("%s", buffer);
    bzero(buffer, 1024);
    printf("Enter complete file path and name of source file: ");
    scanf(" %[^\n]s",recvfile);

    send(sockfd, recvfile, 32, 0);
    recv(sockfd, &flag, 8, 0);

    if(flag == 0)
    {
        printf("File not found.\nExiting...\n");
        close(sockfd);
        exit(1);
    }

    //bzero(recvfile, 32);

    fp = fopen(filename, "w+");
    while (1)
    {
        n = recv(sockfd, buffer, 1024, 0);
        if (strcmp(buffer, "stop")==0)
        {
            printf("File received successfully\n");
            fclose(fp);
            close(sockfd);
            exit(1);
        }
        printf("%s", buffer);
        fputs(buffer, fp);
        bzero(buffer, 1024);
    }
}
