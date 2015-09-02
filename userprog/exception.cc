// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define Server_Addr 	  0
#define Server_Mbox_Addr  0

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include "synch.h"
#include "thread.h"
#include "stdlib.h"
#include <string>
#include <sstream>

using namespace std;

/*#define LockTableSize 100
#define ConditionTableSize 100*/
#define ProcessTableSize 100

	int nummems=0;
	int nummemd=0;
typedef int SpaceId;

//Need Locks!			think think . . .  
int inExit=0;
int execThreadCount=1;
int forkThreadCount=0;


void exec_thread();
void kernel_thread(unsigned int);

/*struct LockPtrID
{
	Lock *lock;
	int id;
	int status;
};

LockPtrID lockTable[LockTableSize];
int flag=1;

struct ConditionStruct
{
	Condition* cd;
	int id;
	int status;
};

ConditionStruct conditionTable[ConditionTableSize];
int flag1=1;		  		   		  		      */

/*struct childStruct
{	
	int childID;	
	Thread* thread;
	unsigned int stack;
};

struct processTableStruct
{
	SpaceId processID;
	AddrSpace* mainSpace;
	Thread* parentThread;	
	Thread* createdThread;
	int cntChildThreads;
	childStruct childEntry[50];
};	  		        */

/*int processTableIndex=0;		  		  //contains the max size process table reached till now (some entries may be empty)
int processCnt=0;

processTableStruct processTable[ProcessTableSize];

SpaceId processID=0;		  		       */

//int allocStackAddr=numPagesIncStack;



int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
		result=FALSE;
		while(!result)
		{
			result = machine->ReadMem( vaddr, 1, paddr );
		}
      buf[n++] = *paddr;
	//	cout<<"\nresult in copyin :: "<<result;
	//	cout<<"\nbuf[n] :: "<<buf[n-1];
 /*     if ( !result ) {
	//translation failed
	return -1;
      }		  		        */

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) 
	  {
	//translation failed
		cout<<"\ntranslation failed . . . \n";
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}
void Yield_Syscall()
{
	cout<<"\nYielding . . .";
	currentThread->Yield();
//	cout<"\nReturning from yield . . .";
}

/*		  	      Start of  CreateLock Syscall		and        Create Lock RPC  		        */

int CreateLock_Syscall(unsigned int vaddr)
{	
/*New Code for CreateLock RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;		       //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;			   //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];    //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1]; // Buffer to receive
	
	char* syscallType="CrLock";		  		        //specifies that Create Lock RPC is there
	int len=6;		   	                            //length of the name for lock
	char* namebuf;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting CreateLock
	int cphldr=0;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	if ( !(namebuf = new char[len]) ) 		      //Get name of the Lock
	{
		printf("%s","Error allocating kernel buffer for write!\n");
		return -1;
    } 
	else
	{		  	              
        if ( copyin(vaddr,len,namebuf) == -1 )
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] namebuf;
			return -1;
		}
    }		  	 
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
	
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());
	
/*	cout<<"\nString Length of syscallType :: "<<strlen(syscallType)<<"\n"<<endl;
	cout<<"\nString Length of namebuf :: "<<strlen(namebuf)<<"\n"<<endl;
	cout<<"\nString Length of threadName :: "<<strlen(threadName)<<"\n"<<endl;		    */
	
	outPktHdr.to=Server_Addr;
	outPktHdr.from=postOffice->netAddr;		  	        //Client Address from system.cc(postoffice object)
		
	outMailHdr.to=Server_Mbox_Addr;
	outMailHdr.from=currentThread->mbox;		  					        //More than one Mail box now
	
	
	
	sprintf(bufferToSend,"%s %s %d %s %d",syscallType,namebuf,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  		     		    //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting for response . . . \n"<<endl;
	
	char* msgacknol=new char[15];
	char* id_c=new char[4];
	int id;
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);   		  	               //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s %s",msgacknol,id_c);
	
	id=atoi(id_c);
	
	cout<<"\n"<<msgacknol<<"\n"<<endl;
	cout<<"\n"<<id<<"\n"<<endl;
	
	delete msgacknol;
	delete id_c;
	
	return id;
#endif
	
/*Old Code for CreateLock Syscall*/
#ifdef USER_PROGRAM
	int i=0;
	lockTableLock->Acquire();
	
	for(i=0;i<LockTableSize;i++)
	{
		if(lockTable[i].status==0)
		{				
			break;
		}
	}
	
	if(i>=LockTableSize)
	{
		cout<<"Lock Quota exhausted . . .\n";
		lockTableLock->Release();
		return -1;
	}
		
	
	lockTable[i].lock=new Lock();
	lockTable[i].status=1;
	lockTable[i].creatingThreadSpace = currentThread->space;
	
	lockTableLock->Release();
	
//	cout<<"\nLock with Lock ID "<<i<<" Created Successfully . . . ";
	
	return lockTable[i].id;
#endif
}

/*		  	      end of  CreateLock Syscall		and      CreateLock RPC  		        */

/*		  	      Start of  DestroyLock Syscall		  		        */

void DestroyLock_Syscall(int idLoc)
{	
/*New Code for DestroyLock RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;    //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;    //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="DeLock";		  		        //specifies that Destroy Lock RPC is there
	int len=6;		   	                            //length of the name for lock
	int lockid;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting DestroyLock
	int cphldr=0;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    lockid=idLoc;										 //Lock ID 
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);    //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Destroy Lock . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);    //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to DestroyLock RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	return;
#endif
/*Old Code for DestroyLock Syscall*/
#ifdef USER_PROGRAM
	lockTableLock->Acquire();
	
	if(lockTable[idLoc].status==0)
	{
		cout<<"\nTrying to Destroy a Lock that has not been created . . . ";
		lockTableLock->Release();		
		return;
	}
	
	if(lockTable[idLoc].creatingThreadSpace != currentThread->space)
	{
		cout<<"Not Authorized to destroy the lock . . . Was created by a different Process\n";		
		lockTableLock->Release();		
		return;
	}
	
	
	delete lockTable[idLoc].lock;
	lockTable[idLoc].status=0;
	lockTable[idLoc].creatingThreadSpace=NULL;
	
//	cout<<"\nLock Destroyed Successfully . . . ";
	
	lockTableLock->Release();
	
	return;
#endif	
}

/*		  	      end of  DestroyLock Syscall	    and DestroyLock RPC	  		        */

/*		  	      Start of  CreateCondition Syscall		  	and CreateCondition RPC	        */

