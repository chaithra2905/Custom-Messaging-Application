/*Filename: server.c
*
 * 
 *File Description: -The current file handles the server side program.
                    -Handles users information
                    -Handles chat history
                    -Sends and recieves messages and files from clients
 */
#include "server.h"

client_t *clients[MAX_CLIENTS]; // array to store client records

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // to provide mutex lock functionality

FILE *fptr;

time_t t;   

// Function to trim next line character from the buffer
void rm_next (char* arr, int len)
{
    int i;
    for (i = 0; i < len; i++)
    { 
        if (arr[i] == '\n') 
        {
            arr[i] = '\0';
            break;
        }
    }
}


/*
    // Function to maintain chat history
    Input:       an array pointer consisting the message
    Output:      the message will be stored
    Description: the recieved message will be stored in a file in a certain format with timestamp
*/
void store_msg(char *arr)
{
    t = time(NULL);
    char *ti = ctime(&t);

    // printing '->' in chat history after time stamp
    for(int i=0; i<strlen(ti); i++)
    {
        if(ti[i] == '\n')
        {
            ti[i] = '-';
            ti[i+1] = '>';
            break;
        }
    }

    rm_next(arr, strlen(arr));
    fptr = fopen("Files/History.txt", "a+");
    fputs(ti, fptr);
    fputs(arr, fptr);
    fputs("\n", fptr);
    fclose(fptr);
}

