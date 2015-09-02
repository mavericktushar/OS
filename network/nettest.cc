// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <iostream.h>

#define Server_Addr 		0
#define Server_Mbox_Addr	0

#define LockTblSize		   100
#define ConditionTblSize   100
#define VarTblSize         50

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

/****************************************************************************
				Data Structure for Holding Created Locks
*****************************************************************************/

struct LockTblStruct
{
	int id;
	char name[50];
	int machineId;
	char threadName[50];
	int valid;
	int acq;
};


/****************************************************************************
				Data Structure for Holding Reply messages
*****************************************************************************/

class ReplyMsg
{
	public:
	
	int outPktHdrfrom;
	int outPktHdrto;
	int outMailHdrfrom;
	int	outMailHdrto;
	char msg[10];
};

LockTblStruct lockTbl[LockTblSize];		  	            //Table for Holding locks at server
List* lq[100];											//Queue for each lock 

/****************************************************************************
				Data Structure for Holding Created Conditions
*****************************************************************************/

struct ConditionTblStruct
{
	int id;
	char name[50];
	int machineId;
	char threadName[50];
	int valid;
	int iswait;
};

ConditionTblStruct conditionTbl[ConditionTblSize];    //Table for Holding condiitons at server
List* cq[100];					   	                  //Queue for each condition

/****************************************************************************
				Data Structure for Senator
*****************************************************************************/

struct varTblStruct 
{
	char name[50];
	int val[20];
};

varTblStruct varTbl[VarTblSize];				  

void initVar()
{
	strcpy(varTbl[0].name,"opCnt");		  	                        //operator Count
	varTbl[0].val[0]=operatornum;
	
	strcpy(varTbl[1].name,"vsCnt");				 	 	 	 	    //visitor Count
	varTbl[1].val[0]=visitornum;			 	                                
	
	strcpy(varTbl[2].name,"seCnt");			  	   			        //senator Count
	varTbl[2].val[0]=senatornum;

	strcpy(varTbl[3].name,"avlPhoneCnt");		  	  	  		    //avlPhoneCnt
	varTbl[3].val[0]=20;
	
	strcpy(varTbl[4].name,"avlOperatorCnt");		 	  	 	  	//avlOperatorCnt
	varTbl[4].val[0]=-1;
	
	strcpy(varTbl[5].name,"presidentStatus");		  	            //presidentStatus
	varTbl[5].val[0]=0;
	
	strcpy(varTbl[6].name,"waitingVisitorsCnt");
	varTbl[6].val[0]=0;			 	                                //waitingVisitorsCnt
	
	strcpy(varTbl[7].name,"waitingSenatorsCnt");			 		//waitingSenatorsCnt
	varTbl[7].val[0]=0;

	strcpy(varTbl[8].name,"visitorID");		  	  	  		        //visitorID
	varTbl[8].val[0]=0;
	
	strcpy(varTbl[9].name,"senatorID");		 	  	 			  	//senatorID
	varTbl[9].val[0]=-1;
	
	strcpy(varTbl[10].name,"operatorID");		  	               	//operatorID
	varTbl[10].val[0]=0;
	
	//Arrays
	strcpy(varTbl[11].name,"phoneStatus");		  	              	//phoneStatus
	for(int i=0;i<20;i++)		 	 		 	 	 	 	 	    //Monitor Variable to check status of phones (BUSY or FREE)
	{
		varTbl[11].val[i]=0;
	}
	
	strcpy(varTbl[12].name,"operatorStatus");						//operatorStatus
	for(int i=0;i<operatornum;i++)			 	                	//Monitor Variable to check status of each Operator (BUSY or FREE)    
	{
		varTbl[12].val[i]=0;
	}
	
	strcpy(varTbl[13].name,"talkWithOperator");			  	   		//talkWithOperator
	for(int i=0;i<operatornum;i++)					 	  			//Variable to make Caller talk to an Operator
	{
		varTbl[13].val[i]=0;
	}
	
	strcpy(varTbl[14].name,"callerID");		  	  	  		        //callerID
	for(int i=0;i<operatornum;i++)				 	          		//Variable to hold present caller's ID of callers calling various Operators
	{
		varTbl[14].val[i]=0;
	}
}

