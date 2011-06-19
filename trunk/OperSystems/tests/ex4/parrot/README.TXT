Parrot shell script
A simple test for HW 4 in OS course
 
To run this test:
First time you will need to
1) unzip 
2) make -f mkfl

Then simply run:
	./parrot.sh [list]
when list is an optional whitespace-seperated numbers from 1 to 5
What it tests:
0) Loading and unloading module. this test will always run.
1) Correctness of 'max_vsf_devices' parameter.
2) Simple message sending.
3) Correctness of 'max_vsf_devices' parameter,
    when minors no longer point to devices.
4) Use of the whole minors range with ending / receiving messages.

for example, to run only tests 0, 1 and 5 type:
	./parrot 4 5
	
   clean:
       kill remove and unload everything
       
   help : 
       show this help
       
Files in this zip:
1) parrot.sh
2) errno.sh - a file I've found in the web, useful to understand errno values
3) ioctl.c - run ioctl() syscall with create/free
    Usage: ./ioctl [create | free] read_minor write_minor [controller path]
    defaults:        create            -          -       /dev/vsf_controller
4) reader.c - run read() syscall on device. Can expect specific message.
    Usage: ./reader filename [message]
    default: no specific message
5) writer.c - run write() syscall with message on a file
    Usage: ./writer filename message
6) mkfl - the makefile for the above files. It's not called "makefile" to avoid confusing with your original makefile.
    Usage: make -f mkfl
7) vsf.h - to easily find VSF_CREATE and VSF_DESTROY values
