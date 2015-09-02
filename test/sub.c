#include "syscall.h"

int a=10,b=20;

int main()
{
  int c=b-a;
  Write("\n\nSubtraction of a=10 from b=20 :: ",35,1);
  Print(c);
  
  Exit(0);

}
