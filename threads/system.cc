// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "iostream.h"

#define LockTableSize 100
#define ConditionTableSize 100
#define ProcessTableSize 100

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *timer;				// the hardware timer device,
					// for invoking context switches
					
LockPtrID lockTable[LockTableSize];		  		       //lockTable for locks
ConditionStruct conditionTable[ConditionTableSize];		//Condition table for condition variables	

Lock* lockTableLock;
Lock* conditionTableLock;
Lock* lockAcquireLock;	
Lock* lockReleaseLock;
Lock* lockSignalLock;
Lock* lockWaitLock;
Lock* lockBroadcastLock;
Lock* forkLock;

//Lock* exec_thread_lock;
//Lock* kernel_thread_lock;

BitMap* memMap;						  		         //Bitmap
Lock* memLock;		  		  //Lock for memory operations


Lock* memReleaseLock;		  		         //doubtful, Don't know for sure



Lock* writeLock;							  //doubtful, Don't know for sure
Lock* readLock;		  		          //doubtful, Don't know for sure

int processTableIndex;		  		  //contains the max size process table reached till now (some entries may be empty)

int processCnt;
Lock* processCntLock;

processTableClass processTable[ProcessTableSize];
Lock* processTableLock;

int currentTLBSlot;
int proId;

IPT_TranslationEntry* IPT[NumPhysPages];
OpenFile *swapFile;

List *IPTQueue;
BitMap *swapMap;

int prPolicy;

//int processID;


/*int numPagesIncStack;		  //number of pages plus pages for 50 stacks
int allocStackAddr;					          */

					
#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers

processTableClass::processTableClass()
{
	int i,j;
	
	for(i=0;i<ProcessTableSize;i++)
	{
		processTable[i].processID=i;
		processTable[i].status=0;
//		processTable[i].type=-1;
		processTable[i].mainSpace=NULL;	
		processTable[i].createdThread=NULL;
		processTable[i].cntChildThreads=0;		
		processTable[i].toStackReg=-1;
		for(j=0;j<50;j++)
		{
			processTable[i].childEntry[j].childID=j;
			processTable[i].childEntry[j].thread=NULL;
		}
	}
}

processTableClass::~processTableClass()
{	
	
}

#endif

#ifdef NETWORK
PostOffice *postOffice;
/*
IPT_TranslationEntry::IPT_TranslationEntry()
{
	for(int j=0;j<NumPhysPages;j++)															// initializing IPT Bits.
	{
		IPT_Table[j]->valid = false;
		IPT_Table[j]->readOnly = false;
		IPT_Table[j]->use = false;
		IPT_Table[j]->dirty = false;
		IPT_Table[j]->timestamp = 0;
	}	
}

IPT_TranslationEntry::~IPT_TranslationEntry()
{
}
*/
int mboxnum=0;			//Mailbox Number
int visitornum;
int senatornum;
int operatornum;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();


//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = "";
    bool randomYield = FALSE;
	
	memMap=NULL;
	memLock=NULL;
	memReleaseLock=NULL;
	writeLock=NULL;
	readLock=NULL;
	currentTLBSlot=0;
	proId=0;
	int cnt_var=0;
	for(cnt_var=0;cnt_var<NumPhysPages;cnt_var++)
	{
		IPT[cnt_var]=new IPT_TranslationEntry();
	}
	
//    cout<<"system.cc in Initialize";
#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program
#endif
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	}
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    argCount = 2;
	}
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);

    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
	
	
	int i;
	for(i=0;i<LockTableSize;i++)
	{
		lockTable[i].lock=NULL;
		lockTable[i].creatingThreadSpace=NULL;
		lockTable[i].id=i;
		lockTable[i].status=0;
	}
	
	for(i=0;i<ConditionTableSize;i++)
	{
		conditionTable[i].cd=NULL;
		conditionTable[i].creatingThreadSpace=NULL;
		conditionTable[i].id=i;
		conditionTable[i].status=0;
	}
	
/*	for(i=0;i<ProcessTableSize;i++)
	{
		processTable[i]=new processTableClass();
	}		  		        */
	
	processCnt=0;
	
	
    memMap=new BitMap(NumPhysPages);
	memLock=new Lock();
	memReleaseLock=new Lock();
	lockTableLock=new Lock();
	conditionTableLock=new Lock();
	processTableIndex=0;		  		  //contains the max size process table reached till now (some entries may be empty)
	
//	processID=0;
	
	writeLock=new Lock();
	readLock=new Lock();
	
	processCntLock=new Lock();
	processTableLock=new Lock();
	
	lockAcquireLock=new Lock();
	lockReleaseLock=new Lock();
	lockSignalLock=new Lock();
	lockWaitLock=new Lock();
	lockBroadcastLock=new Lock();
	forkLock=new Lock();
	
//	exec_thread_lock=new Lock();
//	kernel_thread_lock=new Lock();
	
fileSystem->Create("Swapfile",500000);											// creating a swap file
swapFile = fileSystem->Open("Swapfile");
delete swapFile;
swapFile = fileSystem->Open("Swapfile");
swapMap=new BitMap(3900);

IPTQueue=new List();
prPolicy=0;
	
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
/*	int cnt_var=0;
	for(cnt_var=0;cnt_var<NumPhysPages;cnt_var++)
	{
		IPT[cnt_var]=new IPT_TranslationEntry();
	}  */
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    
    Exit(0);
}

