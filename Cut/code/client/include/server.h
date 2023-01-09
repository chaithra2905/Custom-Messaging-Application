#define MAX_CLIENTS 1
#define MAX_USERS   5
#define PORT 4444
#define SEND_BUFFER_SIZE 512
#define RECV_BUFFER_SIZE 512
#define USER_FOUND  1
#define USER_NOT_FOUND 2
#define USER_PASSWD_MATCH_FOUND 3
#define USER_PASSWD_MATCH_NOT_FOUND 4

typedef struct user_info
{
  char uname[32];
  char pwd[8];
  struct user_info *next;

} user_info_t;
