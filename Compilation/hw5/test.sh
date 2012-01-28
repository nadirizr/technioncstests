#!/bin/bash

# compile
cd ..
make
if [ $? -ne 0 ]; then
  echo "Compile of hw5.lex or hw5.ypp failed!"
  exit 1
fi

# Run the tests
test_dirs=( #"tests/staff/"
            "tests/dan_lagi/"
          )

for test_dir in ${test_dirs[@]}
do
  for test_file in `ls ${test_dir}*.in`
  do
    test="${test_file:0:${#test_file}-3}"
    echo -n "Running test ${test} ... "
    ./hw5 < $test.in > $test.ir

    if [ -f $test.stdin ]; then
      #cat $test.stdin | ./bvm.pl $test.ir > $test.res
      ./bvm.pl $test.ir < $test.stdin > $test.res
    else
      ./bvm.pl $test.ir > $test.res
    fi
    diff $test.res $test.out > $test.diff
    if [ -s $test.diff ]; then
      echo "FAILED (see $test.diff)"
    else
      echo "OK"
    fi
  done
done