void Server_RPC()
{
	PacketHeader outPktHdr,inPktHdr;		//Packet and Mail Headers for the data to be sent		
	MailHeader outMailHdr,inMailHdr;        //specifying Source and Destination Addresses
	
	int lockCnt=0;							//initialize lock count and condition count to 0
	int conditionCnt=0;
	
	//Set all locks and condiitons as not valid and non aquired or non waiting initially

	for(int i=0;i<LockTblSize;i++)
	{
		lockTbl[i].valid=0;
		lockTbl[i].acq=0;
	}				  		
	
	for(int i=0;i<ConditionTblSize;i++)
	{
		conditionTbl[i].valid=0;
		conditionTbl[i].iswait=0;
	}	
	
	initVar();
	
	cout<<"\nInitializing the Server . . .\n"<<endl;
	
	while(true)  		        //Server - continuosly waiting for messages from Clients
	{
	
		char* bufferToReceive=new char[MaxMailSize+1];		// Buffer to receive
		char* bufferToSend=new char[MaxMailSize+1];         //Buffer To send
		char* syscallTypeSv=new char[6];         //Syscall type
		char* namebufSv=new char[6];             //Will hold name for CreateLock, CreateCondition and lock ID otherwise
		char* machineIdSvC=new char[6];          //machine ID
		int machineIdSv;  					     //machine ID in int
		int lockidSv;   					     //lockid - extracted from namebufSv
		char* threadNameSv=new char[10];  	     //thread Name
		char* conditionidSvC=new char[4];    	 //Condition ID
		int conditionidSv;  					 //Condition ID in int
		int arrindexSv;				 	 	  	 //index into array
		int valSv;			 	 	 	 	     //Value
		
		bufferToSend[MaxMailSize]='\0';
		bufferToReceive[MaxMailSize]='\0';
		
		cout<<"Server Waiting to Receive . . .\n"<<endl;
		postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);   //Receive message from a client
		
		sscanf(bufferToReceive,"%s %s %s %s %s",syscallTypeSv,namebufSv,machineIdSvC,threadNameSv,conditionidSvC);   //Parse the message
		
		machineIdSv=atoi(machineIdSvC);
		conditionidSv=atoi(conditionidSvC);
		arrindexSv=conditionidSv;
		
		valSv=atoi(threadNameSv);
		
		cout<<"\nSyscall Type :: "<<syscallTypeSv<<"\n"<<endl;
//		cout<<"\nnamebuf :: "<<namebufSv<<"\n"<<endl;
		cout<<"\nMachineID :: "<<machineIdSv<<"\n"<<endl;
		cout<<"\nRequesting Thread name :: "<<threadNameSv<<"\n"<<endl;		  	          
		
		outPktHdr.to=inPktHdr.from;		  		  //Build the Sending header -  specifying Addresses
		outPktHdr.from=Server_Addr;
		
		outMailHdr.to=inMailHdr.from;
		outMailHdr.from=Server_Mbox_Addr;
		
		
/**************************************************************************************************
					Create Lock RPC
***************************************************************************************************/
		if(!strcmp(syscallTypeSv,"CrLock"))
		{
			int j;
			int lockexist=0;
			int index;
			for(j=0;j<LockTblSize;j++)		       //Check if Lock exist
			{
				if(!strcmp(namebufSv,lockTbl[j].name))
				{	
					cout<<"Same Lock requested . . .\n"<<endl;
					cout<<"\nlockTbl[j].name :: "<<lockTbl[j].name<<"\n"<<endl;
					cout<<"\nnamebufSv :: "<<namebufSv<<"\n"<<endl;
					lockexist=1;
					index=j;
					break;
				}
			}
			if(lockexist==1)		  //If lock already exists
			{
				char* acknol="Lock_exist";
				int lockid=lockTbl[index].id;
					
				sprintf(bufferToSend,"%s %d",acknol,lockid);
				
				outMailHdr.length=strlen(bufferToSend)+1;
			
				delete acknol;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
			else    //If lock does not exist - create new in the table
			{
				lockTbl[lockCnt].id=lockCnt;
				strcpy(lockTbl[lockCnt].name,namebufSv);
				lockTbl[lockCnt].machineId=inPktHdr.from;
				strcpy(lockTbl[lockCnt].threadName,threadNameSv);
				lockTbl[lockCnt].valid=1;
				
				lq[lockCnt]=new List();			  	         //Queue for the Lock
				
				lockCnt++;
				char* acknol="New_Lock";
				int lockid=lockCnt-1;	
				sprintf(bufferToSend,"%s %d",acknol,lockid);
				
				outMailHdr.length=strlen(bufferToSend)+1;
				
				delete acknol;
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}	
		}
/**************************************************************************************************
					Destroy Lock RPC
***************************************************************************************************/
		else if(!strcmp(syscallTypeSv,"DeLock"))
		{	
			int lockexist=0;
			int index=999;
			lockidSv=atoi(namebufSv);

			if(lockTbl[lockidSv].valid==1)				//Check if lock to be destroyed exist
			{	
				cout<<"Lock to be destroyed exist . . .\n"<<endl;
				cout<<"\nlockTbl[i].id :: "<<lockTbl[lockidSv].id<<"\n"<<endl;
				cout<<"\nlockTbl[i].name :: "<<lockTbl[lockidSv].name<<"\n"<<endl;
				cout<<"\nlockTbl[i].machineId :: "<<lockTbl[lockidSv].machineId<<"\n"<<endl;
				cout<<"\nlockTbl[i].threadName :: "<<lockTbl[lockidSv].threadName<<"\n"<<endl;
				index=lockidSv;
				lockexist=1;
			}
			
			if(lockexist==1)
			{
				if(lockTbl[index].acq==0)  //Check if lock has been acquired - Not acquired by anyone - Destroy the lock
				{
					char* temp="";
					strcpy(lockTbl[index].name,temp);
					lockTbl[index].valid=0;
					delete temp;
					char* deackmsg=new char[10];
					strcpy(deackmsg,"De_ACK");
					sprintf(bufferToSend,"%s",deackmsg);
					
					outMailHdr.length=strlen(bufferToSend)+1;
					bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
					if ( !success ) 
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}
					delete deackmsg;
					cout<<"\nLock Successfully destroyed . . .\n"<<endl;
				}
				else  //Lock acquired
				{
					char* deackmsg=new char[10];
					strcpy(deackmsg,"De_NAK");
					sprintf(bufferToSend,"%s",deackmsg);
					delete deackmsg;	
					
					outMailHdr.length=strlen(bufferToSend)+1;
					bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
					if ( !success ) 
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}
					cout<<"\nThreads waiting to Acquire Lock. Lock cannot be destroyed . . .\n"<<endl;
				}
			}
			else   //lock does not exist
			{
				char* deackmsg="De_NAK";
				sprintf(bufferToSend,"%s",deackmsg);
				delete deackmsg;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);

				outMailHdr.length=strlen(bufferToSend)+1;
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
				cout<<"\nLock to be destroyed does not exist .Error . . .\n"<<endl;
			}
			
		}
		
