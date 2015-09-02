
#include "syscall.h"


/*----------------------------------------------------------------------------------------------------------------------------------------------
Declaration  of monitor variables, condition variables, Locks and other variables
----------------------------------------------------------------------------------------------------------------------------------------------*/

int stationLock;				      /*Lock on the Station*/
int phoneLock[20];				  /*Array of Locks for 20 phone booths*/
int genOperatorLock;				  /*Lock on All operators*/
int operatorLock[3];				  /*Lock on each Operator*/
/*Lock freeLock;*/
int idLock;						  /*Lock  to Acquire id*/

int opCnt;							  /*Operator Count*/
int vsCnt;							  /*Visitor Count*/
int seCnt;			                  /*Senator Count*/
int avlPhoneCnt=20;					  /*Monitor Variable to keep Count of available phones*/
int avlOperatorCnt;					  /*Monitor Variable to keep track of available Operators*/	
int presidentStatus=0;				  /*Monitor Variable to Monitor President's Status*/
int waitingVisitorsCnt=0;			  /*Monitor Variable to keep count of waiting Visitors*/
int waitingSenatorsCnt=0;			  /*Monitor Variable to keep count of waiting Senators*/
int phoneStatus[20];				  /*Monitor Variable to check status of phones (BUSY or FREE)*/
int operatorStatus[20];				  /*Monitor Variable to check status of each Operator (BUSY or FREE)	*/
int talkWithOperator[20];				  /*Variable to make Caller talk to an Operator*/
int callerID[20];						  /*Variable to hold present caller's ID of callers calling various Operators*/
int visitorIDCnt=0;					  /*Variable used to assign IDs to Visitors*/
int senatorIDCnt=0;					  /*Variable used to assign IDs to Senators*/
int operatorIDCnt=0;				  /*Variable used to assign IDs to Operators*/

int pp=0;							  /*Flag variable*/
int ss=0;							  /*Flag variable*/

/*Condition Variables  for various conditions */

int visitorWaitingCondition;	  

int senatorWaitingCondition;

int presidentWaitingCondition;

int operatorCondition;

int operatorNeeded[3];

int writeLock;
int readLock;

/*
void delay(int);
void Visitor(int);
void Operator(int);
void Senator(int);
void President(int);
void SenateFuncDync();		  		            */

/*Delay Function*/

void delay(int a)
{
	int i,j;
  
	for(i=0;i<100000;i++)
	{   
		for(j=0;j<a;j++)
		{
		}
	}
}

/*Visitor Function*/

