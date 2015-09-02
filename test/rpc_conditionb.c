/*******************************************************************
	Test to Demonstrate Broadcast - Run with rpc_conditionw
	
	1) Create lock1
	2) Create condition - cdit1
	
	3) Broadcast cdit1,lock1  
	
*******************************************************************/
#include "syscall.h"

int cdit1;
int lock1;

int main()
{
	lock1=CreateLock("lock1");
	cdit1 = CreateCondition("Cdit1");
	
	Broadcast(cdit1,lock1);		  	            
  
	Exit(0);
}