/**************************************************************************************************
					Acquire Lock RPC
***************************************************************************************************/		
		if(!strcmp(syscallTypeSv,"AqLock"))		  	                 //lockTbl[lockidSv].machineId holds the machine ID for thread acquiring Lock
		{
			lockidSv=atoi(namebufSv);
			if(lockTbl[lockidSv].valid==1)  //CheckIf lock exist
			{
			//	if(lq[lockidSv]->IsEmpty())
				if(lockTbl[lockidSv].acq==0)  //Check If lock not Acquired
				{
					char* aqackmsg=new char[10];
					strcpy(aqackmsg,"Aq_ACK");
					lockTbl[lockidSv].acq=1;
					lockTbl[lockidSv].machineId=inPktHdr.from;
					sprintf(bufferToSend,"%s",aqackmsg);
					delete aqackmsg;
					outMailHdr.length=strlen(bufferToSend)+1;
					bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
					if ( !success ) 
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}
				}
				else  //If lock Acquire put Reply messages into a Queue for that lock
				{
					char* aqackmsg=new char[10];
					strcpy(aqackmsg,"Aq_ACKq");
					ReplyMsg* rplmsg=new ReplyMsg();
					rplmsg->outPktHdrfrom=Server_Addr;
					rplmsg->outPktHdrto=inPktHdr.from;
					
					rplmsg->outMailHdrfrom=Server_Mbox_Addr;
					rplmsg->outMailHdrto=inMailHdr.from;
					
					strcpy(rplmsg->msg,aqackmsg);
					delete aqackmsg;	
					lq[lockidSv]->Append(rplmsg);
				//	delete rplmsg;
				}
				
			}
			else  //Lock does not exist
			{
				cout<<"\nLock cannot be Acquired. Lock does not exist. Error . . .\n"<<endl;
				char* aqackmsg=new char[10];
				strcpy(aqackmsg,"Aq_NAK");
				
				sprintf(bufferToSend,"%s",aqackmsg);
				
				delete aqackmsg;
				
				outMailHdr.length=strlen(bufferToSend)+1;
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
		}
		
