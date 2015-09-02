#include "syscall.h"


int Sum()
{
	int a=10,b=20,c;
	c=a+b;
	
	Write("\nIn Sum function by user level fork . .  \n",41,1);
	
	Exit(0);
	
	return c;
	
}

int Sub()
{
	int a=10,b=20,c;
	c=b-a;
	
	Write("\nIn Subtract function by user level fork . .  \n",46,1);
	
	Exit(0);
	
	return c;
	
}


int main()
{

/*  Exec("../test/halt",12);
  Exec("../test/Yield",13);		*/		  		    
    		   
  Write("Code\n",5,1);
  
  Fork(Sub,0);
  Fork(Sum,0);
 

 
  Exec("../test/halt",12);
  Exec("../test/Yield",13);		
  
  Exec("../test/useritoa",16);
  
  Exit(0);
  
  return 0;
}
