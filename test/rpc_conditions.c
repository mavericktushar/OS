/*******************************************************************
	Test to Demonstrate Signal - Run with rpc_conditionw
	
	1) Create lock1
	2) Create condition - cdit1
	
	3) Signal cdit1,lock1
	4) Signal cdit1,lock1
	5) Signal cdit1,lock1
	6) Signal cdit1,lock1
	
	7) Signal cdit1,lock1
	8) Signal 10,lock1
	
*******************************************************************/
#include "syscall.h"

int cdit1,cdit2,cdit3,cdit4;
int lock1;

int main()
{
	lock1=CreateLock("lock1");
	cdit1 = CreateCondition("Cdit1");
  
	Signal(cdit1,lock1);
	Signal(cdit1,lock1);
	Signal(cdit1,lock1);
	Signal(cdit1,lock1);
  
	Signal(cdit1,lock1);
	Signal(10,lock1);		  	        
		
	Exit(0);
}
