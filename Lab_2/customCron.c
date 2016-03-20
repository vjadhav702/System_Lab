/*
        This C file is the custom implementaion of the cron tab.
        It will parse the cron tab file and schedule all the commands to be executed at that time
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <linux/inotify.h>

/*
        This structure maintains all the information for given cronCommand.
        As clear from the name each field specifies perticular operation
*/
typedef struct cronCommand
{
	int min;
	int hour;
	int DOM;
	int month;
	int DOW;
	char cmd[1000];	
	int sAtMin;	
	int sAtHour;
	int sAtDOM;
	int starAtMonth;
	int sAtDOW;	
	int exeFlag;	
}cronCommand;

double minimumTime=999999;
struct cronCommand listCommands[100];
int cnt=0;


/*
        This method is used to calculate remaining time for commadn to get executed.
        After this much time command is expected to get executed.
*/
double calculateRemainingTime(cronCommand tempCmd)
{
	time_t now;
	time_t timeCmd;
	struct tm cmdTime;
	double seconds;
	time(&now);  
	cmdTime = *localtime(&now);
	cmdTime.tm_sec = 0;
	
	//Assing all the values
	cmdTime.tm_min = tempCmd.min;
	cmdTime.tm_hour = tempCmd.hour;  
	cmdTime.tm_mday = tempCmd.DOM;
	cmdTime.tm_mon = tempCmd.month;  
	cmdTime.tm_wday = tempCmd.DOW;	
	timeCmd = mktime(&cmdTime);	
	seconds = difftime(timeCmd,now);//find diff time
	return seconds;
}

/*
        This method is used to prin the command which is going to be excuted.
        It will display next command which is going to be executed.
*/
void printCommand(cronCommand temp)
{
	printf("#################################################################\n");	
	printf("Hour->%d ",temp.hour);
	printf("Minute->%d\n",temp.min); 
	printf("DayOfMonth->%d ",temp.DOM);
	printf("Month->%d ",temp.month);
	printf("DayOfWeek->%d\n",temp.DOW);
	printf("Command->%s \n",temp.cmd);
	printf("##################################################################\n");
}//End






