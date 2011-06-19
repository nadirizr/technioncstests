1. Put it in the directory with your vsf.o file
2. open the test.c and change "MY_MAJOR" to the number on your system
3. There are two important "variables" (defines) in this test that you can play with:
	- CHILD - determines the number of proccesses created
	- VSF_PER_PRC - number of vsf devices per proccesses
The number of lines in the test is CHILD*VSF_PER_PRC+1
for some reason there are problem with setting more than VSF_PER_PRC above 75 or so when there are many proccesses, but even smaller numbers (100 & 60) should generate large enough file (around 70K).
4. compile: gcc -o test test.c -Wall
5. run: ./test
6. It will generate a file called "out.txt" which is the state of the summery when all procceses are running and all the vsf are opened.
7. You might (and prolly will) get an error "module is not loaded" when running the test - it's ok.

* I've attached an example output for CHILD=5, VSF-PER_PRC=3 so you'll know what to expect.