/**************************************************************************************************
					Release Lock RPC
***************************************************************************************************/		
		
		else if(!strcmp(syscallTypeSv,"ReLock"))
		{
			lockidSv=atoi(namebufSv);
			if(lockTbl[lockidSv].valid==1)  //Check If lock exist
			{
				if(lockTbl[lockidSv].acq==1)  //Check if Lock Acquired
				{
					if(lq[lockidSv]->IsEmpty())    //If the lock Queue empty - just Release the lock
					{
						lockTbl[lockidSv].acq=0;
						char* reackmsg=new char[10];
						strcpy(reackmsg,"Re_ACK");
				
						sprintf(bufferToSend,"%s",reackmsg);
					
						delete reackmsg;
					
						outMailHdr.length=strlen(bufferToSend)+1;
						bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
						if ( !success ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							interrupt->Halt();
						}
					}
					else        //If the lock Queue not empty - Release the lock and make the next waiting Acquire the lock
					{
						/*		  	           Msg --------------- To Release		  	        */
						char* reackmsg1=new char[10];
						strcpy(reackmsg1,"Re_ACK");
				
						sprintf(bufferToSend,"%s",reackmsg1);
					
						delete reackmsg1;
					
						outMailHdr.length=strlen(bufferToSend)+1;
						bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
						if ( !success ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							interrupt->Halt();
						}
						
						/*		    Msg       ---------------		     To Acquire						  				        */
						
						char* bufferToSend1=new char[MaxMailSize+1];
						bufferToSend[MaxMailSize]='\0';
						char reackmsg[10];
						ReplyMsg* rplmsg=(ReplyMsg*)lq[lockidSv]->Remove();
						cout<<"\ncheck : rplmsg->outPktHdrto :: "<<rplmsg->outPktHdrto<<"\n"<<endl;
						outPktHdr.from=rplmsg->outPktHdrfrom;
						
						outPktHdr.to=rplmsg->outPktHdrto;
						
						outMailHdr.from=rplmsg->outMailHdrfrom;
						outMailHdr.to=rplmsg->outMailHdrto;
						
						lockTbl[lockidSv].machineId=rplmsg->outPktHdrto;
						
						strcpy(reackmsg,rplmsg->msg);
						delete rplmsg;
						
						sprintf(bufferToSend1,"%s",reackmsg);
						outMailHdr.length=strlen(bufferToSend1)+1;
						bool success1=postOffice->Send(outPktHdr,outMailHdr,bufferToSend1);	
						if ( !success1 ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							delete bufferToSend1;
							interrupt->Halt();
						}
						delete bufferToSend1;
					}
				}
				else
				{
					cout<<"\nLock not Acquired. Cannot be Released. Error . . .\n"<<endl;
					char* reackmsg=new char[10];
					strcpy(reackmsg,"Re_NAKa");
				
					sprintf(bufferToSend,"%s",reackmsg);
				
					delete reackmsg;
				
					outMailHdr.length=strlen(bufferToSend)+1;
					bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
					if ( !success ) 
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}
				}
				
			}
			else
			{
				cout<<"\nLock not created. Cannot be Released. Error . . .\n"<<endl;
				char* reackmsg=new char[10];
				strcpy(reackmsg,"Re_NAKv");
				
				sprintf(bufferToSend,"%s",reackmsg);
				
				delete reackmsg;
				
				outMailHdr.length=strlen(bufferToSend)+1;
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
		}
