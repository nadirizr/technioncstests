#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/times.h>
#include <sys/file.h>
#include "hw2_syscalls.h"

#define MONITOR_TASK_CREATED 				1
#define MONITOR_TASK_ENDED 					2
#define MONITOR_TASK_YIELDED 				4
#define MONITOR_TASK_REGULAR_TO_OVERDUE 	8
#define MONITOR_TASK_OVERDUE_RUN_ENOUGH 	16
#define MONITOR_TASK_GOES_TO_WAITING		32
#define MONITOR_TASK_CHANGE_IN_PARAMETERS	64
#define MONITOR_TASK_HIGHER_PRIO_RETURNED	128
#define MONITOR_TASK_TIME_SLICE_ENDED		256

#define IDLE_LOOP_LENGTH 60000000
#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_SHORT		4

#define NEW_LINE(pid)			if(getpid() == pid)					\
									write(fd,"\n",1);

#define TEST(test)		do {										\
								int status;							\
								if(!fork()) 						\
								{									\
									test(); 						\
									while( wait(&status) != -1);	\
									return 0;						\
								}									\
								else 								\
								{									\
									while( wait(&status) != -1); 	\
									print_footer();					\
								}									\
						} while(0)
						
/*#define RUN_TEST(test,pid)		if(getpid() == pid)					\
								{									\
									TEST(test);						\
									int status;						\
									while( wait(&status) != -1); 	\
									if(getpid() == pid)				\
										print_footer();				\
								}									\
								else								\
									return 0;*/
									
#define SECONDS(x) ((int)(x*1000000))
#define SECONDS_IN_MILI(x) ((int)(x*1000))
#define LINE_LENGTH 110

int fd;
char str[LINE_LENGTH + 1];

struct sched_param param;

struct sched_param param_out;

void convert_policy_to_string(int policy,char* str)
{
	if(policy == SCHED_OTHER)
		sprintf(str,"%s","OTHER");
		
	if(policy == SCHED_FIFO)
		sprintf(str,"%s","FIFO");
		
	if(policy == SCHED_RR)
		sprintf(str,"%s","RR");

	if(policy == SCHED_SHORT)
		sprintf(str,"%s","SHORT");
}

void print_header_params(char *header)
{
	sprintf(str,"+------------------------------------------------------------------------------------------------------------+\n");
	write(fd,str,sizeof(str));
	sprintf(str,"|%-108s|\n",header);
	write(fd,str,sizeof(str));
	sprintf(str,"+-----------------------------------------------------------------+---------------+-----------------+--------|\n");	
	write(fd,str,sizeof(str));
	sprintf(str,"|test desc.                                                       |actual ret val |expected ret val |result  |\n");
	write(fd,str,sizeof(str));
	sprintf(str,"+-----------------------------------------------------------------+---------------+-----------------+--------|\n");
	write(fd,str,sizeof(str));	
}

void print_message_params(char *desc,char *actual,char *expected,char *res)
{
	sprintf(str,"|%-65s|%-15s|%-17s|%-8s|\n",desc,actual,expected,res);
	write(fd,str,sizeof(str));		
}

void print_header(char *header)
{
	sprintf(str,"+------------------------------------------------------------------------------------------------------------+\n");	
	write(fd,str,sizeof(str));
	sprintf(str,"|%-108s|\n",header);
	write(fd,str,sizeof(str));
	sprintf(str,"+---------+--------+--------+-----------+-----------+--------------------------------------------------------|\n");	
	write(fd,str,sizeof(str));
	sprintf(str,"|time     |pid     |policy  |rem time   |ovr time   |message                                                 |\n");
	write(fd,str,sizeof(str));
	sprintf(str,"+---------+--------+--------+-----------+-----------+--------------------------------------------------------|\n");
	write(fd,str,sizeof(str));		
}

void print_footer()
{
	sprintf(str,"+------------------------------------------------------------------------------------------------------------+\n");
	write(fd,str,sizeof(str));			
}