int CreateCondition_Syscall(unsigned int vaddr)
{
/*New Code for CreateCondition RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;  //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;  //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="CrCdit";		  		        //specifies that Create Condition RPC is there
	int len=6;		   	                           
	char* namebuf;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting CreateCondition
	int cphldr=0;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	if ( !(namebuf = new char[len]) ) 		      //Get name of the Condition
	{
		printf("%s","Error allocating kernel buffer for write!\n");
		return -1;
    } 
	else
	{		  	              
        if ( copyin(vaddr,len,namebuf) == -1 )
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] namebuf;
			return -1;
		}
    }
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
	
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());
	
/*	cout<<"\nString Length of syscallType :: "<<strlen(syscallType)<<"\n"<<endl;
	cout<<"\nString Length of namebuf :: "<<strlen(namebuf)<<"\n"<<endl;
	cout<<"\nString Length of threadName :: "<<strlen(threadName)<<"\n"<<endl;  */
	
	outPktHdr.to=Server_Addr;
	outPktHdr.from=postOffice->netAddr;		  	        //Client Address from system.cc(postoffice object)
		
	outMailHdr.to=Server_Mbox_Addr;
	outMailHdr.from=currentThread->mbox;		  					        //More than one Mail box now
	
	
	
	sprintf(bufferToSend,"%s %s %d %s %d",syscallType,namebuf,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting for response . . . \n"<<endl;
	
	char* msgacknol=new char[15];
	char* id_c=new char[4];
	int id;
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);    //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s %s",msgacknol,id_c);
	
	id=atoi(id_c);
	
	cout<<"\n"<<msgacknol<<"\n"<<endl;
	cout<<"\n"<<id<<"\n"<<endl;
	
	delete msgacknol;
	delete id_c;
	
	return id;
	
#endif
/*Old Code for CreateCondition Syscall*/
#ifdef USER_PROGRAM
	int i=0;	
	
	conditionTableLock->Acquire();
	
	for(i=0;i<ConditionTableSize;i++)
	{
		if(conditionTable[i].status==0)
		{
			break;
		}
	}
	
	if(i>=ConditionTableSize)
	{
		cout<<"\nCondition Variable Quota exhausted . . . Cannot create a new Condition Variable . . . ";
		conditionTableLock->Release();
		return -1;
	}
	
	conditionTable[i].cd=new Condition();
	conditionTable[i].status=1;
	conditionTable[i].creatingThreadSpace=currentThread->space;
	
	cout<<"Condition Variable with ID :: "<<i<<" Created . . . ";
	
	conditionTableLock->Release();
	
	return conditionTable[i].id;
#endif
}

/*		  	      end of  CreateCondition Syscall		  and CreateCondition RPC  		        */

/*		  	      Start of  DestroyCondition Syscall	and DestroyCondition RPC	  		        */

void DestroyCondition_Syscall(int cid)
{
/*New Code for DestroyCondition RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;  //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;  //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="DeCdit";		  		        //specifies that Destroy Condition RPC is there
	int lockid=0;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting DestroyCondition
	int cphldr=0;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    cphldr=cid;											 //Condition ID
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Destroy Condition . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);  //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to DestroyCondition RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	return;
#endif
/*Old Code for DestroyCondition Syscall*/
#ifdef USER_PROGRAM
	conditionTableLock->Acquire();
	
	if(conditionTable[cid].status==0)
	{
		cout<<"\nTrying to Destroy a Condition Variable that has not been created . . . ";
		conditionTableLock->Release();
		return;
	}
	
	if(conditionTable[cid].creatingThreadSpace==currentThread->space)
	{	
		delete conditionTable[cid].cd;
		conditionTable[cid].status=0;
		conditionTable[cid].creatingThreadSpace=NULL;
		cout<<"\nCondition Variable with ID :: "<<cid<<" destroyed successfully . . . ";
		conditionTableLock->Release();
		return;
	}
	else
	{
		cout<<"\nError. Destroying A condition Variable created by another process . . . ";
		conditionTableLock->Release();
		return;
	}
#endif
}

/*		  	      end of  DestroyCondition Syscall		and DestroyCondition RPC  		        */

/*		  	      Start of  Acquire Syscall		  	and Acquire RPC	        */

void Acquire_Syscall(int idLock)
{
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;		//Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;        //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="AqLock";		  		        //specifies that Acquire RPC is there
	int len=6;		   	                            //length of the name for lock
	int lockid;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting Acquire
	int cphldr=0;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    lockid=idLock;										 //Lock ID to Acquire
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend); //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Acquire Lock . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);  //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to Acquire RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	return;
#endif
/*Old Acquire syscall*/
#ifdef USER_PROGRAM
//	lockAcquireLock->Acquire();
	
	if(lockTable[idLock].status==0)
	{
		cout<<"\nLock not Present. First Create Lock . . . \n";
//		lockAcquireLock->Release();
		return;
	}
	
	if(lockTable[idLock].creatingThreadSpace != currentThread->space)
	{
		cout<<"Not Authorized to acquire the lock . . . Was not created by the same process\n";
// 		lockAcquireLock->Release();
		return;
	}
	
//	lockTable[idLock].lock->Acquire();
				
	Lock* newLock=lockTable[idLock].lock;	
//	cout<<"\n999About to Acquire Lock :: "<<idLock<<endl;
	newLock->Acquire();
	
//	cout<<"\nLock with Lock ID :: "<<idLock<<" Successfully Acquired . . . "<<endl; 
	
//	lockAcquireLock->Release();
	
	return;
#endif
}

/*		  	      end of Acquire Syscall	and Acquire RPC	  		        */

/*		  	      Start of  Release Syscall		and Release RPC  		        */

void Release_Syscall(int idLock)
{
/*Code for Release Lock RPC */
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;  //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;  //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="ReLock";		  		        //specifies that Release RPC is there
	int len=6;		   	                            //length of the name for lock
	int lockid;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting Release
	int cphldr=0;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    lockid=idLock;										 //Lock ID to Release
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Release Lock . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);  //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to Release RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	
	return;
#endif
/*Old code for Release Lock syscall */
#ifdef USER_PROGRAM
//	lockReleaseLock->Acquire();
	
	if(lockTable[idLock].status==0)
	{
		cout<<"\nLock not Present. First Create Lock . . . ";
	//	lockReleaseLock->Release();
		return;
	}

	if(lockTable[idLock].creatingThreadSpace == currentThread->space)
	{
		Lock* newLock=lockTable[idLock].lock;
		newLock->Release();	
//		cout<<"\nLock with Lock ID :: "<<idLock<<" Successfully Released . . . ";
	//	lockReleaseLock->Release();
		return;
	}
	else
	{
		cout<<"Not Authorized to Release the lock . . . Was not created by the same process\n";
	//	lockReleaseLock->Release();
		return;
	}
#endif
}

/*		  	      end of  Release Syscall	and Release RPC	  		        */

/*		  	      Start of  Signal Syscall		  	and Signal RPC	        */

void Signal_Syscall(int Cid,int idLock)
{
/*New Code for Signal RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;  //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;  //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="SgCdit";		  		        //specifies that Signal RPC is there
	int lockid;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting Signal
	int cphldr=0;		  	   	 	 	 	        //condition ID
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    lockid=idLock;										 //Lock ID 
	cphldr=Cid;
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Signal . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);  //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to Signal RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	return;
#endif
/*Old Code for Signal Syscall*/
#ifdef USER_PROGRAM
//	lockSignalLock->Acquire();
	
	if(lockTable[idLock].status==0)
	{
		cout<<"\nLock not Present. First Create Lock . . . ";
//		lockSignalLock->Release();
		return;
	}
	
	if(conditionTable[Cid].status==0)
	{
		cout<<"\nCondition Variable not Present. First Create Condition Variable . . . ";
//		lockSignalLock->Release();
		return;
	}

	if(lockTable[idLock].creatingThreadSpace == currentThread->space && conditionTable[Cid].creatingThreadSpace == currentThread->space)
	{
		Condition* newCd=conditionTable[Cid].cd;
		Lock* newLock=lockTable[idLock].lock;
		newCd->Signal(newLock);
		cout<<"\nCondition Variable with ID :: "<<Cid<<" and Lock ID :: "<<idLock<<" Successfully Signalled . . . ";
//		lockSignalLock->Release();
		return;
	}
	else
	{
		cout<<"Not Authorized to Signal the Condition Variable . . . Either the Condition Variable or the Lock (Or may be both) was not created by the same process\n";
//		lockSignalLock->Release();
		return;
	}
#endif	
}

