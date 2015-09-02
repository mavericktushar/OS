#include "syscall.h"

int main()
{	
  Exec("../test/add",11);
  Exec("../test/sub",11);		  		  
  Exec("../test/multiply",16);
  Exec("../test/div",11);		

  Exit(0);
  
  return 0;
}
