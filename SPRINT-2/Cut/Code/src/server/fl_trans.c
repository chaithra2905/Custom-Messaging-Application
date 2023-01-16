/*Filename: fl_trans.c
*
 * 
 *
 *File Description: The current file handles sending and recieving of files from server to clients.
 *
 */

#include "server.h"

/*
    // Writing File
    Input:       a socket file descriptor
    Output:      void function
    Description: it serves the sending option in the menu, 
                 it opens a file and writes the data coming from client
*/
void receive_file(int sockfd)
{
    int n;
    int flag=0;
    FILE *fp;
    char *filename = "../Files/recv.txt";
    char buffer[1024];

    recv(sockfd, &flag, 8, 0);
    if(flag==0)
    {
        printf("File not found\n");
        return;
    }

    fp = fopen(filename, "w+");
    while (1) 
    {
        n = recv(sockfd, buffer, 1024, 0);
        if (n <= 0)
        {
            break;
            return;
        }
        fputs(buffer, fp);
        bzero(buffer, 1024);
    }
    fclose(fp);
    printf("File received successfully\n\n");
    return;
}


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
    int flag = 0;
    char fileName[32];
    char data[1024] = {0};
    FILE *fp;

    DIR *d;
    struct dirent *dir;
    d = opendir("Files");
    strcat(data, "Files in the Files/ folder are:\n");
    if(d)
    {
        while((dir = readdir(d)) != NULL)
        {
            strcat(data, dir->d_name);
            strcat(data, "\n");
        }
        closedir(d);
    }
    send(sockfd, data, 1024, 0); // sending files data to cilent
    bzero(data, 1024);

    recv(sockfd, fileName, 32, 0);
    fp = fopen(fileName, "r");
    if(fp == NULL)
    {
        printf("File not found\n");
        send(sockfd, &flag, 8, 0);
        return;
    }

    flag = 1;
    send(sockfd, &flag, 8, 0);
    //sending file to client
    while(fgets(data, 1024, fp)!= NULL)
    {
        if (send(sockfd, data, sizeof(data), 0) == -1)
        {
            perror("Error in sending file.");
            exit(1);
        }
        bzero(data, 1024);
    }
    send(sockfd, data, 1024, 0);
    send(sockfd, "stop", 4, 0);
    fclose(fp);
    printf("File send successfully\n\n");
    return;
}