/*		  	      end of  Signal Syscall	and Signal RPC	  		        */

/*		  	      Start of  Wait Syscall	and Wait RPC	  		        */

void Wait_Syscall(int Cid,int idLock)
{
/*New Code for Wait RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;    //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;    //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive
	
	char* syscallType="WaCdit";		  		        //specifies that Wait RPC is there
	int lockid;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting Wait
	int cphldr=0;		  	   	 	 	 	        //Condition ID
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    lockid=idLock;										//Lock ID 
	cphldr=Cid;
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Wait . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);  //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to Wait RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	return;
#endif
/*Old Code for Wait Syscall*/
#ifdef USER_PROGRAM
//	lockWaitLock->Acquire();
	
	if(lockTable[idLock].status==0)
	{
		cout<<"\nLock not Present. First Create Lock . . . ";
//		lockWaitLock->Release();
		return;
	}
	
	if(conditionTable[Cid].status==0)
	{
		cout<<"\nCondition Variable not Present. First Create Condition Variable . . . ";
//		lockWaitLock->Release();
		return;
	}

	if(lockTable[idLock].creatingThreadSpace == currentThread->space && conditionTable[Cid].creatingThreadSpace == currentThread->space)
	{
		Condition* newCd=conditionTable[Cid].cd;
		Lock* newLock=lockTable[idLock].lock;
		newCd->Wait(newLock);
	//	cout<<"\nCondition Variable with ID :: "<<Cid<<" and Lock ID :: "<<idLock<<" Successfully Signalled . . . ";
//		lockWaitLock->Release();
		return;
	}
	else
	{
		cout<<"Not Authorized to Wait on the Condition Variable . . . Either the Condition Variable or the Lock (Or may be both) was not created by the same process\n";
//		lockWaitLock->Release();
		return;
	}
#endif
}

/*		  	      end of  Wait Syscall		and Wait RPC  		        */

/*		  	      Start of  Broadcast Syscall	  and Broadcast RPC	  		        */

void Broadcast_Syscall(int Cid,int idLock)
{
/*New Code for Broadcast RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;  //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;  //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];  //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1];  // Buffer to receive

	
	char* syscallType="BtCdit";		  		        //specifies that Broadcast RPC is there
	int lockid=idLock;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;									//mailbox ID for this client
	char* threadName=new char[10];		    	    //name of the current thread requesting Broadcast
	int cphldr=0;		  	   	 	 	 	        //Condition ID
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
    cphldr=Cid;											 //Lock ID 
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());        //Get the threads name
	
	outPktHdr.from=postOffice->netAddr;
	outPktHdr.to=Server_Addr;
	
	outMailHdr.from=currentThread->mbox;;
	outMailHdr.to=Server_Mbox_Addr;
	
	sprintf(bufferToSend,"%s %d, %d %s %d",syscallType,lockid,machineId,threadName,cphldr);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend); //Send the message to the Server 
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting to Broacast . . .\n"<<endl;
	
	char* msgacknol=new char[10];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);  //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\nMessage from server to Broadcast RPC :: "<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	return;
#endif	
/*Old Code for Broadcast Syscall*/
#ifdef USER_PROGRAM
//	lockBroadcastLock->Acquire();
	
	if(lockTable[idLock].status==0)
	{
		cout<<"\nLock not Present. First Create Lock . . . ";
//		lockBroadcastLock->Release();
		return;
	}
	
	if(conditionTable[Cid].status==0)
	{
		cout<<"\nCondition Variable not Present. First Create Condition Variable . . . ";
//		lockBroadcastLock->Release();
		return;
	}

	if(lockTable[idLock].creatingThreadSpace == currentThread->space && conditionTable[Cid].creatingThreadSpace == currentThread->space)
	{
		Condition* newCd=conditionTable[Cid].cd;
		Lock* newLock=lockTable[idLock].lock;
		newCd->Broadcast(newLock);
		cout<<"\nCondition Variable with ID :: "<<Cid<<" and Lock ID :: "<<idLock<<" Successfully Broadcasted . . . ";
//		lockBroadcastLock->Release();
		return;
	}
	else
	{
		cout<<"Not Authorized to Broadcast the Condition Variable . . . Either the Condition Variable or the Lock (Or may be both) was not created by the same process\n";
//		lockBroadcastLock->Release();
		return;
	}
#endif
	
}

/*		  	      end of  Broadcast Syscall		  and Broadcast RPC		        */

/*		  	      Start of GetValue RPC  		        */

int GetValue_Syscall(unsigned int vaddr,int index)
{
	char* varname;
/*Code for GetValue RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;		       //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;			   //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];    //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1]; // Buffer to receive
	
	char* syscallType="getVal";		  		        //specifies that GetValue RPC is there
	int len=15;		   	                            //length of the name for variable name
	char* namebuf;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;
	char* threadName=new char[10];		    	    //name of the current thread requesting CreateLock
	int arrindex=index;		  	   	 	 	 	    //index into array(If any)
	
	if ( !(namebuf = new char[len]) ) 		      //Get name of the Lock
	{
		printf("%s","Error allocating kernel buffer for write!\n");
		return -1;
    } 
	else
	{		  	              
        if ( copyin(vaddr,len,namebuf) == -1 )
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] namebuf;
			return -1;
		}
    }	

	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
	
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;					 	//Get Mailbox ID for this client
		
	strcpy(threadName,currentThread->getName());
	
/*	cout<<"\nString Length of syscallType :: "<<strlen(syscallType)<<"\n"<<endl;
	cout<<"\nString Length of namebuf :: "<<strlen(namebuf)<<"\n"<<endl;
	cout<<"\nString Length of threadName :: "<<strlen(threadName)<<"\n"<<endl;		    */
	
	outPktHdr.to=Server_Addr;
	outPktHdr.from=postOffice->netAddr;		  	        //Client Address from system.cc(postoffice object)
		
	outMailHdr.to=Server_Mbox_Addr;
	outMailHdr.from=mailboxId;		  					        //More than one Mail box
	
	
	
	sprintf(bufferToSend,"%s %s %d %s %d",syscallType,namebuf,machineId,threadName,arrindex);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  		     		    //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting for response . . . \n"<<endl;
	
	char* msgacknol=new char[15];
	char* val_c=new char[4];
	int val;
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);   		  	               //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s %s",msgacknol,val_c);
	
	val=atoi(val_c);
	
	cout<<"\n"<<msgacknol<<"\n"<<endl;
	cout<<"\n"<<val<<"\n"<<endl;
	
	delete msgacknol;
	delete val_c;
	
	return val;
#endif
}

/*		  	      end of GetValue RPC  		        */

