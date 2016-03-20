#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>

#define MSGLEN  64
#define maxMATCH 2

char expression[81];
char filePath[50];
char cwd[1024];
char allFiles[10000]={0};
char parentBuffer[10000];
char regExp[20];
char command[50];
char dataCommand[50];
int fileCount = 0;
int allFileCount = 0;
regmatch_t noMatches[maxMATCH];
regex_t reg;

int listFile = 0;
int countFile = 0;
int filterFile = 0;
int directory = 0;


void getAllFileNames(int flag) {

    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;
    
    
    if (filePath[0] == '|') {
        mydir = opendir(cwd);
    } else {
        mydir = opendir(flag == 1? cwd : filePath);
    }
    
    
    if (mydir == NULL) {
    
        printf("Can not open the requested directory. Please check again.\n");
        exit(0);
    } 
    
    if ((myfile = readdir(mydir)) == NULL) {
        printf("Given directory does not exits !!!! Please check....");
        exit(0);
    }

        
    while((myfile = readdir(mydir)) != NULL)
    {
    
        if (myfile->d_type == DT_REG) {

                stat(myfile->d_name, &mystat);    
                strcat(allFiles,myfile->d_name);
                strcat(allFiles,"@@@@");
        }
    }
    closedir(mydir);
}

void getPathFromCommand () {

        int i;
        int j = 0;
        int count = 0;
        for (i=0;i<81;i++) {
        
                if (command[i] == ' ' && count == 0) {
                        
                        count++;
                } else if (command[i] == ' ' && count > 0) {
                        
                        break;
                } else if (count > 0) {
                        filePath[j] = command[i]; 
                        j++;
                }
        }
        filePath[j] = '\0';
        
}

void findListFiles() {

        if (listFile == 1) {//get from the path 
        
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                        ;//fprintf(stdout, "Current working dir: %s\n", cwd);
                else
                       perror("getcwd() error");

                getPathFromCommand(2);
                getAllFileNames(2);
        } else if (listFile == 2) {//get from the current dirctory
        
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                        ;//fprintf(stdout, "Current working dir: %s\n", cwd);
                else
                       perror("getcwd() error");

                        getAllFileNames(1);                
        }
        
}

int resetExpression (char *exp) {

        int status;
        //reset the array
        memset(expression, '\0', sizeof expression);

        strncpy(expression, exp, 81);

        status = regcomp(&reg,expression,REG_EXTENDED);
        if (status) return ;
        
        regmatch_t noMatches[maxMATCH];
        status = regexec(&reg,command,maxMATCH,noMatches,0);
        
        return status;
}

void testExpression () {


        int status;
        
        strncpy(expression, "ls [0-9A-Za-z.@/]+ \\| grep -c|-c grep *", 81);

        status = regcomp(&reg,expression,REG_EXTENDED);
        if (status) return ;
        
        
        status = regexec(&reg,command,maxMATCH,noMatches,0);

        if (status == 0) {//matach found
                
                listFile = 1;
                countFile = 1;
                filterFile = 1;
                directory = 1;
                
        } else {
        
                if (resetExpression ("ls /| grep -c|-c grep *") == 0) {//match found
                
                        listFile = 2;
                        countFile = 1;
                        filterFile = 1;
                        
                } else {
                                        
                        if (resetExpression ("ls \\| grep *") == 0) {//match found
                        
                                listFile = 2;
                                filterFile = 1;
                        } else {
                        
                                if (resetExpression ("ls [0-9A-Za-z.@\\]+") == 0) {//match found
                                
                                        listFile = 1;
                                        directory = 1;
                                        filterFile = 1;
                                        
                                } else {
                                
                                        if (resetExpression ("ls \\|") == 0) {//match found
                                                
                                                printf("Not valid command enter command followed by '|' and try again.");
                                        } else {
                                        
                                                 if (resetExpression ("ls") == 0) {//match found
                                                        listFile = 2;
                                                        
                                                } else {
                                                
                                                        printf("Invalid command please try again.");
                                                        exit;
                                                }        
                                        }
                                        
                                }
                                
                        }
                }
                
        }
        
        //printf("listFile = %d, countFile= %d, filterFile= %d, directory=%d ", listFile, countFile, filterFile, directory);
        regfree(&reg);
}

