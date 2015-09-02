/*******************************************************************
	Test to Demonstrate Acquiring Lock - Run with rpc_locktestrel
	
	1) Create lock1
	2) Acquire lock1     
	3) Acquire lock1
	
	4) Release lock1

	5) Trying to Acquire Invalid Lock
	
*******************************************************************/

#include "syscall.h"

int lock1;

int main()
{
  lock1 = CreateLock("lock1");
  
  Acquire(lock1);
  Acquire(lock1);
  
  Release(lock1);
  
  Acquire(10);
  
  Exit(0);
}