/*		  	      Start of SetValue RPC  		        */

void SetValue_Syscall(unsigned int vaddr,int val,int index)
{
	char* varname;
/*Code for SetValue RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;		       //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;			   //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];    //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1]; // Buffer to receive
	
	char* syscallType="setVal";		  		        //specifies that Create Lock RPC is there
	int len=15;		   	                            //length of the name for lock
	char* namebuf;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;
//	char* threadName=new char[10];		    	    //name of the current thread(Not needed for Set Value)
	int valtosend=val;
	int arrindex=index;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	if ( !(namebuf = new char[len]) ) 		      //Get name of the Lock
	{
		printf("%s","Error allocating kernel buffer for write!\n");
		return ;
    } 
	else
	{		  	              
        if ( copyin(vaddr,len,namebuf) == -1 )
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] namebuf;
			return ;
		}
    }		  	 
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
	
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;
		
//	strcpy(threadName,currentThread->getName());
	
/*	cout<<"\nString Length of syscallType :: "<<strlen(syscallType)<<"\n"<<endl;
	cout<<"\nString Length of namebuf :: "<<strlen(namebuf)<<"\n"<<endl;
	cout<<"\nString Length of threadName :: "<<strlen(threadName)<<"\n"<<endl;		    */
	
	outPktHdr.to=Server_Addr;
	outPktHdr.from=postOffice->netAddr;		  	        //Client Address from system.cc(postoffice object)
		
	outMailHdr.to=Server_Mbox_Addr;
	outMailHdr.from=mailboxId;		  					        //Only one Mail box now
	
	
	
	sprintf(bufferToSend,"%s %s %d %d %d",syscallType,namebuf,machineId,valtosend,arrindex);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  		     		    //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting for response . . . \n"<<endl;
	
	char* msgacknol=new char[15];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);   		  	               //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\n"<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	
	return;
#endif
}

/*		  	      end of SetValue RPC  		        */

/*		  	      Start of IncrementbyVal RPC  		        */

void IncrementbyVal_Syscall(unsigned int vaddr,int val,int index)
{
	char* varname;
/*Code for IncrementbyVal RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;		       //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;			   //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];    //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1]; // Buffer to receive
	
	char* syscallType="incVal";		  		        //specifies that Create Lock RPC is there
	int len=15;		   	                            //length of the name for lock
	char* namebuf;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;
//	char* threadName=new char[10];		    	    //name of the current thread (Not needed for IncrementbyVal)
	int valtosend=val;
	int arrindex=index;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	if ( !(namebuf = new char[len]) ) 		      //Get name of the Lock
	{
		printf("%s","Error allocating kernel buffer for write!\n");
		return ;
    } 
	else
	{		  	              
        if ( copyin(vaddr,len,namebuf) == -1 )
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] namebuf;
			return ;
		}
    }		  	 
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
	
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;
		
//	strcpy(threadName,currentThread->getName());
	
/*	cout<<"\nString Length of syscallType :: "<<strlen(syscallType)<<"\n"<<endl;
	cout<<"\nString Length of namebuf :: "<<strlen(namebuf)<<"\n"<<endl;
	cout<<"\nString Length of threadName :: "<<strlen(threadName)<<"\n"<<endl;		    */
	
	outPktHdr.to=Server_Addr;
	outPktHdr.from=postOffice->netAddr;		  	        //Client Address from system.cc(postoffice object)
		
	outMailHdr.to=Server_Mbox_Addr;
	outMailHdr.from=mailboxId;		  					        //Only one Mail box now
	
	
	
	sprintf(bufferToSend,"%s %s %d %d %d",syscallType,namebuf,machineId,valtosend,arrindex);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  		     		    //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting for response . . . \n"<<endl;
	
	char* msgacknol=new char[15];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);   		  	               //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\n"<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;
	
	return;
#endif	
}

/*		  	      end of IncrementbyVal RPC  		        */

/*		  	      Start of DecrementbyVal RPC  		        */

void DecrementbyVal_Syscall(unsigned int vaddr,int val,int index)
{
	char* varname;
/*Code for DecrementbyVal RPC*/
#ifdef NETWORK
	PacketHeader outPktHdr,inPktHdr;		       //Packet and Mail Headers for the data to be sent
	MailHeader outMailHdr,inMailHdr;			   //specifying Source and Destination Addresses
	
	char* bufferToSend=new char[MaxMailSize+1];    //Buffer To send
	char* bufferToReceive=new char[MaxMailSize+1]; // Buffer to receive
	
	char* syscallType="decVal";		  		        //specifies that Create Lock RPC is there
	int len=15;		   	                            //length of the name for lock
	char* namebuf;
	int machineId;			  	                    //machine ID for this client
	int mailboxId;
//	char* threadName=new char[10];		    	    //name of the current thread (Not needed for DecrementbyVal)
	int valtosend=val;
	int arrindex=index;		  	   	 	 	 	        //Place holder for use in Wait RPC
	
	if ( !(namebuf = new char[len]) ) 		      //Get name of the Lock
	{
		printf("%s","Error allocating kernel buffer for write!\n");
		return ;
    } 
	else
	{		  	              
        if ( copyin(vaddr,len,namebuf) == -1 )
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] namebuf;
			return ;
		}
    }		  	 
	
	bufferToSend[MaxMailSize]='\0';
	bufferToReceive[MaxMailSize]='\0';
	
	
	machineId=postOffice->netAddr;		  	            //Get machine ID for this client
	mailboxId=currentThread->mbox;
		
//	strcpy(threadName,currentThread->getName());
	
/*	cout<<"\nString Length of syscallType :: "<<strlen(syscallType)<<"\n"<<endl;
	cout<<"\nString Length of namebuf :: "<<strlen(namebuf)<<"\n"<<endl;
	cout<<"\nString Length of threadName :: "<<strlen(threadName)<<"\n"<<endl;		    */
	
	outPktHdr.to=Server_Addr;
	outPktHdr.from=postOffice->netAddr;		  	        //Client Address from system.cc(postoffice object)
		
	outMailHdr.to=Server_Mbox_Addr;
	outMailHdr.from=mailboxId;		  					        //Only one Mail box now
	
	
	
	sprintf(bufferToSend,"%s %s %d %d %d",syscallType,namebuf,machineId,valtosend,arrindex);
	
	cout<<"\nString Length of bufferToSend :: "<<strlen(bufferToSend)<<"\n"<<endl;
	
	outMailHdr.length=strlen(bufferToSend)+1;
	
	bool success=postOffice->Send(outPktHdr,outMailHdr,bufferToSend);  		     		    //Send the message to the Server
	
	if ( !success ) 
	{
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }
	
	cout<<"Message sent . . . Waiting for response . . . \n"<<endl;
	
	char* msgacknol=new char[15];
	
	postOffice->Receive(0,&inPktHdr,&inMailHdr,bufferToReceive);   		  	               //Receive Acknowledgement from server
	
	sscanf(bufferToReceive,"%s",msgacknol);
	
	cout<<"\n"<<msgacknol<<"\n"<<endl;
	
	delete msgacknol;

	return ;
#endif
}

/*		  	      end of DecrementbyVal RPC  		        */



//Exec System Call

//AddrSpace* exec_threadSpace=NULL;