/*
        This method is used to find the next command to be executed.
        This method will find the command executeTheCommand the command.
*/
void findNCommand()
{
	time_t now;
	struct tm curTime;
	time(&now);  
	curTime = *localtime(&now);
	cronCommand tempCmd;
	
	double sec = 0;

	int i =0 ;
	for(i=0;i<cnt;i++)//This loop will executed for givne count measure.
	{
		tempCmd = listCommands[i];
		tempCmd.sAtMin = 1;
		tempCmd.sAtHour = 1;
		tempCmd.sAtDOM = 1;
		tempCmd.starAtMonth = 1;
		tempCmd.sAtDOW = 1;

		//printCommand(tempCmd);
		if(tempCmd.sAtMin==1)
		{	
			tempCmd.min=tempCmd.min+1;
			if(tempCmd.min==60)
			{
				tempCmd.min=0;
				tempCmd.hour=tempCmd.hour+1;
			}
		}
		if(tempCmd.sAtHour==1)
		{
			if(tempCmd.sAtMin!=1)
			{
				if((tempCmd.min-curTime.tm_min)>0)
					tempCmd.hour=curTime.tm_hour;
				else
					tempCmd.hour=curTime.tm_hour+1;
			}
			else
			{
				tempCmd.hour=curTime.tm_hour;
			}	
		}
		if(tempCmd.sAtDOM==1)
		{
			if(tempCmd.sAtMin!=1 && tempCmd.sAtHour!=1)
			{
				if((tempCmd.hour-curTime.tm_hour)>0)
				{
					tempCmd.DOM=curTime.tm_mday;	
				}
				else if((tempCmd.hour-curTime.tm_hour)==0)
				{
					if(tempCmd.min-curTime.tm_min>0)
						tempCmd.DOM=curTime.tm_mday;
					else
						tempCmd.DOM=curTime.tm_mday+1;
				}
				else
					tempCmd.DOM=curTime.tm_mday+1;
			}
			else
			{
				tempCmd.DOM=curTime.tm_mday;
			}
    		}
    		if(tempCmd.starAtMonth==1)
    		{
    			if(tempCmd.sAtMin!=1 && tempCmd.sAtHour!=1 && tempCmd.sAtDOM!=1)
			{
    				if((tempCmd.DOM-curTime.tm_mday)>0)
    					tempCmd.month=curTime.tm_mon;
    				else if(tempCmd.DOM-curTime.tm_mday==0)
    				{
    					if((tempCmd.hour-curTime.tm_hour)>0)
					{
						tempCmd.month=curTime.tm_mon;	
					}
					else if((tempCmd.hour-curTime.tm_hour)==0)
					{
						if(tempCmd.min-curTime.tm_min>0)
							tempCmd.month=curTime.tm_mon;
						else
							tempCmd.month=curTime.tm_mon+1;
					}
					else
						tempCmd.month=curTime.tm_mon+1;	
    				}
				
				else
					tempCmd.month=curTime.tm_mon+1;
			}
			else
			{
				tempCmd.month=curTime.tm_mon;
			}
    		}
    		if(tempCmd.sAtDOW==1)
    		{
			tempCmd.DOW=curTime.tm_wday;	
    		}
    		/*if(tempCmd.starAtMonth==0 && tempCmd.sAtDOM==1)
    		{
    			tempCmd.DOM=1;
    			if(tempCmd.sAtHour==1)
    				tempCmd.hour=0;
    			if(tempCmd.sAtMin==1)
    				tempCmd.min=0;	
    		}*/
    		
		printCommand(tempCmd);
		listCommands[i] = tempCmd;
		sec = calculateRemainingTime(tempCmd);
			
		printf("Next Command will execute in : %lf seconds\n\n",sec);
		if(sec >= 0 && sec < minimumTime)
		{
			minimumTime = sec;
		}
	}//end for
}//End


/*
        This method as name suggest will executeTheCommand the given command.
*/
void executeTheCommand()
{
	int i;
	for(i=0;i<cnt;i++)
	{
		if(listCommands[i].exeFlag == 1)
		{
			printf("\n\nExecuting cmd = %s\n",listCommands[i].cmd);		
			system(listCommands[i].cmd);
		}
	}
}


/*
        This method is usedto parse the command. And identify the meaning out of it to get executed.
*/
cronCommand pCommand(char* str)
{
	time_t now;
	struct tm cmdTime;
	time(&now);  
	cmdTime = *localtime(&now);//locate the time

	char *token;
	cronCommand tempCmd;
	tempCmd.sAtMin = 1;
	tempCmd.sAtHour = 1;
	tempCmd.sAtDOM = 1;
	tempCmd.starAtMonth = 1;
	tempCmd.sAtDOW = 1;	
	tempCmd.exeFlag = 0;	
	char delim[2]="\t";
	int cnt = 4;
	int i=2;
	//Perform tokenization on the given string to get the required token out of it.
	token = strtok(str,delim);
	if(strcmp(token,"*") != 0)//find occurance of * for tokenization
	{
		tempCmd.sAtMin = 0;
		tempCmd.min=atoi(token);		
		if(tempCmd.min > 60)
		{
			printf("Invalid minute...\n");
			exit(0);		
		}		
	}
	else
	{
		tempCmd.min = cmdTime.tm_min;
	}
	for(;cnt > 0;)
	{
		cnt--;
		token = strtok(NULL,delim);

		switch(i)
		{
			case 2:
				tempCmd.hour=cmdTime.tm_hour;		
			break;
			case 3:
				tempCmd.DOM=cmdTime.tm_mday;		
			break;
			case 4:
				tempCmd.month=cmdTime.tm_mon;				
			break;
			case 5:
				tempCmd.DOW=cmdTime.tm_wday;				
			break;
		}
		if(strcmp(token,"*") != 0)		
		{
			switch(i)
			{
				case 2:
					tempCmd.hour=atoi(token);		
					tempCmd.sAtHour = 0;
					if(tempCmd.hour > 24)
					{
						printf("Invalid hour...\n");
						exit(0);		
					}		
				break;
				case 3:
					tempCmd.DOM=atoi(token);	
					tempCmd.sAtDOM = 0;
					if(tempCmd.DOM > 31)
					{
						printf("Invalid day of month...\n");
						exit(0);		
					}		
				break;
				case 4:
					tempCmd.month=atoi(token);							
					tempCmd.starAtMonth = 0;
					if(tempCmd.month > 60)
					{
						printf("Invalid month...\n");
						exit(0);		
					}	
				break;
				case 5:
					tempCmd.DOW=atoi(token);		
					tempCmd.sAtDOW = 0;
					if(tempCmd.DOW > 60)
					{
						printf("Invalid day of week...\n");
						exit(0);		
					}	
				break;
			}		
		}
		i++;
	}
	token = strtok(NULL,"\n");
	strcpy(tempCmd.cmd,token);
	return tempCmd;
}//End


