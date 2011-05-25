import commands
import sys

args = " ".join(sys.argv[1:])

# Check valgrind leak checks.
print "Testing 'valgrind --leak-check=full' on '%s'..." % args
output_valgrind = commands.getoutput("valgrind --leak-check=full %s" % args)
error = False
if output_valgrind.find("definitely lost: 0 bytes in 0 blocks") == -1:
  print "ERROR: There is unfreed memory!"
  error = True
if output_valgrind.find("ERROR SUMMARY: 0 errors from 0 contexts") == -1:
  print "ERROR: There are errors found by valgrind!"
  error = True

if not error:
  print "PASSED!"
  print
  print
  sys.exit(0)
else:
  print "Valgrind Output:"
  print output_valgrind
  sys.exit(1)

# Check helgrind checks.
print "Testing 'valgrind --tool=helgrind' on '%s' ..." % args
output_valgrind = commands.getoutput("valgrind --tool=helgrind %s" % args)
error = False
if output_valgrind.find("unlocked a not-locked lock") != -1:
  print "ERROR: Unlocked a not-locked lock!"
  error = True
if output_valgrind.find("call to pthread_mutex_lock failed") != -1:
  print "ERROR: A call to pthread_mutex_lock has failed!"
  error = True
if output_valgrind.find("call to pthread_mutex_unlock failed") != -1:
  print "ERROR: A call to pthread_mutex_unlock has failed!"
  error = True

if not error:
  print "PASSED!"
  print
  print
else:
  print "Valgrind Output:"
  print output_valgrind
