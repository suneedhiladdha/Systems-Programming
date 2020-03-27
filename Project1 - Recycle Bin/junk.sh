#!/bin/bash
# Siddhanth Patel
# I pledge my honor that I have abided by the Stevens Honor System.
readonly JUNK="/home/cs392/.junk"

if [ ! -d "$JUNK" ]; then
	mkdir -p "$JUNK";
fi

showusage(){
	cat << USETEXT
Usage: `basename $0` [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
USETEXT
}

while getopts ':hlp' opt; do
	case ${opt} in
		h) if [ $# -gt 1 ];
			then
				echo "Error: Too many options enabled."
				showusage
				exit 1
			else
				showusage
				exit 0
			fi
			;;
		l) if [ $# -gt 1 ];
			then
				echo "Error: Too many options enabled."
				showusage
				exit 1
			else
				DIR=/home/cs392/.junk
				ls -lAF $DIR
				exit 0
			fi
			;;
		p) if [ $# -gt 1 ];
			then
				echo "Error: Too many options enabled."
				showusage
				exit 1
			else
				rm -rf /home/cs392/.junk/*
				rm -rf /home/cs392/.junk/.* 2> /dev/null
				exit 0
			fi
			;;
		\?) printf "Error: Unknown option \'-%s\n" "$OPTARG"\'"."
			showusage
			exit 1;
			;;
	esac
done
if [ $# -eq 0 ]; then showusage; fi
shift $((OPTIND-1))

for var in "$@"
do
	FILE=$var
	if [ -f /home/cs392/.junk/"$FILE" ]; then
		echo "Warning: '""$FILE""' not found."
	else
		mv $var /home/cs392/.junk
	fi
done

exit 0