#include "mesq.h"
#include <signal.h>
#include <sys/wait.h>
//#include <sys/time.h>

#define PRIORITY -5

void read_queue(int);
void file_trans(int, struct my_msg *);
void stop_request(int, struct my_msg *);


void sig_handler (int);
void close_queue(int);


pid_t parentId;
int msq_id;
