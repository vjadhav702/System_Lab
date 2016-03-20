#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

int b1,b2;
char strNumber[50];
char strAnswer[50]={0};
char temp[50]={0};

int isNumberValid(char *strNumber,int b1);
char* DecAny(int base, char *actual);
char *reverseString(char *str);
char* ATODec(int base, char *valueStr);

//To check whether number is valid
int isNumberValid(char *strNumber,int b1)
{
	int right = 1;
	int len = strlen(strNumber);
	int i = 0;
	int temp = 0;
	for(i=0;i<len;i++)
	{
		temp = 0;
		temp = (int)strNumber[i]-48;
		if(temp < 0 || temp >= b1)
		{
			right = 0;
			break;
		}
	}
	return right;
}


//This method is to reverse the string.
char *reverseString(char *str) {
      char *first, *second;

      if (! str || ! *str)
            return str;
      for (first = str, second = str + strlen(str) - 1; second > first; ++first, --second)
      {
            *first ^= *second;
            *second ^= *first;
            *p1 ^= *second;
      }
      return str;
}

//This method is use to convert number from decimal to any base.
char* DecAny(int base, char *actual)
{
        char *finalAns= (char*)malloc(sizeof(char)*50);
	
	int num = atoi(actual);
	int quo = 0;
	int rem = 0;
	int ans = 0;
	int i = 1;
	int j = 0;
	
	while(num != 0)
	{ 
		int quo = num/base;
		int rem = num%base;
		//char ch = rem + 48;
		
		if (rem >= 0 && rem <= 9)   //0-9
                {
                    finalAns[j] = (rem + 48);                   
                }
                else if (rem >= 10 && rem <= 35)   //A-Z
                {
                    finalAns[j] = (rem + 55);
                }
		
		//ans += (rem * i);
		//i *= 10;
		num = quo;
		
		j++;
	}
	reverseString(finalAns);
	return finalAns;
}



//This method is use to convert number from any base to decimal.
char* ATODec(int base, char *valueStr) {
	char *finalAns;
	finalAns = (char*) malloc (sizeof(char)*50);
	int power = strlen(valueStr)-1;
	int ans = 0;
	int i = 0;
	char ch;
	while(valueStr[i] != '\0') {
		ch = valueStr[i++];
		if (ch >= 48 && ch <= 57)
                {
                    ans += ((int)ch -48) * pow((double)base,(double)power);
                }
                else if (ch >= 65 && ch <= 90)
                {
                    ans += ((int)ch -55) * pow((double)base,(double)power);
                }
                else if (ch >= 97 && ch <= 122)
                {
                    ans += ((int)ch -87) * pow((double)base,(double)power);
                }
		power--;
	}
	sprintf(finalAns,"%d",ans);
	return finalAns;
}



void main()
{
	int i = 0;
	int right = 0;
	char ch;
	while(1)
	{
		while(1)
		{
			printf("Input from file ??(y/n) : ");
			scanf("%c",&ch);
			if(ch == '\n')
				scanf("%c",&ch);
			if(ch == 'y' || ch == 'Y' )
			{
				
				FILE *fp = fopen("input.txt","r");
				int right = -1;
				int i;
				if(fp != NULL)
				{
					FILE *op = fopen("outputC.txt","w");
					while(fscanf(fp,"%s %d %d",strNumber,&b1,&b2) != EOF)
					{
						
						if(b1<= 10)
							right = isNumberValid(strNumber,b1);
						if(right == 0 && b1 <= 10)
						{
							printf("Wrong number base %d\n",b1);
							fprintf(op,"Error..Wrong number base %d\n",b1);
						}
						else
						{
							
							for( i=0;i<50;i++ )
							{
								strAnswer[i] = '\0';
								temp[i] = '\0';
							}
							strcpy(temp,ATODec(b1,strNumber));
							strcpy(strAnswer,DecimalToAnyBase(b2,temp));
							fprintf(op,"%s\n",strAnswer);
							printf("number is = %s \n",strNumber);
							printf("Input Base is = %d\n", b1); 
							printf("Output Base is = %d\n", b2);
							printf("Output is = %s\n",strAnswer);
						}
					}
					fclose(op);
					fclose(fp);
				}
				else
				{
					printf("\nError ...\n");
				}
				
				break;
			}
			else if(ch == 'n' || ch == 'N' )
			{
				printf("Enter the number : ");
				scanf("%s",strNumber);
				printf("Input the base : ");
				scanf("%d",&b1);
				printf("Output the base : ");
				scanf("%d",&b2) ;
				int i;
				int right;
				for( i=0;i<50;i++ )
				{
					strAnswer[i] = '\0';
					temp[i] = '\0';
				}
		
				if(b1<= 10)
					right = isNumberValid(strNumber,b1);
				if(right == 0 && b1 <= 10)
				{
					printf("Entered number is not of base %d\n",b1);	
				}
				else
				{
					strcpy(temp,ATODec(b1,strNumber));
					strcpy(strAnswer,DecimalToAnyBase(b2,temp));
					printf("output  = %s\n",strAnswer);
				}
				
				break;
			}	
			else
			{
				printf("Invalid choice...\n");
			}
		}	
	}
}
