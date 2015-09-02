// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

#include "list.h"		//Own_Code [Includes the list.h file]
#include "thread.h"
#include "iostream.h"
//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName)
{
  name=debugName;
  isLock=0;
  ownerThread=NULL;
  queue=new List();
}

Lock::Lock()
{
	isLock=0;
	ownerThread=NULL;
	queue=new List();
}

Lock::~Lock()
{
	ownerThread=NULL;
}



void Lock::Acquire() 
{
	IntStatus oldIntStatus = interrupt->SetLevel(IntOff); //Own_Code

	if(isLock == 0)						//Own_Code
	{
		isLock=1;						//Own_Code
		ownerThread=currentThread;			//Own_Code
	}
	else
	{
		queue->Append(currentThread);			//Own_Code
		currentThread->Sleep();				//Own_Code
	}

	(void)interrupt->SetLevel(oldIntStatus);			//Own_Code
}


void Lock::Release() 
{
	IntStatus oldIntStatus = interrupt->SetLevel(IntOff);	//Own_Code

	if(ownerThread==currentThread)                  //Own_Code 
	{
		if(queue->IsEmpty())                             //Own_Code
		{
			isLock=0;                           //Own_Code  
			ownerThread=NULL;					//Own_Code 				 		                                           
		
		}
		else							//Own_Code
		{
		  char *name1="debug";
			Thread* newThread;			//Own_Code
			newThread=new Thread(name1);
			newThread=(Thread*)queue->Remove();	//Own_Code
			scheduler->ReadyToRun(newThread);	//Own_Code
			ownerThread=newThread;			//Own_Code
		}
	}
	else								//Own_Code
	{	
		cout<<"Thread trying to put another thread to sleep. ";//Own_Code
		interrupt->SetLevel(oldIntStatus);		//Own_Code		
	}

	(void)interrupt->SetLevel(oldIntStatus);			//Own_Code
	
	
}

Condition::Condition(char* debugName)
{
        name=debugName;
        cLock=NULL;
	flag=0;
	queue=new List();
}

Condition::Condition()
{
	cLock=NULL;
	flag=0;
	queue=new List();
}

Condition::~Condition() { }

void Condition::Wait(Lock* conditionLock)
{
	IntStatus oldIntStatus=interrupt->SetLevel(IntOff);
	
	if(!flag)
	{
		cLock=conditionLock;
		flag=1;
	}
	
	
	if(conditionLock==cLock)
	{
		conditionLock->Release();
		queue->Append(currentThread);
		currentThread->Sleep();
		conditionLock->Acquire();
	}
	
	
	(void)interrupt->SetLevel(oldIntStatus);
}
void Condition::Signal(Lock* conditionLock)
{
	IntStatus oldIntStatus=interrupt->SetLevel(IntOff);
	
	if(queue->IsEmpty())
	{	
		(void)interrupt->SetLevel(oldIntStatus);
		return ;
	}
	else
	{
		if(cLock==conditionLock)
		{
		  char *name1="debug";
			Thread* newThread;
			newThread=new Thread(name1);
			newThread=(Thread*)queue->Remove();
			scheduler->ReadyToRun(newThread);
			
			if(queue->IsEmpty())
			{
				cLock=NULL;
				flag=0;
			}
		}
		else
		{
			cout<<"Trying to signal a thread waiting on a different Lock . . . ";
			return ;
		}
	}
	
	(void)interrupt->SetLevel(oldIntStatus);
	
	
}

void Condition::Broadcast(Lock* conditionLock) 
{	
	while(!queue->IsEmpty())
	{
		Signal(conditionLock);
	}
}
