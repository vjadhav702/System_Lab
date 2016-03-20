#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<mpi.h>
#include<unistd.h>
#include<time.h>

#define add_item 2
#define exchange_item 0
#define buy_item 1
#define server 0
#define dealer 1
#define item_tag 1001
#define numberOfInputs 5


FILE *fp;

int update=0;
int my_id, root_process, errstat, num_procs;
MPI_Status status;      
int s1=-1;int z=1,i,k,l,m,n,sender;


int numDealer;//This variable is used to keep track of number of users in the system
int numUsers;//This variable is used to keep track of number of dealers in the system


//Contains the number of items which the dealer has in the repository 
int items[10]={0};
int pid,process_status=0;
char *array="hello";
int outgoing_opr_msg_confirmation[10];

/* format for incoming_opr_msg is:
index 0 will contain the item id,
index 1 will contain the operation; 0 -> Buy Operation, 1 -> Exhange Operation
index 2 will contain the quantity
index 3 will contain the dealer identifier
index 4 will contain the msg type; 0 req message, -99: All requests over*/
int incoming_opr_msg[10];

/* format for outgoing is:
index 0 will be for success or failure,
index 1 for timestamp,
index 2 for item,
index 3 for quantity
index 4 for operation;0 -> Sell, 1 -> Exchange
index 5 for dealer identifier
index 6 for the msg type; 0 req message, -99: All requests over*/
int outgoing_opr_msg[10];

int st[10];//status array

int parent_process,dealer_process,user_process[10]={-1};
int item_id,item_operation,item_quantity;// operation is to store whether we want to buy or add item or exchange  to the repository

int invCount[] = {100, 90, 80, 70, 60};//This array is used to keep track of inventory available.

int exitedUsers = 0;


/*
        This method is used to create log file for users as well as dealers
        It will also create input for users
*/
void user_log(int usernum, int msg, int *out)
{
	char fname[200];
	sprintf(fname,"user%d.log",usernum);
	FILE *fp=fopen(fname,"a");
	time_t cc=time(NULL);//=out[1];
	char tt[50];
	memset(tt,'\0',50);
	strcpy(tt,ctime(&cc));
	int len=strlen(tt);
	tt[len-1]='\0';
	fprintf(fp,"<%s>\t",tt);
	
	if(msg==1)
	{//Treat out as Outgoing_msg
		if(out[0]==1)
		{
			//Successful Transaction
			fprintf(fp,"Granted\t");
			fprintf(fp,"ITEM%d\t",out[2]);
			fprintf(fp,"%d<QTY>\n",out[3]);
		}//***********************************************Exchange code******************************************
		else
		{
			//Rejected
			fprintf(fp,"Rejected\t");
			fprintf(fp,"ITEM%d\t",out[2]);
			fprintf(fp,"%d<QTY>\n",out[3]);
		}
	}
	else
	{
		//Request
		fprintf(fp,"Requested\t");
		if(out[1]==1)//Exchange Operation
		{
			fprintf(fp,"Exchange\t");
		}
		else
		{
			fprintf(fp,"Buy\t");
		}
		
		fprintf(fp,"ITEM%d\t",out[0]);
		fprintf(fp,"%d<QTY>\t",out[2]);
		fprintf(fp,"From Dealer%d\n",out[3]);
	}
	
	fclose(fp);
}//End

/*
        This method is used to log different activities of the dealer.
*/
void update_dealer_log(int dealernum, int *out, int st)
{
	char fname[200];
	sprintf(fname,"dealer%d.log",dealernum);
	FILE *fp=fopen(fname,"a");
	
	time_t cc=time(NULL);
	char tt[50];
	memset(tt,'\0',50);
	strcpy(tt,ctime(&cc));
	int len=strlen(tt);
	tt[len-1]='\0';
	fprintf(fp,"<%s>\t",tt);
	
	if (st == 1)
	{	
		if (out[4] == 0) {
			fprintf(fp,"SOLD\t");
		} else {
			fprintf(fp,"EXCHANGED\t");
		}
		
		fprintf(fp,"ITEM%d\t",out[2]);
		fprintf(fp,"%d<QTY>\n",out[3]);
	}
	else if (st == 0)
	{
		if (out[4] == 0) {
			fprintf(fp,"REJECTED\t");
		} else {
			fprintf(fp,"EXCHANGE REJECTED\t");
		}
		fprintf(fp,"ITEM%d\t",out[2]);
		fprintf(fp,"%d<QTY>\n",out[3]);
	}

	fclose(fp);	
}//End