// Add clients to clients array
void add_to_list(client_t *cl)
{
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i < MAX_CLIENTS; ++i)
    {
        if(!clients[i])
        {
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// Remove clients from clients array
void rmv_from_list(int uid)
{
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i < MAX_CLIENTS; ++i)
    {
        if(clients[i])
        {
            if(clients[i]->uid == uid)
            {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send message to all clients except sender 
void send_message(char *s, int uid, char *status)
{
    pthread_mutex_lock(&clients_mutex);

    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        if(clients[i])
        {
            if(clients[i]->uid != uid && (strcmp(clients[i]->status, status)==0))
            {
                if(write(clients[i]->sockfd, s, strlen(s)) < 0)
                {
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}


/*
    // Private chat
    Input:       a client structure and a socket descriptor
    Output:      void function
    Description: makes the private chat users BUSY and send some messages to client
*/
void p_chat(client_t *cli, int sockfd)
{
    char pname[32];
    char buffer[1024] = {0};
    int i;

    // displaying active users list
    strcat(buffer, "Active users are:\n");
    for(i=0; i<MAX_CLIENTS; ++i)
    {
        if(clients[i])
        {
            if(strcmp(clients[i]->name, cli->name)!=0)
            {
                strcat(buffer, clients[i]->name);
                strcat(buffer, "\n");
            }
        }
    }

    send(sockfd, buffer, 1024, 0);
    bzero(buffer, 1024);

    recv(sockfd, pname, 32, 0);
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        if(clients[i])
        {
            if((strcmp(clients[i]->name, pname)==0))
            {
                sprintf(buffer, "%s is active\n", pname);
                send(sockfd, buffer, 32 , 0);
                strcpy(clients[i]->status, "BUSY");
                strcpy(cli->status, "BUSY");
                return;
            }
        }
    }

    sprintf(buffer, "%s is not present\n", pname);
    send(cli->sockfd, buffer, 32, 0);
}


/*
    // group chat
    Input:       a client structure
    Output:      void function
    Description: sends active groups data to the client and makes server ready for group chat
*/
void g_chat(client_t *cli)
{
    char buffer[1024] = {0};
    int counter;

    // displaying active groups list
    strcat(buffer, "Active groups are:\n");
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        counter = 0;
        if(clients[i])
        {
            for(int j = i+1; j<MAX_CLIENTS && clients[j]!=NULL; ++j)
            {
                if(strcmp(clients[i]->status, clients[j]->status)==0)
                {
                    counter++;
                    break;
                }
            }       
            if((strcmp(clients[i]->status, "ACTIVE")!=0) &&
                (strcmp(clients[i]->status, "BUSY") != 0) && 
                counter==0)
            {
                strcat(buffer, clients[i]->status);
                strcat(buffer, "\n");
            }
        }
    }

    send(cli->sockfd, buffer, 1024, 0);
    bzero(buffer, 1024);
    recv(cli->sockfd, buffer, 32, 0);
    strcpy(cli->status, buffer);
}


/*
    // Validate clients
    Input:       a client structure and a socket descriptor
    Output:      void function
    Description: takes the user data and validates it and sends messages to client
*/
void validate_user(client_t *cli, int sockfd)
{
    char buff_out[SIZE];
    char trim_buff[SIZE];
    char uname[32];
    char pass[8];
    int flag = 0;

    recv(sockfd, uname, 32, 0);
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        if(clients[i])
        {
            if(strcmp(clients[i]->name, uname) == 0)
            {
                send(sockfd, &flag, 1, 0);
                recv(sockfd, uname, 32, 0);
                i=0;
            }
        }
    }
    flag = 1;
    send(sockfd, &flag, 1, 0);
    recv(sockfd, &pass, 8, 0);

    strcpy(cli->name, uname);
    strcpy(cli->password, pass);
    sprintf(buff_out, "%s has joined\n", cli->name);
    strcpy(trim_buff, buff_out);
    printf("%s", buff_out);
    store_msg(trim_buff);
    send_message(buff_out, cli->uid, cli->status);

    bzero(buff_out, SIZE);
    bzero(trim_buff, SIZE);
}


/*
    // Handle all communication with the client
    Input:       a client structure in the form of void pointer
    Output:      void function
    Description: uses all the functions and performs all communication between client and server 
*/
void *handle_client(void *arg)
{
    char buff_out[SIZE];
    char trim_buff[SIZE];
    int leave_flag = 0;
    int choice;

    cli_count++;
    client_t *cli = (client_t *)arg;

    // Recieving user name and password
    validate_user(cli, cli->sockfd);

    // Receiving user's choice
    recv(cli->sockfd, &choice, 1, 0); 
    if(choice == 1)
    {
        p_chat(cli, cli->sockfd);
    }

    else if(choice == 2)
    {
        g_chat(cli);
    }

    else if(choice == 3)
    {
        printf("Recieving file from %s\n", cli->name);
        receive_file(cli->sockfd);
    }

    else if(choice == 4)
    {
        printf("Sending file to %s\n", cli->name);
        send_file(cli->sockfd);
    }

    while(1)
    {
        if(leave_flag)
        {
            break;
        }
        int receive = recv(cli->sockfd, buff_out, SIZE, 0);
        if (receive > 0)
        {
            if(strlen(buff_out) > 0)
            {
                send_message(buff_out, cli->uid, cli->status);
                rm_next(buff_out, strlen(buff_out));
                printf("%s\n", buff_out);
                store_msg(buff_out);
            }
        } 
        // exiting from the chat
        else if (receive == 0 || strcmp(buff_out, "exit") == 0)
        {
            sprintf(buff_out, "%s has left\n", cli->name);
            send_message(buff_out, cli->uid, cli->status);
            strcpy(trim_buff, buff_out);
            printf("%s", buff_out);
            store_msg(trim_buff);
            leave_flag = 1;
        } 

        else 
        {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buff_out, SIZE);
        bzero(trim_buff, SIZE);
    }

    /* Delete client from queue and yield thread */
    close(cli->sockfd);
    rmv_from_list(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

// main function starts here
int main()
{

    char *ip = "127.0.0.1";
    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    // File for chat backup
    fptr = fopen("Files/History.txt", "w+");
    if(fptr == NULL)
    {
        perror("");
        printf("Unable to store chat history\n");
    }
    fputs("\n==== NEW SESSION HISTORY STARTED ====\n\n", fptr);
    fclose(fptr);

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(ip);
    serv_addr.sin_port = htons(PORT);


    // Ignore pipe signals 
    signal(SIGPIPE, SIG_IGN);

    // Stops binding error, address already in use
    if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),
                (char*)&option,sizeof(option)) < 0)
    {
        perror("ERROR: setsockopt failed\n");
        return EXIT_FAILURE;
    }

    // Bind 
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR: Socket binding failed\n");
        return EXIT_FAILURE;
    }

    // Listen
    if (listen(listenfd, 10) < 0) 
    {
        perror("ERROR: Socket listening failed\n");
        return EXIT_FAILURE;
    }

    printf("\n=== WELCOME TO LET'S CHAT ===\n");

    while(1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        // Check if max clients is reached
        if((cli_count + 1) == MAX_CLIENTS)
        {
            printf("Max clients reached. Rejected ");
            close(connfd);
            continue;
        }

        // Client settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;
        strcpy(cli->status, "ACTIVE");

        // Add client to the queue and fork thread
        add_to_list(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        // Reduce CPU usage
        sleep(1);
    }
    return EXIT_SUCCESS;
}


