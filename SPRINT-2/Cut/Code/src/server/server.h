#include "../common/common.h"

#define MAX_CLIENTS 10

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

/* Client structure */
typedef struct{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32];
    char password[8];
    char status[10];
} client_t;

extern client_t *clients[MAX_CLIENTS];

extern FILE *fptr;

extern time_t t;

// function declarations of server
void send_file(int);
void receive_file(int);

