#include "syscall.h"

int main()
{	
	Exec("../test/add",11);
	Exec("../test/sub",11);		  		  
	Exec("../test/multiply",16);
	Exec("../test/div",11);
	
	Exec("../test/matmult",15);
	
	Exec("../test/sort",12);
	
  Exit(0);
  
  return 0;
}