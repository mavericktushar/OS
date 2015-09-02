#include "syscall.h"

/*Visitor Function*/

int lock1;

void Visitor_rpc()
{  
	lock1=CreateLock("lock1");
	
	Acquire(lock1);
	
	SetValue("avlPhoneCnt",10,-1);
	
	GetValue("avlPhoneCnt",-1);
	
	GetValue("phoneStatus",10);
	SetValue("phoneStatus",1,10);
	GetValue("phoneStatus",10);
	
	GetValue("avlOperatorCnt",-1);
	IncrementbyVal("avlOperatorCnt",1,-1);
	GetValue("avlOperatorCnt",-1);
	
	Release(lock1);
/*
	idLock.Acquire(); //May be visitor id lock required for this
	int idLocal=visitorIDCnt++;			//Assign ID to The Visitor
	idLock.Release();
	
	stationLock.Acquire();		  		//Acquire Lock to Phone Station
	
	cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : "<<"Acquired Lock on Station . . . \n";
	
	if(presidentStatus==1 || waitingVisitorsCnt>0 || waitingSenatorsCnt>0 || ss==1 || pp==1)
	{
		waitingVisitorsCnt++;								//Thread  will go to wait  if President is there, 
		visitorWaitingCondition->Wait(&stationLock);		//If  there are other waiters who had arrived before 
		waitingVisitorsCnt--;
	}
	
	cout<<"\nAbout to check phone booths . . . \n";
	
	while(avlPhoneCnt==0)           
	{	  
		waitingVisitorsCnt++;		
		visitorWaitingCondition->Wait(&stationLock);		//Thread will go to wait 	if no phones are available	
		waitingVisitorsCnt--;
	}

    //Reach this point if Phone has been made available to the thread
	
	cout<<currentThread->getName()<<" "<<idLocal<<" : Phone made available . . . \n";
	
	int place=-999,i;
	
	for(i=0;i<20;i++)
	{	
		if(phoneStatus[i]==0)
		{
			place=i;
			break;
		}
	}
	
	if(place==-999)
	{
		cout<<"Error : Phone was made available, but was consumed by some other thread . . . ";
		cout<<"Aborting . . . ";		
		return ;
	}

	else
	{		
		avlPhoneCnt--;
		phoneLock[place]->Acquire();					//Acquire the phone and decrease the Phone Count
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired lock on phone : "<<place<<" \n";
		phoneStatus[place]=1;							//Set Phone's status to BUSY
		
		stationLock.Release();							//Release Station Lock
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releasing stationLock . . . \n";
		
		genOperatorLock.Acquire();						//Acquire Lock on All Operators, for checking if one is free
		
		while(avlOperatorCnt==0)
		{
			operatorCondition->Wait(&genOperatorLock);	//If none available go to Sleep
		}
		
		//Operator was made available and so this sleeping thread was signalled, now going to Acquire the Operator
		
		int place1=-999;
		for(i=0;i<opCnt;i++)
		{
			if(operatorStatus[i]==0)			
			{
				place1=i;
				break;
			}
		}
		
		if(place1==-999)
		{
			cout<<"Error : Operator made available, consumed by someone else . . . \n";
		}
		else
		{
			avlOperatorCnt--;
			operatorLock[place1]->Acquire();		//Acquires Lock on the Operator and decreases the available Operator Count
			cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Lock on operator : "<<place1<<"\n";
							
			operatorStatus[place1]=1;				//Set Operator Status as BUSY
			
			talkWithOperator[place1]=1;				//Sets flag for the Operator (Operator knows that a caller wants to talk with him/her and
			callerID[place1]=idLocal;				//Checks the value for its corresponding CallerID
			
			operatorLock[place1]->Release();

			operatorNeeded[place1]->Signal(operatorLock[place1]); //Signal the Operator 

			operatorLock[place1]->Acquire();
			
			genOperatorLock.Release();							  //Release the general Operator Lock
			
			operatorNeeded[place1]->Wait(operatorLock[place1]);   
			
			delay(500);		
			
			operatorStatus[place1]=0;			
			avlOperatorCnt++;
			
			operatorCondition->Signal(&genOperatorLock);		 //Talking with the Operator
								
			operatorLock[place1]->Release();					 //Releasing Operator Lock
			
			delay(1000);		
			
			avlPhoneCnt++;									     //Increase available Phone Count
			phoneStatus[place]=0;								 //Make phone available (FREE)	
			
			
			//Phone available, So signal the waiting threads according to the conditions 
			//If President available, Signal him/her,else Senators else Visitors
			
			if(presidentStatus==1)
			{				
				if(avlPhoneCnt==20 && avlOperatorCnt==opCnt)
				{
					presidentWaitingCondition->Signal(&stationLock);
				}
			}							
			else
			{
				if(waitingSenatorsCnt>0)
				{
					senatorWaitingCondition->Signal(&stationLock);
				}
				else
				{
					visitorWaitingCondition->Signal(&stationLock);
				}
			}
			
			phoneLock[place]->Release();						//Release Phone
			
			cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releasing Phone booth : "<<place<<"\n";
			
		}
		
		
		
	}
	
	*/
}

int main(int argc,char* argv[])
{
	Visitor_rpc();
	Exit(0);
}