void Visitor(int arg)
{  
	char buf[10][6];
	int idLocal;
	int place=-999,i;
	int place1=-999,j,k;
	
	Acquire(idLock);
	idLocal=visitorIDCnt++;			/*Assign ID to The Visitor*/
	Release(idLock);
	
	
	
	Acquire(stationLock);		  		/*Acquire Lock to Phone Station*/
	
	
	for(i=0;i<10;i++)
	{
		for(j=0;j<6;j++)
		{
			buf[i][j]='\0';
		}
	}
	
	Acquire(writeLock);
	
	Write("Visitor  \n",10,1);
	
/*	itoa(buf[0],6,idLocal);
	Write(buf[0],6,1);		  */
	
	Print(idLocal);
	
	Write(" Acquired Lock on Station  \n",28,1);
	
	Release(writeLock);
			
	
	if(presidentStatus==1 || waitingVisitorsCnt>0 || waitingSenatorsCnt>0 || ss==1 || pp==1)
	{
		waitingVisitorsCnt++;								/*Thread  will go to wait  if President is there, */
		Wait(visitorWaitingCondition,stationLock);		/*If  there are other waiters who had arrived before */
		waitingVisitorsCnt--;
	}
	
	Acquire(writeLock);
	
	Write("About to check phone booths  \n",30,1);
	
	Release(writeLock);
	
	while(avlPhoneCnt==0)           
	{	  
		waitingVisitorsCnt++;		
		Wait(visitorWaitingCondition,stationLock);		/*Thread will go to wait 	if no phones are available	*/
		waitingVisitorsCnt--;
	}

    /*Reach this point if Phone has been made available to the thread*/
	
	Acquire(writeLock);
	
	Write("Visitor  \n",10,1);
	
/*	itoa(buf[1],6,idLocal);
	Write(buf[1],6,1);		  */
	
	Print(idLocal);
	
	Write("  Phone made available  \n",25,1);
	
	Release(writeLock);
		
	
	
	
	for(i=0;i<20;i++)
	{	
		if(phoneStatus[i]==0)
		{
			place=i;
			break;
		}
	}
	
	if(place==-999)
	{
	/*	cout<<"Error : Phone was made available, but was consumed by some other thread . . . ";
		cout<<"Aborting . . . ";		*/
		return ;
	}

	else
	{		
		avlPhoneCnt--;
		Acquire(phoneLock[place]);					/*Acquire the phone and decrease the Phone Count*/
		
		Acquire(writeLock);
		
		Write("Visitor  \n",10,1);
	
/*		itoa(buf[2],6,idLocal);
		Write(buf[2],6,1);			    */
		
		Print(idLocal);
	
		Write("  Acquired Lock on phone  \n",27,1);
		
/*		itoa(buf[3],6,place);
		Write(buf[3],6,1);		  		      */
		
		Print(place);
		
		Release(writeLock);
				
		phoneStatus[place]=1;							/*Set Phone's status to BUSY*/
		
		Release(stationLock);							/*Release Station Lock*/
		
		Acquire(writeLock);
		
		Write("Visitor \n",9,1);
	
/*		itoa(buf[4],6,idLocal);
		Write(buf[4],6,1);		  		      */
		
		Print(idLocal);
	
		Write("  Releasing stationLock  \n",26,1);		
		
		Release(writeLock);
		
		Acquire(genOperatorLock);						/*Acquire Lock on All Operators, for checking if one is free*/
		
		while(avlOperatorCnt==0)
		{
			Wait(operatorCondition,genOperatorLock);	/*If none available go to Sleep*/
		}
		
		/*Operator was made available and so this sleeping thread was signalled, now going to Acquire the Operator*/
		
		
		for(i=0;i<opCnt;i++)
		{
			if(operatorStatus[i]==0)			
			{
				place1=i;
				break;
			}
		}
		
		if(place1==-999)
		{
		/*	cout<<"Error : Operator made available, consumed by someone else . . . \n";*/
		}
		else
		{
			avlOperatorCnt--;
			Acquire(operatorLock[place1]);		/*Acquires Lock on the Operator and decreases the available Operator Count*/
			
			Acquire(writeLock);
			
			Write("Visitor  \n",10,1);
			
/*			itoa(buf[5],6,idLocal);
			Write(buf[5],6,1);		  		  */
			
			Print(idLocal);
			
			Write("  Acquired Lock on operator  \n",30,1);

/*			itoa(buf[6],6,place1);
			Write(buf[6],6,1);			*/
			
			Print(place1);
			
			Write("\n",1,1);
			
			Release(writeLock);
							
			operatorStatus[place1]=1;				/*Set Operator Status as BUSY*/
			
			talkWithOperator[place1]=1;				/*Sets flag for the Operator (Operator knows that a caller wants to talk with him/her and*/
			callerID[place1]=idLocal;				/*Checks the value for its corresponding CallerID*/
			
			Release(operatorLock[place1]);

			Signal(operatorNeeded[place1],operatorLock[place1]); /*Signal the Operator */

			Acquire(operatorLock[place1]);
			
			Release(genOperatorLock);							  /*Release the general Operator Lock*/
			
			Wait(operatorNeeded[place1],operatorLock[place1]);   
			
			delay(500);		
			
			operatorStatus[place1]=0;			
			avlOperatorCnt++;
			
			Signal(operatorCondition,genOperatorLock);		 /*Talking with the Operator*/
								
			Release(operatorLock[place1]);					 /*Releasing Operator Lock*/
			
			delay(1000);		
			
			avlPhoneCnt++;									     /*Increase available Phone Count*/
			phoneStatus[place]=0;								 /*Make phone available (FREE)	*/
			
			
			/*Phone available, So signal the waiting threads according to the conditions */
			/*If President available, Signal him/her,else Senators else Visitors*/
			
			if(presidentStatus==1)
			{				
				if(avlPhoneCnt==20 && avlOperatorCnt==opCnt)
				{
					Signal(presidentWaitingCondition,stationLock);
				}
			}							
			else
			{
				if(waitingSenatorsCnt>0)
				{
					Signal(senatorWaitingCondition,stationLock);
				}
				else
				{
					Signal(visitorWaitingCondition,stationLock);
				}
			}
			
			Release(phoneLock[place]);						/*Release Phone*/
			
			Acquire(writeLock);
			
			Write("Visitor  \n",10,1);
	
/*			itoa(buf[7],6,idLocal);
			Write(buf[7],6,1);		  		  */
			
			Print(idLocal);
	
			Write("  Releasing Phone booth  \n",27,1);
			
/*			itoa(buf[8],6,place);
			Write(buf[8],6,1);		  		      */
			
			Print(place);
			
			Write("\n",1,1);
			
			Release(writeLock);
			
			
			
		}
		
		
		
	}
	
	Exit(0);
	
}

