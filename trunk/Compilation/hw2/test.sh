#!/bin/bash

# clean up
#rm -f a.out lex.yy.c source.tab.cpp source.tab.hpp

# compile
cd ..
make
#`bison -d ../hw2.ypp` && `flex ../hw2.lex` && `g++ lex.yy.c ../hw2.tab.cpp`
if [ $? -ne 0 ]; then
  echo "Compile of hw2.lex or hw2.ypp failed!"
  exit 1
fi

# Run the tests
tests=( "tests/staff/test"
        "tests/dan_lagi/test_define_var_that_is_already_in_the_scope"
        "tests/dan_lagi/test_cant_use_undefined_var" )

for test in ${tests[@]}
do
  echo -n "Running test $test ... "
  ./a.out < $test.in > $test.res
  diff $test.res $test.out > $test.diff
  if [ -s $test.diff ]; then
    echo "FAILED (see $test.diff)"
  else
    echo "OK"
  fi
done