SpaceId Exec_Syscall(unsigned int vaddr,int len)
{
	char *buf;				// Kernel buffer for output
    OpenFile* executable;	// Open file for output
	AddrSpace* space;
	int i;
	int k=0,indexTbl,flag0=0;
	
	

    buf = new char[len+1];	// Kernel buffer to put the name in
    
    if (!buf) 
	{
		printf("%s","Can't allocate kernel buffer in Open\n");
		return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) 
	{
		printf("%s","Bad pointer passed to Open\n");
		delete[] buf;
		return -1;
    }

    buf[len]='\0';

	executable=fileSystem->Open(buf);
	
	if(executable==NULL)
	{
		printf("Unable to open file %s\n", buf);
		return -1;
	}
	
	  
	processTableLock->Acquire();
	
	space=new AddrSpace(executable);
	
	stringstream ss;
	ss<<"Process_"<<execThreadCount;
	string *s = new string;
	s->assign( ss.str() );
	
	Thread* newThread=new Thread( (char *)s->c_str() );
		
	newThread->space=space;
	
	//exec_threadSpace=space;
	
	//Process Table entry
	
	
//	processCntLock->Acquire();		
	
/*	for(k=0;k<ProcessTableSize;k++)
	{
		if(processTable[k].status==0)
		{			
			processTable[k].status=1;
			indexTbl=k;			
			break;
		}
	}
	
	if(k>=ProcessTableSize)
	{
		cout<<"\nProcess Table full. Cannot accomodate more Processes. Error . . . ";
		processTableLock->Release();	
		return -1;
	}		         */
	
	processCnt++;
	execThreadCount++;
	
	processTable[proId].processID=proId;
	processTable[proId].status=1;
//	processTable[indexTbl].status=1;
	processTable[proId].mainSpace=space;
//	processTable[indexTbl].parentThread=currentThread;
	processTable[proId].createdThread=newThread;
	processTable[proId].cntChildThreads=1;
	processTable[proId].childEntry[0].thread=newThread;
	proId++;
	
	processTableLock->Release();
	printf("released processtablelock in exec\n");
	
//	processTable[indexTbl].type=0;
	
//	processTable[indexTbl].childEntry[0].thread=newThread;
	
//	processTableLock->Release();
		
//    processCntLock->Acquire();			
	
//	processCntLock->Release();
	
//	cout<<"\n"<<currentThread->getName()<<" :Before fork in Exec \n";
	
	newThread->Fork((VoidFunctionPtr)exec_thread,0);
	
//	currentThread->Yield();
	
    delete[] buf;
	//int* paddr;
	//machine->Translate(vaddr,paddr,sizeof(unsigned int),FALSE);
	
//	processCntLock->Release();
	

	return proId-1;
} 

void exec_thread()
{	
//	exec_thread_lock->Acquire();

//	cout<<"\n"<<currentThread->getName()<<" : After fork in exec_thread";
//	cout<<"\nIn exec_thread . . . \n";
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
	
//	cout<<"\nAbout to machine->Run() . . . \n";
	
//	cout<<"\nNumber of Processes :: "<<processCnt<<"\n";
	
//	exec_thread_lock->Release();
	
	machine->Run();
	
}



void Fork_Syscall(unsigned int vaddr,int arg)
{
//	printf("acquiring process table lock\n");
//	cout<<"\nprocessTableLock :: "<<processTableLock<<endl;
	
	
	processTableLock->Acquire();
	
	
	forkThreadCount++;
	
	printf("acquired process table lock\n");
	
	stringstream ss;
	ss<<"Fork Thread : "<<forkThreadCount;
	string *s = new string;
	s->assign( ss.str() );
	
	Thread* newThread=new Thread((char *)s->c_str());	
	
	newThread->space=currentThread->space;
	
	int i,entryFlag=0,j,entryflag1=0,index=999;
	
//	processTableLock->Acquire();
	
	for(i=0;i<ProcessTableSize;i++)
	{
		if(processTable[i].mainSpace==newThread->space)
		{
			index=i;
			entryFlag=1;
			break;
		}
	}
	
	if(i>=ProcessTableSize)
	{
		cout<<"\nNo parent thread found in process Table . Error . . . ";
		processTableLock->Release();		
		return;
	}
	
//	if(entryFlag)
//	{				
		processTable[index].childEntry[processTable[index].cntChildThreads].thread=newThread;
		processTable[index].cntChildThreads++;
		
	
		
		
		printf("about to release process table lock\n");
		processTableLock->Release();
		printf("released process table lock\n");
		
//		forkThreadCount++;
		
	//	cout<<"\nIn Fork Syscall. Before Fork. About to Fork . . . \n";
	
		newThread->Fork((VoidFunctionPtr)kernel_thread,vaddr);
		
		
	
//		currentThread->Yield();

/*		printf("about to release process table lock\n");
		processTableLock->Release();
		printf("released process table lock\n");					  		        */

//	}
//	else
//	{
//		cout<<"\nNo Parent Thread found in process Table ! Error . . . \n";
//		processTableLock->Release();
		return;
//	}	
	
//	forkThreadCount++;
	
	

	
/*	cout<<"\nIn Fork Syscall. Before Fork. About to Fork . . . \n";
	
	newThread->Fork((VoidFunctionPtr)kernel_thread,vaddr);
	
	currentThread->Yield();		  		          */       //   Moved UP
	
}

void kernel_thread(unsigned int vaddr)
{
//	kernel_thread_lock->Acquire();

//	cout<<"\nIn kernel thread of fork . . . Writing PCReg, NextPCReg . . .\n";
	machine->WriteRegister(PCReg,vaddr);				  		        //Fork, therefore PCReg updated with address of The function to be executed by fork (Function's virtual address vaddr)
	machine->WriteRegister(NextPCReg,vaddr+4);
	currentThread->space->RestoreState();
	
	int i;
	
	
	currentThread->space->stackMapLock->Acquire();
	
	int free=currentThread->space->stackMap->Find();
	currentThread->space->stackMap->Mark(free);
	
	for(i=free+1;i<free+8;i++)
	{
		currentThread->space->stackMap->Mark(i);
	}
	
	currentThread->space->stackMapLock->Release();
	
	free=free+7;
	
	int toStackReg=(currentThread->space->numPages*PageSize) + (free*PageSize) - 16;
	
	currentThread->toStackReg=toStackReg;
	/*
	cout<<"\nnumPages :: "<<currentThread->space->numPages;
	
	cout<<"\nfree page found at :: "<<free<<" in stackMap . . . (In Pages) ";
	cout<<"\nStack starting from(toStackReg) :: "<<toStackReg;					    */
	
	machine->WriteRegister(StackReg,toStackReg);
	//cout<<"\nnumPagesIncStack :: "<<numPagesIncStack;
	//cout<<"\nallocStackAddr :: "<<allocStackAddr;
	//allocStackAddr=allocStackAddr-8;
	
//	kernel_thread_lock->Release();

	cout<<"\nStackReg for fork thread starting at :: "<<machine->ReadRegister(StackReg)<<"\n"<<endl;
	
	machine->Run();
	
	
	
//write to the stack register , the starting postion of the stack for this thread. 

}

