// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "iostream.h"



#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//	Simple test cases for the threads assignment.
//


#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}

//----------------------------------------------------------------------------------------------------------------------------------------------//
//Declaration  of monitor variables, condition variables, Locks and other variables
//----------------------------------------------------------------------------------------------------------------------------------------------//

Lock stationLock;				      //Lock on the Station
Lock* phoneLock[20];				  //Array of Locks for 20 phone booths
Lock genOperatorLock;				  //Lock on All operators
Lock* operatorLock[3];				  //Lock on each Operator
//Lock freeLock;
Lock idLock;						  //Lock  to Acquire id

int opCnt;							  //Operator Count
int vsCnt;							  //Visitor Count
int seCnt;			                  //Senator Count
int avlPhoneCnt=20;					  //Monitor Variable to keep Count of available phones
int avlOperatorCnt;					  //Monitor Variable to keep track of available Operators	
bool presidentStatus=0;				  //Monitor Variable to Monitor President's Status
int waitingVisitorsCnt=0;			  //Monitor Variable to keep count of waiting Visitors
int waitingSenatorsCnt=0;			  //Monitor Variable to keep count of waiting Senators
bool phoneStatus[20];				  //Monitor Variable to check status of phones (BUSY or FREE)
bool *operatorStatus;				  //Monitor Variable to check status of each Operator (BUSY or FREE)	
int *talkWithOperator;				  //Variable to make Caller talk to an Operator
int *callerID;						  //Variable to hold present caller's ID of callers calling various Operators
int visitorIDCnt=0;					  //Variable used to assign IDs to Visitors
int senatorIDCnt;					  //Variable used to assign IDs to Senators
int operatorIDCnt=0;				  //Variable used to assign IDs to Operators

int pp=0;							  //Flag variable
int ss=0;							  //Flag variable

//Condition Variables  for various conditions 

Condition* visitorWaitingCondition;	  

Condition* senatorWaitingCondition;

Condition* presidentWaitingCondition;

Condition* operatorCondition;

Condition* operatorNeeded[3];


void delay(int);

//Delay Function

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

//Visitor Function