void print_message_by_pid(char *msg,int pid)
{
	char policy_str[10];
	int policy = sched_getscheduler(pid);
	int remaining_time = short_query_remaining_time(pid);
	int overdue_time = short_query_overdue_time(pid);	
	convert_policy_to_string(policy,policy_str);
	
	struct tms times_struct;
	int ticks = times(&times_struct);	
		
	if(policy != SCHED_SHORT)
	{
		sprintf(str,"|%-9d|%-8d|%-8s|%-11s|%-11s|%-56s|\n",ticks,pid,policy_str,"N/A","N/A",msg);
		write(fd,str,sizeof(str));
	}
	else if((policy == SCHED_SHORT) && (remaining_time))
	{
		sprintf(str,"|%-9d|%-8d|%-8s|%-11d|%-11d|%-56s|\n",ticks,pid,policy_str,remaining_time,0,msg);	
		write(fd,str,sizeof(str));
	}
	else
	{
		sprintf(str,"|%-9d|%-8d|%-8s|%-11d|%-11d|%-56s|\n",ticks,pid,policy_str,0,overdue_time,msg);	
		write(fd,str,sizeof(str));
	}			
}

void print_message(char *msg)
{
	print_message_by_pid(msg,getpid());
}

void idle_time_overdue_sp(int ms)
{

	int current_timestamp;
	int delta;
	int last_timestamp = short_query_overdue_time(getpid());
	int elapsed_time = 0;
	while(elapsed_time < ms)
	{
		current_timestamp = short_query_overdue_time(getpid());
		delta = current_timestamp - last_timestamp;
		last_timestamp = current_timestamp;
		elapsed_time += delta;
	}
}

void idle_time_regular_sp(int ms)
{

	int current_timestamp;
	int delta;
	int last_timestamp = short_query_remaining_time(getpid());
	int elapsed_time = 0;
	while(elapsed_time < ms)
	{
		current_timestamp = short_query_remaining_time(getpid());
		if(!current_timestamp)
		{
			idle_time_overdue_sp(ms - elapsed_time);
			return;
		}
		
		delta = last_timestamp - current_timestamp;
		last_timestamp = current_timestamp;
		elapsed_time += delta;
	}
}

void idle_time_sp(int ms)
{
	int timestamp = short_query_remaining_time(getpid());
	if(timestamp > 0)
		idle_time_regular_sp(ms);
	else
		idle_time_overdue_sp(ms);
}

void idle_loop(int loops)
{
	int counter;
	long i;
	for(counter = 0; counter < loops; counter++)
	{
		for(i = 0; i < IDLE_LOOP_LENGTH; i++) {}
	}
}

void test_real_time_fifo_vs_real_time_fifo()
{
	print_header("TEST 1 - REAL-TIME FIFO VS. REAL-TIME FIFO:");
	int pids[3];
	param.sched_priority = 99;
	sched_setscheduler(getpid(),SCHED_FIFO,&param);	
	int res = fork();
	pids[0] = res;
	if(res)
	{
		res = fork();
		pids[1] = res;		
		if(res)
		{
			res = fork();
			pids[2] = res;			
			if(res)
			{
				param.sched_priority = 10;
				sched_setscheduler(pids[2],SCHED_FIFO,&param);		

				param.sched_priority = 30;
				sched_setscheduler(pids[0],SCHED_FIFO,&param);	
				
				param.sched_priority = 35;
				sched_setscheduler(pids[1],SCHED_FIFO,&param);	
			
				print_message("1");
			}
			else
			{
				print_message("4");
			}
		}
		else
		{
			print_message("2");
		}
	}
	else
	{
		print_message("3");
	}	
}

void test_real_time_fifo_vs_other()
{
	print_header("TEST 2 - REAL-TIME FIFO VS. OTHER: (FIFO SHOULD DIE BEFORE OTHER)");
	int res = fork();
	if(!res)
	{
		print_message("2");
	}
	else
	{
		param.sched_priority = 50;
		sched_setscheduler(getpid(),SCHED_FIFO,&param);	
		sched_yield();
		print_message("1");
	}
}

void test_real_time_rr_vs_other()
{
	print_header("TEST 3 - REAL-TIME RR VS. OTHER:");
	int res = fork();
	if(!res)
	{
		print_message("2");
	}
	else
	{
		param.sched_priority = 50;
		sched_setscheduler(getpid(),SCHED_RR,&param);	
		sched_yield();
		print_message("1");
	}
}

void test_real_time_fifo_vs_short()
{
	print_header("TEST 4 - REAL-TIME FIFO VS. REGULAR SHORT:");
	int res = fork();
	if(res)
	{
		param.sched_priority = 500;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);	
		param.sched_priority = 50;
		sched_setscheduler(res,SCHED_FIFO,&param);	
		print_message("2");
	}
	else
	{
		sched_yield();
		print_message("1");
	}
}

