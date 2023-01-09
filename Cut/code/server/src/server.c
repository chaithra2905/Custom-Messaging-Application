#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"
#include "server.h"

int cl_count = 0;
msg_action_t  msg_actions[MSG_MAX];
user_info_t  *user_list_start = NULL;

user_info_t* add_user_to_list(char *uname, char *pwd)
{
  user_info_t *new_user = malloc(sizeof(user_info_t));
  user_info_t *temp = user_list_start;

  if (new_user == NULL)
  {
    printf("Failed to alloc memory \n");
    return NULL;
  }

  strncpy(new_user->uname, uname, strlen(uname));
  strncpy(new_user->pwd, pwd, strlen(pwd));
  new_user->next = NULL;

  if (user_list_start == NULL)
  {
    user_list_start = new_user;
    printf("Added first user \n");
    return new_user;
  }

  while(temp->next != NULL)
    temp = temp->next;

  temp->next = new_user;
  printf("Added new user \n");
    
  return user_list_start;
}

int search_user_pwd_in_list(char *uname, char *pwd)
{
  user_info_t *temp = user_list_start;

  if (temp == NULL)
  {
    printf("List is empty \n");
    return USER_NOT_FOUND;
  }
 

  while(temp != NULL)
  {
    if (strcmp(temp->uname, uname) == 0)
    {
      printf("User %s found \n", uname);

      if (pwd != NULL) 
      {
        if (strcmp(temp->pwd, pwd) == 0)
        {
          printf("User %s password matched \n", uname);
          return USER_PASSWD_MATCH_FOUND;
        }
        else
        {
          printf("User %s password not matched \n", uname);
          return USER_PASSWD_MATCH_NOT_FOUND;
        }

      }
      else
      {
        return USER_FOUND;
      }
    }

    temp = temp->next;
  }

  printf("User %s not found \n", uname);
  return USER_NOT_FOUND;
}

void handle_login_msg (int sockfd, msg_t *msg)
{
  msg_t reply_msg;
  int ret;

  printf("Handling message: %d \n", msg->msg_type);
  printf("Username : %s \n", msg->msg_data);
  printf("Password : %s \n", msg->msg_add_data);

  ret = search_user_pwd_in_list(msg->msg_data, msg->msg_add_data);

  if (ret == USER_PASSWD_MATCH_FOUND)
  {
    printf("Auth Success : %s \n", msg->msg_add_data);
    reply_msg.msg_type = MSG_AUTH_SUCCESS;
  }
  else 
  {
    printf("Auth Failure : %s \n", msg->msg_add_data);
    reply_msg.msg_type = MSG_AUTH_FAIL;
  }
  
  ret = send(sockfd, (char *) &reply_msg, sizeof(msg), 0);

  printf("Sent %d bytes reply .. \n", ret);
}

void handle_uname_passwd_msg (int sockfd, msg_t *msg)
{
  msg_t reply_msg;
  
  printf("Handling message: %d \n", msg->msg_type);
  printf("Username : %s \n", msg->msg_data);
  printf("Password : %s \n", msg->msg_add_data);

  add_user_to_list(msg->msg_data, msg->msg_add_data);

  reply_msg.msg_type = MSG_AUTH_SUCCESS;
  
  int ret = send(sockfd, (char *) &reply_msg, sizeof(msg), 0);

  printf("Sent %d bytes reply .. \n", ret);
}

void setup_message_handlers()
{
  msg_actions[MSG_UNAME_PASSWD].msg_action_function = handle_uname_passwd_msg;
  msg_actions[MSG_LOGIN].msg_action_function = handle_login_msg;

}

void handle_client_close(int sock_fd)
{

  printf("Client on socket %d closed \n", sock_fd);

  /* TODO */
  close(sock_fd);

}

void process_client_messages(int sockfd, char *recv_buffer)
{
  /* TODO */

  msg_t *m = (msg_t *) recv_buffer;

  switch (m->msg_type)
  {
    case MSG_UNAME_PASSWD:
                  printf("Received msg type %d socket = %d ... \n", m->msg_type, sockfd);
                  msg_actions[m->msg_type].msg_action_function(sockfd, m);
                  break;
    case MSG_LOGIN:
                  printf("Received msg type %d socket = %d ... \n", m->msg_type, sockfd);
                  msg_actions[m->msg_type].msg_action_function(sockfd, m);
                  break;
    default:
                  printf("Received invalid msg type ... \n");
                  break;
  }

  return;

}

/* Thread to handle clients */
void* client_handler(void *client_sock_fd)
{
  int *cl_sock_fd = (int *) client_sock_fd;
  int ret = 0;

  char send_buffer[SEND_BUFFER_SIZE];
  char recv_buffer[RECV_BUFFER_SIZE];

  printf("%s():%d Client Fd = %d\n",__FUNCTION__, __LINE__, *cl_sock_fd);

  while(1)
  {
    printf("%s():%d Waiting on recv for fd = %d \n",
           __FUNCTION__, __LINE__, *cl_sock_fd);

    ret = recv(*cl_sock_fd, recv_buffer, sizeof(recv_buffer), 0);

    if (ret == 0)
    {
      printf("%s():%d Client has closed on socket fd = %d \n",
           __FUNCTION__, __LINE__, *cl_sock_fd);

      /* TODO: 
       * client has closed the connection, do any cleanup required and exit */

      handle_client_close(*cl_sock_fd);

      cl_count--;
      /* END: */
      pthread_exit(NULL);
    }

    /* TODO: 
     * Implement message processing
     *  - Identify received message type i.e. proper type casting
     *  - Do appropirate action for the received message type 
     *  - If required, create a reply message with proper type and send to client*/

    process_client_messages(*cl_sock_fd, recv_buffer);


    /* END: */

  }

  pthread_exit(NULL);
}

int main()
{

  int server_fd, ret;

  struct sockaddr_in server_addr;
  
  int cl_sock_fd[MAX_CLIENTS];  
  
  setup_message_handlers();

  pthread_t cl_threads[MAX_CLIENTS];

  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd < 0) 
  {
    printf("Error in connection.\n");
    exit(1);
  }

  printf("TCP Server Socket is created.\n");

  memset(&server_addr, '\0',  sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  ret = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

  if (ret < 0) 
  {
    printf("Error in binding.\n");
    exit(1);
  }

  if (listen(server_fd, 10) == 0) 
  {
    printf("Listening...\n\n");
  }

  while (1) 
  {
    ret = accept(server_fd, NULL, NULL);

    if (ret < 0) 
    {
      perror("accept failed: ");
      exit(1);
    }

    cl_sock_fd[cl_count] = ret;
      
    printf("cl_count = %d fd = %d clfd = %d \n",
             cl_count, ret, cl_sock_fd[cl_count]);

    ret = pthread_create(&cl_threads[cl_count], NULL, client_handler,
                         &cl_sock_fd[cl_count]);
   
    if (ret == 0)
    {
      cl_count++;

      printf("A new thread is created for client on fd: %d \n",
             cl_sock_fd[cl_count]);
      printf("Total clients connected : %d\n\n", cl_count);
    }  

    if (cl_count == MAX_CLIENTS)
    {
      printf("Max clients %d are connected..No more connections will be accepted\n", 
             cl_count);
      break;
    }
  }

  for (int i = 0; i < cl_count; i++)
  {  
    pthread_join(cl_threads[i], NULL);
  }

  return 0;
}

