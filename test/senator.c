#include "syscall.h"

/*Senator Function*/

void Senator_rpc()
{
/*
	ss=1;
	
	idLock.Acquire();	
	int idLocal=senatorIDCnt;						 	//Assigning ID to Senator
	senatorIDCnt++;	
	idLock.Release();
	
	int cnt=0;
	while(true)
	{
		ss=1;
		
		cnt++;
		if(cnt==4)
		{
			break;
		}
	
		stationLock.Acquire();							//Acquire Lock on station
	
		
		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Lock on Station \n";
	
		if(presidentStatus==1 || waitingSenatorsCnt>0 || pp==1)
		{
			waitingSenatorsCnt++;			
			senatorWaitingCondition->Wait(&stationLock); //Senator waits if President is there or other Senators are waiting			
			waitingSenatorsCnt--;		
		}
	
		while(avlPhoneCnt==0)
		{
			waitingSenatorsCnt++;			
			senatorWaitingCondition->Wait(&stationLock); //Waits if pnone not available			
			waitingSenatorsCnt--;
		}
		
		ss=0;

		cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Phone made available . . . \n";
	
		//Phone made available for the thread, Goes to Acquire it
	
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
			cout<<"\nError : Phone made available, was consumed by someone else . . . ";			//Exception ! 
			cout<<"\nAborting . . . ";		
			return ;
		}
		else
		{		
			avlPhoneCnt--;
			phoneStatus[i]=1;					//Acquires  Lock on Phone,Decreases available phone count, sets phone status BUSY
			phoneLock[place]->Acquire();				

			cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Acquired Lock on Phone : "<<place;
		
			stationLock.Release();				//Releases station Lock
		
			genOperatorLock.Acquire();			//Acquires general Operator Lock
			
       		while(avlOperatorCnt==0)
			{
				operatorCondition->Wait(&genOperatorLock); //Wait till an operator becomes available
			}
		
			int place1=-999;
			
			for(i=0;i<opCnt;i++)
			{
				if(operatorStatus[i]==0)
				{
					place1=i;
					break;
				}
			}
		
			if(place==-999)
			{
				cout<<"Error : Phone made available, was consumed by someone else . . . ";			//Exception ! 
				cout<<"Aborting . . . Press any key to abort";
			//			getch();
				return ;
			}
			else
			{
				operatorLock[place1]->Acquire();		//Acquires Lock an available Operator
				
				genOperatorLock.Release();				//Releases the general Operator Lock
				
				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<": Acquired Lock on Operator : "<<place1;
				avlOperatorCnt--;
				operatorStatus[place1]=1;				//Decreases available Operator Count , makes the operator BUSY
									
				talkWithOperator[place1]=1;				//Sets the Flag for Operator to know that a thread wants to talk
				callerID[place1]=idLocal;				//Gives the Caller ID
			
				operatorLock[place1]->Release();							
			
				operatorNeeded[place1]->Signal(operatorLock[place1]);						
			
				operatorLock[place1]->Acquire();
			
				operatorNeeded[place1]->Wait(operatorLock[place1]);							
			
				delay(500);			
				
				operatorStatus[place1]=0;				//Sets the Operator Status to FREE
				avlOperatorCnt++;						//Increase available Operator Count
				
				operatorCondition->Signal(&genOperatorLock);
			
				
				operatorLock[place1]->Release();

				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Operator : "<<place1<<"Released\n";
				
				delay(1000);			
				
				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Done talking. Senator Exiting . . . \n";
				
				avlPhoneCnt++;							//Increase available phone count, sets phone status to FREE
				phoneStatus[place]=0;
				
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
				
				phoneLock[place]->Release();
				
				cout<<"\n"<<currentThread->getName()<<" "<<idLocal<<" : Releases Lock on phone : "<<place<<"\n";
			
			}
		}
		
		int r;
				
		r=(rand()%3)+1;
		
		
		if(cnt!=3)
		{
			cout<<"\nSenator Going. will come after "<<r<<" seconds . . .\n";
		}
		
		r=r*1000;
		delay(r);
		
	}
	*/
}

void main(int argc,char* argv[])
{
	Senator_rpc();
	Exit(0);
}