/*End of visitor Function*/

/*Operator Function*/

void Operator(int arg)
{
	char buf[10][6];
	int id;
	int i=0,j,k;
	
	for(j=0;j<10;j++)
	{
		for(k=0;k<6;k++)
		{
			buf[j][k]='\0';
		}
	}
	
	Acquire(idLock);
	id=operatorIDCnt;								/*Assign ID to Operator*/
	operatorIDCnt++;
	Release(idLock);
				
				
	Acquire(operatorLock[id]);
	Wait(operatorNeeded[id],operatorLock[id]);			/*Operator Waiting for a Request*/
	Release(operatorLock[id]);
    
	
	talkWithOperator[id]=1;								/*Flag Variable checking if Operator is needed*/
	
	
	while(1)
	{
	 
		if(talkWithOperator[id]==0)
		{						
			Acquire(operatorLock[id]);
			Wait(operatorNeeded[id],operatorLock[id]); /*Operator Waiting for Requests*/
			Release(operatorLock[id]);
			talkWithOperator[id]=1;
		 	
	        }
		else
		{	
			/*Operator Checking the Caller IDs to check who called, and accordingly taking money or Allow to proceed, on recognizing*/
			/*the caller to be a Senator or a President*/
			if(callerID[id]>=0 && callerID[id]<vsCnt)
			{
				Acquire(writeLock);
				
				Write("Hello Visitor  \n",16,1);
	
/*				itoa(buf[0],6,callerID[id]);
				Write(buf[0],6,1);										*/
				
				Print(callerID[id]);
	
				Write(" ! Please pay $1  Money Paid  Call can be made\n",47,1);
				
				Release(writeLock);
								
			}
			if(callerID[id]>=vsCnt && callerID[id]<(vsCnt+seCnt))
			{
				Acquire(writeLock);
				
				Write("Hello Senator with Senator ID  \n",32,1);
	
/*				itoa(buf[1],6,callerID[id]);
				Write(buf[1],6,1);		  		  */
				
				Print(callerID[id]);
	
				Write(" ! Call can be made now  \n",26,1);				
				
				Release(writeLock);
			}
			if(callerID[id]==(vsCnt+seCnt))
			{
				Acquire(writeLock);
				
				Write("Hello Mr. President . Call can be made now  \n",45,1);
				
				Release(writeLock);
			}
			
			talkWithOperator[id]=0;						/*sets the flag to 0 indicating Operator FREE*/
			
			Signal(operatorNeeded[id],operatorLock[id]);  
		}
		i++;
	          
	}
	
	Exit(0);
	
}

