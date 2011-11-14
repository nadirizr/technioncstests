#!/bin/bash

# clean up
rm -f a.out lex.yy.c

# compile
flex ../hw1.lex
g++ lex.yy.c ../hw1.cpp

# run the staff test and diff
echo "Running staff test..."
./a.out < staff/t1.in > staff/t1.res
diff staff/t1.res staff/t1.out

# run our tests
echo "Running Lagi's comment test..."
./a.out < lagi_tests/comment_test.in > lagi_tests/comment_test.res
diff lagi_tests/comment_test.res lagi_tests/comment_test.out

# run forum test
#echo "Running forum tests..."
#./a.out < from_forum/t6.in > from_forum/t6.res
#diff from_forum/t6.res from_forum/t6.out

