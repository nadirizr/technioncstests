#!/bin/tcsh

if (-d tests) then
	svn up test
else
	svn checkout http://technioncstests.googlecode.com/svn/trunk/DataStructures/tests tests
endif

# run the test itself
cd tests
python test_all.py $argv