/*End of Operator function*/

/*Senator Function*/

void Senator(int arg)
{
	char buf[20][6];
	int idLocal;	
	int cnt=0;
	int place=-999,i;
	int place1=-999;
	int r,j,k;
	ss=1;
	
	for(j=0;j<20;j++)
	{
		for(k=0;k<6;k++)
		{
			buf[j][k]='\0';
		}
	}
	
	Acquire(idLock);	
	idLocal=senatorIDCnt;						 	/*Assigning ID to Senator*/
	senatorIDCnt++;	
	Release(idLock);
	
	
	while(1)
	{
		ss=1;
		
		cnt++;
		if(cnt==4)
		{
			break;
		}
	
		Acquire(stationLock);							/*Acquire Lock on station*/
		
		Acquire(writeLock);
	
		Write("Senator  \n",10,1);
	
/*		itoa(buf[0],6,idLocal);
		Write(buf[0],6,1);				      */
		
		Print(idLocal);
	
		Write(" Acquired Lock on Station  \n",28,1);
		
		Release(writeLock);
				
	
		if(presidentStatus==1 || waitingSenatorsCnt>0 || pp==1)
		{
			waitingSenatorsCnt++;			
			Wait(senatorWaitingCondition,stationLock); /*Senator waits if President is there or other Senators are waiting			*/
			waitingSenatorsCnt--;		
		}
	
		while(avlPhoneCnt==0)
		{
			waitingSenatorsCnt++;			
			Wait(senatorWaitingCondition,stationLock); /*Waits if pnone not available			*/
			waitingSenatorsCnt--;
		}
		
		ss=0;
		
		Acquire(writeLock);
		
		
		Write("Senator  \n",10,1);
	
/*		itoa(buf[1],6,idLocal);
		Write(buf[1],6,1);				  		    */
		
		Print(idLocal);
	
		Write("  Phone made available  \n",24,1);
		
		Release(writeLock);
	
		/*Phone made available for the thread, Goes to Acquire it*/
	
		
	
		for(i=0;i<20;i++)
		{
			if(phoneStatus[i]==0)
			{
				place=i;			
				break;
			}
		}	
	
		if(place==-999)
		{
		/*	cout<<"\nError : Phone made available, was consumed by someone else . . . ";*/			/*Exception ! */
		/*	cout<<"\nAborting . . . ";		*/
			return ;
		}
		else
		{		
			avlPhoneCnt--;
			phoneStatus[i]=1;					/*Acquires  Lock on Phone,Decreases available phone count, sets phone status BUSY*/
			Acquire(phoneLock[place]);		
			
			Acquire(writeLock);

			Write("Senator  \n",10,1);
	
/*			itoa(buf[2],6,idLocal);
			Write(buf[2],6,1);				*/
			
			Print(idLocal);
	
			Write("  Acquired Lock on phone  \n",27,1);
		
/*			itoa(buf[3],6,place);
			Write(buf[3],6,1);		  		      */
			
			Print(place);
			
			Release(writeLock);
		
			Release(stationLock);				/*Releases station Lock*/
		
			Acquire(genOperatorLock);			/*Acquires general Operator Lock*/
			
       		while(avlOperatorCnt==0)
			{
				Wait(operatorCondition,genOperatorLock); /*Wait till an operator becomes available*/
			}
		
			
			
			for(i=0;i<opCnt;i++)
			{
				if(operatorStatus[i]==0)
				{
					place1=i;
					break;
				}
			}
		
			if(place==-999)
			{
			/*	cout<<"Error : Phone made available, was consumed by someone else . . . ";		*/	/*Exception ! */
			/*	cout<<"Aborting . . . Press any key to abort";*/
			/*			getch();*/
				return ;
			}
			else
			{
				Acquire(operatorLock[place1]);		/*Acquires Lock an available Operator*/
				
				Release(genOperatorLock);				/*Releases the general Operator Lock*/
				
				Acquire(writeLock);
				
				Write("Visitor  \n",10,1);
			
/*				itoa(buf[4],6,idLocal);
				Write(buf[4],6,1);		  		      */
				
				Print(idLocal);
			
				Write("  Acquired Lock on operator  \n",30,1);

/*				itoa(buf[5],6,place1);
				Write(buf[5],6,1);					  		  */
				
				Print(place1);
			
				Write("\n",1,1);
				
				Release(writeLock);
				
				
				avlOperatorCnt--;
				operatorStatus[place1]=1;				/*Decreases available Operator Count , makes the operator BUSY*/
									
				talkWithOperator[place1]=1;				/*Sets the Flag for Operator to know that a thread wants to talk*/
				callerID[place1]=idLocal;				/*Gives the Caller ID*/
			
				Release(operatorLock[place1]);							
			
				Signal(operatorNeeded[place1],operatorLock[place1]);						
			
				Acquire(operatorLock[place1]);
			
				Wait(operatorNeeded[place1],operatorLock[place1]);							
			
				delay(500);			
				
				operatorStatus[place1]=0;				/*Sets the Operator Status to FREE*/
				avlOperatorCnt++;						/*Increase available Operator Count*/
				
				Signal(operatorCondition,genOperatorLock);
			
				
				Release(operatorLock[place1]);
				
				Acquire(writeLock);
				
				Write("Senator  \n",10,1);
				
/*				itoa(buf[6],6,idLocal);
				Write(buf[6],6,1);		  		  */
				
				Print(idLocal);
				
				Write("  Operator  \n",13,1);
				
/*				itoa(buf[7],6,place1);
				Write(buf[7],6,1);		  		  */
				
				Print(place1);
				
				Write(" Released\n",10,1);

				Release(writeLock);
				
				delay(1000);

				Acquire(writeLock);
				
				Write("Senator  \n",10,1);
				
/*				itoa(buf[8],6,idLocal);
				Write(buf[8],6,1);		  		          */
				
				Print(idLocal);
				
				Write("  Done talking. Senator Exiting  \n",34,1);								
				
				Release(writeLock);
				
				avlPhoneCnt++;							/*Increase available phone count, sets phone status to FREE*/
				phoneStatus[place]=0;
				
				/*Phone available, So signal the waiting threads according to the conditions */
				/*If President available, Signal him/her,else Senators else Visitors*/
				
				if(presidentStatus==1)
				{
					if(avlPhoneCnt==20 && avlOperatorCnt==opCnt)
					{
						Signal(presidentWaitingCondition,stationLock);	
					}				
					
					
				}
				else
				{
					if(waitingSenatorsCnt>0)
					{
						Signal(senatorWaitingCondition,stationLock);
					}
					else
					{
						Signal(visitorWaitingCondition,stationLock);
					}
				}
				
				Release(phoneLock[place]);
				
				Acquire(writeLock);
				
				Write("Senator  \n",10,1);
				
/*				itoa(buf[9],6,idLocal);
				Write(buf[9],6,1);		  		      */
				
				Print(idLocal);
				
				Write("  Releases Lock on phone  \n",27,1);
				
/*				itoa(buf[10],6,place);
				Write(buf[10],6,1);		  		    */
				
				Print(place);
				
				Release(writeLock);
										
			}
		}
		
		
				
		r=(rand()%3)+1;
		
		
		if(cnt!=3)
		{
			Acquire(writeLock);
			
			Write("Senator Going. will come after \n",32,1);
			
/*			itoa(buf[11],6,r);
			Write(buf[11],6,1);				  		  */
			
			Print(r);
			
			Write(" seconds \n",10,1);
			
			Release(writeLock);
			
		}
		
		r=r*1000;
		delay(r);
		
	}
	
	Exit(0);
	
}