/*
        This is used to create log files for users as well as dealers
*/
void createlogfiles()
{
	srand(getpid());
	int i;
	char fname[200];
	char fnameInven[200];
	
	//This loop create log file for each user
	for(i=1;i<=numUsers;i++)
	{
		sprintf(fname,"user%d.log",i);
		fp=fopen(fname,"r");
		if(fp==NULL)
		{
			fp=fopen(fname,"w");
			fclose(fp);
		}
		else
		{
			fclose(fp);
		}
	}
	
	//This loop will create input for each user
	for(i=1;i<=numUsers;i++)
	{
		sprintf(fname,"user%d.inp",i);
		fp=fopen(fname,"r");
		
		int operation,itemId,itemQty,dealerId;
		
		if(fp==NULL)
		{
			int k;
			fp=fopen(fname,"w");
			for (k=0;k<numberOfInputs;k++)
			{
				//writing 5 user request in file
				int temp = rand()%97;
				
				operation = temp%2;			//Operation
				itemId = (temp%5);			//ItemID
				itemQty = (temp%100)+1;			//Quantity
				dealerId = (temp%numDealer)+1;	        //DealerID
				
				fprintf(fp, "%d %d %d %d\n", operation, itemId, itemQty, dealerId);
			}
			
			fclose(fp);
		}
		else
		{
			fclose(fp);
		}
	}
	
	//This loop will create log file for each dealer
	for(i=1;i<=numDealer;i++)
	{
		sprintf(fname,"dealer%d.log",i);
		
		fp=fopen(fname,"r");
		if(fp==NULL)
		{
			fp=fopen(fname,"w");
			fclose(fp);
		}
		else
		{
			fclose(fp);
		}
	}
	
	//This loop will create inventory for each dealer
	for(i=1;i<=numDealer;i++)
	{
		int j;
		
		sprintf(fnameInven,"dealer%d.inv",i);
		fp=fopen(fnameInven,"r");
		
		if (fp==NULL) {
		
			fp=fopen(fnameInven,"w");
			for (j=0;j<5;j++)
			{
				fprintf(fp, "%d\n", invCount[j]);
			}
		}
		fclose(fp);
	}
	
	//create server log file
	fp=fopen("server.log","r");
	if(fp==NULL)
	{
		fp=fopen("server.log","w");
		fclose(fp);
	}
	else
	{
		fclose(fp);
	}
}//End


/*
        This method is used to log different activities at server side by user
*/
void update_serverlog_user(FILE *fp2, int msg,int sender, int *out)
{
	time_t cc=out[1];
	char tt[50];
	memset(tt,'\0',50);
	strcpy(tt,ctime(&cc));
	int len=strlen(tt);
	tt[len-1]='\0';
	fprintf(fp2,"<%s>\t",tt);
	if(out[0]==1)
	{
		if(msg==0)
		{	
			if (out[4] == 0)
			{
				fprintf(fp2,"Buy Request\tUser%d\t",sender);
				printf("Buy Request\tUser%d\t",sender);
			}
			else
			{
				fprintf(fp2,"Exchange Request\tUser%d\t",sender);
				printf("Exchange Request\tUser%d\t",sender);
			}
			
		}
		else
		{
			fprintf(fp2,"Granted\tUser%d\t",sender);
			if (out[4] == 0)
			{
				fprintf(fp2,"BOUGHT\t");
				printf("BOUGHT\t");
			}
			else
			{
				fprintf(fp2,"EXCHANGED\t");
				printf("EXCHANGED\t");
			}
			
		}
		fprintf(fp2,"ITEM%d\t",out[2]);
		printf("ITEM%d\t",out[2]);
		fprintf(fp2,"%d<QTY>\t",out[3]);
		printf("%d<QTY>\t",out[3]);
		fprintf(fp2,"From Dealer%d\n",out[5]);
		printf("From Dealer%d\n",out[5]);
	}
	else
	{
		if(msg==0)
		{
			if (out[4] == 0)
			{
				fprintf(fp2,"Buy Request\tUser%d\t",sender);
				printf("Buy Request\tUser%d\t",sender);
			}
			else
			{
				fprintf(fp2,"Exchange Request\tUser%d\t",sender);
				printf("Exchange Request\tUser%d\t",sender);
			}
		}
		else if(msg==1)
		{
			if (out[4] == 0)
			{
				fprintf(fp2,"Buy Rejected\tUser%d\t",sender);
				printf("Buy Rejected\tUser%d\t",sender);
			}
			else
			{
				fprintf(fp2,"Exchange Rejected\tUser%d\t",sender);
				printf("Exchange Rejected\tUser%d\t",sender);
			}
		}
		//Rejected
		
		fprintf(fp2,"ITEM%d\t",out[2]);
		printf("ITEM%d\t",out[2]);
		fprintf(fp2,"%d<QTY>\n",out[3]);
		printf("%d<QTY>\n",out[3]);
		fprintf(fp2,"From Dealer%d\n",out[5]);
		printf("From Dealer%d\n",out[5]);
	}
}//End