void test_overdue_sp_vs_other()
{
	print_header("TEST 5 - OVERDUE SHORT VS. OTHER:");	
	int res = fork();
	if(res)
	{
		param.sched_priority = 50;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);	
		idle_time_sp(SECONDS_IN_MILI(1));
		print_message("2");
	}
	else
	{
		sched_yield();
		print_message("1");
	}
}

void test_regular_sp_vs_other()
{
	print_header("TEST 6 - REGULAR SHORT VS. OTHER:");		
	int res = fork();
	if(res)
	{
		param.sched_priority = 600;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);	
		idle_time_sp(SECONDS_IN_MILI(0.3));
		print_message("1");	
	}
	else
	{
		print_message("2");
	}
}

void test_regular_sp_vs_regular_sp()
{
	print_header("TEST 7 - REGULAR SHORT VS. REGULAR SHORT:");	
	int res = fork();
	if(res)
	{
		param.sched_priority = 3000;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);
		print_message("1");
		idle_time_sp(SECONDS_IN_MILI(2));
		print_message("2");
		usleep(SECONDS(1));
		print_message("4");
		print_message("5");
	}
	else
	{
		param.sched_priority = 3000;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);
		print_message("3");
		idle_time_sp(SECONDS_IN_MILI(2));
		print_message("6");
	}
}



void test_regular_sp_vs_overdue_sp()
{
	print_header("TEST 8 - REGULAR SHORT VS. OVERDUE SHORT:");	
	int res = fork();
	if(res)
	{
		param.sched_priority = 1000;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);
		print_message("1");
		idle_time_sp(SECONDS_IN_MILI(1.5));
		print_message("5");
	}
	else
	{		
		param.sched_priority = 5000;
		sched_setscheduler(getpid(),SCHED_SHORT,&param);
		print_message_by_pid("2",getppid());
		print_message("3");
		idle_time_sp(SECONDS_IN_MILI(2));
		print_message("4");
	}
}

void test_regular_sp_vs_regular_sp_with_fork()
{
	print_header("TEST 9 - REGULAR SHORT VS. REGULAR SHORT + FORK:");	
	param.sched_priority = 10000;
	sched_setscheduler(getpid(),SCHED_SHORT,&param);
	print_message("1");
	int res = fork();
	if(res)
	{
		print_message("4");
		res = fork();
		if(res)
		{
			print_message("9");
			idle_time_sp(SECONDS_IN_MILI(2));
			print_message("13");			
		}
		else
		{
			print_message("5");
			idle_time_sp(SECONDS_IN_MILI(1));
			print_message("8");						
			usleep(SECONDS(1));	
			print_message("11");				
			print_message("12");				
		}
	}
	else
	{
		print_message("2");
		idle_time_sp(SECONDS_IN_MILI(3));
		print_message("3");			
		usleep(SECONDS(0.3));
		print_message("6");
		idle_time_sp(SECONDS_IN_MILI(1));
		print_message("7");		
		usleep(SECONDS(2));
		print_message("10");			
	}
}

void test_regular_sp_sorting()
{
	print_header("TEST 10 - TESTING REGULAR SHORT PROCESSES SORTING (REMAINING TIME SHOULD BE SORTED IN ASCENDING ORDER):");
	int pids[6];
	int res = fork();
	if(res)
	{
		pids[0] = res;
		res = fork();
		if(res)
		{
			pids[1] = res;		
			res = fork();
			if(res)
			{
				pids[2] = res;				
				res = fork();
				if(res)
				{
					pids[3] = res;			
					res = fork();
					if(res)
					{
						pids[4] = res;		
						res = fork();
						if(res)
						{
								pids[5] = res;		
								param.sched_priority = 40;
								sched_setscheduler(getpid(),SCHED_FIFO,&param);
								
								param.sched_priority = 100;
								sched_setscheduler(pids[0],SCHED_SHORT,&param);		
								param.sched_priority = 10000;
								sched_setscheduler(pids[1],SCHED_SHORT,&param);	
								param.sched_priority = 2000;
								sched_setscheduler(pids[2],SCHED_SHORT,&param);	
								param.sched_priority = 500;
								sched_setscheduler(pids[3],SCHED_SHORT,&param);	
								param.sched_priority = 4000;
								sched_setscheduler(pids[4],SCHED_SHORT,&param);	
								param.sched_priority = 2000;
								sched_setscheduler(pids[5],SCHED_SHORT,&param);									
	
						}
						else
						{
							print_message("#5 child is dead!");					
						}	
					}
					else
					{
						print_message("#5 child is dead!");					
					}
				}
				else
				{	
					print_message("#4 child is dead!");				
				}			
			}
			else
			{
				print_message("#3 child is dead!");			
			}		
		}
		else
		{		
			print_message("#2 child is dead!");		
		}
	}
	else
	{
		print_message("#1 child is dead!");
	}
}