/*End of Senator function*/

/*President Function*/

void President(int arg)
{
	char buf[10][6];
	int idLocal;
	int cnt=0;
	int a=0;
	int r=0,j,k;
	pp=1;
	
	for(j=0;j<10;j++)
	{
		for(k=0;k<6;k++)
		{
			buf[j][k]='\0';
		}
	}
	
	Acquire(idLock);
	idLocal=30;
	Release(idLock);
	
	
	
	while(1)
	{
		pp=1;
		cnt++;												/*President allowed to come 5 times*/		  
		if(cnt==6)
		{		  
		  break;
		}
		
		Acquire(stationLock);							   /*Acquires stationLock*/
		
		Acquire(writeLock);
		
		Write("President  \n",12,1);
	
/*		itoa(buf[0],6,idLocal);
		Write(buf[0],6,1);			  		    */
		
		Print(idLocal);
	
		Write(" Acquired Lock on Station  \n",28,1);
		
	    Release(writeLock);
						
		presidentStatus=1;								   /*Sets status of President to True, for others to note*/ 
		
		Acquire(writeLock);
				
		Write("President  \n",12,1);
	
/*		itoa(buf[1],6,idLocal);
		Write(buf[1],6,1);				    		      */
		
		Print(idLocal);
	
		Write("  Releasing stationLock  \n",26,1);		
		
		Release(writeLock);
				
		
		
		if(avlPhoneCnt!=20)
		{
			Wait(presidentWaitingCondition,stationLock);	
		}
		
		Release(stationLock);							  /*Releasing station Lock				*/

		Acquire(phoneLock[0]);						  /*Acquire lock on phone */
		
		Acquire(writeLock);
		
		Write("President Acquired Phone  \n",27,1);
		
		Write("Number of available phones  \n",29,1);
		
/*		itoa(buf[2],6,avlPhoneCnt);
		Write(buf[2],6,1);  				      */
		
		Print(avlPhoneCnt);
		
		Write("\n",1,1);
		
		Release(writeLock);
		
		Acquire(genOperatorLock);		
		
		Acquire(operatorLock[0]);						 /*Acquires Lock on an Operator*/
		
		Acquire(writeLock);

		Write("President talking with Operator  \n",34,1);
		
		Release(writeLock);
		
		talkWithOperator[0]=1;					/*Set flag to inform Operator that President wants to talk*/

		delay(500);               
	
		
		Release(genOperatorLock);
		Release(operatorLock[0]);
		
		
		while(a<4)
		{
			a=rand()%6;
		}
		
		a=a*1000;
		delay(a);       
	
		Release(phoneLock[0]);
		
		Acquire(writeLock);

		Write("President Releases Phone  \n",27,1);
		
		Release(writeLock);
		
		presidentStatus=0;			/*Sets president Status as to False*/
		pp=0;
		
		
		Broadcast(senatorWaitingCondition,stationLock);		
		Broadcast(visitorWaitingCondition,stationLock);
						    	
		
		while(r<5)
		{
		
		  r=rand()%8;
		}
		
		if(cnt!=5)
		{
			Acquire(writeLock);
			
			Write("President Going. will come after ",33,1);
			
/*			itoa(buf[3],6,r);
			Write(buf[3],6,1);		  		          */
			
			Print(r);
			
			Write(" seconds \n",10,1);
			
			Release(writeLock);
		}
		
		r=r*1000;

		Yield();
		
		delay(r);                    				

	}
		
	Exit(0);
	
}

