/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: mainSV.c
--
-- PROGRAM:     File Transfer server
--
-- FUNCTIONS:	  main(int argc, char *argv[])
--              void read_queue(int);
--              void sv_answer(int, struct my_msg *, int);
--
--
-- DATE: 		    Jan 26, 2016
--
-- REVISIONS:   Jan 29, 2016
--
-- DESIGNER:	  Eunwon Moon
--
-- PROGRAMMER: 	Eunwon Moon
--
-- NOTES:
--		This file is file transfer server-side. It start message queue using keyval
--    and make a message queue to share the data. And read a message queue from
--    client side. To handle priority, the type of message queue receive value
--    is '-5', so there will be 5 level of priority.
--    When the program read a queue properly, open child process to execute
--    file transfer.
--    In child tranfer, read data from client and if there is file, send file.
--    Otherwise send error message back to client.
--    The message is back to client using client process id as a type.
--
----------------------------------------------------------------------------------------------------------------------*/

#include "server.h"

void mqstat_print (int mqid, struct msqid_ds *mstat);

/*---------------------------------------------------------------------------------------
--FUNCTION: 	 main
--
--DATE :  		 Jan 26, 2016
--
--REVISIONS :  Jan 29, 2016      -  seperate message queue read part
--
--DESIGNER : 	 Eunwon Moon
--
--PROGRAMMER : Eunwon Moon
--
--INTERFACE : int main(void)
--
--RETURNS : 	int
--
--NOTES :	The main function to start this program.
--		In this function include all preparence to use message queue
--    create and delete message queue using current directory and 'a'.
--
---------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  key_t mkey;     //key number

  parentId = getpid();

  signal(SIGINT, sig_handler);
  signal(SIGKILL, sig_handler);
  /*---- Get message queue identifier ------*/
  mkey = ftok(".", 'a');
  if ((msq_id = msgget (mkey, 0660|IPC_CREAT)) < 0)
  {
    fatal ("msgget failed!");
  }

  read_queue(msq_id);

  return 0;

}



/*---------------------------------------------------------------------------------------
--FUNCTION: 	 read_queue
--
--DATE :  		 Jan 26, 2016
--
--REVISIONS :  Jan 29, 2016      -  seperate message queue read part
--
--DESIGNER : 	 Eunwon Moon
--
--PROGRAMMER : Eunwon Moon
--
--INTERFACE : void read_queue(int msq_id)
--                            int msq_id : message queue to communicate
--RETURNS : 	void
--
--NOTES :	The main function to read message queue and make child processor.
--		    If the message is read without error, create child processor
--        and call the sv_answer function in child processor.
--        The parent processor keep working to read message queues.
--
---------------------------------------------------------------------------------------*/
void read_queue(int msq_id)
{
  struct my_msg    *msg;
  int rcvq, pid;
  msg = (struct my_msg *)malloc (sizeof(struct my_msg));

  while(1){
    if((rcvq = recv_msg(msq_id, PRIORITY, msg)) > 0)
    {
      pid = fork();
      switch(pid){
        case -1 :
          fatal("ERROR");
        case 0 : //child process
          if(msg->end_flag == 1)
            file_trans(msq_id, msg);
          else{
            stop_request(msq_id, msg);
            printf("kill process %d\n", msg->sender_id);
          }
      }
   }
 }

   free(msg);
}