void test_overdue_processes()
{
	print_header("TEST 11 - TESTING OVERDUE FORKING RULES (OVERDUE TIME SHOULD BE SORTED IN ASCEDNING ORDER):");	
	param.sched_priority = 20;
	sched_setscheduler(getpid(),SCHED_SHORT,&param);
	idle_time_sp(SECONDS_IN_MILI(5.02));	
	print_message("Father started!");	
	int res = fork();
	if(res)
	{
		idle_time_sp(SECONDS_IN_MILI(0.1));	
		res = fork();
		if(res)
		{	
			idle_time_sp(SECONDS_IN_MILI(0.1));				
			res = fork();
			if(res)
			{		
				idle_time_sp(SECONDS_IN_MILI(0.1));					
				res = fork();
				if(res)
				{
					idle_time_sp(SECONDS_IN_MILI(0.1));	
					idle_loop(5);
					res = fork();
					if(res)
					{
						idle_time_sp(SECONDS_IN_MILI(0.1));		
					}
					else
					{
						print_message("child #5 started!");					
					}
				}
				else
				{
					print_message("child #4 started!");						
				}			
			}
			else
			{
				print_message("child #3 started!");							
			}		
		}
		else
		{
			print_message("child #2 started!");				
		}
	}
	else
	{
		print_message("child #1 started!");
	}
}


void print_overdue_progress(int i)
{
	param.sched_priority = 100;
	sched_setscheduler(getpid(),SCHED_SHORT,&param);
	idle_time_sp(SECONDS_IN_MILI(1.1));	

	char str[20];
	sprintf(str,"Overdue #%d is running!",i);
	
	print_message(str);
	idle_time_sp(SECONDS_IN_MILI(0.5));	
	print_message(str);
	idle_time_sp(SECONDS_IN_MILI(0.5));	
	print_message(str);
	idle_time_sp(SECONDS_IN_MILI(0.5));	
	print_message(str);
	idle_time_sp(SECONDS_IN_MILI(0.5));	
	print_message(str);	
}

void test_overdue_processes_zigzag()
{
	print_header("TEST 12 - TESTING OVERDUE SHORT PROCESSES ZIGZAG (THEY HAVE TO CHASE EACH OTHER):");	

	int res = fork();
	if(res)
	{
		res = fork();
		if(res)
		{			
			res = fork();
			if(res)
			{				
				res = fork();
				if(res)
				{
					res = fork();
					if(res)
					{	
					}
					else
					{
						print_overdue_progress(5);
					}
				}
				else
				{
					print_overdue_progress(4);			
				}			
			}
			else
			{
				print_overdue_progress(3);
			}		
		}
		else
		{
			print_overdue_progress(2);		
		}
	}
	else
	{
		print_overdue_progress(1);
	}
}

void convert_error(char *error_str)
{
	switch(errno)
	{
		case EINVAL:
			sprintf(error_str,"EINVAL");
			break;
		case EFAULT:
			sprintf(error_str,"EFAULT");
			break;
		case ESRCH:
			sprintf(error_str,"ESRCH");	
			break;
		case EPERM:
			sprintf(error_str,"EPERM");	
			break;		
	}
}

#define PRINT_RESULT(desc,actual,expected,condition)		do 																\
															{																\
																if(condition) 												\
																	print_message_params(desc,actual,expected,"OK"); 		\
																else 														\
																	print_message_params(desc,actual,expected,"FAIL");		\
															} 																\
															while(0)

