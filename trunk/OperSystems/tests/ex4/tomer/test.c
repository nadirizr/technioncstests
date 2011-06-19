#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "vsf.h"
#include <string.h>
#include <stdio.h>
#include "vsf_utils.h"
#include <sys/wait.h>
#include <errno.h>
#include <sched.h>


#define CHILD 120
#define VSF_PER_PRC 50

#define D_NUM CHILD*VSF_PER_PRC
#define MY_MAJOR 253

void halt_func(char *msg) {
  printf("%s\n",msg);
  getchar();
}



#define HALT(MSG) halt_func(MSG)


int main () {
  struct sched_param a;
  a.sched_priority=1;
  sched_setscheduler(getpid(),SCHED_FIFO,&a);
  int retval;
  struct vsf_command_parameters bind_args;
  int i;
  int pid;
  RMMOD;
  INSMOD(D_NUM);
  int child[CHILD];
  char* ctrl="_vsf_controller";
  char *base_reader="_vsf_read_";
  char *base_writer="_vsf_read_";
  char* reader[CHILD];
  char* writer[CHILD];
  char tmp[20];
  for (i=0 ; i < CHILD ; i++) {
    int ret;
    ret=sprintf(tmp,"%s_r_%d",base_reader,i);
    reader[i]=malloc(ret);
    strcpy(reader[i],tmp);
    ret=sprintf(tmp,"%s_w_%d",base_writer,i);
    writer[i]=malloc(ret);
    strcpy(writer[i],tmp);
  }

  MKNOD(ctrl,MY_MAJOR,0);
  for (i=1 ; i<=CHILD ; i++) {
    if (!(pid=fork())) {
      MKNOD(reader[i-1],MY_MAJOR,i*2);
      MKNOD(writer[i-1],MY_MAJOR,i*2 + 1);
      bind_args.read_minor=i*2;
      bind_args.write_minor=i*2+1;
      int ctl_fd=open(ctrl,O_RDONLY);
      int j;
      int read_fd;
      int write_fd;
      int ret_read;
      int ret_write;
      for (j=0; j < VSF_PER_PRC ; j++) {
	retval = ioctl(ctl_fd , VSF_CREATE , &bind_args);
	read_fd=open(reader[i-1],O_RDONLY);
	write_fd=open(writer[i-1],O_WRONLY);
	if ( j+1 == VSF_PER_PRC)
	  break;
	
	retval = ioctl(ctl_fd,VSF_FREE,&bind_args);
      }
      char* buf[10];
      printf("Proccess %d : %d ready!\n",i,getpid());
      if (i==CHILD)
	printf("-Press any key to continue-\n");

      if (i % 2) 
	ret_read=read(read_fd,buf,10);
      else 
	ret_write=write(write_fd,"tomer",6);

      //      retval = ioctl(ctl_fd,VSF_FREE,&bind_args);
      close(read_fd);
      close(write_fd);
      retval = ioctl(ctl_fd,VSF_FREE,&bind_args);
      close(ctl_fd);
      RM(reader[i-1]);
      RM(writer[i-1]);
      return 0;
    }
    child[i-1]=pid;
  }
  getchar();
  DO_SYSTEM("cat /proc/driver/vsf > ./out.txt");
  for (i=0; i<CHILD; i++) {
    kill(child[i],SIGSTOP);
    kill(child[i],SIGCONT);
  }
  while (wait(NULL) != -1);
  RM(ctrl);
  RMMOD;
  
  for (i=0 ; i < CHILD ; i++) {
    free(reader[i]);
    free(writer[i]);
  }
  return 0;
  /*if ((pid=fork())) {
    int read_fd=open(reader,O_RDONLY);
    //    HALT("-0-");
    char buf[8];
    int ret_read=0;
    HALT("-0-");
    kill(pid,SIGSTOP);
    kill(pid,SIGCONT);
    HALT("-1-");
    while (!ret_read) {
      ret_read=read(read_fd,buf,8);
      if (ret_read==0)
	printf("%d Read INTERRUPTED\n",getpid());
    }

    if (ret_read != -1) 
      printf("%d Read %d bytes : %s\n",getpid(),ret_read,buf);
    else 
      printf("%d Read ERROR %d\n",getpid(),errno);

    wait(NULL);
    HALT("-2-");
    close(read_fd);
    retval = ioctl(ctl_fd,VSF_FREE,&bind_args);
    close(ctl_fd); 
    RM(reader);
    RM(writer);
    RM(ctrl);
    return 0;
  }
  
  //  kill(pid,SIGSTOP);
  //  kill(pid,SIGCONT);
  int ret_write=0;
  int write_fd=open(writer,O_WRONLY);
  while (!ret_write) {
    ret_write=write(write_fd,"tomer",6);
    if (ret_write==0)
	printf("%d Write INTERRUPTED\n",getpid());
  }

  if (ret_write != -1) 
    printf("%d Write %d bytes\n",getpid(),ret_write);
  else 
    printf("%d Write ERROR %d\n",getpid(),errno);
  */
  //  wait(NULL);
  //  HALT("-2-");
  //close(write_fd);
  /*  retval = ioctl(ctl_fd,VSF_FREE,&bind_args);
  close(ctl_fd); 
  RM(reader);
  RM(writer);
  RM(ctrl);*/
  //  return 0;
}
