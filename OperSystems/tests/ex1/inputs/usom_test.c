#include <stdlib.h>
#include "syscall_tags.h"
#include <assert.h>

#define size 7

void printA(int *ar,int len) {
  int i;
  printf("\n");
  for (i=0;i<len;i++) {
    printf(" %d/%d ",ar[i],gettag(ar[i]));
  }
  printf("\n");
}

int rest_empty(int *ar,int begin,int end) {
  int i;
  for (i=begin;i<end;i++) {
    if (ar[i]!=-5) {
      return 0;
    }
  }
  return 1;
}

void check1 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[2]&&ar[1]==ctrl[3]&&rest_empty(ar,2,size)){ 
    printf ("TEST 1 PASSED\n");
    return;
  }
   printf ("TEST 1 FAILED\n");
}

void check2 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[3]&&ar[1]==ctrl[4]&&rest_empty(ar,2,size)) {
    printf ("TEST 2 PASSED\n");
    return;
  }
   printf ("TEST 2 FAILED\n");
}

void check3 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[2]&&ar[1]==ctrl[3]&&ar[2]==ctrl[4]&&rest_empty(ar,3,size)) {
    printf ("TEST 3 PASSED\n");
    return;
  }
  printf ("TEST 3 FAILED\n");
}

void check4 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[3]&&ar[1]==ctrl[4]&&ar[2]==ctrl[5]&&rest_empty(ar,3,size)) {
    printf ("TEST 4 PASSED\n");
    return;
  }
  printf ("TEST 4 FAILED\n");
}

void check5 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[2]&&ar[1]==ctrl[4]&&ar[2]==ctrl[5]&&rest_empty(ar,3,size)) {
    printf ("TEST 5 PASSED\n");
    return;
  }
  printf ("TEST 5 FAILED\n");
}

void check6 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[2]&&ar[1]==ctrl[3]&&ar[2]==ctrl[5]&&rest_empty(ar,3,size)) {
    printf ("TEST 6 PASSED\n");
    return;
  }
  printf ("TEST 6 FAILED\n");
}

void check7 (int *ar) {
  if (gettag(ar[0])==133&&gettag(ar[1])==132&&gettag(ar[2])==131&&
      gettag(ar[3])==64&&gettag(ar[4])==66&&gettag(ar[5])==65) {
    printf("TEST 7 PASSED\n");
    return;
  }
  printf ("TEST 7 FAILED\n");
}
  

int cmp_ar (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  int res=1;
  int i;
  for (i=0;i<size;i++) {
    if (ar[i]!=ctrl[i]) {
      res=0;
      return 0;
    }
  }
  return 1;
}

void check8(int *ar,int *ctrl) {
  int res=cmp_ar(ar,ctrl);
  if (res) {
    printf ("TEST 8 PASSED\n");
    return;
  }
  printf ("TEST 8 FAILED\n");
}


void check9 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[0]&&ar[1]==ctrl[2]&&ar[2]==ctrl[3]&&ar[3]==ctrl[4]&&rest_empty(ar,4,size)) {
    printf ("TEST 9 PASSED\n");
    return;
  }
  printf ("TEST 9 FAILED\n");
}


void check10 (int *ar,int* ctrl) {
  int res=cmp_ar(ar,ctrl);
  if (res&&gettag(ar[0])==134&&gettag(ar[1])==133&&gettag(ar[2])==66&&
      gettag(ar[3])==66&&gettag(ar[4])==65&&gettag(ar[5])==-1) {
    printf("TEST 10 PASSED\n");
    return;
  }
  printf ("TEST 10 FAILED\n");
}

void check11 (int *ar,int *ctrl) {
  printA(ar,size);
  printA(ctrl,size);
  if (ar[0]==ctrl[0]&&ar[1]==ctrl[1]&&ar[2]==ctrl[4]&&rest_empty(ar,3,size)) {
    printf ("TEST 11 PASSED\n");
    return;
  }
  printf ("TEST 11 FAILED\n");
}

void initA(int* ar,int len) {
  int i;
  for (i=0;i<size;i++) {ar[i]=-5;}
}

int main () {

  if (fork()) {
    return 0;
  }
  sleep(1);
  settag(getpid(),128);
  //  makegoodprocesses();
  int* control=(int*)malloc(sizeof(int)*size);
  int i;
  initA(control,size);
  control[0]=0;
  control[1]=1;
  control[2]=getpid();
  int son1;
  if (!(son1=fork())) {
    control[3]=getpid();
    int* ar=(int*)malloc(sizeof(int)*size);
    initA(ar,size);
    getgoodprocesses(ar,size);
    check1(ar,control); //TEST 1
    sleep(6); //7
    printf("Process 1 died :( \n");
    return 0;
  }
  sleep(1); //1
  control[3]=son1;
  if (!(son1=fork())) {
    control[4]=getpid();
    int* ar=(int*)malloc(sizeof(int)*size);
    initA(ar,size);
    getgoodprocesses(ar,size);
    check2(ar,control); //TEST 2
    sleep(2); //3
    initA(ar,size);
    getgoodprocesses(ar,size);
    check3(ar,control); //TEST 3
    while (gettag(control[2])!=-1){}
    sleep(1);
    printf("Process 2 died :( \n");
    return 0;
  }
  control[4]=son1;  
  sleep(1); //2
  settag(getpid(),129);
  sleep(3);//5
  if (!(son1=fork())) {
    control[5]=getpid();
    int* ar=(int*)malloc(sizeof(int)*size);
    initA(ar,size);
    getgoodprocesses(ar,size);
    check4(ar,control); //TEST 4
    sleep(3); //8
    initA(ar,size);
    getgoodprocesses(ar,size);
    check5(ar,control); //TEST 5
    control[3]=control[4];control[4]=control[5];control[5]=-5;control[6]=-5; //fixing control to remove dead process
    if (!(son1=fork())) {
	control[5]=getpid();
	int* ar=(int*)malloc(sizeof(int)*size);
	initA(ar,size);
	sleep(1); //9
	getgoodprocesses(ar,size);
	check6(ar,control); //TEST 6
	makegoodprocesses();
	check7(control); //TEST7
	initA(ar,size);
	getgoodprocesses(ar,size);
	check8(ar,control); //TEST 8
	while(gettag(control[2])!=-1){}
	control[2]=control[3];control[3]=control[4];control[4]=control[5];control[5]=-5; //fixing control to remove dead process
        initA(ar,size);
	getgoodprocesses(ar,size);
	check9(ar,control); //TEST 9
	makegoodprocesses();
	initA(ar,size);
	getgoodprocesses(ar,size);
	check10(ar,control); //TEST 10
	while(gettag(control[2])!=-1){}
	printf("Process 4 died :( \n");
	return 0;
      }
    control[5]=son1;
    if (settag(son1,65)!=0) {
      printf("FAILED TO SET TAG, COME ON...\n");
    }
    while(gettag(control[2])!=-1){}
    wait(NULL);
    initA(ar,size);
    getgoodprocesses(ar,size);
    check11(ar,control);
    printf("Process 3 died :( \n");
    printf("test over, click Enter to do smt\n");
    return 0;
  }
  control[5]=son1;
  wait(NULL);
  sleep(5); //about 11
  settag(control[4],66);
  printf("Process 0 died :( \n");
  //printf("test over, click Enter to do smt\n");
  return 0;
}