void Visitor()
{  
	idLock.Acquire();
	int idLocal=visitorIDCnt++;			//Assign ID to The Visitor
	idLock.Release();
	
	stationLock.Acquire();		  		//Acquire Lock to Phone Station
	
	cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : "<<"Acquired Lock on Station . . . \n";
	
	if(presidentStatus==1 || waitingVisitorsCnt>0 || waitingSenatorsCnt>0 || ss==1 || pp==1)
	{
		waitingVisitorsCnt++;								//Thread  will go to wait  if President is there, 
		visitorWaitingCondition->Wait(&stationLock);		//If  there are other waiters who had arrived before 
		waitingVisitorsCnt--;
	}
	
	cout<<"\nAbout to check phone booths . . . \n";
	
	while(avlPhoneCnt==0)           
	{	  
		waitingVisitorsCnt++;		
		visitorWaitingCondition->Wait(&stationLock);		//Thread will go to wait 	if no phones are available	
		waitingVisitorsCnt--;
	}

    //Reach this point if Phone has been made available to the thread
	
	cout<<currentThread->getName()<<" "<<idLocal<<" : Phone made available . . . \n";
	
	int place=-999,i;
	
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
		cout<<"Error : Phone was made available, but was consumed by some other thread . . . ";
		cout<<"Aborting . . . ";		
		return ;
	}

	else
	{		
		avlPhoneCnt--;
		phoneLock[place]->Acquire();					//Acquire the phone and decrease the Phone Count
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired lock on phone : "<<place<<" \n";
		phoneStatus[place]=1;							//Set Phone's status to BUSY
		
		stationLock.Release();							//Release Station Lock
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releasing stationLock . . . \n";
		
		genOperatorLock.Acquire();						//Acquire Lock on All Operators, for checking if one is free
		
		while(avlOperatorCnt==0)
		{
			operatorCondition->Wait(&genOperatorLock);	//If none available go to Sleep
		}
		
		//Operator was made available and so this sleeping thread was signalled, now going to Acquire the Operator
		
		int place1=-999;
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
			cout<<"Error : Operator made available, consumed by someone else . . . \n";
		}
		else
		{
			avlOperatorCnt--;
			operatorLock[place1]->Acquire();		//Acquires Lock on the Operator and decreases the available Operator Count
			cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Lock on operator : "<<place1<<"\n";
							
			operatorStatus[place1]=1;				//Set Operator Status as BUSY
			
			talkWithOperator[place1]=1;				//Sets flag for the Operator (Operator knows that a caller wants to talk with him/her and
			callerID[place1]=idLocal;				//Checks the value for its corresponding CallerID
			
			operatorLock[place1]->Release();

			operatorNeeded[place1]->Signal(operatorLock[place1]); //Signal the Operator 

			operatorLock[place1]->Acquire();
			
			genOperatorLock.Release();							  //Release the general Operator Lock
			
			operatorNeeded[place1]->Wait(operatorLock[place1]);   
			
			delay(500);		
			
			operatorStatus[place1]=0;			
			avlOperatorCnt++;
			
			operatorCondition->Signal(&genOperatorLock);		 //Talking with the Operator
								
			operatorLock[place1]->Release();					 //Releasing Operator Lock
			
			delay(1000);		
			
			avlPhoneCnt++;									     //Increase available Phone Count
			phoneStatus[place]=0;								 //Make phone available (FREE)	
			
			
			//Phone available, So signal the waiting threads according to the conditions 
			//If President available, Signal him/her,else Senators else Visitors
			
			if(presidentStatus==1)
			{				
				if(avlPhoneCnt==20 && avlOperatorCnt==opCnt)
				{
					presidentWaitingCondition->Signal(&stationLock);
				}
			}							
			else
			{
				if(waitingSenatorsCnt>0)
				{
					senatorWaitingCondition->Signal(&stationLock);
				}
				else
				{
					visitorWaitingCondition->Signal(&stationLock);
				}
			}
			
			phoneLock[place]->Release();						//Release Phone
			
			cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releasing Phone booth : "<<place<<"\n";
			
		}
		
		
		
	}
	
	
}

//Operator Function

void Operator()
{
	idLock.Acquire();
	int id=operatorIDCnt;								//Assign ID to Operator
	operatorIDCnt++;
	idLock.Release();
				
				
	operatorLock[id]->Acquire();
	operatorNeeded[id]->Wait(operatorLock[id]);			//Operator Waiting for a Request
	operatorLock[id]->Release();
    
	
	talkWithOperator[id]=1;								//Flag Variable checking if Operator is needed
	
	int i=0;
	while(true)
	{
	 
		if(talkWithOperator[id]==0)
		{						
			operatorLock[id]->Acquire();
			operatorNeeded[id]->Wait(operatorLock[id]); //Operator Waiting for Requests
			operatorLock[id]->Release();
			talkWithOperator[id]=1;
		 	
	        }
		else
		{	
			//Operator Checking the Caller IDs to check who called, and accordingly taking money or Allow to proceed, on recognizing
			//the caller to be a Senator or a President
			if(callerID[id]>=0 && callerID[id]<vsCnt)
			{
				cout<<"\nHello Visitor "<<callerID[id]<<"! Please pay $1 . . . \n";
				cout<<"Money Paid . . . Call can be made\n";
			}
			if(callerID[id]>=vsCnt && callerID[id]<(vsCnt+seCnt))
			{
				cout<<"\nHello Senator with Senator ID : "<<callerID[id]<<"! Call can be made now . . . \n";
			}
			if(callerID[id]==(vsCnt+seCnt))
			{
				cout<<"\nHello Mr. President . Call can be made now . . . \n";
			}
			
			talkWithOperator[id]=0;						//sets the flag to 0 indicating Operator FREE
			
			operatorNeeded[id]->Signal(operatorLock[id]);  
		}
		i++;
	          
		}
}

