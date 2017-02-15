/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: transfer.c
--
-- PROGRAM:     File Transfer server
--
-- FUNCTIONS:	int recv_msg(int, int, struct my_msg *);
--              int send_msg(int, struct my_msg *);
--
--
-- DATE: 		Jan 26, 2016
--
-- REVISIONS:
--
-- DESIGNER:	Eunwon Moon
--
-- PROGRAMMER: 	Eunwon Moon
--
-- NOTES:
--		This file is file transfer main communication function. Both server-side
--    and cleint-side using this functions to send the message and receive
--    message through the message queue.
--
----------------------------------------------------------------------------------------------------------------------*/
#include "mesq.h"

/*---------------------------------------------------------------------------------------
--FUNCTION: 	send_msg
--
--DATE :  		Jan 26, 2016
--
--REVISIONS :
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE :   send_msg(int msq_id, struct my_msg *rcv_msg){
--                       int msq_id   : message queue id to communicate
--                       struct my_msg *rcv_msg: the message form to send back
--RETURNS : 	int
--					the number of byte to read
--
--NOTES :	This method is to receive message through message queue
--        using msgrcv function. Using type parameter, only get same type message
--        from the message queue.
--
---------------------------------------------------------------------------------------*/

int recv_msg(int msq_id, int type, struct my_msg *rcv_msg){
  int rcvq =0;
  struct my_msg msgbuf;

  if((rcvq = msgrcv(msq_id, &msgbuf, sizeof(struct my_msg), type, 0))<0)
  {
    perror ("TRANSFER:msgrcv failed!");
    return -1;
  }

  *rcv_msg = msgbuf;
  return rcvq;
}

/*---------------------------------------------------------------------------------------
--FUNCTION: 	send_msg
--
--DATE :  		Jan 26, 2016
--
--REVISIONS :
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE :   send_msg(int msq_id, struct my_msg *snd_msg){
--                       int msq_id   : message queue id to communicate
--                       struct my_msg *snd_msg: the message form to send back
--RETURNS : 	int
--
--NOTES :	This method is to send message through message queue
--        using msgsnd function.
--
---------------------------------------------------------------------------------------*/

int send_msg(int msq_id, struct my_msg *snd_msg){
  int rcvq;

  if((rcvq = msgsnd(msq_id, snd_msg, sizeof(struct my_msg)-sizeof(long), 0))<0)
  {
    perror(strerror(errno));
    perror ("send_msg function!! msgsnd failed!");
    return -1;
  }
  return rcvq;
}
