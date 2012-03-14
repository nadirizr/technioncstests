#!/bin/bash

# compile
if [ ! -f ../1.cpp ]; then
  echo "couldn't find 1.cpp in parent dir"
  exit 1
fi
g++ ../1.cpp -o 1
if [ $? -ne 0 ]; then
  echo "Compile of 1.cpp failed!"
  exit 1
fi

if [ ! -f ../2.cpp ]; then
  echo "couldn't find 2.cpp in parent dir"
  exit 1
fi
g++ ../2.cpp -o 2
if [ $? -ne 0 ]; then
  echo "Compile of 2.cpp failed!"
  exit 1
fi

# Run the tests
test_dirs=( "lagi/" )
            # "from_site/" )

for test_dir in ${test_dirs[@]}
do
  for test_file in `ls ${test_dir}*.in`
  do
    test="${test_file:0:${#test_file}-3}"
    echo -n "Running test tests/${test} (on 1) ... "
    ./1 < $test.in > $test.1.res

    diff $test.1.res $test.out > $test.1.diff
    if [ -s $test.1.diff ]; then
      echo "FAILED (see tests/$test.1.diff)"
    else
      echo "OK"
    fi

    echo -n "Running test tests/${test} (on 2) ... "
    ./2 < $test.in > $test.2.res

    diff $test.2.res $test.out > $test.2.diff
    if [ -s $test.2.diff ]; then
      echo "FAILED (see tests/$test.2.diff)"
    else
      echo "OK"
    fi
  done
done