void RemoveForkStack()
{
	int toStackReg,readStackReg,pageStackReg;
	int i=999;
	toStackReg=currentThread->toStackReg;		  		        //This  and the next line do the same thing
	readStackReg=toStackReg;/*machine->ReadRegister(StackReg);*/				        //This and the previous line do the same thing	
	readStackReg=readStackReg+16;
	pageStackReg=divRoundUp(readStackReg, PageSize);
	pageStackReg=pageStackReg - currentThread->space->numPages;
	
	currentThread->space->stackMapLock->Acquire();	
	for(i=pageStackReg;i>(pageStackReg-8);i--)
	{
		currentThread->space->stackMap->Clear(i);
	}
	currentThread->space->stackMapLock->Release();
	
	return;
}

void Exit_Syscall(int status)
{
	int i,j,k,stackRemovalFlag=0,index=999,procFoundFlag=0,forkFoundFlag=0,indexProc=999,indexChild=999;
	inExit++;
	
	
	
	cout<<"\n"<<currentThread->getName()<<" :: In Exit System Call for "<<inExit<<" time . . .\n"<<endl;
	cout<<"\n"<<currentThread->space->pid<<" :: Process In Exit System Call for "<<inExit<<" time . . .\n"<<endl;
	cout<<"\n Output of test function is"<<status<<endl;
/*	if(postOffice->netAddr>0)
	{
		interrupt->Halt();
	}
	else
	{		  	  	                   */
		currentThread->Finish();
//		currentThread->Yield();
//	}
	/*
	
	processTableLock->Acquire();
	
	for(i=0;i<ProcessTableSize;i++)
	{
		if(processTable[i].createdThread==currentThread)
		{
			procFoundFlag=1;
			if(currentThread->getName()!="main")
			{
				execThreadCount--;
			}
			processCnt--;
			indexProc=i;
			break;
		}
	}
	
	if(procFoundFlag!=1)
	{
		for(j=0;j<ProcessTableSize;j++)
		{
			for(k=0;k<50;k++)
			{
		
				if(processTable[j].childEntry[k].thread==currentThread)
				{
					forkFoundFlag=1;
					forkThreadCount--;
				//	stackRemovalFlag=1;
					indexProc=j;
					indexChild=k;
					break;
				}
			}
			if(forkFoundFlag==1)
			{
				break;
			}
		}
	}
	
	if(indexProc==999 && indexChild==999)
	{
		cout<<"\nNo thread entry in Process Table. Error . . . \n"<<endl;
		processTableLock->Release();
		currentThread->Finish();
		return;
	}
	
	cout<<"\nAbout to check exiting conditions execThreadCount :: "<<execThreadCount<<" : forkThreadCount :: "<<forkThreadCount<<"\n"<<endl;

	cout<<"\nAbout to check exiting conditions. Total number of threads remaining :: "<<execThreadCount+forkThreadCount;

	if(processCnt==0 && forkThreadCount==0)
	{
		if(forkFoundFlag)
		{
			RemoveForkStack();
		}
		processTable[indexProc].status=0;
		processTable[indexProc].cntChildThreads--;
//		processTable[index].type=-1;
		delete currentThread->space;
		processTableLock->Release();
		cout<<"\nMachine about to Halt. Last thread of last process . . . \n"<<endl;
		interrupt->Halt();
	}
	
	if(processTable[indexProc].cntChildThreads==1)  //remove indexProc by i and see the result
	{
		if(forkFoundFlag)
		{
			RemoveForkStack();
		}
		processTable[indexProc].status=0;
		processTable[indexProc].cntChildThreads--;
//		processTable[index].type=-1;
//		delete currentThread->space;
		processTableLock->Release();
		cout<<"\nLast thread of a process . . . \n"<<endl;
		currentThread->Finish();
	}
	
	cout<<"\n"<<currentThread->getName()<<" : Any other general thread . . . \n"<<endl;
	if(forkFoundFlag)
	{
	
		RemoveForkStack();
	}
	processTable[indexProc].cntChildThreads--;
	processTableLock->Release();
	
	currentThread->Finish();
	
	//stackRemovalFlag=0;
	
	
	*/
	return;
}
	




void Cwrite_Syscall(unsigned int vaddr,int len)
{
	char *buf;		// Kernel buffer for output
    
    
    if ( !(buf = new char[len+1]) )
	{
		printf("%s","Error allocating kernel buffer for Cwrite!\n");
		return;
    }
	else
	{
        if ( copyin(vaddr,len,buf) == -1 )
		{
			printf("%s","Bad pointer passed to to Cwrite: data not written\n");
			delete[] buf;
			return;
		}
    }	
	
	buf[len]='\0';
	
	fflush(stdin);
	cout<<"\n"<<buf;
	
		
	
}




int myexp ( int count ) 
{
  int i, val=1;
  for (i=0; i<count; i++ ) 
  {
	val = val * 10;
  }
  
  return val;
  
}



void itoa_Syscall( unsigned int vaddr, int size, int val)
{
	int i, max, dig, subval, loc;
	
	char arr[6];		// Kernel buffer for input    

//   cout<<"\nIn itoa_Syscall . . . ";
	
	for (i=0; i<size; i++ ) 
	{
		arr[i] = '\0';
	}

	for ( i=1; i<=2; i++ ) 
	{
		if (( val / myexp(i) ) == 0 ) 
		{
		max = i-1;
		break;
		}
	}

	subval = 0;
	loc = 0;
	
	char c='\0';
	
//	cout<<"\nc is :: "<<c;
	
	for ( i=max; i>=0; i-- ) 
	{
		dig = 48 + ((val-subval) / myexp(i));
		subval = (dig-48) * myexp(max);
		arr[loc] = dig;
		loc++;
	}
	
//	cout<<"\nBefore copyout . . . \n";

	if ( copyout(vaddr, size, arr) == -1 ) 
	{	
		printf("%s","Bad pointer passed to Read: data not copied\n");
	}
    
  //  cout<<"\nAfter copyout . . . \n";

	return;
	
}



/*		  	      Start of  HandlePageFault Syscall		  		        */		

/************************************
TLB POPULATION
************************************/
	
void UpdateTLBFromIPT(int physPage)			//Page is in IPT update entries in TLB
			{
				if(machine->tlb[currentTLBSlot].valid == TRUE && machine->tlb[currentTLBSlot].dirty == TRUE) //Check dirty bit of overwritten TLB slot 
						{
							IPT[machine->tlb[currentTLBSlot].physicalPage]->dirty = TRUE;
							processTable[IPT[machine->tlb[currentTLBSlot].physicalPage]->processId].mainSpace->pageTable[IPT[machine->tlb[currentTLBSlot].physicalPage]->virtualPage].dirty = TRUE;
						}
			
				
					machine->tlb[currentTLBSlot].virtualPage  = IPT[physPage]->virtualPage;					
					machine->tlb[currentTLBSlot].physicalPage = IPT[physPage]->physicalPage;
					machine->tlb[currentTLBSlot].valid = IPT[physPage]->valid;
					machine->tlb[currentTLBSlot].use = IPT[physPage]->use;
					machine->tlb[currentTLBSlot].dirty = IPT[physPage]->dirty;
					machine->tlb[currentTLBSlot].readOnly = IPT[physPage]->readOnly;
		
				
					
					currentTLBSlot=(currentTLBSlot + 1)%TLBSize;
			
			}

										
