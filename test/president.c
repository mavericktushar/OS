#include "syscall.h"

/*President Function*/

void President_rpc()
{
/*
	pp=1;
	
	idLock.Acquire();
	int idLocal=30;
	idLock.Release();
	
	int cnt=0;
	
	while(true)
	{
		pp=1;
		cnt++;												//President allowed to come 5 times
		if(cnt==6)
		{		  
		  break;
		}
		
		stationLock.Acquire();							   //Acquires stationLock
		
		
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Station Lock . . . \n"; 
		
		presidentStatus=1;								   //Sets status of President to True, for others to note 
				
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releasing Station Lock . . . \n"; 
		
		if(avlPhoneCnt!=20)
		{
			presidentWaitingCondition->Wait(&stationLock);	
		}
		
		stationLock.Release();							  //Releasing station Lock				

		phoneLock[0]->Acquire();						  //Acquire lock on phone 
		

		cout<<"\nPresident Acquired Phone . . . \n";
		cout<<"\nNumber of available phones : "<<avlPhoneCnt<<"\n";
		
		genOperatorLock.Acquire();		
		
		operatorLock[0]->Acquire();						 //Acquires Lock on an Operator

		cout<<"\nPresident talking with Operator . . . ";
		
		talkWithOperator[0]=1;					//Set flag to inform Operator that President wants to talk

		delay(500);               
	
		
		genOperatorLock.Release();
		operatorLock[0]->Release();
		
		int a=0;
		while(a<4)
		{
			a=rand()%6;
		}
		
		a=a*1000;
		delay(a);       
	
		phoneLock[0]->Release();

		cout<<"\nPresident Releases Phone . . . ";
		
		presidentStatus=0;			//Sets president Status as to False
		pp=0;
		
		
		senatorWaitingCondition->Broadcast(&stationLock);		
		visitorWaitingCondition->Broadcast(&stationLock);
						    	
		int r=0;
		while(r<5)
		{
		
		  r=rand()%8;
		}
		
		if(cnt!=5)
		{
			cout<<"\nPresident Going. will come after "<<r<<" seconds . . .\n";
		}
		
		r=r*1000;

		currentThread->Yield();
		
		delay(r);                    				

	}
		
*/	
}

int main(int argc,char* argv[])
{
	President_rpc();
	Exit(0);
}