/*
        This method is used to log different activities at server side by dealer
*/
void update_serverlog_dealer(FILE *fp2, int msg,int d, int *out)
{
	time_t cc=out[1];
	char tt[50];
	memset(tt,'\0',50);
	strcpy(tt,ctime(&cc));
	int len=strlen(tt);
	tt[len-1]='\0';
	fprintf(fp2,"<%s>\t",tt);
	if(out[0]==1)
	{
		if(msg==0)
		{	
			if (out[4] == 0) 
			{
				fprintf(fp2,"Buy\tRequest\tDealer%d\t",d);
				printf("Buy\tRequest\tDealer%d\t",d);
			}
			else 
			{
				fprintf(fp2,"Exchange\tRequest\tDealer%d\t",d);
				printf("Exchange\tRequest\tDealer%d\t",d);
			}

		}
		else if(msg==1)
		{	
			if (out[4] == 0)
			{
				fprintf(fp2,"SOLD\tDealer%d\t",d);
				printf("SOLD\tDealer%d\t",d);
			} 
			else
			{
				fprintf(fp2,"EXCHANGED\tDealer%d\t",d);
				printf("EXCHANGED\tDealer%d\t",d);
			}
			
		}
		fprintf(fp2,"ITEM%d\t",out[2]);
		printf("ITEM%d\t",out[2]);
		fprintf(fp2,"%d<QTY>\t",out[3]);
		printf("%d<QTY>\t",out[3]);
		fprintf(fp2,"From Dealer%d\n",out[5]);
		printf("From Dealer%d\n",out[5]);
	}
	else
	{
		if(msg==0)
		{	
			if (out[4] == 0) 
			{
				fprintf(fp2,"Buy\tRequest\tDealer%d\t",d);
				printf("Buy\tRequest\tDealer%d\t",d);
			} 
			else 
			{
				fprintf(fp2,"Exchange\tRequest\tDealer%d\t",d);
				printf("Exchange\tRequest\tDealer%d\t",d);
			}
			
		}
		else if(msg==1)
		{	
			if (out[4] == 0)
			{
				fprintf(fp2,"Buy\tRejected\tDealer%d",d);
				printf("Buy\tRejected\tDealer%d",d);
			} 
			else 
			{
				fprintf(fp2,"Exchange\tRejected\tDealer%d",d);
				printf("Exchange\tRejected\tDealer%d",d);
			}
			
		}
		
		fprintf(fp2,"ITEM%d\t",out[2]);
		printf("ITEM%d\t",out[2]);
		fprintf(fp2,"%d<QTY>\t",out[3]);
		printf("%d<QTY>\t",out[3]);
		fprintf(fp2,"From Dealer%d\n",out[5]);
		printf("From Dealer%d\n",out[5]);
	}
}//End

/*
        This method is used to check the inventory if any user has requested the user for given item.
        This method will check the availabilt of the given item and give reply
*/
int checkdealerinv(int id,int itemid,int qty)
{
	char fname[200];
	sprintf(fname,"dealer%d.inv",id);
	FILE *fp=fopen(fname,"r");
	int i;
	for(i=0;i<5;i++)
	{
		int data;
		errstat = fscanf(fp,"%d",&data);
		//printf("%d\n",data);
		if(i == itemid)
		{
			if(data >= qty)
			return 1;
			else 
			return 0;
		}
	}
	fclose(fp);
}//End