/**************************************************************************************************
					Create Condition RPC
***************************************************************************************************/
		
		else if(!strcmp(syscallTypeSv,"CrCdit"))
		{
			int conditionexist=0;
			int index=0;
			for(int i=0;i<ConditionTblSize;i++)    //Check if condition exist
			{
				if(!strcmp(namebufSv,conditionTbl[i].name))
				{
					cout<<"Same Condition requested to be created . . .\n"<<endl;
					cout<<"\nconditionTbl[i].name :: "<<conditionTbl[i].name<<"\n"<<endl;
					cout<<"\nnamebufSv :: "<<namebufSv<<"\n"<<endl;
					conditionexist=1;
					index=i;
					break;
				}
			}
			
			if(conditionexist==1)  //If it exist, Send ID of existing to the Client
			{
				char* acknol=new char[15];
				strcpy(acknol,"Cdit_exist");
				int conditionid=conditionTbl[index].id;
					
				sprintf(bufferToSend,"%s %d",acknol,conditionid);
				
				outMailHdr.length=strlen(bufferToSend)+1;
			
				delete acknol;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
			else  //Does not exist - Create a new one
			{
				conditionTbl[conditionCnt].id=conditionCnt;
				strcpy(conditionTbl[conditionCnt].name,namebufSv);
				conditionTbl[conditionCnt].machineId=inPktHdr.from;
				strcpy(conditionTbl[conditionCnt].threadName,threadNameSv);
				conditionTbl[conditionCnt].valid=1;
				
				cq[conditionCnt]=new List();			  	         //Queue for the Lock
				
				conditionCnt++;
				
				char* acknol=new char[15];
				strcpy(acknol,"New_Cdit");
				int conditionid=conditionCnt-1;	
				sprintf(bufferToSend,"%s %d",acknol,conditionid);
				
				outMailHdr.length=strlen(bufferToSend)+1;
				
				delete acknol;
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
		}

/**************************************************************************************************
					Destroy Condition RPC
***************************************************************************************************/

		else if(!strcmp(syscallTypeSv,"DeCdit"))
		{
			int conditionexist=0;
			int index=999;

			if(conditionTbl[conditionidSv].valid==1)    //If condiiton exist
			{	
				cout<<"Condition Variable to be destroyed exist . . .\n"<<endl;
				cout<<"\nconditionTbl[i].id :: "<<conditionTbl[conditionidSv].id<<"\n"<<endl;
				cout<<"\nconditionTbl[i].name :: "<<conditionTbl[conditionidSv].name<<"\n"<<endl;
				cout<<"\nconditionTbl[i].machineId :: "<<conditionTbl[conditionidSv].machineId<<"\n"<<endl;
				cout<<"\nconditionTbl[i].threadName :: "<<conditionTbl[conditionidSv].threadName<<"\n"<<endl;
				index=conditionidSv;
				conditionexist=1;
			}

			
			if(conditionexist==1)
			{
				if(cq[index]->IsEmpty())  //No one is waiting - Destroy condition
				{
					char* temp="";
					strcpy(conditionTbl[index].name,temp);
					conditionTbl[index].valid=0;
					conditionTbl[conditionidSv].iswait=0;
					delete temp;
					char* deackmsg="Dc_ACK";
					sprintf(bufferToSend,"%s",deackmsg);
					
					outMailHdr.length=strlen(bufferToSend)+1;
					bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
					if ( !success ) 
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}
					delete deackmsg;
					cout<<"\nCondition Variable Successfully destroyed . . .\n"<<endl;
				}
				else
				{
					char* deackmsg="De_NAK";
					sprintf(bufferToSend,"%s",deackmsg);
					delete deackmsg;	
					
					outMailHdr.length=strlen(bufferToSend)+1;
					bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
					if ( !success ) 
					{
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
						interrupt->Halt();
					}
					cout<<"\nThreads Waiting . Condition Variable cannot be destroyed . . .\n"<<endl;
				}
			}
			else
			{
				char* deackmsg="De_NAK";
				sprintf(bufferToSend,"%s",deackmsg);
				delete deackmsg;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);

				outMailHdr.length=strlen(bufferToSend)+1;
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
				cout<<"\nCondition Variable to be destroyed does not exist .Error . . .\n"<<endl;
			}
		}

/**************************************************************************************************
					Wait RPC
***************************************************************************************************/
		
		else if(!strcmp(syscallTypeSv,"WaCdit"))		  		            //Validations - Lock should be valid and should be acquired by the wait requesting mahine
		{
			lockidSv=atoi(namebufSv);
			int nakflag=0;
			if(conditionTbl[conditionidSv].valid==1)
			{
				if(lockTbl[lockidSv].valid==1)
				{
					if(lockTbl[lockidSv].acq==1 && lockTbl[lockidSv].machineId==inPktHdr.from)
					{
						//Put Reply message for wait into a Queue
						conditionTbl[conditionidSv].iswait=1;
						ReplyMsg* rplmsgw=new ReplyMsg();
						
						rplmsgw->outPktHdrfrom=Server_Addr;
						rplmsgw->outPktHdrto=inPktHdr.from;
						
						rplmsgw->outMailHdrfrom=Server_Mbox_Addr;
						rplmsgw->outMailHdrto=inMailHdr.from;
						cout<<"\nWhile going to wait putting wait msg in Queue for Machine ID :: "<<rplmsgw->outPktHdrto<<"\n"<<endl;
						strcpy(rplmsgw->msg,"WaAck-");
						
						cq[conditionidSv]->Append(rplmsgw);
						
						//delete rplmsgw;
						
						
						//Release the lock
						if(lq[lockidSv]->IsEmpty())		  		            //Nobody in Queue to Acquire the Lock
						{
							lockTbl[lockidSv].acq=0;
						}
						else
						{
							char reackmsg[10];
							ReplyMsg* rplmsg=(ReplyMsg*)lq[lockidSv]->Remove();  
							  	             
							outPktHdr.from=rplmsg->outPktHdrfrom;			
							outPktHdr.to=rplmsg->outPktHdrto;
							
							outMailHdr.from=rplmsg->outMailHdrfrom;
							outMailHdr.to=rplmsg->outMailHdrto;
							
							lockTbl[lockidSv].machineId=rplmsg->outPktHdrto;
							
							strcpy(reackmsg,rplmsg->msg);
							delete rplmsg;
							
							sprintf(bufferToSend,"%s",reackmsg);
							outMailHdr.length=strlen(bufferToSend)+1;
							bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);	
							if ( !success ) 
							{
								printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
								interrupt->Halt();
							}
						}
					}
					else
					{
						cout<<"\nEither the Lock not Acquired or a process not presently Acquiring the lock asking to wait. Error . . .\n"<<endl;
						nakflag=1;
					}
				}
				else
				{
					cout<<"\nLock does not exist. Error in Wait . . .\n"<<endl;
					nakflag=1;
				}
			}
			else
			{
				cout<<"\nCondition Variable does not exist. Cannot Wait on it. Error . . .\n"<<endl;
				nakflag=1;
			}
			
			if(nakflag==1)
			{
				char* waackmsg="Wa_NAK";
				sprintf(bufferToSend,"%s",waackmsg);
				delete waackmsg;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);

				outMailHdr.length=strlen(bufferToSend)+1;
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
		}
		
