#include "syscall.h"

char buf1[10][6];

int main()
{	
	int i;
    for(i=0;i<2;i++)
  {
	
	
    /*itoa(buf1[i],6,i);
      Write(buf1[i],6,1);*/
	
	Write("th test starts executing\n",25,1);
	Exec("../test/newtest",15);
	/*itoa(buf1[i],6,i);
	  Write(buf1[i],6,1);*/
	Write("th test finishs executing\n",27,1);
  }

  Exit(0);
  
  return 0;
}
