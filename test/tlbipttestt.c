#include "syscall.h"


int main()
{	

	Exec("../test/exectest",16);
	Exec("../test/forktest",16);
	Exec("../test/matmult",15);
	Exec("../test/matmult",15);

  Exit(0);
  
  return 0;
}