/**************************************************************************************************
					Signal RPC
***************************************************************************************************/
		
		if(!strcmp(syscallTypeSv,"SgCdit"))
		{
			lockidSv=atoi(namebufSv);
			int nakflag=0;
			if(conditionTbl[conditionidSv].valid==1)
			{
				if(lockTbl[lockidSv].valid==1)
				{
					if(cq[conditionidSv]->IsEmpty())
					{
						conditionTbl[conditionidSv].iswait=0;		  	        
						cout<<"\nNone waiting to be signalled . . .\n"<<endl;   
						char* acknol=new char[15];
						strcpy(acknol,"Sg_NAK");
						
						sprintf(bufferToSend,"%s",acknol);
				
						outMailHdr.length=strlen(bufferToSend)+1;
				
						delete acknol;
						bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
						if ( !success ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							interrupt->Halt();
						}	
					}
					else
					{
						ReplyMsg* rplmsg=(ReplyMsg*)cq[conditionidSv]->Remove();		  	         //Remove from the wait queue and then Acquire Lock (see Below)
						char* acknol=new char[15];
						strcpy(acknol,"Sg_ACK");
						
						sprintf(bufferToSend,"%s",acknol);
				
						outMailHdr.length=strlen(bufferToSend)+1;
				
						delete acknol;
						bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
						if ( !success ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							interrupt->Halt();
						}	
						//After removing from the wait queue Acquire the Lock
						
						
							
						if(lockTbl[lockidSv].acq==0)		  	        //If nobody is presently acquring the lock, just acquire it(acq=1) and send the message to the client waiting on Wait RPC
						{
							char* waackmsg=new char[10];
							
							lockTbl[lockidSv].acq=1;
	
							outPktHdr.from=rplmsg->outPktHdrfrom;
						
							outPktHdr.to=rplmsg->outPktHdrto;
							
							outMailHdr.from=rplmsg->outMailHdrfrom;
							outMailHdr.to=rplmsg->outMailHdrto;
						
							strcpy(waackmsg,rplmsg->msg);
		
							sprintf(bufferToSend,"%s",waackmsg);
							outMailHdr.length=strlen(bufferToSend)+1;
							bool success1=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);	
							if ( !success1 ) 
							{
								printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
								interrupt->Halt();
							}	
						}
						else			  		  			            //If somebody presently acquiring the lock, then put the rplmsg for Wait into locks' Acquiring queue
						{			  	                                //(So, msg to Wait will be sent on a release by release)
							lq[lockidSv]->Append(rplmsg);
						}
						
					//	delete rplmsg;
					
					}
				}
				else
				{
					cout<<"\nLock does not exist. Error in Signal . . .\n"<<endl;
					nakflag=1;
				}
			}
			else
			{
				cout<<"\nCondition Variable does not exist. Cannot Signal on it. Error . . .\n"<<endl;
				nakflag=1;
			}
			
			if(nakflag==1)
			{
				char* sgackmsg="Sg_NAK";
				sprintf(bufferToSend,"%s",sgackmsg);
				delete sgackmsg;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);

				outMailHdr.length=strlen(bufferToSend)+1;
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
		}
		
