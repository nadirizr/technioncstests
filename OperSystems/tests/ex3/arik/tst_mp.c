
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mp_interface.h"

#define REG 0
#define UNREG 1
#define SEND 3
#define BROADCAST 4
#define RECV 5
#define THREAD_NUM 100
pthread_mutex_t lock;
barrier_t *bar;


typedef struct thread_param
{
  pthread_t* threads_array;
  context_t* con;
  int id;
}thread_param;



#define NUM_OPERATIONS 1000
context_t* con;
pthread_t t[THREAD_NUM];
char* msgs[5];
int param[THREAD_NUM];
int wating_for_recv_sync[THREAD_NUM];  //if thread a waits for other thread to receieve thread a's msg then 1
int waiting_for_send[THREAD_NUM];  // if thread a waits for other thread to send thread a a mmsg then 1
pthread_t waiting_for_thread[THREAD_NUM]; //a is waiting for waiting_for_thread[a] to read a's msg
int sync_broadcast_active;

int get_thread_id(pthread_t thread_num)
{
  int j;
  for(j=0;j<THREAD_NUM;j++)
    if(t[j]==thread_num)
      return j;
    assert(!"no thread");
  return -1;
}

int check_circle_send_sinc(int start)
{
  int j=start;
  printf("%d(%ld wf %ld)",j,t[j],waiting_for_thread[j]);
  while(waiting_for_thread[j] != t[start] && ((int)waiting_for_thread[j]) != -1)
  {
    j=get_thread_id(waiting_for_thread[j]);
    printf("->%d(%ld)",j,waiting_for_thread[j]);
  }
  printf("\n");
  if(((int)waiting_for_thread[j]) == -1) //no circle
    return 0; 
  else
    return 1;
}

void* rw_thread(void* data)
{
  int j=0;
  int msglen=0;
  int id=*(int*)data;
  int ok_to_send_sync=1;
  int jj;
  //printf("%d\n",id);
  char* buf=(char*)malloc(sizeof(char)*13);
  *buf='\0';
  int rn1,rn2,flg;
  mp_register(con);
  mp_barrier(con,bar);
  sleep(5);
  //rw
  for(j=0;j<NUM_OPERATIONS;j++)
  {
    
    rn1=random()%3;
    rn2=random()%THREAD_NUM;
    flg=random()%1000;
    if(flg<900)  //send less sync and urgent
      flg=0;
    if(rn1==0 && ((int)t[rn2]) != -1)  //send
    {
      if(flg%4 == 2  || flg%4 == 3)  //send_sync active
      {
	//printf("sending sync msg to %ld",(t)[rn2]);
	wating_for_recv_sync[id]=1;
	waiting_for_thread[id]=(t)[rn2];
	if(check_circle_send_sinc(id)==1 || sync_broadcast_active==1)  //dont send sync
	{
	  wating_for_recv_sync[id]=0;
	  waiting_for_thread[id]=((pthread_t)-1);
	  flg=(flg-2)%2;
	}
	//else
	//{
	//  waiting_for_thread[id]=(t)[rn2]broadcast;
	//}
      }
     // printf("sending msg to %ld\n",(t)[rn2]);
      
           if(flg%4==1)
       printf("urgent msg was sent to %ld\n",(t)[rn2]);
     
          if(flg%4==2)
       printf("sync msg was sent to %ld\n",(t)[rn2]);
     
          if(flg%4==3)
       printf("urgent and sync msg was sent to %ld\n",(t)[rn2]);
     
          if(flg%4==0)
       printf("regular msg was sent to %ld\n",(t)[rn2]);
       
      mp_send( con,&(t)[rn2],
	      msgs[(rn2)%5],strlen(msgs[rn2%5])+1,flg%4);
      wating_for_recv_sync[id]=0;
      waiting_for_thread[id]=((pthread_t)-1);
    }
    
    if(rn1==1 && ((int)t[rn2]) != -1)  //broadcast
    {
      if(flg%4 == 2  || flg%4 == 3)  //send_sync active
      {
// 	printf("checking if ok to send sync\n");
	int prev_sync_broadcast_active=sync_broadcast_active;
	ok_to_send_sync=1;
	sync_broadcast_active=1;
	wating_for_recv_sync[id]=1;
	//pthread_mutex_lock(&lock);
	for(jj=0;jj<THREAD_NUM;jj++)
	{
	  if(wating_for_recv_sync[jj] == 1 && jj != id)
	  {
	    printf("1");
	    ok_to_send_sync=0;
	  }
	  else
	    printf("0");
	}
	printf("\n");
	if(ok_to_send_sync == 0)
	{ 
	  printf("not ok to send sync\n");
	  sync_broadcast_active=prev_sync_broadcast_active;
	  wating_for_recv_sync[id]=0;
	  flg=(flg-2)%4;  // dont send sync	  
	}
	//else
	//{
	//  waiting_for_thread[id]=(t)[rn2];
	//}
	//pthread_mutex_unlock(&lock);
      }
     if(flg%4==1)
       printf("urgent broadcast msg was sent\n");
     
          if(flg%4==2)
	  {
	    printf("sync broadcast msg was sent\n");
	  } 
	    
	    
          if(flg%4==3)
       printf("urgent and sync broadcast msg was sent\n");
     
          if(flg%4==0)
       printf("regular broadcast msg was sent\n");
     
     mp_broadcast(con,
		   msgs[rn2%5],strlen(msgs[rn2%5])+1,flg%4);
      wating_for_recv_sync[id]=0;
      if(flg%4 ==2 || flg%4 == 3)
	sync_broadcast_active=0;
    }
    
    if(rn1==2 && ((int)t[rn2]) != -1)  //recv
    {
      if(flg%2 != 0 )  //recv_sync active
      {
	waiting_for_send[id]=1;
      }
      mp_recv(con,buf,13,&msglen,flg%2);
      waiting_for_send[id]=0;
      if(msglen>0)
	printf("%ld received msg %s\n",t[rn2],buf);
    }
    
  }
 msglen=1;
 while(msglen != 0)  //release all threads waiting for this thread to recv
   mp_recv(con,buf,13,&msglen,0);
  
  free(buf);
  mp_unregister(con);
  param[id]=-1;
}




