// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "string.h"
#include "iostream.h"

#define QUANTUM 100

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
void
StartProcess(char *filename)
{
//	cout<<"\nIn StartProcess Called by Main. Will Initialize Address Space for the First implicit Exec Process Now . . . \n";
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;
	//cout<<"In start process";
	printf("In start process . . . ");
    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
   
    space = new AddrSpace(executable);

    currentThread->space = space;

    //delete executable;			// close file
	
	//Acquiring Lock for Process Table access	
	processTableLock->Acquire();
	
	processTable[0].processID=proId;
	proId++;
	processTable[0].status=1;
	processTable[0].mainSpace=space;
//	processTable[0].parentThread=currentThread;				  		        //First Entry into the processTable 	
	processTable[0].createdThread=currentThread;	
	processTable[0].cntChildThreads=1;
	processTable[0].childEntry[0].thread=currentThread;

	
	//Releasing Lock after Process Table access
	processTableLock->Release();
	
	//Acquiring Lock for ProcessCnt access	
	processCntLock->Acquire();
	
	processCnt++;
//	execThreadCount++;
	
	//Releasing Lock after processCnt access
	processCntLock->Release();
	
	
//	cout<<"\nNumber of Processes :: "<<processCnt<<"\n";
//	cout<<"\n\nGoing to machine->Run() for the main implicit process . . . \n";

	space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

	
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}