/*End of President function*/




/*Main Calling Function --------------------------  (Test Case -  Dynamic Values)*/

void SenateFuncDync()
{		
	char buf[10][6];
	char opCntc,vsCntc,seCntc;
	int i,j,k;
	
	for(j=0;j<10;j++)
	{
		for(k=0;k<6;k++)
		{
			buf[j][k]='\0';
		}
	}
	
	visitorWaitingCondition=CreateCondition();
	senatorWaitingCondition=CreateCondition();
	presidentWaitingCondition=CreateCondition();
	
	operatorCondition=CreateCondition();

/*	Write("\nEnter the number of Operators :: \n\n",37,1);*/
	
/*	cin>>opCnt;		  		    */
	
/*	Write("\nEnter the number of Visitors ::  \n\n",37,1);*/
	
/*	cin>>vsCnt;		  		    */
	
/*	Write("Enter the Number of Senators ::  \n\n",37,1);
	
	Write("\n\n------------------\n\n",22,1);
	
	Read(&opCntc,1,0);
	Read(&vsCntc,1,0);
	Read(&seCntc,1,0);
	
	opCnt=opCntc;
	vsCnt=vsCntc;
	seCnt=seCntc;
	
	opCnt=opCnt-48;
	vsCnt=vsCnt-48;
	seCnt=seCnt-48;*/
	
/*	cin>>seCnt;		 		    */

	opCnt=3;
	vsCnt=6;
	seCnt=5;

	avlOperatorCnt=opCnt;
	visitorIDCnt=0;
	senatorIDCnt=vsCnt;
	operatorIDCnt=0;

/*	operatorStatus=(int *)calloc(opCnt,sizeof(int));
	talkWithOperator=(int *)calloc(opCnt,sizeof(int));
	callerID=(int *)calloc(opCnt,sizeof(int));		  		    */
	
	stationLock=CreateLock();
	idLock=CreateLock();
	
	

	for(i=0;i<opCnt;i++)
    {
	  	operatorNeeded[i]=CreateCondition();
    }
	
	for(i=0;i<20;i++)
	{
		phoneStatus[i]=0;
	}
	
	for(i=0;i<20;i++)
	{	
		phoneLock[i]=CreateLock();
	}
	
	for(i=0;i<opCnt;i++)
	{
		operatorStatus[i]=0;
	}
	
	for(i=0;i<opCnt;i++)
	{
		operatorLock[i]=CreateLock();
	}
	
	for(i=0;i<opCnt;i++)
	{
		talkWithOperator[i]=0;
	}

	for(i=0;i<opCnt;i++)
	{
	  callerID[i]=0;
	}


	Acquire(writeLock);
	for(i=0;i<opCnt;i++)
	{
		
		
		Write("Forking Thread Operator  \n",26,1);
		
	/*	itoa(buf[0],6,(i+1));
		Write(buf[0],6,1);		  		            */
		
		Print((i+1));
		
/*		Write("\n",1,1);		  		         */

		

		Fork(Operator,0);
	}	
	Release(writeLock);
	

	
	Acquire(writeLock);
	for(i=0;i<vsCnt;i++)
	{
		
		
		Write("Forking Thread Visitor   \n",26,1);
		
/*		itoa(buf[1],6,(i+1));
		Write(buf[1],6,1);
		
		Write("\n",1,1);		  */
		
		Print((i+1));
		
		
		

		Fork(Visitor,0);
	}
	Release(writeLock);
	

	 
	Acquire(writeLock);
	for(i=0;i<seCnt;i++)
	{
		
		
		Write("Forking Thread Senator   \n",26,1);
		
	/*	itoa(buf[2],6,(i+1));
		Write(buf[2],6,1);
		
		Write("\n",1,1);		  */
		
		Print((i+1));
		
		
		

		Fork(Senator,0);
	}	
	Release(writeLock);
	
	
	
	Acquire(writeLock);
		
	Write("Forking Thread President  \n",27,1);
	
	
	Release(writeLock);
	
	Fork(President,0);	
		
}


int main()
{	
	writeLock=CreateLock();
	readLock=CreateLock();
	
	Acquire(writeLock);
	
	Write("Test  0123456789  \n",19,1);
	Print(1220);
	
	Write("Test  9876543210  \n",19,1);
	Print(221);
	
	Release(writeLock);
	

	SenateFuncDync();
	
	Exit(0);		  		            
	
	return 0;
}



