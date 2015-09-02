// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "machine.h"
#include "bitmap.h"
#include "synch.h"

#define LockTableSize 100
#define ConditionTableSize 100
#define ProcessTableSize 100

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock


typedef int SpaceId;

struct LockPtrID		   		    //structure for Kernel Lock Table
{
	Lock *lock;
	int id;
	int status;
	AddrSpace* creatingThreadSpace;
};

extern LockPtrID lockTable[LockTableSize];  //Lock table


struct ConditionStruct		  		   //structure for kernel Condition variable table
{
	Condition* cd;
	int id;
	int status;
	AddrSpace* creatingThreadSpace;
};

extern ConditionStruct conditionTable[ConditionTableSize];  //Condition Table

extern BitMap* memMap;		  		         //BitMap for keeping track of main memory

/*extern int numPagesIncStack;		  		    //Number of pages including pages for 50 stacks
extern int allocStackAddr;		  	       					      */
extern Lock* memLock;		  		  //Lock for memory operation(Find, Clear etc.)

extern Lock* memReleaseLock;

extern Lock* lockTableLock;
extern Lock* conditionTableLock;
extern Lock* lockAcquireLock;
extern Lock* lockReleaseLock;
extern Lock* lockSignalLock;
extern Lock* lockWaitLock;
extern Lock* lockBroadcastLock;
extern Lock* forkLock;

//extern Lock* exec_thread_lock;
//extern Lock* kernel_thread_lock;

extern Lock* writeLock;
extern Lock* readLock;

/*extern Lock* lockTableLock_d;
extern Lock* conditionTableLock_d;		  		      */

struct childStruct			  		        //structure for child thread in process table
{	
	int childID;	
//	int parentStatus;
	Thread* thread;	
};

class processTableClass		   		      //structure for  process table
{
	public:

	processTableClass();
	~processTableClass();
	
	int processID;		  		        //processID
	int status;				  	        
//	int type;		  		            //process thread or fork thread 0-process 1-thread
	AddrSpace* mainSpace;	
	Thread* createdThread;
	int cntChildThreads;
	int toStackReg;
	childStruct childEntry[50];
};

extern int processTableIndex;		  		  //contains the max size process table reached till now (some entries may be empty)

extern int processCnt;
extern Lock* processCntLock;

extern processTableClass processTable[ProcessTableSize];
extern Lock* processTableLock;

extern int currentTLBSlot;

extern int proId;

// IPT class is defined
class IPT_TranslationEntry												//u
 {

	public:
 
	int virtualPage;  													
	int physicalPage;  																				
                        																	
	bool valid;           																		
                        																		
	bool readOnly;																						
                        																		
	bool use;             																	
                        																		
	bool dirty;           																	
  
	int processId;
	int timeStamp;																						
	typee pageType;		//CODE, INITD, UNINITD, STACK (code, data, mixed, etc.)
	place	PL;			// MEM, DISK, SWAP (main memory, swapfile, executable, etc.) 
 
 };																				//u

//extern int processID;
 
 extern IPT_TranslationEntry* IPT[NumPhysPages];
 extern OpenFile *swapFile;
 extern List *IPTQueue;
 extern BitMap *swapMap;

class IntFifo
{
	public:
	
	int phypage;
};

extern int prPolicy;

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
extern int mboxnum;
extern int visitornum;
extern int senatornum;
extern int operatornum;
#endif

#endif // SYSTEM_H
