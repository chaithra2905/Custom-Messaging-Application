
#define MAX_DATA_LEN  128

typedef enum 
{
  MSG_UNAME_PASSWD = 1,
  MSG_AUTH_SUCCESS = 2,
  MSG_AUTH_FAIL = 3,
  MSG_LOGIN = 4,
  MSG_MAX = 5

} msg_type_e;


typedef struct 
{
  msg_type_e  msg_type; 
  char        msg_data[MAX_DATA_LEN];
  char        msg_add_data[MAX_DATA_LEN];

} msg_t;

typedef struct 
{
  void        (*msg_action_function) (int, msg_t *);

} msg_action_t;
