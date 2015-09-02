void Exit_Syscall(int status)
{
	int i,j,exitflag=0,exitflag1=0;
	int indexProc=999,indexChild=999;
	int spaceDelFlag=0;
	int localFlag=0;
	int spaceDelFlag1=0;
	
	inExit++;
	
	cout<<"\nIn Exit System Call for "<<inExit<<" time . . .\n";
	
	processTableLock->Acquire();
	processCntLock->Acquire();
	
	for(i=0;i<ProcessTableSize;i++)
	{
		if(processTable[i].createdThread==currentThread)		  				  //To check if the thread was made by an Exec
		{
			exitflag=1;
			break;
		}
	}
	
	if(exitflag)
	{
		indexProc=i;
			  //Thread created by Exec	
	}
	else
	{				  		          //check if thread was created by a fork syscall
		for(i=0;i<ProcessTableSize;i++)
		{
			for(j=0;j<50;j++)
			{
				if(processTable[i].childEntry[j].thread==currentThread)
				{
					exitflag1=1;
					break;
				}
			}
			if(exitflag1)
			{
			//	flag2=1;
				break;
			}
		}
		
		if(exitflag1)
		{
			indexProc=i;
			indexChild=j;
		}		
		else
		{
			cout<<"\n1 . Thread not matching any process or thread (Neither Exec thread, nor Fork Thread ! Error . . . \n";
			cout<<currentThread->getName()<<" : indexProc :: "<<indexProc;
			cout<<currentThread->getName()<<" : indexChild :: "<<indexChild;
			
		}
	}
	
	//Till here we found if it was thread created by Exec or by fork (For Exec (fork) thread we get indexProc) (For Fork (fork) we get indexProc and indexChild)
	
	if(exitflag)
	{
		if(processCnt==1)
		{
			if(processTable[indexProc].cntChildThreads>1)
			{
				processTable[indexProc].mainSpace=NULL;
				processTable[indexProc].parentThread=NULL;
				processTable[indexProc].createdThread=NULL;		  		          
				processTable[indexProc].cntChildThreads--;
				
				processTable[indexProc].childEntry[0].thread=NULL;
				
	//			delete currentThread->space;		  		        //Temporary fix  (space)
			
	//			cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
	
				processCnt--;
			
				currentThread->Finish();
			}
			else
			{
				processTable[indexProc].mainSpace=NULL;
				processTable[indexProc].parentThread=NULL;
				processTable[indexProc].createdThread=NULL;
				processTable[indexProc].cntChildThreads--;
				
				for(i=0;i<50;i++)
				{					
					processTable[indexProc].childEntry[i].thread=NULL;					
				}
				
				delete currentThread->space;									    //		    16/10/2008 - Just to check
				
				
				cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
				cout<<"\nMachine about to Halt . . .\n";
				
				cout<<"\nexecThreadCount :: "<<execThreadCount;
				cout<<"\nforkThreadCount :: "<<forkThreadCount;
	
				processCnt--;
				
				interrupt->Halt();
			}
		}
		else
		{
	/*		if(processTable[indexProc].cntChildThreads>0)
			{
				currentThread->Finish();
			}
			else
			{		  		          */
			
//			int spaceDelFlag=0;
			
			if(processTable[indexProc].cntChildThreads)
			{
			
				spaceDelFlag=1;
			/*	for(i=0;i<50;i++)
				{					
					processTable[indexProc].childEntry[i].thread=NULL;					
				}		  		        */							
			}
			
			processTable[indexProc].mainSpace=NULL;
			processTable[indexProc].parentThread=NULL;
			processTable[indexProc].createdThread=NULL;
			processTable[indexProc].cntChildThreads--;
			
			processTable[indexProc].childEntry[0].thread=NULL;
			
			
	//		cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
	
		    if(spaceDelFlag)
			{
				delete currentThread->space;				  	//	      16/10/2008 - Just to check
			}
			
			processCnt--;
				
			currentThread->Finish();
			
		}
		
		
	}
	else
	{
		int toStackReg=0;
		if(exitflag1)
		{	
		//	Thread* exitThread=processTable[indexProc].childEntry[indexChild].thread;
			
			//Stack Clearing (Clearing currentThread->space->StackMap)
			
//			int localFlag=0;
			
			if(processCnt==0 && processTable[indexProc].cntChildThreads==1)
			{
				localFlag=1;
			}
			
			if(currentThread->toStackReg==-1)
			{
				cout<<"\nThread not created by a fork syscall . . . . At wrong place ! Error . . . \n";
			}
			else
			{
				toStackReg=currentThread->toStackReg;
			}
			
			int readStackReg=currentThread->toStackReg;/*machine->ReadRegister(StackReg);*/
			readStackReg=readStackReg+16;
			
			int pageStackReg=divRoundUp(readStackReg, PageSize);
			pageStackReg=pageStackReg-currentThread->space->numPages;
			
		//	cout<<"\nStack starting position in stackMap :: "<<pageStackReg<<"\n";
			
			int l=0;
			
			currentThread->space->stackMapLock->Acquire();
			for(l=pageStackReg;l>pageStackReg-8;l--)
			{
				currentThread->space->stackMap->Clear(l);
			}
			currentThread->space->stackMapLock->Release();
			
			
			
			//Updating Process Table
			
//			int spaceDelFlag1=0;
			
			if(processTable[indexProc].cntChildThreads)
			{		
				spaceDelFlag1=1;
				
			}
			
			processTable[indexProc].cntChildThreads--;
			processTable[indexProc].childEntry[indexChild].thread=NULL;	
			
			if(spaceDelFlag1)
			{
				delete currentThread->space;		  		//       16/10/2008 - Just to check
			}
			
			if(localFlag)
			{
				cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
				cout<<"\nMachine about to Halt . . .\n";
				
				cout<<"\nexecThreadCount :: "<<execThreadCount;
				cout<<"\nforkThreadCount :: "<<forkThreadCount;
				
				interrupt->Halt();
			}
			else
			{			
	//			cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
				currentThread->Finish();
			}
			
		/*	if(processTable[indexProc].cntChildThreads==1)
			{
				if(processCnt==1)
				{
					processTable[indexProc].mainSpace=NULL;
					processTable[indexProc].parentThread=NULL;
					processTable[indexProc].createdThread=NULL;
					processTable[indexProc].cntChildThreads=0;
				
				
					for(i=0;i<50;i++)
					{					
						processTable[indexProc].childEntry[i].thread=NULL;					
					}
								
				
				//	allocStackAddr=allocStackAddr+8;
			
					int l;
					int stackRegVal_i=currentThread->toStackReg;
			
					int stackRegVal=divRoundUp(stackRegVal_i,PageSize);
			
					stackRegVal=stackRegVal-currentThread->space->numPages-1;
			
					for(l=stackRegVal;l>stackRegVal-8;l--)
					{
						currentThread->space->stackMap->Clear(l);
					}			  		       
			
					delete currentThread->space;		  		  //Doubtful.......................................
				
					interrupt->Halt();
					}
					else
					{				
						processTable[indexProc].mainSpace=NULL;
						processTable[indexProc].parentThread=NULL;
						processTable[indexProc].createdThread=NULL;
						processTable[indexProc].cntChildThreads=0;
					
					
						for(i=0;i<50;i++)
						{					
							processTable[indexProc].childEntry[i].thread=NULL;					
						}
									
					
					//	allocStackAddr=allocStackAddr+8;
				
					int l;
					int stackRegVal_i=currentThread->toStackReg;
				
					int stackRegVal=divRoundUp(stackRegVal_i,PageSize);
				
					stackRegVal=stackRegVal-currentThread->space->numPages-1;
				
			        for(l=stackRegVal;l>stackRegVal-8;l--)
					{
						currentThread->space->stackMap->Clear(l);
					}			  		
				
					delete currentThread->space;		  		  //Doubtful.......................................
					
					currentThread->Finish();
				}
			}		  		          */
		}
		else
		{
			cout<<"\n2 . Thread not matching any process or thread (Neither Exec thread, nor Fork Thread ! Error . . . \n";
		
		}
	}
	
	processCntLock->Release();
	processTableLock->Release();
	
	
}