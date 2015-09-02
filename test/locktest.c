#include "syscall.h"

int lock;

int main()
{
  lock = CreateLock();

  Acquire(lock);

  DestroyLock(20);

  Release(lock);

  DestroyLock(lock);
}