/************************************
IPT POPULATION
************************************/
void UpdateIPTFromPT(int badVPNum)
			{
				
				//It is not in memory so find a place first
				int foundPhyPageOnMem;
				int swapPage;
				int Policy=1;
			
				memLock->Acquire();
				foundPhyPageOnMem = memMap->Find();
						
				if(foundPhyPageOnMem != -1) //If there is an empty space in memory directly write to there
					{
					currentThread->space->pageTable[badVPNum].physicalPage = foundPhyPageOnMem;
					
					
					if(currentThread->space->pageTable[badVPNum].PL == DISK)  //Read from Executable and write to memory if the bad virtual page is in executable
						{
							currentThread->space->executable_show->ReadAt(&(machine->mainMemory[currentThread->space->pageTable[badVPNum].physicalPage*PageSize]),PageSize,currentThread->space->pageTable[badVPNum].execPLoc);
							
							//cout<<"MEMORY STATUS, the virtual page number "<<badVPNum<<"is written to memory's "<<nummemd<<" th place"<<endl;
							nummemd++;
						}
						
					else if(currentThread->space->pageTable[badVPNum].PL == SWAP) //Read from Swap and write to memory if the bad virtual page is in swap
						{
							//FIND IN SWAP,write to like readeat, delete swap location
							swapFile->ReadAt(&(machine->mainMemory[currentThread->space->pageTable[badVPNum].physicalPage*PageSize]),PageSize,currentThread->space->pageTable[badVPNum].swapPLoc* PageSize);
							swapMap->Clear(currentThread->space->pageTable[badVPNum].swapPLoc); // Delete swap location
							
						}
	
					IPT[foundPhyPageOnMem]->virtualPage = currentThread->space->pageTable[badVPNum].virtualPage;					
					IPT[foundPhyPageOnMem]->physicalPage = currentThread->space->pageTable[badVPNum].physicalPage;
					IPT[foundPhyPageOnMem]->valid = currentThread->space->pageTable[badVPNum].valid;	
					IPT[foundPhyPageOnMem]->use = currentThread->space->pageTable[badVPNum].use;
					IPT[foundPhyPageOnMem]->dirty = currentThread->space->pageTable[badVPNum].dirty;
					IPT[foundPhyPageOnMem]->readOnly = currentThread->space->pageTable[badVPNum].readOnly;
					
					IPT[foundPhyPageOnMem]->PL = currentThread->space->pageTable[badVPNum].PL;
					IPT[foundPhyPageOnMem]->processId = currentThread->space->pid;
					IPT[foundPhyPageOnMem]->pageType = currentThread->space->pageTable[badVPNum].pageType;


					
					IntFifo*  fifoval=new IntFifo();			//Append it to the fifo queue
					fifoval->phypage=foundPhyPageOnMem;
					IPTQueue->Append(fifoval);
				
					
						
						memLock->Release();
						return;
					}
				else if (foundPhyPageOnMem == -1)		//If there is no space in memory choose a place according to policy
					{    
						int Flag = 0;
						//Random or FIFO take a page from memory
						
						if(prPolicy == 1)	//FIFO Policy															
							{		
								
								IntFifo*  fifoval=new IntFifo();
								fifoval=  (IntFifo*)IPTQueue->Remove();
								foundPhyPageOnMem=fifoval->phypage;
								
								
							}
						else if(prPolicy == 2) 	//RANDOM Policy
							{
								foundPhyPageOnMem = rand()%32;								//CHOOSE A NUMBER BETWEEN 0-32 equate it to foundPhyPageOnMem
							}
						else if (prPolicy == 0)
							{
								cout<<"No valid policy selected, error machine halting"<<endl;
								interrupt->Halt();
							}
						//According to its type put it in swap or ignore it
							
						
							
						for (int m=0; m<TLBSize; m++)   //Before swapping out form the memory invalidate TLB entry
							{
								
								if(machine->tlb[m].physicalPage == foundPhyPageOnMem)
									{
										
										machine->tlb[m].valid = FALSE;
									}
							}
							
						//Check TLB if it is there, check dirty and update for IPT and PT invalidate TLB	
						for (int m=0; m<TLBSize; m++) 
							{
								
								if(machine->tlb[m].physicalPage == foundPhyPageOnMem && machine->tlb[m].dirty == TRUE)
									{
										IPT[foundPhyPageOnMem]->dirty = TRUE;
										processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].dirty = TRUE; 
										
									}
							}
						
						// If the page is in swap before find a place in swap write it to there update PT
						if (IPT[foundPhyPageOnMem]->PL == SWAP)
							{
								swapPage = swapMap->Find();
								swapFile->WriteAt(&(machine->mainMemory[foundPhyPageOnMem*PageSize]),PageSize,swapPage* PageSize);
								int tmp5; 
								int tmp6 = processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].swapPLoc;
								processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].PL = SWAP;
								processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].swapPLoc = swapPage;
								
								//cout<<"SAVING from memory to SWAP, the virtual page number "<<IPT[foundPhyPageOnMem]->virtualPage<<" located in the memory's"<<foundPhyPageOnMem<<"th page will be written to swap's "<<swapPage<<"th page"<<endl;
								
							}
						
						// If the page is in disk before and get dirty now find a place in swap write it to there update PT	
						if (IPT[foundPhyPageOnMem]->PL == DISK && IPT[foundPhyPageOnMem]->dirty == TRUE)
							{
								swapPage = swapMap->Find();
								swapFile->WriteAt(&(machine->mainMemory[foundPhyPageOnMem*PageSize]),PageSize,swapPage* PageSize);
								int tmp7;
								int tmp8 = processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].swapPLoc;
								processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].PL = SWAP; 
								processTable[IPT[foundPhyPageOnMem]->processId].mainSpace->pageTable[IPT[foundPhyPageOnMem]->virtualPage].swapPLoc = swapPage;
								
								//cout<<"SAVING from memory to SWAP, the virtual page number "<<IPT[foundPhyPageOnMem]->virtualPage<<" located in the memory's"<<foundPhyPageOnMem<<"th page will be written to swap's "<<swapPage<<"th page"<<endl;
								
									
							}
						
							// If the page is in disk before and is not still dirty do nothing just ignore it	
						if(IPT[foundPhyPageOnMem]->PL == DISK && IPT[foundPhyPageOnMem]->dirty == FALSE)
							{
								//cout<<"SAVING from memory to DISK, the virtual page number "<<IPT[foundPhyPageOnMem]->virtualPage<<" located in the memory's"<<foundPhyPageOnMem<<"th page will be written to disk NO ACTION "<<endl;
								
							}
					
							
						/****************************
							SAVING ABOVE WRITING  BELOW
						*****************************/
						currentThread->space->pageTable[badVPNum].physicalPage = foundPhyPageOnMem;
					
					
						// If the page is in executable read from executable and write the page to the place found by the policy 	
						if(currentThread->space->pageTable[badVPNum].PL == DISK) 
							{
								int tmp1 = currentThread->space->pageTable[badVPNum].physicalPage;
								int tmp2 = currentThread->space->pageTable[badVPNum].execPLoc;
								int tmp10= tmp2/PageSize;
								currentThread->space->executable_show->ReadAt(&(machine->mainMemory[tmp1*PageSize]),PageSize, tmp2);
								////cout<<"WRITING form DISK to memory, the virtual page number "<<badVPNum<<" stored in the "<<tmp10<<" th place on disk will be written to "<<tmp1<<"on the memory"<<endl;
								
							}
							
						// If the page is in swap read from swap and write the page to the place found by the policy	
						else if(currentThread->space->pageTable[badVPNum].PL == SWAP)
							{
								//FIND IN SWAP,write to like readeat, delete swap location
								int tmp3 = currentThread->space->pageTable[badVPNum].physicalPage;
								int tmp4 = currentThread->space->pageTable[badVPNum].swapPLoc;
								swapFile->ReadAt(&(machine->mainMemory[tmp3*PageSize]),PageSize, tmp4*PageSize);
								swapMap->Clear(currentThread->space->pageTable[badVPNum].swapPLoc);
								////cout<<"WRITING form SWAP to memory, the virtual page number "<<badVPNum<<" stored in the "<<tmp4<<" th place on swap will be written to "<<tmp3<<"on the memory"<<endl;
								
							
							}
						
						
						//cout<<"MEMORY STATUS, the virtual page number "<<badVPNum<<"is written to memory's "<<currentThread->space->pageTable[badVPNum].physicalPage<<" th place"<<endl;
						IPT[foundPhyPageOnMem]->virtualPage = currentThread->space->pageTable[badVPNum].virtualPage;					
						IPT[foundPhyPageOnMem]->physicalPage = currentThread->space->pageTable[badVPNum].physicalPage;
						IPT[foundPhyPageOnMem]->valid = currentThread->space->pageTable[badVPNum].valid;	
						IPT[foundPhyPageOnMem]->use = currentThread->space->pageTable[badVPNum].use;
						IPT[foundPhyPageOnMem]->dirty = currentThread->space->pageTable[badVPNum].dirty;
						IPT[foundPhyPageOnMem]->readOnly = currentThread->space->pageTable[badVPNum].readOnly;
						
						IPT[foundPhyPageOnMem]->PL = currentThread->space->pageTable[badVPNum].PL;
						IPT[foundPhyPageOnMem]->processId = currentThread->space->pid;
						IPT[foundPhyPageOnMem]->pageType = currentThread->space->pageTable[badVPNum].pageType;
						
						
						//Append the page to the fifo queue
						IntFifo*  fifoval=new IntFifo();
						fifoval->phypage=foundPhyPageOnMem;
						IPTQueue->Append(fifoval);
					
		
						memLock->Release();
						return;
					}
				
				
				

			}									

																	

	
	void HandlePageFaultIPT(unsigned int badVAddress)
	{
		
		int badVPageNumber=machine->ReadRegister(BadVAddrReg)/PageSize;		//Find bad virtual adress page
		int u=1;
		int physAddrPage;
		
		
			{
		
				for(int m=0;m < NumPhysPages; m++)
				{
					if (IPT[m]->virtualPage == badVPageNumber && (IPT[m]->processId == currentThread->space->pid && IPT[m]->valid == TRUE ))
						{
							
							UpdateTLBFromIPT(IPT[m]->physicalPage); 		//If virtual page is in IPT update TLB from IPT
							u=10;
							return; 
						}
				}
		
				if(u==1)
					{
						
						UpdateIPTFromPT(badVPageNumber);					//If virtual page isnot in IPT update IPT from PT
						return;
						
					}
		

			}
		
		
	}		
	
