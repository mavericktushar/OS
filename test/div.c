#include "syscall.h"

int a=10,b=20;

int main()
{
  int c=b/a;
  Write("\n\nDivision of a=10, b=20 (b/a) :: ",35,1);
  Print(c);
  
  Exit(0);

}