void test_parameters()
{
	int res;
	int pid = getpid();
	char actual[10];
	
	print_header_params("TESTING PARAMETERS AND RETURN VALUES");
	
	param.sched_priority = 0;
	res = sched_setscheduler(pid,SCHED_SHORT,&param);
	convert_error(actual);
	PRINT_RESULT("Trying to set SCHED_SHORT with Req Time = 0",actual,"EINVAL",((res == -1) && (errno == EINVAL)));
	
	res = sched_getscheduler(pid);
	convert_policy_to_string(res,actual);
	PRINT_RESULT("Checking that policy did not change...",actual,"OTHER",(res == SCHED_OTHER));	
	
	param.sched_priority = 31000;
	res = sched_setscheduler(pid,SCHED_SHORT,&param);
	convert_error(actual);
	PRINT_RESULT("Trying to set SCHED_SHORT with Req Time = 31000",actual,"EINVAL",((res == -1) && (errno == EINVAL)));	
			
	param.sched_priority = 6000;
	res = sched_setscheduler(pid,SCHED_SHORT,&param);	
	sprintf(actual,"%d",res);
	PRINT_RESULT("Trying to set SCHED_SHORT with Req Time = 6000",actual,"0",(res == 0));

	res = sched_getscheduler(pid);
	convert_policy_to_string(res,actual);
	PRINT_RESULT("Checking that policy did change...",actual,"SHORT",(res == SCHED_SHORT));	
	
	res = sched_yield();
	sprintf(actual,"%d",res);	
	PRINT_RESULT("Trying to yield a short process...",actual,"-1",(res == -1));	
		
	param.sched_priority = 50;
	res = sched_setscheduler(pid,SCHED_FIFO,&param);	
	convert_error(actual);	
	PRINT_RESULT("Trying to set a short process to SCHED_FIFO",actual,"EPERM",((res == -1) && (errno == EPERM)));		

	param.sched_priority = 50;
	res = sched_setscheduler(pid,SCHED_RR,&param);	
	convert_error(actual);	
	PRINT_RESULT("Trying to set a short process to SCHED_RR",actual,"EPERM",((res == -1) && (errno == EPERM)));

	param.sched_priority = 0;
	res = sched_setscheduler(pid,SCHED_OTHER,&param);
	convert_error(actual);	
	PRINT_RESULT("Trying to set a short process to SCHED_OTHER",actual,"EPERM",((res == -1) && (errno == EPERM)));

	param.sched_priority = 5700;
	res = sched_setparam(pid,&param);	
	convert_error(actual);	
	PRINT_RESULT("Trying to change the req time of SCHED_SHORT to 5700",actual,"EINVAL",((res == -1) && (errno == EINVAL)));

	res = sched_getparam(pid,&param_out);
	sprintf(actual,"%d",param_out.sched_priority);
	PRINT_RESULT("Checking that requested time did not change...",actual,"6000",((param_out.sched_priority == 6000) && (res == 0)));
	
	param.sched_priority = 7500;
	res = sched_setparam(pid,&param);	
	sprintf(actual,"%d",res);	
	PRINT_RESULT("Trying to change the req time of SCHED_SHORT to 7000",actual,"0",(res == 0));

	res = sched_getparam(pid,&param_out);
	sprintf(actual,"%d",param_out.sched_priority);
	PRINT_RESULT("Checking that requested time did change...",actual,"7500",((param_out.sched_priority == 7500) && (res == 0)));	
}

int main(int argc, char* argv[])
{
	printf("Running test, please wait... (It may take more than one minute).\n");
	
	fd = open("out", O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	int pid = getpid();	

	TEST(test_parameters);	
	NEW_LINE(pid);		
	TEST(test_real_time_fifo_vs_real_time_fifo);
	NEW_LINE(pid);
	TEST(test_real_time_fifo_vs_other);
	NEW_LINE(pid);	
	TEST(test_real_time_rr_vs_other);
	NEW_LINE(pid);
	TEST(test_real_time_fifo_vs_short);
	NEW_LINE(pid);		
	TEST(test_overdue_sp_vs_other);
	NEW_LINE(pid);	
	TEST(test_regular_sp_vs_other);
	NEW_LINE(pid);	
	TEST(test_regular_sp_vs_regular_sp);		
	NEW_LINE(pid);		
	TEST(test_regular_sp_vs_overdue_sp);
	NEW_LINE(pid);		
	TEST(test_regular_sp_vs_regular_sp_with_fork);	
	NEW_LINE(pid);		
	TEST(test_regular_sp_sorting);	
	NEW_LINE(pid);		
	TEST(test_overdue_processes);		
	NEW_LINE(pid);		
	TEST(test_overdue_processes_zigzag);	
	
	close(fd);
	
	printf("Test finished, output was redirected to a file named 'out'.\n");	
	
	return 0;
}
