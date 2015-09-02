/*******************************************************************
	Test to Demonstrate Creating and Destroying Conditions
	
	Also Creating an already created Condition
	Destroying a destroyed Condition
	
	1) Create Condition cdt1
	2) Create Condition cdt1
	3) Create Condition cdt2
	4) Create Condition cdt2
	
	5)Destroy Condition cdt1
	6)Destroy Condition cdt2
	7)Destroy Condition cdt1
	8)Destroy Condition cdt2
	
*******************************************************************/
#include "syscall.h"

int cdt1,cdt2,cdt3,cdt4;

int main()
{
  cdt1=CreateCondition("Cdt1");
  cdt2=CreateCondition("Cdt1"); 
  cdt3=CreateCondition("Cdt2");
  cdt4=CreateCondition("Cdt2");
  
  DestroyCondition(cdt1);
  DestroyCondition(cdt2);
  
  DestroyCondition(cdt1);
  DestroyCondition(cdt2);
  
  Exit(0);
}
