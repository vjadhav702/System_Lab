#!/bin/bash

########### This is command use to get all the file in the current directory ############
FILECOUNT="$(ls $1 | grep -c 'A.*.txt')"

echo "Number of files are : $FILECOUNT"

######## Write the result into file.##########
cat > output/thirdOP << EOF1
Output : 
========
Number of files starting with 'A' and ending with '.txt ' in directory '$1' are  : $FILECOUNT
EOF1

