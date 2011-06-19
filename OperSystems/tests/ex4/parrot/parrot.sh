#!/bin/bash
VERBOSE='-v'
NUM_REPEAT=$2
NUM_DEVICES=500
CONTROLLER_NAME='/dev/vsf_controller'
READ_NAME='/dev/vsf_read'
WRITE_NAME='/dev/vsf_write'
DEVNAME='vsf'
MAJOR=
OUTPUT='/dev/null'
IOCTL_CMD='ioctl'
READER_CMD='reader'
WRITER_CMD='writer'
RESULT='false'

function makenods {
	rm -f /dev/vsf_r* /dev/vsf_w* $CONTROLLER_NAME $OUTPUT tmp
	for (( i=1 , j=2 ; i < 255 ; i+=2 , j+=2 ))
	do
#		printf "%s%d %s%d \r" $READ_NAME $i $WRITE_NAME $j && sync
		mknod $READ_NAME$i c $MAJOR $i || exit 1
		mknod $WRITE_NAME$j c $MAJOR $j || exit 1
	done
	mknod $CONTROLLER_NAME c $MAJOR 0 || exit 1
	printf "                                          \r" 
}

function vprint {
	[ $VERBOSE ] && echo -e '<TEST>' $* | tee -a $OUTPUT
}

function find_module {
	VSF_MODULE="../vsf.o"
	test -f $VSF_MODULE && return
	VSF_MODULE="vsf.o"
	test -f $VSF_MODULE && return
	echo 'could not find your module file.'
	read -p'enter full path: ' VSF_MODULE
	test -f $VSF_MODULE && return
	VSF_MODULE=$(echo $VSF_MODULE'/vsf.o' )
	test -f $VSF_MODULE && return
	echo 'bad path.'
	exit 1
}

function cleanup {
	grep -q $DEVNAME /proc/devices && rmmod $DEVNAME
	rm -f tmp
}

function make_helpers {
	make -f mkfl 2>&1 >>$OUTPUT
}

function load_module {
	cleanup
	insmod -N $VSF_MODULE max_vsf_devices=$1 2>>$OUTPUT 1>>$OUTPUT || return 1
	test -f /proc/driver/vsf || echo 'No proc file!' && return 0
	test $(cat /proc/driver/vsf) -eq $1 || { vprint 'bad proc file' ; return 1 ; }
	return 0
}

function unload_module {
	rmmod 'vsf' 2>>$OUTPUT 1>>$OUTPUT
}

function set_major {
	MAJOR=$(grep vsf /proc/devices | cut -f1 -d' ')
	vprint 'In /proc/devices: major=' $MAJOR ', device name=' $DEVNAME
}

function ioctl {
	./$IOCTL_CMD $1 $2 $3
}

function assert {
	$1 || { vprint $2\. `./errno.sh $?` && exit 1; }
}

function assert_fail {
	$1 && vprint $3 && exit 1
	$2
	true
}

function test[0] {
	vprint 'running ' $FUNCNAME ': load and unload module'

	assert "load_module 999999" "Cannot load module. max=999999"
	assert unload_module "Cannot unload module"
	assert "load_module 1" "Cannot load module. max=1"
	set_major
	assert unload_module "Cannot unload module"

	vprint 'trying bad arguments'

	assert_fail "load_module 1000001" unload_module "Module shouldn't load 1000001"
	let A=$?
	assert_fail "load_module 0" unload_module 'Module should not load 0'
	let B=$?
	assert_fail "load_module -1" unload_module 'Module should not load -1'
	let C=$?

	return $(( $A + $B + $C ))
}

