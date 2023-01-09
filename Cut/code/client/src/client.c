#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

#define PORT 4444
#define SEND_BUFFER_SIZE 512
#define RECV_BUFFER_SIZE 512

#define MSG_SUCCESS  1
#define MSG_FAIL     2
#define MSG_INVALID  3
#define MAX_INPUT_LEN   32


int cl_sock_fd;

char send_buffer[SEND_BUFFER_SIZE];
char recv_buffer[RECV_BUFFER_SIZE];

void display_hotel_menu()
{
  int option;

  while(1)
  {
    printf(" Welcome to Custom Messaging Application \n");
    printf("===================================\n");

    printf("1. Private Chat \n");
    printf("2. Group Chat \n");
    printf("3. Active Users \n");
    printf("4. Exit to main menu \n");
    
    printf("Enter option: ");
    scanf("%d", &option);

    switch(option)
    {
      case 1:

      case 2:

      case 3:

      case 4:
              return;
      default:
              continue;
    }
  }
}

int validate_user_name_passwd(char *uname, char *pwd)
{
  if ((strlen(uname) > MAX_INPUT_LEN) || (strlen(uname) < 4))
  {
    printf("Invalid username, length must be between 4 and 32 \n");
    return -1;
  }
  else if ((strlen(pwd) > 6) || (strlen(pwd) < 4)) 
  {
    printf("Invalid password, length must be between 4 and 6 \n");
    return -1;
  }

  return 0;
}

int get_and_validate_input(char *input1, char *input2)
{
  printf("Enter User Name(Max 32 characters): \n");

  scanf("%s", input1);
  printf("Enter User Passwd(Max 6 chars): \n");

  scanf("%s", input2);

  return validate_user_name_passwd(input1, input2); 
}


void handle_server_close (int sockfd)
{

  close(sockfd);

}

int proces_login_msg(char *uname, char *pwd)
{
  int ret; 
  msg_t msg, *rx_msg;
  
  memset(msg.msg_data, '\0', sizeof(msg.msg_data));
  memset(msg.msg_add_data, '\0', sizeof(msg.msg_add_data));

  msg.msg_type = MSG_LOGIN;
  strncpy(msg.msg_data, uname, strlen(uname));
  strncpy(msg.msg_add_data, pwd, strlen(pwd));

  printf("Messaage Created \n");
  printf("  Type: %d \n", msg.msg_type);
  printf("  Data: %s len = %ld \n", msg.msg_data, strlen(uname));
  printf("  Additional Data: %s len = %ld\n", msg.msg_add_data,strlen(pwd));

  ret = send(cl_sock_fd, (char *) &msg, sizeof(msg), 0);

  printf("Sent bytes = %d \n", ret);


  ret = recv(cl_sock_fd, (char *) recv_buffer, sizeof(recv_buffer), 0);

  printf("Received %d bytes from Server \n", ret);


  if (ret == 0)
  {
    printf("%s():%d Server has closed on socket fd = %d \n",
        __FUNCTION__, __LINE__, cl_sock_fd);

    handle_server_close(cl_sock_fd);

    return -2;
  }

  rx_msg = (msg_t *) recv_buffer;

  if (rx_msg->msg_type == MSG_AUTH_SUCCESS)
  {
    return MSG_SUCCESS;
  }
  else if (rx_msg->msg_type == MSG_AUTH_FAIL)
  {
    return MSG_FAIL;
  }
  else
  {
    return MSG_INVALID;
  }

}

