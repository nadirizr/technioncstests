#!/bin/sh
module="vsf"
device="vsf"
max_vsf_devices=254
gcc hw4_test.c -o hw4_test
insmod ./$module.o max_vsf_devices=$max_vsf_devices $* || exit 1
major=`awk "\\$2==\"$module\" {print \\$1}" /proc/devices`
if [ "$1" == "" ]
then
    num_of_processes=60
elif [ "$1" -ge 128 ]
then 
    num_of_processes=127
else
	num_of_processes=$1
fi
if [ "$2" == "" ]
then
    out_address="C"
else
	out_address=$2
fi	
./hw4_test $num_of_processes $major $max_vsf_devices
rmmod vsf
rm -f /mnt/hgfs/$out_address/out
cp out /mnt/hgfs/$out_address
