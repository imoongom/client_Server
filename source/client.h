#ifndef CHILD_H
#define CHILD_H

#include  "mesq.h"
#include <pthread.h>
#include <signal.h>


int sigid;
int msq_id;

typedef struct
{
  int q_id;
  char *filename;
}ThreadInfo;


void file_request(char*);
void msg_rcv(char*);
void *threadfunc(void *);
void sig_cl_handler (int sig_num);




#endif
