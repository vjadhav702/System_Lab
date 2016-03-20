#!/bin/bash


while true; do

## To Display user name text box
USERNAME=$(whiptail --title "User-Name" --inputbox "User Name"  10 60 3>&1 1>&2 2>&3 ) #To enter the user name
res=$?
if [ $res -eq 1 ]
then
        exit 0
fi

## To Display password box
PASSWORD=$(whiptail --title "Password" --passwordbox "Password" 10 60 3>&1 1>&2 2>&3) # To enter the password
res=$?
if [ $res -eq 1 ]
then
        exit 0
fi

##validation
if [ ! $USERNAME = "vinayak" ] && [ ! $PASSWORD = "welcome" ];
then
	$(whiptail --title "Wrong-details" --msgbox "Wrong credentials. Please try again "  10 60 3>&1 1>&2 2>&3 )
else
	break
fi

done

while true; do  
allUsers=$(cut -d: -f1 /etc/passwd)

arr=$(echo $allUsers | tr " " "\n")


newStr=""
newLine="\n"
for x in $arr
do
    newStr=$newStr$x$newLine
done

{
    for ((i = 0 ; i < 100 ; i+=1)); do
        sleep 0.02
        echo $i
    done
} | whiptail --gauge "User list is loading" 6 60 0

##Display list all the users
CurUSER=$(whiptail --title "List Users" --inputbox "$newStr"  41 60 3>&1 1>&2 2>&3 ) #To enter list of all user
res=$?
if [ $res -eq 1 ]
then
        exit 0
fi

data=$(ps -u $CurUSER -o pid,etime,%cpu,%mem,cmd r)

##Uncomment this in case of all the processes
#data=$(ps -u $CurUSER -o pid,etime,%cpu,%mem,cmd)

data=$data$newLine$newLine" Press q to quit, <Enter> to re­run ..."

{
    for ((i = 0 ; i < 100 ; i+=1)); do
        sleep 0.02
        echo $i
    done
} | whiptail --gauge "User processes loading" 6 60 0

runProcess=$(whiptail --title "Running Processes" --inputbox "$data"  41 80 3>&1 1>&2 2>&3 ) #To enter list of all user
res=$?
if [ $res -eq 1 ]
then
        exit 0
fi


if [ $runProcess = $'q' ];
then
exit
fi

done