/**************************************************************************************************
					Broadcast RPC
***************************************************************************************************/

		if(!strcmp(syscallTypeSv,"BtCdit"))
		{
			lockidSv=atoi(namebufSv);
			int nakflag=0;
			if(conditionTbl[conditionidSv].valid==1)  //Check if condition Valid
			{
				if(lockTbl[lockidSv].valid==1)  //Check if lock valid
				{
					if(cq[conditionidSv]->IsEmpty())  //If none waiting
					{
						conditionTbl[conditionidSv].iswait=0;		  	       
						cout<<"\nNone waiting to be Broadcasted . . .\n"<<endl;   
						char* acknol=new char[15];
						strcpy(acknol,"Bt_NAK");
						
						sprintf(bufferToSend,"%s",acknol);
				
						outMailHdr.length=strlen(bufferToSend)+1;
				
						delete acknol;
						bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
						if ( !success ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							interrupt->Halt();
						}	
					}
					else
					{
						char* acknol=new char[15];
						cout<<"\nBroadcasting All waiting . . .\n"<<endl;
						
						strcpy(acknol,"Bt_ACK");
						
						sprintf(bufferToSend,"%s",acknol);
				
						outMailHdr.length=strlen(bufferToSend)+1;
				
						delete acknol;
						bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
						if ( !success ) 
						{
							printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
							interrupt->Halt();
						}	
						
						while(!cq[conditionidSv]->IsEmpty())
						{
							ReplyMsg* rplmsgb=new ReplyMsg();
							rplmsgb=(ReplyMsg*)cq[conditionidSv]->Remove();
							if(lockTbl[lockidSv].acq==0)  //If none has the lock Acqure lock and send message to Wait
							{
								lockTbl[lockidSv].acq=1;
								lockTbl[lockidSv].machineId=rplmsgb->outPktHdrto;
								
								
								char* bufferToSend1=new char[MaxMailSize+1];
								char* btackmsg=new char[10];
								
								bufferToSend1[MaxMailSize]='\0';
								cout<<"\nSending message to. Machine ID (rplmsgb):: "<<rplmsgb->outPktHdrto<<"\n"<<endl;
								outPktHdr.from=rplmsgb->outPktHdrfrom;
							
								outPktHdr.to=rplmsgb->outPktHdrto;
								
								outMailHdr.from=rplmsgb->outMailHdrfrom;
								outMailHdr.to=rplmsgb->outMailHdrto;
							
								strcpy(btackmsg,rplmsgb->msg);
								
			
								sprintf(bufferToSend1,"%s",btackmsg);
								outMailHdr.length=strlen(bufferToSend1)+1;
								bool success1=postOffice->Send(outPktHdr,outMailHdr,bufferToSend1);	
								if ( !success1 ) 
								{
									printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
									delete bufferToSend1;
									interrupt->Halt();
								}	
								delete bufferToSend1;		  	         
							}
							else  //If lock being Acquire - Put the Reply message to locks queue
							{
								lq[lockidSv]->Append(rplmsgb);
							}
						//	delete rplmsgb;
						}
					}
				}
				else
				{
					cout<<"\nLock does not exist. Error in Broadcast . . .\n"<<endl;
					nakflag=1;
				}
			}
			else
			{
				cout<<"\nCondition Variable does not exist. Cannot Broadcast on it. Error . . .\n"<<endl;
				nakflag=1;
			}
			
			if(nakflag==1)
			{
				char* btackmsg=new char[10];
				strcpy(btackmsg,"Bt_NAK");
				sprintf(bufferToSend,"%s",btackmsg);
				delete btackmsg;
				
				bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);

				outMailHdr.length=strlen(bufferToSend)+1;
				if ( !success ) 
				{
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					interrupt->Halt();
				}
			}
					
			
		}
		
/**************************************************************************************************
					GetValue RPC
***************************************************************************************************/

		if(!strcmp(syscallTypeSv,"getVal"))
		{
			int retval,flag=0;
			if(arrindexSv==-1)
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						retval=varTbl[i].val[0];
						flag=1;
						cout<<"\nReturning Value for variable :: "<<varTbl[i].name<<" value :: "<<varTbl[i].val[0]<<"\n";
						break;
					}
				}
			}
			else
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						retval=varTbl[i].val[arrindexSv];
						flag=1;
						cout<<"\nReturning Value for variable :: "<<varTbl[i].name<<"["<<arrindexSv<<"] value :: "<<varTbl[i].val[arrindexSv]<<"\n";
						break;
					}
				}

			}
			
			char* acknol="ACK";
			if(flag==0)
			{
				cout<<"\nNot existent variable requested. Error . . .\n";
				strcpy(acknol,"NACK");
				retval=-1;
			}
		
			sprintf(bufferToSend,"%s %d",acknol,retval);
				
			outMailHdr.length=strlen(bufferToSend)+1;
				
			delete acknol;
			bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
			if ( !success ) 
			{
				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				interrupt->Halt();
			}
		}

