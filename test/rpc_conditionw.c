/*******************************************************************
	Test to Demonstrate Wait - Run with rpc_conditions or rpc_conditionb
	
	1) Create lock1
	2) Create condition - cdit1
	
	3) Acquire lock1
	4) Wait  cdit1,lock1
	5) Release lock1     
	
*******************************************************************/
#include "syscall.h"

int cdit1;
int lock1;

int main()
{	
	lock1=CreateLock("lock1");
	cdit1 = CreateCondition("Cdit1");

	Acquire(lock1);
	Wait(cdit1,lock1);
	Release(lock1);
	
  Exit(0);
}
