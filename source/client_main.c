/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: mainCL.c
--
-- PROGRAM:     File Transfer server
--
-- FUNCTIONS:	  main(int argc, char *argv[])
--              void file_request(int);
--              void sv_answer(int, struct my_msg *, int);
--
-- DATE: 		Jan 26, 2016
--
-- REVISIONS:   Jan 29, 2016		-- seperate the functions
--				Feb  1, 2016		-- add thread function.
--				Feb  2, 2016		-- add signal to request stop sending to server.
--
-- DESIGNER:	  Eunwon Moon
--
-- PROGRAMMER: 	Eunwon Moon
--
-- NOTES:
--		This file is file transfer client-side. This is working by starting
--    message queue using key value and make a message queue to share the data
--    with sercerside in the same message queue.
--    The user input the file name and priority value between 1-5 and wait.
--    If the Server find the file, the client-side can receive that file,
--    otherwise, just display error message.
--
----------------------------------------------------------------------------------------------------------------------*/

#include "client.h"

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
--    	Open a message queue using a current directory.
--
---------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  key_t mkey;     //key number

  signal(SIGINT, sig_cl_handler);

  if (argc != 2)
  {
    fprintf (stderr, "Usage: ./%s [filename]\n",argv[0]);
    exit(1);
  }

  mkey = ftok(".", 'a');
  if ((msq_id = msgget (mkey, 0660|IPC_CREAT)) < 0)
  {
    fatal("msgget failed!");
    exit(2);
  }

  //send filename to server
  file_request(argv[1]);

  msg_rcv(argv[1]);

  return 0;
}



/*---------------------------------------------------------------------------------------
--FUNCTION: 	file_request
--
--DATE :  		Jan 26, 2016
--
--REVISIONS :  	Jan 28, 2016      -  seperate message queue read part
--             	Jan 29, 2016      -  change my_msg to pointer
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER : 	Eunwon Moon
--
--INTERFACE : 	void file_request(char* filename)
--                            char* filename : filename to receive from server.
--RETURNS : 	void
--
--NOTES :
--		The function is to send the file name that the user want to receive.
--		Before sending message structure, fill the information of client
--		and message information on the structure.
--		Also, read a priority from standard input and calculate message data size
--		depending on priority. Afterward, call the send_msg function to send
--		my_msg structure.
--		** The priority is using for type number and calculationg data length.
--
---------------------------------------------------------------------------------------*/
void file_request(char* filename)
{
  struct my_msg     *qbuf;


  if((qbuf = (struct my_msg *)malloc (sizeof(struct my_msg))) == NULL)
  {
    fatal("malloc");
  }

  //get priority value between 1 to 5;
  do{
    printf("priority level(1:high - 5:low): ");
    scanf("%li", &qbuf->type);
  } while(qbuf->type < 0 || qbuf->type >5);

  //calculate the length of each message depending on prioroty
  qbuf->length = MAXMSGDATA - (qbuf->type - 1) * 800;

  //declare the message
  qbuf->end_flag = 1;
  qbuf->sender_id = getpid();
  strcpy(qbuf->msg, filename);


  if((send_msg(msq_id, qbuf))<0)
  {
    fatal("msgrcv failed!");
  }


  free(qbuf);
  return;
}

/*---------------------------------------------------------------------------------------
--FUNCTION: 	 msg_rcv
--
--DATE :  		Jan 26, 2016
--
--REVISIONS :	Jan 28, 2016      -  seperate message queue read part
--            	Jan 29, 2016      -  change my_msg to pointer
--			 	Feb 01, 2016	  -  add the thread function to read.
--
--DESIGNER :	Eunwon Moon
--
--PROGRAMMER :	Eunwon Moon
--
--INTERFACE :  	msg_rcv(char *filename)
--					   char *filename : the name of file to request to server.
--RETURNS : 	void
--
--NOTES :	The main function to receive message. To receive message,
--		create thread and pass Both the message queue id and filename
--		as a structure. Using join funtion, combine each thread work.
--
---------------------------------------------------------------------------------------*/
void msg_rcv(char *filename){

  pthread_t threadid;

  pthread_create(&threadid, NULL, threadfunc,(void *) filename);
  pthread_join(threadid,NULL);


}

