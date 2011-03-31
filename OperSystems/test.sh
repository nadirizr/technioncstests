#!/bin/tcsh

if (-d tests) then
	svn up tests
else
	svn checkout http://technioncstests.googlecode.com/svn/trunk/OperSystems/tests tests
endif

# run the test itself
cd tests
python test_all.py $argv
