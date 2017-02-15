#ifndef MESQ_H
#define MESQ_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>   //getpid
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <sys/time.h>

#define MAXMSGDATA	(4096-16)
#define FILENAME 256

extern int errno;

typedef struct my_msg {
  long type;      //client: type number & priority,
                  // server: client id
  int length;     //length of msg
  int sender_id;  //client id to return msg
  char msg[MAXMSGDATA];     //server-side : contain msg
                            //client-side:  file name.
  int end_flag;             //check if this file is end or not
}qMsg;

int recv_msg(int, int, struct my_msg *);
int send_msg(int, struct my_msg *);
void timegap(struct timeval, struct timeval);
void fatal (char *);


#endif
