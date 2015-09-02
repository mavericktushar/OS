#include "syscall.h"

int a=10,b=20;

int main()
{
  int c=a*b;
  Write("\n\nMultiplication of a=10, b=20 :: ",35,1);
  Print(c);
  
  Exit(0);

}
