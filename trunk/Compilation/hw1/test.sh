#!/bin/bash

# clean up
rm -f a.out lex.yy.c

# compile
`flex ../hw1.lex` && `g++ lex.yy.c ../hw1.cpp`
if [ $? -ne 0 ]; then
  echo "Compile of hw1.lex failed!"
  exit 1
fi

# Run the tests
tests=( "staff/t1" "dan_tests/dan" "lagi_tests/real_test" "lagi_tests/string_test" "lagi_tests/comment_test" "from_forum/t6" )

for test in ${tests[@]}
do
  echo -n "Running test $test ..."
  ./a.out < $test.in > $test.res
  diff $test.res $test.out > $test.diff
  if [ -s $test.diff ]; then
    echo "FAILED (see tests/$test.diff)"
  else
    echo "OK"
  fi
done

