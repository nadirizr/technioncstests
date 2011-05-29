#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h> // errno
#include <errno.h> // EXXXXX errors

#include <sys/types.h> // for mkfifo
#include <sys/stat.h>

#define TEMP_PIPE_PATH	"/tmp/pipes"

int* out_numbers = NULL;
pthread_t* out_threads = NULL;

int input_num, output_num;
char* input_prefix = NULL;
char* output_prefix = NULL;
char* monitor_file = NULL;

void print_errno() {
	switch (errno) {
		case EPERM : printf("EPERM"); return;
		case EACCES: printf("EACCES"); return;
		case EEXIST: printf("EEXIST"); return;
		case ENAMETOOLONG: printf("ENAMETOOLONG"); return;
		case ENOENT: printf("ENOENT"); return;
		case ENOSPC: printf("ENOSPC"); return;
		case ENOTDIR: printf("ENOTDIR"); return;
		case EROFS: printf("EROFS"); return;
	}
	printf("%d",errno);
}

void run_router(void* ptr) {
	char command_line[256];
	sprintf(command_line,"./router %d %s %d %s %s",
		input_num, TEMP_PIPE_PATH "/in",
		output_num, TEMP_PIPE_PATH "/out", TEMP_PIPE_PATH "/mon");
	system(command_line);
	pthread_exit(0);
}

void file_writer(void* ptr) {
	int file_num = *((int*) ptr);
	char command_line[256];
	if (file_num) 
		sprintf(command_line,"cat %s%d > %s%d",
			TEMP_PIPE_PATH "/out",file_num, output_prefix ,file_num);
	else
		sprintf(command_line,"cat %s > %s", TEMP_PIPE_PATH "/mon", monitor_file);
	system(command_line);
	pthread_exit(0);
}

int parse_integer(char* str) {
	int ret; char *endptr;
	ret = strtol(str, &endptr, 10);
	if ( ret < 0 || errno == EINVAL || errno == ERANGE ||
			endptr == str || *endptr != '\0') exit(1);
	return ret;
}

void copy_string(char** dest, char* src) {
	*dest = (char*)malloc((strlen(src)+1)*sizeof(char));
	if (!(*dest)) exit(1);
	strcpy((*dest),src);
}

int main(int argc, char *argv[]) {
	int i, iret, error_mode = 0;
	char str_path[256], command_line[256];
	pthread_t router;
	
	if (argc != 6) {
		printf("Error: Usage:\n\t./routest <inputnum> <inputprefix> <outputnum> <outputprefix> <monitor>\n");
		exit(1);
	}
	
	input_num = parse_integer(argv[1]);
	copy_string(&input_prefix, argv[2]);
	output_num = parse_integer(argv[3]);
	copy_string(&output_prefix, argv[4]);
	copy_string(&monitor_file, argv[5]);
	
	out_numbers = (int*) malloc((output_num + 1) * sizeof(int));
	out_threads = (pthread_t*) malloc((output_num + 1) * sizeof(pthread_t));
	if (!out_numbers || !out_threads){
		printf("Error: No memory\n");
		exit(1);
	}
	
	//create temp dir
	mkdir(TEMP_PIPE_PATH,S_IREAD | S_IWRITE);
	//create pipes
	printf("** Creating pipes\n");
	for (i = 1; i <= input_num ; i++) {
		sprintf(str_path,"%s%d",TEMP_PIPE_PATH "/in",i);
		iret = mkfifo(str_path,  S_IREAD | S_IWRITE);
		if (iret) {
			printf("Error: Couldn't make pipe (%s), errno = ",str_path);
			print_errno();
			printf(".\n");
			error_mode = 1;
		}
	}
	if (!error_mode)
	for (i = 1; i <= output_num ; i++) {
		sprintf(str_path,"%s%d",TEMP_PIPE_PATH "/out",i);
		iret = mkfifo(str_path,  S_IREAD | S_IWRITE);
		if (iret) {
			printf("Error: Couldn't make pipe (%s), errno = ",str_path);
			print_errno();
			printf(".\n");
			error_mode = 1;
		}
	}
	//monitor
	sprintf(str_path,"%s",TEMP_PIPE_PATH "/mon");
	iret = mkfifo(str_path,  S_IREAD | S_IWRITE);
	if (iret) {
		printf("Error: Couldn't make pipe (%s), errno = ",str_path);
		print_errno();
		printf(".\n");
		error_mode = 1;
	}
	if (!error_mode) {
		printf("** Starting Router with created pipes\n");
		iret = pthread_create(&router,NULL,(void*)&run_router,NULL);
		if (iret) {
			printf("Error: Couldn't create thread, errno = ",str_path);
			print_errno();
			printf(".\n");
			error_mode = 1;
		}
		if (!error_mode) {
			
			printf("** Create writing threads\n");
			for (i = 0; i <= output_num ; i++) {
				out_numbers[i] = i;
				pthread_create(&out_threads[i],NULL,(void*)&file_writer,(void*)&out_numbers[i]);
			}		
			printf("** Starting streaming information to in-pipes\n");
			for (i = 1; i <= input_num ; i++) {
				sprintf(command_line,"cat %s%d > %s%d",
					input_prefix ,i,
					TEMP_PIPE_PATH "/in",i);
				//run cat to output the input file to the in-pipe
				system(command_line);
			}
		}
	}
	printf("** Waiting for file writers threads to join\n");
	//wait for threads to join
	
	for (i = 0 ; i <= output_num ; i++) {
		iret = pthread_join(out_threads[i], NULL);
		if (iret) 
			printf("Error joining\n");
	}
	printf("** Waiting for router thread to join\n");
	iret = pthread_join(router, NULL);
	if (iret) 
		printf("Error joining\n");
	//delete pipes
	
	printf("** Deleting pipes\n");
	for (i = 1; i <= input_num ; i++) {
		sprintf(str_path,"%s%d",TEMP_PIPE_PATH "/in",i);
		remove(str_path);
	}
	if (!error_mode)
	for (i = 1; i <= output_num ; i++) {
		sprintf(str_path,"%s%d",TEMP_PIPE_PATH "/out",i);
		remove(str_path);
	}
	sprintf(str_path,"%s",TEMP_PIPE_PATH "/mon");
	remove(str_path);
	remove(TEMP_PIPE_PATH);
	
	free(out_numbers);
	free(out_threads);
	
	return 0;
}