function test[1] {
	load_module 1	
	rm -f tmp
	vprint 'running ' $FUNCNAME ': Testing ioctl...'
	assert_fail "ioctl free 1 1" "true" 'Freed nonexisted minors!' | tee -a tmp
	assert_fail "./reader /dev/vsf_reader1" 'true'  'Files opened before the vsfs created.'
	assert 'ioctl create 1 2' 'Simple ioctl failed' | tee -a tmp
	assert_fail "ioctl create 3 4" "true" 'max_vsf_devices is not working' | tee -a tmp
	assert 'ioctl free 1 2' 'Cannot free devices' 	| tee -a tmp
	assert 'ioctl create 3 4' 'Cannot create devices' | tee -a tmp
	assert 'ioctl free 3 4' 'Cannot free devices' 	| tee -a tmp
	assert 'ioctl create 1 2' 'Cannot fcreate devices' | tee -a tmp
	assert_fail "ioctl create 256 257" "true" 'Created vsf with minor > 255!' | tee -a tmp
	assert_fail "ioctl create 25600 25700" "true" 'Created vsf with minor >> 255!' | tee -a tmp
	assert_fail "ioctl create -1 -2" "true" 'Created vsf with negative minor!' | tee -a tmp
	assert_fail "ioctl create -25600 -25700" "true" 'Created vsf with minor << 0!' | tee -a tmp
	test ! -s tmp 
}

todo[0]()  { assert "ioctl create $1 $2 >> tmp" "cannot create driver for $1 $2"; }
todo[1]()  { ./$READER_CMD ${READ_NAME}$1  ${MESSAGE[$1]} >> $OUTPUT ; }
todo[2]()  { ./$WRITER_CMD ${WRITE_NAME}$2 ${MESSAGE[$1]} >> $OUTPUT ; }
todo[11]() { todo[1] $1 $2 & true ; }
todo[22]() { todo[2] $1 $2 & true ; }
todo[3]()  { MESSAGE[$1]=$(</dev/urandom tr -dc A-Za-z0-9 | head -c$RANDOM) ; }

function test[2] {
	load_module 1 && ioctl create 1 2
	vprint 'Running ' $FUNCNAME ': check simple write+read with random message'
	todo[3]  1
	todo[11] 1 2
	todo[2]  1 2
	A=$?
	todo[3]  1
	todo[22] 1 2
	todo[1]  1 2
	B=$?
	return $(( $A + $B ))
}

function test[3] {	
	load_module 2 && ioctl create 1 2
	vprint 'Running ' $FUNCNAME ': exceeding maximum devices after minors freed.'	
	for i in `seq 2`
	do
		ioctl free 1 2 
		ioctl create 1 2 
		todo[11] 1 2
		sleep 1
	done
	ioctl free 1 2 
	assert_fail "ioctl create 1 2"  'true' 'max_vsf_devices is not working'
	A=$?
	vprint 'Terminating lost children:'
	killall reader
	sleep 1
	wait
	return $(( $A ))
}


function run256 {
	A=0
	for (( i=1 , j=2 ; i < 255 ; i+=2 , j+=2 ))
	do
		for name in $@
		do
			printf "%3d %4d %4d \r" $name $i $j && sync
			todo[$name] $i $j
			A=$(( $A + $? ))
		done
	done
	return $A
}

function test[4] {
	rm -f tmp
	vprint 'running ' $FUNCNAME ': intensive use of 254 minors.'
	vprint 'it may take about one minute'
	load_module 256 && 
	{
		run256 0 ;		(( A=$? ))
		run256 3 11 2 ; (( A+=$? ))
		run256 3 22 1 ; (( A+=$? ))
		run256 3 22	  ; (( A+=$? ))
		run256 1	  ; (( A+=$? ))
		run256 3 11	  ; (( A+=$? ))
		run256 2	  ; (( A+=$? ))
	}
	return $A
}

function run_test {
	cleanup && $@ && vprint $1 ' pass!' || vprint $1 ' failed!'
}

function clean_all {
	vprint 'Cleaning up...'
	[[ ! -s tmp ]] && rm -f tmp
	killall -q reader writer ioctl
	sleep 2
	unload_module
	true
}

#########################
#		MAIN SCRIPT		*
#########################

[ $1 ] && [ $1 == 'clean' ] && { clean_all ; exit 0; }
[ $1 ] && [ $1 == 'help'  ] && { cat README.TXT ; exit 0; }

make_helpers

find_module

run_test test[0]
vprint 'Making nods...'
makenods
vprint "Running main tests:"

SEQ=`seq 1 4`
(( $# > 0 )) && SEQ=${@:1}
for i in $SEQ
do
	run_test test[$i]
done

vprint 'Done!'
