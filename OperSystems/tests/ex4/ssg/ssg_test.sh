#!/bin/tcsh -f

set curr_dir = `pwd`
echo INIT: Current dir is $curr_dir
echo INIT: removing old vsf.o
rm -f vsf.o
echo INIT: Building vsf module
make
rmmod vsf # just in case it is already installed

echo INIT: Installing VSF Module
insmod ./vsf.o max_vsf_devices=50
set major = `cat /proc/devices | grep vsf | cut -f1 -d" "`
echo INIT: Module Installed, major is $major
echo INIT: Changing directory to /root
cd /root
echo INIT: Deleting old VSF devices
rm -f /root/vsf_controller
rm -f /root/vsf_reader_??
rm -f /root/vsf_writer_??
echo INIT: Making nodes
mknod vsf_controller c $major 0
mknod vsf_reader_01 c $major 1
mknod vsf_reader_02 c $major 2
mknod vsf_reader_03 c $major 3
mknod vsf_reader_04 c $major 4
mknod vsf_reader_05 c $major 5
mknod vsf_reader_06 c $major 6
mknod vsf_reader_07 c $major 7
mknod vsf_reader_08 c $major 8
mknod vsf_reader_09 c $major 9
mknod vsf_reader_10 c $major 10

mknod vsf_writer_11 c $major 11
mknod vsf_writer_12 c $major 12
mknod vsf_writer_13 c $major 13
mknod vsf_writer_14 c $major 14
mknod vsf_writer_15 c $major 15
mknod vsf_writer_16 c $major 16
mknod vsf_writer_17 c $major 17
mknod vsf_writer_18 c $major 18
mknod vsf_writer_19 c $major 19
mknod vsf_writer_20 c $major 20
echo INIT: Nodes ready for use:
echo INIT: vsf_controller : minor 0
echo INIT: vsf_reader_\#\# : minors 1-10
echo INIT: vsf_writer_\#\# : minors 11-20
echo INIT: changing path to $curr_dir
cd "$curr_dir"
echo INIT: Building SSG_test.exe
gcc SSG_test.c -o SSG_test.exe
echo INIT: Running test
./SSG_test.exe >& ssg_test.log
cat ssg_test.log
echo
echo cleaning up
echo
rm -f SSG_test.exe
rm -f /root/vsf_controller
rm -f /root/vsf_reader_??
rm -f /root/vsf_writer_??
rmmod vsf

exit