/**************************************************************************************************
					Set Value RPC
***************************************************************************************************/

		if(!strcmp(syscallTypeSv,"setVal"))
		{
			int flag=0;
			if(arrindexSv==-1)
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						varTbl[i].val[0]=valSv;
						flag=1;
						cout<<"\nSetting Value for variable :: "<<varTbl[i].name<<" value :: "<<varTbl[i].val[0]<<"\n";
						break;
					}
				}
			}
			else
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						varTbl[i].val[arrindexSv]=valSv;
						flag=1;
						cout<<"\nSetting Value for variable :: "<<varTbl[i].name<<"["<<arrindexSv<<"] value :: "<<varTbl[i].val[arrindexSv]<<"\n";
						break;
					}
				}

			}
			
			char* acknol="ACK";
			if(flag==0)
			{
				cout<<"\nNot existent variable requested. Error . . .\n";
				strcpy(acknol,"NACK");
			}
		
			sprintf(bufferToSend,"%s",acknol);
				
			outMailHdr.length=strlen(bufferToSend)+1;
				
			delete acknol;
			bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
			if ( !success ) 
			{
				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				interrupt->Halt();
			}
		}
		
/**************************************************************************************************
					IncrementbyVal RPC
***************************************************************************************************/

		if(!strcmp(syscallTypeSv,"incVal"))
		{
			int flag=0;
			if(arrindexSv==-1)
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						varTbl[i].val[0]=varTbl[i].val[0]+valSv;
						flag=1;
						cout<<"\nIncrementing Value for variable :: "<<varTbl[i].name<<" value :: "<<varTbl[i].val[0]<<"\n";
						break;
					}
				}
			}
			else
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						varTbl[i].val[arrindexSv]=varTbl[i].val[arrindexSv]+valSv;
						flag=1;
						cout<<"\nIncrementing Value for variable :: "<<varTbl[i].name<<"["<<arrindexSv<<"] value :: "<<varTbl[i].val[arrindexSv]<<"\n";
						break;
					}
				}

			}
			
			char* acknol="ACK";
			if(flag==0)
			{
				cout<<"\nNot existent variable requested. Error . . .\n";
				strcpy(acknol,"NACK");
			}
		
			sprintf(bufferToSend,"%s",acknol);
				
			outMailHdr.length=strlen(bufferToSend)+1;
				
			delete acknol;
			bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
			if ( !success ) 
			{
				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				interrupt->Halt();
			}
		}
		
/**************************************************************************************************
					DecrementbyVal RPC
***************************************************************************************************/		

		if(!strcmp(syscallTypeSv,"decVal"))
		{
			int flag=0;
			if(arrindexSv==-1)
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						varTbl[i].val[0]=varTbl[i].val[0]-valSv;
						flag=1;
						cout<<"\nDecrementing Value for variable :: "<<varTbl[i].name<<" value :: "<<varTbl[i].val[0]<<"\n";
						break;
					}
				}
			}
			else
			{
				for(int i=0;i<VarTblSize;i++)
				{
					if(!strcmp(namebufSv,varTbl[i].name))
					{
						varTbl[i].val[arrindexSv]=varTbl[i].val[arrindexSv]-valSv;
						flag=1;
						cout<<"\nDecrementing Value for variable :: "<<varTbl[i].name<<"["<<arrindexSv<<"] value :: "<<varTbl[i].val[arrindexSv]<<"\n";
						break;
					}
				}

			}
			
			char* acknol="ACK";
			if(flag==0)
			{
				cout<<"\nNot existent variable requested. Error . . .\n";
				strcpy(acknol,"NACK");
			}
		
			sprintf(bufferToSend,"%s",acknol);
				
			outMailHdr.length=strlen(bufferToSend)+1;
				
			delete acknol;
			bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);
	
			if ( !success ) 
			{
				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				interrupt->Halt();
			}
		}
		
		
		fflush(stdout);
		//Delete all 
		delete bufferToReceive;
		delete bufferToSend;	   
		delete syscallTypeSv;
		delete namebufSv;
		delete machineIdSvC;
		delete threadNameSv;
	}
	
	interrupt->Halt();
	
	
}