//Senator Function

void Senator()
{
	ss=1;
	
	idLock.Acquire();	
	int idLocal=senatorIDCnt;						 	//Assigning ID to Senator
	senatorIDCnt++;	
	idLock.Release();
	
	int cnt=0;
	while(true)
	{
		ss=1;
		
		cnt++;
		if(cnt==4)
		{
			break;
		}
	
		stationLock.Acquire();							//Acquire Lock on station
	
		
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Lock on Station \n";
	
		if(presidentStatus==1 || waitingSenatorsCnt>0 || pp==1)
		{
			waitingSenatorsCnt++;			
			senatorWaitingCondition->Wait(&stationLock); //Senator waits if President is there or other Senators are waiting			
			waitingSenatorsCnt--;		
		}
	
		while(avlPhoneCnt==0)
		{
			waitingSenatorsCnt++;			
			senatorWaitingCondition->Wait(&stationLock); //Waits if pnone not available			
			waitingSenatorsCnt--;
		}
		
		ss=0;

		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Phone made available . . . \n";
	
		//Phone made available for the thread, Goes to Acquire it
	
		int place=-999,i;
	
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
			cout<<"\nError : Phone made available, was consumed by someone else . . . ";			//Exception ! 
			cout<<"\nAborting . . . ";		
			return ;
		}
		else
		{		
			avlPhoneCnt--;
			phoneStatus[i]=1;					//Acquires  Lock on Phone,Decreases available phone count, sets phone status BUSY
			phoneLock[place]->Acquire();				

			cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Lock on Phone : "<<place;
		
			stationLock.Release();				//Releases station Lock
		
			genOperatorLock.Acquire();			//Acquires general Operator Lock
			
       		while(avlOperatorCnt==0)
			{
				operatorCondition->Wait(&genOperatorLock); //Wait till an operator becomes available
			}
		
			int place1=-999;
			
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
				cout<<"Error : Phone made available, was consumed by someone else . . . ";			//Exception ! 
				cout<<"Aborting . . . Press any key to abort";
			//			getch();
				return ;
			}
			else
			{
				operatorLock[place1]->Acquire();		//Acquires Lock an available Operator
				
				genOperatorLock.Release();				//Releases the general Operator Lock
				
				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<": Acquired Lock on Operator : "<<place1;
				avlOperatorCnt--;
				operatorStatus[place1]=1;				//Decreases available Operator Count , makes the operator BUSY
									
				talkWithOperator[place1]=1;				//Sets the Flag for Operator to know that a thread wants to talk
				callerID[place1]=idLocal;				//Gives the Caller ID
			
				operatorLock[place1]->Release();							
			
				operatorNeeded[place1]->Signal(operatorLock[place1]);						
			
				operatorLock[place1]->Acquire();
			
				operatorNeeded[place1]->Wait(operatorLock[place1]);							
			
				delay(500);			
				
				operatorStatus[place1]=0;				//Sets the Operator Status to FREE
				avlOperatorCnt++;						//Increase available Operator Count
				
				operatorCondition->Signal(&genOperatorLock);
			
				
				operatorLock[place1]->Release();

				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Operator : "<<place1<<"Released\n";
				
				delay(1000);			
				
				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Done talking. Senator Exiting . . . \n";
				
				avlPhoneCnt++;							//Increase available phone count, sets phone status to FREE
				phoneStatus[place]=0;
				
				//Phone available, So signal the waiting threads according to the conditions 
				//If President available, Signal him/her,else Senators else Visitors
				
				if(presidentStatus==1)
				{
					if(avlPhoneCnt==20 && avlOperatorCnt==opCnt)
					{
						presidentWaitingCondition->Signal(&stationLock);	
					}				
					
					
				}
				else
				{
					if(waitingSenatorsCnt>0)
					{
						senatorWaitingCondition->Signal(&stationLock);
					}
					else
					{
						visitorWaitingCondition->Signal(&stationLock);
					}
				}
				
				phoneLock[place]->Release();
				
				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releases Lock on phone : "<<place<<"\n";
			
			}
		}
		
		int r;
				
		r=(rand()%3)+1;
		
		
		if(cnt!=3)
		{
			cout<<"\nSenator Going. will come after "<<r<<" seconds . . .\n";
		}
		
		r=r*1000;
		delay(r);
		
	}
}

