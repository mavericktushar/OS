#include "syscall.h"

/*Operator Function*/

void Operator_rpc()
{
/*
	idLock.Acquire();
	int id=operatorIDCnt;								//Assign ID to Operator
	operatorIDCnt++;
	idLock.Release();
				
				
	operatorLock[id]->Acquire();
	operatorNeeded[id]->Wait(operatorLock[id]);			//Operator Waiting for a Request
	operatorLock[id]->Release();
    
	
	talkWithOperator[id]=1;								//Flag Variable checking if Operator is needed
	
	int i=0;
	while(true)
	{
	 
		if(talkWithOperator[id]==0)
		{						
			operatorLock[id]->Acquire();
			operatorNeeded[id]->Wait(operatorLock[id]); //Operator Waiting for Requests
			operatorLock[id]->Release();
			talkWithOperator[id]=1;
		 	
	        }
		else
		{	
			//Operator Checking the Caller IDs to check who called, and accordingly taking money or Allow to proceed, on recognizing
			//the caller to be a Senator or a President
			if(callerID[id]>=0 && callerID[id]<vsCnt)
			{
				cout<<"\nHello Visitor "<<callerID[id]<<"! Please pay $1 . . . \n";
				cout<<"Money Paid . . . Call can be made\n";
			}
			if(callerID[id]>=vsCnt && callerID[id]<(vsCnt+seCnt))
			{
				cout<<"\nHello Senator with Senator ID : "<<callerID[id]<<"! Call can be made now . . . \n";
			}
			if(callerID[id]==(vsCnt+seCnt))
			{
				cout<<"\nHello Mr. President . Call can be made now . . . \n";
			}
			
			talkWithOperator[id]=0;						//sets the flag to 0 indicating Operator FREE
			
			operatorNeeded[id]->Signal(operatorLock[id]);  
		}
		i++;
	          
		}
*/
}

int main(int argc,char* argv[])
{
	Operator_rpc();
	Exit(0);
}

