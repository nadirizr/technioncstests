#!/bin/bash

# compile
cd ..
#make
#if [ $? -ne 0 ]; then
#  echo "Compile of hw4.lex or hw4.ypp failed!"
#  exit 1
#fi

# Run the tests
test_dirs=( "tests/staff/"
            "tests/dan_lagi/"
          )
#        "tests/dan_lagi/test_define_var_that_is_already_in_the_scope"
#        "tests/dan_lagi/test_cant_use_undefined_var" )

for test_dir in ${test_dirs[@]}
do
  for test_file in `ls ${test_dir}*.in`
  do
    test="${test_file:0:${#test_file}-3}"
    echo -n "Running test ${test} ... "
    ./hw4 < $test.in > $test.res
    diff $test.res $test.out > $test.diff
    if [ -s $test.diff ]; then
      echo "FAILED (see $test.diff)"
    else
      echo "OK"
    fi
  done
done