//President Function

void President()
{
	pp=1;
	
	idLock.Acquire();
	int idLocal=30;
	idLock.Release();
	
	int cnt=0;
	
	while(true)
	{
		pp=1;
		cnt++;												//President allowed to come 5 times
		if(cnt==6)
		{		  
		  break;
		}
		
		stationLock.Acquire();							   //Acquires stationLock
		
		
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Station Lock . . . \n"; 
		
		presidentStatus=1;								   //Sets status of President to True, for others to note 
				
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releasing Station Lock . . . \n"; 
		
		if(avlPhoneCnt!=20)
		{
			presidentWaitingCondition->Wait(&stationLock);	
		}
		
		stationLock.Release();							  //Releasing station Lock				

		phoneLock[0]->Acquire();						  //Acquire lock on phone 
		

		cout<<"\nPresident Acquired Phone . . . \n";
		cout<<"\nNumber of available phones : "<<avlPhoneCnt<<"\n";
		
		genOperatorLock.Acquire();		
		
		operatorLock[0]->Acquire();						 //Acquires Lock on an Operator

		cout<<"\nPresident talking with Operator . . . ";
		
		talkWithOperator[0]=1;					//Set flag to inform Operator that President wants to talk

		delay(500);               
	
		
		genOperatorLock.Release();
		operatorLock[0]->Release();
		
		int a=0;
		while(a<4)
		{
			a=rand()%6;
		}
		
		a=a*1000;
		delay(a);       
	
		phoneLock[0]->Release();

		cout<<"\nPresident Releases Phone . . . ";
		
		presidentStatus=0;			//Sets president Status as to False
		pp=0;
		
		
		senatorWaitingCondition->Broadcast(&stationLock);		
		visitorWaitingCondition->Broadcast(&stationLock);
						    	
		int r=0;
		while(r<5)
		{
		
		  r=rand()%8;
		}
		
		if(cnt!=5)
		{
			cout<<"\nPresident Going. will come after "<<r<<" seconds . . .\n";
		}
		
		r=r*1000;

		currentThread->Yield();
		
		delay(r);                    				

	}
		
	
}
 
// Main Calling Function -------------------------- 20 Visitors, 5 Senators, 1 President  (Test Case -  Static Values)

