/*******************************************************************
	Test to Demonstrate Aqcuiring an Releasing Lock
	
	1) Create lock1
	2) Create lock2
	
	3) Acquire lock1
	4) Release lock1     
	
 	5) Acquire lock2
	6) Release lock2
	
	7) Destroy lock1
	8) Destroy lock2
	
	9) Trying to Acquire Invalid lock
	10) Trying to Release Invalid Lock
	11) Trying to Destroy Invalid Lock
	
*******************************************************************/
	

#include "syscall.h"

int lock1,lock2;

int main()
{
	lock1=CreateLock("lock1");
	lock2=CreateLock("lock2");
	
	Acquire(lock1);	    
	
	Release(lock1);
	
	Acquire(lock2);

	Release(lock2);
	
	DestroyLock(lock1);
	DestroyLock(lock2);
	
	Acquire(10);
	Release(15);
	DestroyLock(10);
	Exit(0);
}