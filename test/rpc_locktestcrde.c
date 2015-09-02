/*******************************************************************
	Test to Demonstrate Creating and Destroying Lock
	
	1) Create lock1
	2) Create lock2
	3) Create lock3
	4) Create lock4
	
	5)Destroy lock1
	6)Destroy lock2
	7)Destroy lock3
	8)Destroy lock4
	
	9) Trying to Destroy Invalid Lock
	
*******************************************************************/

#include "syscall.h"

int lock1,lock2,lock3,lock4;

int main()
{
	lock1=CreateLock("lock1");		  	        /*Creating 4 Locks*/
	lock2=CreateLock("lock2");
	lock3=CreateLock("lock3");
	lock4=CreateLock("lock4");
	
	DestroyLock(lock1);						   /*Destroying 4 Locks*/
	DestroyLock(lock2);
	DestroyLock(lock3);
	DestroyLock(lock4);
	
	DestroyLock(99);							/*Destroying Lock that is not created*/
	
	Exit(0);
}