/*
        This method set flags of the command to executed.
        This is very importent method as this method set flag which leads to correct execution of the command
*/
void setFlagsForCommand(double sec)
{
	//printf("inside executeflags.. cnt = %d\n",cnt);
	//printf("\nmin seconds = %lf\n",sec);
	int i;
	for(i=0;i<cnt;i++)
	{
		double seconds=	calculateRemainingTime(listCommands[i]);	
    		//printf("seconds = %lf",seconds);
    		if(sec<=seconds && sec+5>=seconds)
    		{
    			listCommands[i].exeFlag=1;
    		}	
	}
}

/* 
        This method is used to read all the commands.
        This commands can be executed.
*/
void readAllCommands()
{
	FILE *fp = fopen("/home/vinayak/mycrontab","r");
	char str[1000];
	if(fp != NULL)
	{
		while(fgets(str,1000,fp)!= NULL)		
		{
			listCommands[cnt++] = pCommand(str);		
		}
		fclose(fp);
	}
}//End

/*
        This is start method.
        Execution starts from here.
*/
void main()
{
	int result=0;
	int fd[2];
	pid_t processId;
	printf("\n Running =========================== >>>> Fork \n");
	processId = fork();
	if (processId < 0) 
	{
		 perror ("Error,Child process are not created.\n");
		 return;
	}

	if (processId == 0)  	//Child Process - will read file periodically
	{
		cnt = 0;
		for(;1>0;)
		{
			minimumTime = 999999;
			//system("clear");
			printf("\n\n********************* GETTING NEXT COMMAND ***********************\n");
			cnt = 0;
			readAllCommands();	
			findNCommand();				
			printf("************************ GOT COMMAND *******************************\n");
			setFlagsForCommand(minimumTime);
			if(minimumTime < 0 || minimumTime == 999999)
			{
				continue;
			}
			//printf("Sleep time of %lf seconds\n",minimumTime);
			sleep(minimumTime);
			printf("\n\n********************** EXECUTING THE COMMAND **********************\n");
			executeTheCommand();
			printf("\n************************ EXECUTED THE COMMAND ***********************\n");
			//sleep(60);
		}
		
	}//end parent
	else 	//parent process - keep track of if file is modified or not
	{
		int fd1,wd,BUF_LEN=10000,length;
		fd1 = inotify_init();
		char buffer[BUF_LEN];
		if ( fd1 < 0 ) 
		{
		    perror( "inotify_init" );
		}
		wd = inotify_add_watch( fd1, "/home/vinayak/", IN_MODIFY | IN_CREATE | IN_DELETE );
		
		printf("************** NOTIFY ME WHEN FILE MODIFIED *********************\n");
		length = read( fd1, buffer, BUF_LEN );
		sleep(1);
		//system("kill -9 -1");
		//kill(0,SIGTERM); //killing processId=0 ie child
		//printf("I am parent. Child is killed..\n");
		kill(processId, SIGKILL);
		main();	
	}//end child
}