/*---------------------------------------------------------------------------------------
--FUNCTION: 	 file_trans
--
--DATE :  		 Jan 26, 2016
--
--REVISIONS :  Jan 27, 2016      -  spirit file sender part
--
--DESIGNER : 	 Eunwon Moon
--
--PROGRAMMER : Eunwon Moon
--
--INTERFACE : file_trans(int msq_id, struct my_msg *rcv_msg)
--                       int msq_id   : message queue to communicate
--                       struct my_msg *rcv_msg: file request message from client
--RETURNS : 	void
--
--NOTES :	The main function to read message queue and make child processor.
--		    If the message is read without error, create child processor
--        and call the sv_answer function in child processor.
--        The parent processor keep working to read message queues.
--
---------------------------------------------------------------------------------------*/
void file_trans(int msq_id, struct my_msg *rcv_msg){
  int nbyte;
  FILE *fstream;                //file stream to read
  struct my_msg new_msg;        //the message to send back
  char filename[FILENAME];       //temporal location to store file name
  struct timeval begin, end;


  //make new message to send back to client
  new_msg.type = rcv_msg->sender_id;
  new_msg.sender_id = getpid();
  new_msg.length = rcv_msg->length;
  new_msg.end_flag = 0;


  //make file name to read
  sprintf(filename,"./%s", rcv_msg->msg);

  printf("The Client %d request file \'%s\' priority %li\n",
                      rcv_msg->sender_id, rcv_msg->msg, rcv_msg->type);
  //open file and check if the file is exist or not.
  if((fstream = fopen(filename, "rb")) == NULL){
    sprintf(new_msg.msg, "There is no file : %s\n", rcv_msg->msg); //ERROR MSG TO SEND
	  new_msg.end_flag = 2;										   //ERROR MSG FLAG

    if(send_msg(msq_id, &new_msg) == -1)
        fatal ("msgsnd failed!");
    fatal("file open fail");
    return ;

  }

  gettimeofday(&begin, NULL);


  //if previous message was not a last message and if the data is read,
  // keep working
  while(new_msg.end_flag != 1
    &&(nbyte = fread((void *)new_msg.msg, sizeof(char), rcv_msg->length, fstream))>0){

  //if the data is end, or less than MSGSIZE, change end data flag
    if(nbyte < rcv_msg->length || feof(fstream)){
      new_msg.end_flag = 1;
    }
      //send message using send_msg function in transfer.c
    if(send_msg(msq_id, &new_msg) == -1)
    {
      fatal ("msgsnd failed!");
      break;
    }
    memset(new_msg.msg, '\0', rcv_msg->length);
  }


  gettimeofday(&end, NULL);
  memset(filename, '\0', FILENAME);


  if(new_msg.end_flag ==1){
    printf("\t- FILE SENT SUCCESSFULLY to [%d] priority: %li\n",
            rcv_msg->sender_id, rcv_msg->type);
    timegap(begin, end);
  }

  fclose(fstream);
}






/*---------------------------------------------------------------------------------------
--FUNCTION: 	stop_request
--
--DATE :  		Feb 02, 2016
--
--REVISIONS :	Feb 02, 2016      -  seperate message queue read part
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE : 	void stop_request(int msq_id, struct my_msg *rcv_msg)
--                      int msq_id             :message queue identifier
--                      struct my_msg *rcv_msg :message of information to ask stop
--RETURNS : 	void
--
--NOTES :
--    The function to request to stop sending when cleint request.
--    when client quit the program by pressing 'Ctrl+C', kill the process
--    to block the sending and receiving the all remaining messages from the queue.
--
---------------------------------------------------------------------------------------*/
void stop_request(int msq_id, struct my_msg *rcv_msg){
  struct my_msg *buf;        //the message to send back

  printf("pid to kill : %d in pid: %d", rcv_msg->sender_id, getpid());
  //allocate memory for receving structure
  kill(rcv_msg->sender_id, SIGKILL);
  buf = (struct my_msg *)malloc (sizeof(struct my_msg));

  while((recv_msg(msq_id, rcv_msg->end_flag, buf))>0 || buf->end_flag == 1){
  }
  free(buf);

}



/*---------------------------------------------------------------------------------------
--FUNCTION: 	sig_handler
--
--DATE :  		Jan 10, 2016
--
--REVISIONS :  Jan 19, 2016   - parents process wait until child processes
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER :Eunwon Moon
--
--INTERFACE : void sig_handler (int sig_num)
--								int sig_num : the detected signal number
--
--RETURNS : 	void
--
--NOTES :
--		This function is to handle signal detection.
-- 		after detect signal when is defined at the main, initialize signal to NULL
-- 		all 3 processes(input, output and translate) will kill using same signal
--		which is get as a parameter.
--		Parent processes will wait until child processes are killed.
--		also, if it is parent process, they system changes recovered
--		at the end of the program
--
---------------------------------------------------------------------------------------*/
void sig_handler (int sig_num)
{
  int status, status2;
  int pid;

	if(sig_num == SIGINT){
    if(getpid() == parentId){
      while ((pid = wait(&status)) > 0) {
        printf("Child %lu killed by signal\n", (unsigned long)pid);
      }
      printf("parent finish\n");
      close_queue(msq_id);
	 }
   signal(SIGINT , NULL);
   kill(getpid(), SIGTERM);
  }

  else if(sig_num == SIGKILL){
    printf("\n here is pid: %d\n",getpid());
    while ( wait(&status2) > 0) {
      printf("Child %lu killed by signal\n", (unsigned long)getpid());
    }
    signal(SIGKILL , NULL);
  }
}


/*---------------------------------------------------------------------------------------
--FUNCTION: 	close_queue
--
--DATE :  		Feb 02, 2016
--
--REVISIONS :  	Feb 02, 2016      -  seperate message queue read part
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE : 	void close_queue(int msq_id)
--                      int msq_id   :message queue identifier
--
--RETURNS : 	void
--
--NOTES :
--    The function to close message queue before exit the program.
--
---------------------------------------------------------------------------------------*/
void close_queue(int msq_id)
{
  //close queue
  if(msgctl(msq_id, IPC_RMID, NULL) == -1)
    fatal("fail to close message queue.");
}