int main()
{
    int r=1;
    srand(time(NULL));
    pthread_mutex_init(&lock,NULL);
    msgs[0]="abcd";
    msgs[1]="string a";
    msgs[2]="12";
    msgs[3]="";
    msgs[4]="123456789abc";
    for(r=0;r<1;r++)
    {
      con=mp_init();
      bar=mp_initbarrier(con,THREAD_NUM);

      int j;
      int a;
      sync_broadcast_active=0;
      
      for(j=0;j<THREAD_NUM;j++)
      {
	  t[j]=((pthread_t)-1);
	  wating_for_recv_sync[j]=0;
	  waiting_for_send[j]=0;
	  waiting_for_thread[j]=((pthread_t)-1);
	  param[j]=0;
      }
      
      //create threads
      for(j=0;j<THREAD_NUM;j++)
      {
	param[j]=j;
	pthread_create(&t[j],NULL,rw_thread,(void*)(&(param[j])));
      }
      //wait until t array initialized
     //for(j=0;j<THREAD_NUM;j++)
     //  printf("%ld  ",t[j]);
     // fflush(stdout);
     
      mp_barrier(con,bar);
      int done=0;
      int c=0;
      char* buf=(char*)malloc(sizeof(char)*13);
      *buf='\0';
      int msglen;
      mp_register(con);
      while(done == 0)
      {
	sleep(1);
	msglen=1;
	while(msglen != 0)  //release all threads waiting for this thread to recv
	  mp_recv(con,buf,13,&msglen,0);
	done=1;
	c=0;
	for(j=0;j<THREAD_NUM;j++)
	{
	  if(param[j] != -1)
	  {
	    c++;
	    done=0;
	  }
	  if(waiting_for_send[j] > 0) // if there is a thread that waits for a msg, send him "abc"
	  {       
	   mp_send( con,&t[j],"abc",4,0);
	    waiting_for_send[j]=0;
	    printf("sending to locked thread %ld \n",t[j]);
	  }
	 }
	 printf("number of running threads %d\n",c);
      }
      free(buf);
      mp_unregister(con);
      printf("abc\n");
      //wait until all threads finished
      for(j=0;j<THREAD_NUM;j++)
	  if(((int)t[j]) != -1)
	    pthread_join(t[j],(void*)(&a));
      
      printf("\n\n");
      mp_destroybarrier(con,bar);
      mp_destroy(con);
  }
  pthread_mutex_destroy(&lock);
  //int* abc=(int*)malloc(5);
  return 0;
}
