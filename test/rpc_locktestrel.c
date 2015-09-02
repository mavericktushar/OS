/*******************************************************************
	Test to Demonstrate Releasing Lock - Run with rpc_locktest
	
	1) Create lock1
	2) Release lock1     

	3) Trying to Release Invalid Lock
	
*******************************************************************/

#include "syscall.h"

int lock1;

int main()
{
  lock1 = CreateLock("lock1");		  	           /*Get the Id of a Lock and then Release it*/
  
  Release(lock1);
  
  Release(10);
  Exit(0);
}