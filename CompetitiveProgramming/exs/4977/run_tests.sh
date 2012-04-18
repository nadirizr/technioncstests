#!/bin/bash

# compile
if [ ! -f ../ex4977.cpp ]; then
  echo "couldn't find ex4977.cpp in parent dir"
  exit 1
fi
g++ ../ex4977.cpp -o ex4977
if [ $? -ne 0 ]; then
  echo "Compile of ex4977.cpp failed!"
  exit 1
fi

# Run the tests
test_dirs=( "./" )

for test_dir in ${test_dirs[@]}
do
  for test_file in `ls ${test_dir}*.in`
  do
    test="${test_file:0:${#test_file}-3}"
    echo -n "Running test ${test} ... "
    ./ex4977 < $test.in > $test.res

    diff $test.res $test.out > $test.diff
    if [ -s $test.diff ]; then
      echo "FAILED (see $test.diff)"
    else
      echo "OK"
    fi
  done
done