int proces_uname_pwd_msg(char *uname, char *pwd)
{
  int ret; 
  msg_t msg, *rx_msg;
  
  memset(msg.msg_data, '\0', sizeof(msg.msg_data));
  memset(msg.msg_add_data, '\0', sizeof(msg.msg_add_data));

  msg.msg_type = MSG_UNAME_PASSWD;
  strncpy(msg.msg_data, uname, strlen(uname));
  strncpy(msg.msg_add_data, pwd, strlen(pwd));

  printf("Messaage Created \n");
  printf("  Type: %d \n", msg.msg_type);
  printf("  Data: %s len = %ld \n", msg.msg_data, strlen(uname));
  printf("  Additional Data: %s len = %ld\n", msg.msg_add_data,strlen(pwd));

  ret = send(cl_sock_fd, (char *) &msg, sizeof(msg), 0);

  printf("Sent bytes = %d \n", ret);


  ret = recv(cl_sock_fd, (char *) recv_buffer, sizeof(recv_buffer), 0);

  printf("Received %d bytes from Server \n", ret);


  if (ret == 0)
  {
    printf("%s():%d Server has closed on socket fd = %d \n",
        __FUNCTION__, __LINE__, cl_sock_fd);

    handle_server_close(cl_sock_fd);

    return -2;
  }



  rx_msg = (msg_t *) recv_buffer;

  if (rx_msg->msg_type == MSG_AUTH_SUCCESS)
  {
    return MSG_SUCCESS;
  }
  else if (rx_msg->msg_type == MSG_AUTH_FAIL)
  {
    return MSG_FAIL;
  }
  else
  {
    return MSG_INVALID;
  }

}

void handle_client_close(int sock_fd)
{

  /* TODO */
  close(sock_fd);

}

int main()
{
  int ret;
  int i, option;

  struct sockaddr_in server_addr;

  int  menu_data_int;
  char menu_data_string1[MAX_INPUT_LEN];
  char menu_data_string2[MAX_INPUT_LEN];


  cl_sock_fd = socket(AF_INET,SOCK_STREAM, 0);

  if (cl_sock_fd < 0) 
  {
    printf("Error in connection.\n");
    exit(1);
  }

  printf("Client Socket is created.\n");

  memset(send_buffer, '\0', sizeof(send_buffer));
  memset(recv_buffer, '\0', sizeof(recv_buffer));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  ret = connect(cl_sock_fd, (struct sockaddr*)&server_addr,
      sizeof(server_addr));

  if (ret < 0) 
  {
    printf("Error in connection.\n");
    exit(1);
  }

  printf("Connected to Server.\n");

  while (1) 
  {
    printf("Menu Interface\n");
    printf("==============\n");

    printf("1. New User \n");
    printf("2. Existing User \n");
    printf("3. Logout \n");

    scanf("%d", &option);
  
    memset(menu_data_string1, '\0', sizeof(menu_data_string1));
    memset(menu_data_string2, '\0', sizeof(menu_data_string2));

    printf("S1: %s \n", menu_data_string1);
    printf("S2: %s \n", menu_data_string2);
    switch(option)
    {
      case 1:

        ret = get_and_validate_input(menu_data_string1, menu_data_string2);

        if (ret == -1)
        {
          printf("Invalid input format \n");
          break;
        }

        ret = proces_uname_pwd_msg(menu_data_string1, menu_data_string2);

        if (ret == MSG_FAIL)
        {
          printf("User Authentication Failed \n");
          break;
        }
        else if (ret == MSG_INVALID)
        {
          printf("User Authentication Failed \n");
          break;

        }
        else if (ret == MSG_SUCCESS)
        {
          printf("User Authentication Success \n");

          display_hotel_menu();
        }

        break;

      case 2:

        ret = get_and_validate_input(menu_data_string1, menu_data_string2);

        if (ret == -1)
        {
          printf("Invalid input format \n");
          break;
        }

        ret = proces_login_msg(menu_data_string1, menu_data_string2);

        if (ret == MSG_FAIL)
        {
          printf("User Authentication Failed \n");
          break;
        }
        else if (ret == MSG_INVALID)
        {
          printf("User Authentication Failed \n");
          break;

        }
        else if (ret == MSG_SUCCESS)
        {
          printf("User Authentication Success \n");

          display_hotel_menu();
        }

        break;
      
      case 3:
        exit(0);

      default:
        printf("Invalid Option .. \n");
        break;
    }

  }

  return 0;
}

