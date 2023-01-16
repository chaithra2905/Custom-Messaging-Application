/*Filename: server.c
*
 * 
 *
 *File Description: -The current file handles the client side program.
                    -Provides user menu to clients.
                    -Sends and recieves messages and files from server.
 */

#include "../common/common.h"

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];
char password[8];

// function declarations
void send_file(int);
void receive_file(int);


/*
    // Entering username and password
    Input:       a socket file descriptor
    Output:      void function
    Description: it serves the login part of the client and takes username and password 
                 from client and validate them
*/
void enter(int sockfd)
{
    char *p;
    int exist = 0;

    while(1)
    {
        printf("Enter your name: ");
        scanf(" %[^\n]s", name);
        if (strlen(name) > 32 || strlen(name) < 2)
        {
            printf("Name must be less than 30 and more than 2 characters.\n");
        }
        else
        {
            // Send name
            send(sockfd, name, 32, 0);
            // Recieving if username exist or not
            recv(sockfd, &exist, 1, 0);
            if(exist == 0)
            {
                printf("Username already taken, enter another\n");
            }
            else
            {
                exist=0;
                break;
            }
        }
    }

    while(1)
    {
        p= getpass("Enter password: ");
        strcpy(password,p);
        if (strlen(password) > 8 || strlen(password) < 2)
        {
            printf("Password must be less than 8 and more than 2 characters.\n");
        }
        else
        {
            // Send password
            send(sockfd, password, 8, 0);
            break;
        }
    }
}

void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
}

void catch_ctrl_c_and_exit(int sig) 
{
    flag = 1;
}


/*
    // Sending message
    Input:       no input required
    Output:      void function
    Description: it handles the messages to be sent to the server and it is a thread handler
*/
void send_msg_handler() 
{
    char message[1024] = {};
    char buffer[SIZE + 32] = {};

    while(1) 
    {
        str_overwrite_stdout();
        scanf(" %[^\n]s", message);

        if (strcmp(message, "exit") == 0) 
        {
            break;
        } else 
        {
            sprintf(buffer, "%s: %s\n", name, message);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(message, 1024);
        bzero(buffer, SIZE + 32);
    }
    catch_ctrl_c_and_exit(2);
}


/*
    // Receiving message
    Input:       no input required
    Output:      void function
    Description: it handles the messages to be recieved from the server and it is a thread handler
*/
void recv_msg_handler() 
{
    char message[SIZE] = {};
    while (1) 
    {
        int receive = recv(sockfd, message, SIZE, 0);
        if (receive > 0) {
            printf("%s", message);
            str_overwrite_stdout();
        } else if (receive == 0) 
        {
            break;
        } else 
        {
            continue;
        }
        memset(message, 0, sizeof(message));
    }
}


/*
    // Asking user's choice
    Input:       a socket file descriptor
    Output:      void function
    Description: it handles the which choice does the client have selected and also prints error messages
*/
void choice(int sockfd)
{
    char ch, pname[32], gname[32], buffer[1024];
    int c;

    printf("\n1. Start individual chat.\n");
    printf("2. Start group chat.\n");
    printf("3. Send file.\n");
    printf("4. Recieve file.\n");

    while(1)
    {
        printf("\nEnter your choice: ");
        scanf(" %c", &ch);

        c = atoi(&ch);

        if(c == 1 || c == 2 || c == 3 || c == 4)
        {
            break;
        }
        printf("Enter valid choice (1-4)\n");
    }

    send(sockfd, &c, 1, 0);
    if (c==1)
    {
        recv(sockfd, buffer, 1024, 0); 
        printf("%s",buffer);
        printf("\nEnter name of person with whom you want to chat privately: ");
        scanf(" %[^\n]s", pname);
        send(sockfd,pname, 32, 0);
        bzero(pname, 32);
        recv(sockfd, pname, 32, 0); 
        printf("%s",pname);
    }

    if(c==2)
    {
        recv(sockfd, buffer, 1024, 0); 
        printf("%s",buffer);
        printf("\nEnter group name: ");
        scanf(" %[^\n]s", gname);
        send(sockfd,gname, 32, 0);
    }
        
    if(c == 3)
    {
        send_file(sockfd);
    }

    if( c == 4)
    {
        receive_file(sockfd);
    }
}

// main function starts here
int main()
{
    char *ip = "127.0.0.1";

    signal(SIGINT, catch_ctrl_c_and_exit);

    struct sockaddr_in server_addr;

    /* Socket settings */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(ip);
    server_addr.sin_port = htons(PORT);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    printf("\n=== WELCOME TO LET'S CHAT ===\n");

    // Calling enter() function to input name and password
    enter(sockfd);

    // Calling choice() function to know user's choice
    choice(sockfd);

    pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while (1){
        if(flag){
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}


