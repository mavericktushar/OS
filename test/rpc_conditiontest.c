/*******************************************************************
	Test to Demonstrate Creating and Destroying Condition Variables
	
	1) Create condition - cdt1
	2) DestroyCondition cdt1
	
	3) Create condition - cdt1
	4) DestroyCondition cdt1
	
	5) Trying to Destroy invalid Condition
	
*******************************************************************/

#include "syscall.h"

int cdt1,cdt2;

int main()
{
  cdt1=CreateCondition("Cdt1");
  DestroyCondition(cdt1);
  
  cdt2=CreateCondition("Cdt2");
  DestroyCondition(cdt2);
  
  DestroyCondition(10);
  
  Exit(0);
}
