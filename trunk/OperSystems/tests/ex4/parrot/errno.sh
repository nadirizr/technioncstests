#!/bin/sh
# /*% cp % $HOME/bin/# && chmod +x $HOME/bin/#
# Copyright 2002 Omicron Software Systems, Inc., All Rights Reserved */

# Just a tiny hack.
# kd, June 1997, v1.01
#
# kd, October 2002, v1.03
#	Modified to work with the execrable bash or the latest zsh.
#
# Elazar, June 2011, v1.04
#   Modified to fit in Red Hat Linux 2.4
#
usage() {
	echo "Usage: errno [ number | name ] ..." >&2
	exit 1
}

test $# -gt 0 || usage
cd /usr/include/asm

for ERR
do
	case "$ERR" in
	[0-9]*)	;; # note that we're only checking the first character
	E*)	;; # note that we're only checking the first character
	*)	usage ;;
	esac
done

for ERR
do
	grep "#define.*[ 	]${ERR}[ 	]" errno.h | (
		read F1 F2 F3 F4

		if test -n "$F1"
		then
			F4=`expr "$F4" : "/.[ 	]*\(.*\)[ 	]*./"`
			echo "errno $F3 is $F2 - $F4"
		else
			echo "errno $ERR not found in /usr/include/asm/errno.h"
		fi
	)
done

exit 0
