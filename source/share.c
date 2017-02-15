/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: share.c
--
-- PROGRAM:     File Transfer server
--
-- FUNCTIONS:	int recv_msg(int, int, struct my_msg *);
--              int send_msg(int, struct my_msg *);
--              void fatal (char *s)
--				timegap(struct timeval begin, struct timeval end
--
-- DATE: 		Jan 26, 2016
--
-- REVISIONS:	Feb 02, 2016	- add calculate transfer time
--
-- DESIGNER:	Eunwon Moon
--
-- PROGRAMMER:  Eunwon Moon
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
    switch(errno){
      case EIDRM:
	  case EPERM:
        fatal("\n\n### SERVER CLOSE MESSAGE QUEUE. ###\n");
      default:
        fatal("\n\n### ERROR to receive message. ###\n");
    return -1;
    }
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



/*---------------------------------------------------------------------------------------
--FUNCTION: 	fatal
--
--DATE :  		Jan 26, 2016
--
--REVISIONS :
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE : 	void fatal (char *s)
--							char *s  : Error message to show
--
--RETURNS : 	void
--
--NOTES :		This function is to display error message when detected error
--
---------------------------------------------------------------------------------------*/
void fatal (char *s)
{
  fflush(stdout);
	perror(s);		//Error Message Print
	exit(1);
}





/*---------------------------------------------------------------------------------------
--FUNCTION: 	timegap
--
--DATE :  		Feb 02, 2016
--
--REVISIONS :
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE : 	void timegap(struct timeval begin, struct timeval end)
--						      struct timeval begin : store starting time info
--              		      struct timeval end   : store ending time info
--
--RETURNS : 	void
--
--NOTES :		This function is to calculate the time duration of sending and receiviting
--
---------------------------------------------------------------------------------------*/
void timegap(struct timeval begin, struct timeval end){
  float time_spent;

  time_spent = (float)(end.tv_sec - begin.tv_sec)*1000 + (end.tv_usec - begin.tv_usec)/1000;
  printf("\t %.2fms\n\n", time_spent);

}