/*		  	      end of HandlePageFault Syscall		  		        */	
	


									//u
void ExceptionHandler(ExceptionType which) {	
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) 
	{
	
/*	if(flag)
	{
		int i;
		for(i=0;i<LockTableSize;i++)
		{
			lockTable[i].lock=NULL;
			lockTable[i].id=i;
			lockTable[i].status=0;
		}
		
		i=0;
		for(i=0;i<ConditionTableSize;i++)
		{
			conditionTable[i].cd=NULL;
			conditionTable[i].id=i;
			conditionTable[i].status=0;
		}
		
		flag=0;
	}		  		      		  		    */
	
	switch (type)
	{
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
		case SC_Yield:
		DEBUG('a', "Yield syscall.\n");
		Yield_Syscall();
		break;
		case SC_CreateLock:
		DEBUG('a', "CreateLock syscall.\n");
		rv=CreateLock_Syscall(machine->ReadRegister(4));
		break;
		case SC_DestroyLock:
		DEBUG('a', "DestroyLock syscall.\n");
		DestroyLock_Syscall(machine->ReadRegister(4));
		break;
		case SC_CreateCondition:
		DEBUG('a', "CreateCondition syscall.\n");
		rv=CreateCondition_Syscall(machine->ReadRegister(4));
		break;
		case SC_DestroyCondition:
		DEBUG('a', "DestroyCondition syscall.\n");
		DestroyCondition_Syscall(machine->ReadRegister(4));
		break;
		case SC_Acquire:
		DEBUG('a', "Acquire syscall.\n");
		Acquire_Syscall(machine->ReadRegister(4));
		break;
		case SC_Release:
		DEBUG('a', "Release syscall.\n");
		Release_Syscall(machine->ReadRegister(4));
		break;
		case SC_Signal:
		DEBUG('a', "Signal syscall.\n");
		Signal_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;
		case SC_Wait:
		DEBUG('a', "Signal syscall.\n");
		Wait_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;
		case SC_Broadcast:
		DEBUG('a', "Signal syscall.\n");
		Broadcast_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;
		case SC_Exec:	
		DEBUG('a', "Exec syscall.\n");
		rv=Exec_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;
		case SC_Fork:		
		DEBUG('a', "Fork syscall.\n");
		Fork_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;		
		case SC_Exit:		
		DEBUG('a', "Exit syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;					    		        
		case SC_itoa:
		DEBUG('a', "itoa syscall.\n");
		itoa_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;		  		  
/*		case SC_rand:
		DEBUG('a', "rand syscall.\n");
		rv=rand();
		break;		 	             */
		case SC_Print:
		DEBUG('a', "Print syscall.\n");
		printf("    %d		\n",machine->ReadRegister(4));
		break;
/*		case SC_Cwrite:
		DEBUG('a', "Cwrite syscall.\n");
		Cwrite_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;		 	                                  */
		case SC_GetValue:
		DEBUG('a', "GetValue syscall.\n");
		fflush(stdin);
		rv=GetValue_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		break;
		case SC_SetValue:
		DEBUG('a', "SetValue syscall.\n");
		fflush(stdin);
		SetValue_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;
		case SC_IncrementbyVal:
		DEBUG('a', "IncrementbyVal syscall.\n");
		fflush(stdin);
		IncrementbyVal_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;
		case SC_DecrementbyVal:
		DEBUG('a', "DecrementbyVal syscall.\n");
		fflush(stdin);
		DecrementbyVal_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;
	
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } 
	
	else if ( which == PageFaultException ) 														
	{															
		DEBUG('a', "PageFaultException.\n");
		
		HandlePageFaultIPT(machine->ReadRegister(BadVAddrReg));			//Handle page fault in IPT

		return;
			
	}																									
	
	else
	{
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