/*---------------------------------------------------------------------------------------
--FUNCTION:		threadfunc
--
--DATE :		Jan 26, 2016
--
--REVISIONS :  	Jan 28, 2016      -  seperate message queue read part
--             	Jan 29, 2016      -  change my_msg to pointer
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER :	Eunwon Moon
--
--INTERFACE :  	*threadfunc(void *filename)
--                     void *filename : filename to save file after receiving
--RETURNS : 	void *
--
--NOTES :
--		The thread function is to receive message from message queue
--		using recv_msg function. Before you receiving open file with new name
--		and receive file until receiving end_flag(1) or error_flag(2).
--		If the error flag is received, delete open file after closing.
--
---------------------------------------------------------------------------------------*/
void *threadfunc(void *filename)
{
  struct my_msg *buf;
  char fname[FILENAME];
  FILE *fstream;
  int rcvq;
  int cnt= 0;

  struct timeval begin, end;

  //allocate memory for receving structure
  buf = (struct my_msg *)malloc (sizeof(struct my_msg));

  //change name to save
  sprintf(fname, "cp_%s", (char *)filename);

  //open file to write.
  if((fstream = fopen(fname, "wb+"))== NULL){
    fatal("file open fail\n");
  }
  buf->end_flag = 0;		        //initialize end_flag

  //If end_flag is not end(1) or no file(2), running the loop to read.
  while(buf->end_flag ==0 || rcvq <0){
    if((rcvq=(recv_msg(msq_id, getpid(), buf)))<0){
        fatal("msgrcv failed!\n");

    }

    if(++cnt == 1)
      gettimeofday(&begin, NULL);   //check send time

	//if the message is arrived more than 0, write the file and display it.
    if(rcvq > 0){
      fwrite((void*)buf->msg, sizeof(char), buf->length, fstream);
      printf("%s",buf->msg);
    }
  }
  gettimeofday(&end, NULL);       //check end time

  fclose(fstream);                // close the file


//  fflush(stdout);
  //if there is no file, delete this file.
  if(buf->end_flag == 2)
    remove(fname);
  else{
    printf("\n Total %d packets", cnt);
    timegap(begin, end);          //if the file end successfully, show the sending time

  }
  free(buf);                      //free the memory


  return NULL;
}



/*---------------------------------------------------------------------------------------
--FUNCTION: 	sig_handler
--
--DATE :  		Jan 10, 2016
--
--REVISIONS :  	Feb  2, 2016   - parents process wait until child processes
--
--DESIGNER : 	Eunwon Moon
--
--PROGRAMMER :	Eunwon Moon
--
--INTERFACE : 	void sig_handler (int sig_num)
--								int sig_num : the detected signal number
--
--RETURNS : 	void
--
--NOTES :
--		This function is to handle signal detection.
-- 		after detect signal when is defined at the main,
--		create new temporal message structure and read one message
--		to get a communication information between server and client.
--		After modifying the structure, send message to server to request block sending.
--		If message sent successfully, initialize signal to NULL and kill process.
--
---------------------------------------------------------------------------------------*/
void sig_cl_handler (int sig_num)
{

	struct my_msg *temp;

    temp = (struct my_msg *)malloc (sizeof(struct my_msg));

    if((recv_msg(msq_id, getpid(), temp))<0){
        fatal("msgrcv failed!\n");
    }
	   temp->type = 1;
     temp->end_flag = temp->sender_id;
	   temp->sender_id = getpid();


    memset(temp->msg,'\0', MAXMSGDATA);

    if((send_msg(msq_id, temp))<0)
      fatal("msgrcv failed!");

    signal(SIGINT , NULL);
    kill(getpid(), SIGTERM);

}
