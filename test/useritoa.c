
#include "syscall.h"

int myints[20];
/*int yourints[20];

int myexp ( int count ) {
  int i, val=1;
  for (i=0; i<count; i++ ) {
    val = val * 10;
  }
  return val;
}

void itoa( char arr[], int size, int val ) {
  int i, max, dig, subval, loc;
  for (i=0; i<size; i++ ) {
    arr[i] = '\0';
  }

  for ( i=1; i<=2; i++ ) {
    if (( val / myexp(i) ) == 0 ) {
      max = i-1;
      break;
    }
  }

  subval = 0;
  loc = 0;
  for ( i=max; i>=0; i-- ) {
    dig = 48 + ((val-subval) / myexp(i));
    subval = (dig-48) * myexp(max);
    arr[loc] = dig;
    loc++;
  }

  return;
}		  		    */

int main() {
  int i;
  char buf[6];

  for (i=0; i<20; i++ ) {
    myints[i] = i;
  }

  for (i=19; i>0; i-- ) {
    itoa( buf, 6, myints[i] );
    Write( "i is ", 5, 1 );
    Write( buf, 6, 1 );
    Write( "\n", 1, 1 );
  }
  
  Exit(0);

}