void SenateFunc()
{	
	opCnt=3;
	vsCnt=20;
	seCnt=5;
	operatorStatus=new bool[opCnt];
	talkWithOperator=new int[opCnt];
	callerID=new int[opCnt];
	avlPhoneCnt=20;					  //Monitor Variable to keep Count of available phones
	avlOperatorCnt=3;
    presidentStatus=0;				  //Monitor Variable to Monitor President's Status
	waitingVisitorsCnt=0;			  //Monitor Variable to keep count of waiting Visitors
	waitingSenatorsCnt=0;			  //Monitor Variable to keep count of waiting Senators
	senatorIDCnt=vsCnt;

	

	visitorWaitingCondition=new Condition("Visitor CV");
	senatorWaitingCondition=new Condition("Senator CV");
	presidentWaitingCondition=new Condition("President CV");
	
	operatorCondition=new Condition("Operator CV");

	
	int i;

	for(i=0;i<3;i++)
        {
	  	operatorNeeded[i]=new Condition("Operator Needed CV"+i);
        }
	
	for(i=0;i<20;i++)
	{
		phoneStatus[i]=0;
	}
	
	for(i=0;i<20;i++)
	{	
		phoneLock[i]=new Lock("Phone lock : "+i);
	}
	
	for(i=0;i<3;i++)
	{
		operatorStatus[i]=0;
	}
	
	for(i=0;i<3;i++)
	{
		operatorLock[i]=new Lock("Operator lock : "+i);
	}
	
	for(i=0;i<3;i++)
	{
		talkWithOperator[i]=0;
	}

	for(i=0;i<3;i++)
	{
	  callerID[i]=0;
	}
	
	cout<<"Forking Thread Operator 1\n";

	Thread* op_t1=new Thread("Operator 1");

	op_t1->Fork((VoidFunctionPtr)Operator,0);



	cout<<"Forking Thread Operator 2\n";
	Thread* op_t2=new Thread("Operator 2");
	op_t2->Fork((VoidFunctionPtr)Operator,0);



	
	cout<<"Forking Thread Operator 3\n";
	Thread* op_t3=new Thread("Operator 3");
	op_t3->Fork((VoidFunctionPtr)Operator,0);



	
	cout<<"Forking Thread Visitor 1\n";
	Thread* v_t1=new Thread("Visitor 1");				//Visitor Thread 1
	v_t1->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 2\n";
	Thread* v_t2=new Thread("Visitor 2");				//Visitor Thread 2
	v_t2->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 3\n";
	Thread* v_t3=new Thread("Visitor 3");				//Visitor Thread 3
	v_t3->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 4\n";
	Thread* v_t4=new Thread("Visitor 4");				//Visitor Thread 4
	v_t4->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 5\n";
	Thread* v_t5=new Thread("Visitor 5");				//Visitor Thread 5
	v_t5->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 6\n";
	Thread* v_t6=new Thread("Visitor 6");				//Visitor Thread 6
	v_t6->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 7\n";
	Thread* v_t7=new Thread("Visitor 7");				//Visitor Thread 7
	v_t7->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 8\n";
	Thread* v_t8=new Thread("Visitor 8");				//Visitor Thread 8
	v_t8->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 9\n";
	Thread* v_t9=new Thread("Visitor 9");				//Visitor Thread 9
	v_t9->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 10\n";
	Thread* v_t10=new Thread("Visitor 10");				//Visitor Thread 10
	v_t10->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 11\n";
	Thread* v_t11=new Thread("Visitor 11");				//Visitor Thread 11
	v_t11->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 12\n";
	Thread* v_t12=new Thread("Visitor 12");				//Visitor Thread 12
	v_t12->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"\nForking Thread Senator 1\n";
	Thread* s_t1=new Thread("Senator 1");				//Senator Thread 1
	s_t1->Fork((VoidFunctionPtr)Senator,0);
	
	cout<<"\nForking Thread Senator 2\n";
	Thread* s_t2=new Thread("Senator 2");				//Senator Thread 2
	s_t2->Fork((VoidFunctionPtr)Senator,0);
	
	cout<<"Forking Thread Visitor 13\n";
	Thread* v_t13=new Thread("Visitor 13");				//Visitor Thread 13
	v_t13->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 14\n";
	Thread* v_t14=new Thread("Visitor 14");				//Visitor Thread 14
	v_t14->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 15\n";
	Thread* v_t15=new Thread("Visitor 15");				//Visitor Thread 15
	v_t15->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 16\n";
	Thread* v_t16=new Thread("Visitor 16");				//Visitor Thread 16
	v_t16->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 17\n";
	Thread* v_t17=new Thread("Visitor 17");				//Visitor Thread 17
	v_t17->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 18\n";
	Thread* v_t18=new Thread("Visitor 18");				//Visitor Thread 18
	v_t18->Fork((VoidFunctionPtr)Visitor,0);
	
	cout<<"Forking Thread Visitor 19\n";
	Thread* v_t19=new Thread("Visitor 19");				//Visitor Thread 19
	v_t19->Fork((VoidFunctionPtr)Visitor,0);
	
	
	
	cout<<"\nForking Thread President\n";
	Thread* p_t=new Thread("President");           		//President
	p_t->Fork((VoidFunctionPtr)President,0);
	
	cout<<"\nForking Thread Senator 4\n";
	Thread* s_t4=new Thread("Senator 4");				//Senator Thread 4
	s_t4->Fork((VoidFunctionPtr)Senator,0);

        cout<<"Forking Thread Visitor 20\n";
	Thread* v_t20=new Thread("Visitor 20");				//Visitor Thread 20
	v_t20->Fork((VoidFunctionPtr)Visitor,0);
		
	cout<<"\nForking Thread Senator 3\n";
	Thread* s_t3=new Thread("Senator 3");				//Senator Thread 3
	s_t3->Fork((VoidFunctionPtr)Senator,0);
	
}