void applyGrep () {


        char *token1;
	token1 = strtok(command, " ");

	while(token1 != NULL) {
	
		const char *invalid_characters = ".";
                char *c = token1;
                while (*c) {
                     
                      if (strchr(invalid_characters, *c)) {
                          strcpy(regExp,token1);
                         // return;
                      }

                      c++;
                }
		token1 = strtok(NULL, " ");	
	}
	
	//int status = regcomp (exp, regExp, REG_NOSUB);
	int status = regcomp(&reg,regExp,REG_EXTENDED);
	
	if (status != 0) 
	{
		printf ("Error in compilation of regular expression\n");
		return;
	}
	
	//tokenize
	char *temp;
	temp = strtok(allFiles, "@@@@");
	
	if (listFile != 0 && countFile == 0)
	        printf("List of all files are : \n");
	
	while(temp != NULL) {
	        
	        if (listFile != 0 && filterFile == 0 && countFile == 0) {
	                printf("%s\n", temp);
	        }
	        allFileCount++;
	        int nomatch = regexec(&reg,temp,maxMATCH,noMatches,0);
	        
	       // if(!nomatch)
	        //printf("Match found for file %s", temp);
	
	        if (!nomatch) {
	                
	                if (listFile != 0 && filterFile == 1 && countFile == 0)
	                        printf("%s\n", temp);
  
		        fileCount++;
	        }
	        temp = strtok(NULL, "@@@@");	
	}
	
	//printf("Number of files : %d", fileCount);
}


void implmentGrepCommand() {
        
        
        if (countFile == 1 && filterFile == 0) {//just disply the count of all files
               
             applyGrep();
             printf("Number of files : %d\n\n", allFileCount);
        } else if (countFile == 1 && filterFile == 1) {//display the count of filter files
        
              applyGrep();
              printf("Number of files : %d\n\n", fileCount);
        } else if (listFile != 0 && filterFile == 0 ) {//display list of files.
        
              applyGrep();
        } else if (listFile != 0 && filterFile != 0 ) {//display filter files
                
             applyGrep();
        }
}


int main(int argc, char *argv[]){

   //char *command;
   //command = (char*)malloc(sizeof(char)*81);
   //command = "ls myFiles | grep -c A*.txt";
   
   if ( argc > 2 ) {
      printf("Too many arguments supplied.\n");
	exit(0);
   }
   else if (argc < 2){
      printf("One argument expected.\n");
	exit(0);
   }
   
   strcpy(command,argv[1]);
   
   //printf("%s", command);
   //command[strlen(command)-1] = '\0';
   //printf("before \n");
   testExpression();

   findListFiles();

  // printf("%d", data.size());
    int fd[2];
    pid_t pid;
    int result;

    //Creating a pipe
    result = pipe (fd);
    if (result < 0) {
        //failure in creating a pipe
        perror("pipe");
        exit (1);
    }

    //Creating a child process
    pid = fork();
    if (pid < 0) {
         //failure in creating a child
         perror ("fork");
         exit(2);
    }

    if (pid == 0) {
        //Child process
         //char message[MSGLEN];

          //while(1) {
                    //Clearing the message
                    //memset (message, 0, sizeof(message));
                    //printf ("Enter a message: ");
		    //scanf ("%20s",message);
                    //Writing message to the pipe
                    write(fd[1], allFiles, strlen(allFiles));
           // }
            exit (1);
    }
    else {
        //Parent Process
         //char message[10000];

         //while (1) {
                    //Clearing the message buffer
                    memset (parentBuffer, 0, sizeof(parentBuffer));

                    //Reading message from the pipe
                   read (fd[0], parentBuffer, sizeof(parentBuffer));
                    
                    implmentGrepCommand();
          //  }

            exit(1);
     }
}
