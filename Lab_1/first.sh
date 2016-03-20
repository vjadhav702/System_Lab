#!/bin/bash
# My first script

count=1 #count to skip first line of the file
count1=0

file="input.txt"

############## check if file exits ################
if [[ ! -f "$file" ]]
	then	
	echo "$file not found. Please check and try again."
	exit
fi

############## read file line by line ################
while read line
do
	if [ $count -gt 0 ]
	then	
		count=0
	else
		count1=`expr $count1 + 1` #increment the counter value
		
		echo "################## Processing $count1 number #################"
		IFS=' ' read -ra array <<< "$line"		

		if [ ${#array[@]} -eq 3 ] #check if there are 2 numbers only in each line
		then				
			echo "Input Number --> ${array[0]}"	
			echo "Input Base   --> ${array[1]}"
			echo "Output Base  --> ${array[2]}"
		else #else continue to next line
			echo "No of numbers in the line is : ${#array[@]}"
			echo "wrong input format, Please check the format"
			continue
		fi

		echo 
		result=$(echo "obase=${array[2]};ibase=${array[1]};${array[0]}" | bc) #perform the conversion
		echo "Output Number is --> $result"
		echo
	fi

done <$file #reached end of the file.