//Main Calling Function --------------------------  (Test Case -  Dynamic Values)

void SenateFuncDync()
{		
	visitorWaitingCondition=new Condition("Visitor CV");
	senatorWaitingCondition=new Condition("Senator CV");
	presidentWaitingCondition=new Condition("President CV");
	
	operatorCondition=new Condition("Operator CV");

	cout<<"\nEnter the number of Operators :: ";
	cin>>opCnt;
	
	cout<<"\nEnter the number of Visitors :: ";
	cin>>vsCnt;
	
	cout<<"Enter the Number of Senators :: ";
	cin>>seCnt;

	avlOperatorCnt=opCnt;
	visitorIDCnt=0;
	senatorIDCnt=vsCnt;
	operatorIDCnt=0;

		operatorStatus=new bool[opCnt];
	talkWithOperator=new int[opCnt];
	callerID=new int[opCnt];
	
	
	int i;

	for(i=0;i<opCnt;i++)
    {
	  	operatorNeeded[i]=new Condition("Operator Needed CV"+i);
    }
	
	for(i=0;i<20;i++)
	{
		phoneStatus[i]=0;
	}
	
	for(i=0;i<20;i++)
	{	
		phoneLock[i]=new Lock("Phone lock : "+i);
	}
	
	for(i=0;i<opCnt;i++)
	{
		operatorStatus[i]=0;
	}
	
	for(i=0;i<opCnt;i++)
	{
		operatorLock[i]=new Lock("Operator lock : "+i);
	}
	
	for(i=0;i<opCnt;i++)
	{
		talkWithOperator[i]=0;
	}

	for(i=0;i<opCnt;i++)
	{
	  callerID[i]=0;
	}

	Thread* op_t[opCnt];
	
	for(i=0;i<opCnt;i++)
	{
		cout<<"\nForking Thread Operator "<<(i+1)<<"\n";
		op_t[i]=new Thread("Operator "+(i+1));		
		op_t[i]->Fork((VoidFunctionPtr)Operator,0);
	}
	
	Thread* vs_t[vsCnt];
	
	for(i=0;i<vsCnt;i++)
	{
		cout<<"\nForking Thread Visitor "<<(i+1)<<"\n";
		vs_t[i]=new Thread("Visitor ");		
		vs_t[i]->Fork((VoidFunctionPtr)Visitor,0);
	}
	
	Thread* se_t[seCnt];
	
	for(i=0;i<seCnt;i++)
	{
		cout<<"\nForking Thread Senator "<<(i+1)<<"\n";
		se_t[i]=new Thread("Senator ");		
		se_t[i]->Fork((VoidFunctionPtr)Senator,0);
	}
	
	Thread* pd_t=new Thread("President");
	pd_t->Fork((VoidFunctionPtr)President,0);
	
	
	
}




#endif