/*
        As name suggest this is the main method. This method contains the actual division of process to user or dealer.
        This method basically assings one process to simulator and some process to dealer and some process to users.
        Based on the process ids we will give work to each process. 
*/
int main(int argc, char **argv)
{	
	
	//Initiate an MPI computation
	errstat = MPI_Init(&argc, &argv);
	
	//Store this process's ID in my_id
	errstat = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	
	
	//Find the number of process started by the simulator
	errstat = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	srand(time(NULL));
	
	numDealer = atoi(argv[1]);
	
	if (numDealer > num_procs-2)
	{
		
		printf("Please enter valid number of dealers\n");
		exit(0);
	}
	
	
	numUsers = num_procs-1-numDealer;
	
	
	//Creating the log files for each user and dealer
	createlogfiles();
	
	
	if(my_id == 0)//Simulator process
	{
		printf("=======================DAEMON TERMINAL SCREEN==========================\n");
		
		while(1)//This will be waiting for request from usres as well as dealer.
		{
			FILE *fp2 = fopen("server.log","a");
			
			//Here server is waiting for the request from user.
			errstat = MPI_Recv(&incoming_opr_msg,5,MPI_INT,MPI_ANY_SOURCE,item_tag,MPI_COMM_WORLD,&status);
			sender = status.MPI_SOURCE;
			
			if(incoming_opr_msg[4]!= -99)
			{
				outgoing_opr_msg[0]=-1;
				outgoing_opr_msg[2]=incoming_opr_msg[0];
				outgoing_opr_msg[3]=incoming_opr_msg[2];
				outgoing_opr_msg[5]=incoming_opr_msg[3];
				outgoing_opr_msg[4]=incoming_opr_msg[1];
				outgoing_opr_msg[6]=0;//Request type message
			
				 
				//sending request to dealer to check availability of item
				errstat = MPI_Send(&outgoing_opr_msg, 7 , MPI_INT,outgoing_opr_msg[5],item_tag, MPI_COMM_WORLD);
                                
                                //Wait for the responce from the dealer 
				errstat = MPI_Recv(&st,1,MPI_INT,outgoing_opr_msg[5],item_tag,MPI_COMM_WORLD,&status);
        
                                //Based on the responce from the delaer coresspoding action will taken by the simulator			
				if(st[0]==1)
				{	
					
					outgoing_opr_msg[0]=1;
					outgoing_opr_msg[1]=time(NULL);
					outgoing_opr_msg[2]=incoming_opr_msg[0];
					outgoing_opr_msg[3]=incoming_opr_msg[2];
					outgoing_opr_msg[5]=incoming_opr_msg[3];
					outgoing_opr_msg[4]=incoming_opr_msg[1];
					//outgoing_opr_msg[6]=0;//Request type message

                                        //Log user request				
					update_serverlog_user(fp2,0,sender-numDealer,outgoing_opr_msg);
				
					//Log dealer request to server
					update_serverlog_dealer(fp2,0,outgoing_opr_msg[5],outgoing_opr_msg);
				
					update_serverlog_dealer(fp2,1,outgoing_opr_msg[5],outgoing_opr_msg);
				
                                        //Send appropiate message to sender about its request				
					errstat = MPI_Send(&outgoing_opr_msg, 6, MPI_INT,sender,item_tag, MPI_COMM_WORLD);
				
					printf("success\n");
				}
				else
				{
					// in case of unsuccessful transaction what we need to do

					outgoing_opr_msg[0]=0;		                //status
					outgoing_opr_msg[1]=time(NULL);	                //timestamp
					outgoing_opr_msg[2]=incoming_opr_msg[0];	//itemcount
					outgoing_opr_msg[3]=incoming_opr_msg[2];	//Quantity
					outgoing_opr_msg[5]=incoming_opr_msg[3];
				
					//Log <Reject_Message> to User about Rejection of request
					errstat = MPI_Send(&outgoing_opr_msg, 6, MPI_INT,sender,item_tag, MPI_COMM_WORLD);
                                        
                                        //Log all the activities				
					update_serverlog_user(fp2,0,sender-numDealer,outgoing_opr_msg);
				
					update_serverlog_dealer(fp2,0,outgoing_opr_msg[5],outgoing_opr_msg);
				
					update_serverlog_dealer(fp2,1,outgoing_opr_msg[5],outgoing_opr_msg);				
				
					printf("Unsuccess\n");
				}
				//Log <Success_Message> to User in server log file
				update_serverlog_user(fp2,1,sender-numDealer,outgoing_opr_msg);
				fprintf(fp2,"\n");
				printf("\n");
				fclose(fp2);
			}
			else
			{
				exitedUsers++;
				if(exitedUsers == numUsers)
				{
					//Send a message to all Dealers to exit
					outgoing_opr_msg[6]=-99;//Exit type message
					int z;
					for(z=1;z<=numDealer;z++)
						errstat = MPI_Send(&outgoing_opr_msg, 7 , MPI_INT,z,item_tag, MPI_COMM_WORLD);
					
					break;
				}
			}
				
		}
		printf("Server Exiting!!...\n");
	}
	/*dealer process code*/
	else if(my_id >=1 && my_id <= numDealer)
	{
		while(1)
		{
			//wait for request
			errstat = MPI_Recv( &outgoing_opr_msg, 7, MPI_INT,MPI_ANY_SOURCE,item_tag, MPI_COMM_WORLD, &status);
			
			if(outgoing_opr_msg[6] !=-99)
			{
				//check the availabilty of the request in the inventory
				st[0] = checkdealerinv(my_id,outgoing_opr_msg[2],outgoing_opr_msg[3]);

				errstat = MPI_Send(&st, 1 , MPI_INT,server,item_tag, MPI_COMM_WORLD);
			
				if(st[0]==1)
				{//Available with dealer
				
			
					sender = status.MPI_SOURCE;		
					update_dealer_log(my_id, outgoing_opr_msg, 1);	//understand what to log based on st		
					//now update the inventory file to represent the present scenario of the stock
			
				
					char fname[200];
					sprintf(fname,"dealer%d.inv",my_id);
					FILE *fp=fopen(fname,"r");
					int i;
					for(i=0;i<5;i++)
					{	
						errstat = fscanf(fp,"%d",&items[i]);
						if(i == outgoing_opr_msg[2])
						{
							items[i] = items[i] - outgoing_opr_msg[3];
						}
					}
					fclose(fp);
	
	                                /*
	                                        If items in the inventory goes below some count then system will add 
	                                        Some amount of items to the inventory.
	                                */
					fp=fopen(fname,"w");
					for(i=0;i<5;i++)
					{
						if(items[i]==0)
						{
							items[i] += invCount[i];
						
							time_t cc=time(NULL);//=out[1];
							char tt[50];
							memset(tt,'\0',50);
							strcpy(tt,ctime(&cc));
							int len=strlen(tt);
							tt[len-1]='\0';
						
							char fnm[200];
							sprintf(fnm,"dealer%d.log",my_id);
							FILE *fr=fopen(fnm,"w");
							fprintf(fr,"<%s>\t",tt);
							fprintf(fr,"<ADD_ITEM> ITEM%d\t%d\n",i,invCount[i]);
							fclose(fr);
						}
						fprintf(fp,"%d\n",items[i]);
					}
					fclose(fp);
				}
				else
				{
					update_dealer_log(my_id, outgoing_opr_msg, 0);//understand what to log based on st
				
				}
			}
			else
			{
				
				printf("Dealer%d Exiting!!...\n",my_id);
				break;
				errstat = MPI_Finalize();
				//exit(0);
			}
			
		}		
	}
	/*user process code */
	else if(my_id > numDealer)
	{
		
		char fname[200];
		sprintf(fname,"user%d.inp",(my_id-numDealer));
		
		FILE *fpInp = fopen(fname,"r");
		
		//Take input from the user file and fire rquest to server
		while(fscanf(fpInp,"%d %d %d %d", &incoming_opr_msg[1], 
		        &incoming_opr_msg[0], &incoming_opr_msg[2], &incoming_opr_msg[3]) != -1)
		{
			user_log(my_id-numDealer,0,incoming_opr_msg);
			
			incoming_opr_msg[4]=0;
			errstat = MPI_Send(&incoming_opr_msg, 5 , MPI_INT,server,item_tag, MPI_COMM_WORLD);
			
			// Receive the response from the server, which will send and update the user log file 	
			errstat = MPI_Recv( &outgoing_opr_msg, 6, MPI_INT,MPI_ANY_SOURCE,item_tag, MPI_COMM_WORLD, &status);
			outgoing_opr_msg[1] = time(NULL);		
			
			sender = status.MPI_SOURCE;		
			
			//format=this_process_id,Req/Response,array_name
			user_log(my_id-numDealer,1,outgoing_opr_msg);
			
		}
		
		incoming_opr_msg[4]=-99;
		errstat = MPI_Send(&incoming_opr_msg, 5 , MPI_INT,server,item_tag, MPI_COMM_WORLD);
		printf("User%d Exiting!!...\n",my_id-numDealer);
	} 	
		
	errstat = MPI_Finalize();
	return 0;
}
