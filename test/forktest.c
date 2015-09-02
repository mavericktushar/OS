#include "syscall.h"

int a=10,b=20;
int writeLock;

/*char buf1[27]="Addition of a=10, b=20 :: \n";
char buf2[35]="Subtraction of a=10,from  b=20 :: \n";
char buf3[35]="Multiplication of a=10, b=20  ::  \n";
char buf4[27]="Division of a=10, b=20 :: \n";		  		          */

void add(int arg)
{
  int c=a+b;

  Acquire(writeLock);
  
/*  buf1="\n\nAddition of a=10, b=20 :: ";		  		          */
/*  Write(buf1,27,1);		  	        */

  Write("Addition of a=10, b=20 :: \n",27,1);
	
  Print(c);
  
  Release(writeLock);
  
  Exit(0);

}

void sub(int arg)
{
  int c=b-a;

  Acquire(writeLock);
  
/*  buf2="\n\nSubtraction of a=10,from  b=20 :: ";			 */
/*  Write(buf2,35,1);		  		          */

  Write("Subtraction of a=10,from  b=20 :: \n",35,1);
	
  Print(c);
  
  Release(writeLock);
  
  Exit(0);

}

void multiply(int arg)
{
  int c=a*b;

  Acquire(writeLock);
  
/*  buf3="\n\nMultiplication of a=10, b=20 ::  ";				    */
/*  Write(buf3,35,1);		  		      */

  Write("Multiplication of a=10, b=20  ::  \n",35,1);
  Print(c);
  
  Release(writeLock);
  
  Exit(0);

}


void div(int arg)
{
  int c=b/a;

  Acquire(writeLock);
  
/*  buf4="\n\nDivision of a=10, b=20 :: ";		  		         */
/*  Write(buf4,27,1);			  		       */

  Write("Division of a=10, b=20 :: \n",27,1);

  Print(c);
  
  Release(writeLock);
  
  Exit(0);

}


int main()
{
  writeLock=CreateLock("abc");

/*  Acquire(writeLock);		  		          */
  Fork(add,0);
/*  Release(writeLock);		  		         */
  
/*  Acquire(writeLock);			     	        */
  Fork(sub,0);
/*  Release(writeLock);		    	       */
  
/*  Acquire(writeLock);		  		       */
  Fork(multiply,0);
/*  Release(writeLock);			                */
  
/*  Acquire(writeLock);				    */
  Fork(div,0);
/*  Release(writeLock);		*/	
  
  Exit(0);

  return 0;

}




