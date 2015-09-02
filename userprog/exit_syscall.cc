void Remove_Stack()
{
	int toStackReg,readStackReg,pageStackReg;
	int i=-999;
	toStackReg=currentThread->toStackReg;		  		        //This  and the next line do the same thing
	readStackReg=currentThread->StackReg;				        //This and the previous line do the same thing
	readStackReg=readStackReg+16;
	pageStackReg=divRoundUp(readStackReg, PageSize);
	pageStackReg=pageStackReg - currentThread->space->numPages;
	
	currentThread->space->stackMapLock->Acquire();	
	for(i=pageStackReg;i>(pageStackReg-8);i--)
	{
		currentThread->space->stackMap->Clear(i);
	}
	currentThread->space->stackMapLock->Release();
}

void Exit_Syscall(int status)
{
	int indexProc=999,indexChild=999;
	int procFoundFlag=0,forkFoundFlag=0,stackRemovalFlag=0;
	int i,j,k;
	
	processTableLock->Acquire();				  	 		      //Acquire processTableLock before accessing processTable
	
	processCnt--;
	
	for(i=0;i<ProcessTableSize;i++)
	{
		if(processTable[i].createdThread==currentThread)
		{			
			procFoundFlag=1;
			indexProc=i;
			break;
		}		
	}
	
	if(procFoundFlag==0)
	{
		for(i=0;i<ProcessTableSize;i++)
		{
			for(j=1;j<50;j++)
			{
				if(processTable[i].childEntry[j].thread==currentThread)
				{
					forkFoundFlag=1;
					stackRemovalFlag=1;
					indexProc=i;
					indexChild=j;
					break;					
				}
			}
		}
	}
	
	if(indexProc==999 && indexChild==999)
	{
		cout<<"\nNo Entry found in page table. Wrong Request for Exit . Error . . . \n"<<endl;
		processTableLock->Release();
		return;
	}
	
	//Last Process and Last Thread
	if(processCnt==0 && processTable[indexProc].cntChildThreads==1)
	{
		processTable[indexProc].status=0;
		processTable[indexProc].mainSpace=NULL;
		processTable[indexProc].createdThread=NULL;
		processTable[indexProc].cntChildThreads=0;
		
		for(k=0;k<50;k++)
		{
			processTable[indexProc].childEntry[k].thread=NULL;
		}
		
		if(stackRemovalFlag==1)
		{
			Remove_Stack();
		}
		
		delete currentThread->space;
		
		processTableLock->Release();
		Interrupt->Halt();
	}
	
	//If Last Thread
	if(processTable[indexProc].cntChildThreads==1)
	{
		processTable[indexProc].status=0;
		processTable[indexProc].mainSpace=NULL;
		processTable[indexProc].createdThread=NULL;
		processTable[indexProc].cntChildThreads=0;
		
		for(k=0;k<50;k++)
		{
			processTable[indexProc].childEntry[k].thread=NULL;
		}
		
		if(stackRemovalFlag==1)
		{
			Remove_Stack();
		}
		
		delete currentThread->space;
		
		processTableLock->Release();
		currentThread->Finish();
	}
	
	//If other  threads are there for the process
	processTable[indexProc].cntChildThreads--;
	processTable[indexProc].childEntry[indexChild].thread=NULL;
	
	if(stackRemovalFlag==1)
	{
		Remove_Stack();
	}
	
	processTableLock->Release();
	currentThread->Finish();
	
	return;
}

/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/

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
			//	processTable[indexProc].parentThread=NULL;
				processTable[indexProc].createdThread=NULL;		  		          
				processTable[indexProc].cntChildThreads--;
				
				processTable[indexProc].childEntry[0].thread=NULL;
				
	//			delete currentThread->space;		  		        //Temporary fix  (space)
			
	//			cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
	
				processCnt--;
				
				cout<<"\nPexit execThreadCount :: "<<execThreadCount;
				cout<<"\nPexit forkThreadCount :: "<<forkThreadCount;
				
				processCntLock->Release();
				processTableLock->Release();
				
				currentThread->Finish();
			}
			else
			{
				processTable[indexProc].mainSpace=NULL;
			//	processTable[indexProc].parentThread=NULL;
				processTable[indexProc].createdThread=NULL;
				processTable[indexProc].cntChildThreads--;
				
				for(i=0;i<50;i++)
				{					
					processTable[indexProc].childEntry[i].thread=NULL;					
				}
				
//				delete currentThread->space;									    //		    16/10/2008 - Just to check
				
				
				cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
				cout<<"\nMachine about to Halt . . .\n";
				
				cout<<"\nPexit execThreadCount :: "<<execThreadCount;
				cout<<"\nPexit forkThreadCount :: "<<forkThreadCount;
	
				processCnt--;
				
				
				processCntLock->Release();
				processTableLock->Release();
				
				cout<<"processTable["<<indexProc<<"] thread deleting address space . . . \n";
				cout<<"\nprocessTable["<<indexProc<<"].cntChildThreads :: "<<processTable[indexProc].cntChildThreads;
					
				delete currentThread->space;									    //		    16/10/2008 - Just to check
				
				
				currentThread->Finish();
			//	interrupt->Halt();		  	                                                                                                                               //                       17/10/2008 - Just to check
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
			
			if(processTable[indexProc].cntChildThreads==1)
			{
			
				spaceDelFlag=1;
			/*	for(i=0;i<50;i++)
				{					
					processTable[indexProc].childEntry[i].thread=NULL;					
				}		  		        */							
			}
			
			processTable[indexProc].mainSpace=NULL;
//			processTable[indexProc].parentThread=NULL;
			processTable[indexProc].createdThread=NULL;
			processTable[indexProc].cntChildThreads--;
			
			processTable[indexProc].childEntry[0].thread=NULL;
			
			
	//		cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
	
/*		    if(spaceDelFlag)
			{
				delete currentThread->space;				  	//	      16/10/2008 - Just to check
			}		  		          */
			
			processCnt--;
			
			cout<<"\nPexit execThreadCount :: "<<execThreadCount;
			cout<<"\nPexit forkThreadCount :: "<<forkThreadCount;
				
			processCntLock->Release();
			processTableLock->Release();	
			
			if(spaceDelFlag)
			{
				cout<<"processTable["<<indexProc<<"] thread deleting address space . . . \n";
				cout<<"\nprocessTable["<<indexProc<<"].cntChildThreads :: "<<processTable[indexProc].cntChildThreads;
				delete currentThread->space;				  	//	      16/10/2008 - Just to check
			}
				
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
			cout<<"\ncurrentThread->space->stackMap :: "<<currentThread->space->stackMap;
			for(l=pageStackReg;l>(pageStackReg-8);l--)
			{
				currentThread->space->stackMap->Clear(l);
			}
			currentThread->space->stackMapLock->Release();
			
			
			
			//Updating Process Table
			
//			int spaceDelFlag1=0;
			
			if(processTable[indexProc].cntChildThreads==1)
			{		
				spaceDelFlag1=1;
				
			}
			
			processTable[indexProc].cntChildThreads--;
			processTable[indexProc].childEntry[indexChild].thread=NULL;	
			
/*			if(spaceDelFlag1)
			{
				delete currentThread->space;		  		//       16/10/2008 - Just to check
			}		  		        */
			
			if(localFlag)
			{
				cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
				cout<<"\nMachine about to Halt . . .\n";
				
				cout<<"\nexecThreadCount :: "<<execThreadCount;
				cout<<"\nforkThreadCount :: "<<forkThreadCount;
				
				processCntLock->Release();
				processTableLock->Release();
				
				if(spaceDelFlag1)
				{
					delete currentThread->space;		  		//       16/10/2008 - Just to check
					cout<<"processTable["<<indexProc<<"].childEntry["<<indexChild<<"] thread deleting address space . . . \n";
					cout<<"\nprocessTable["<<indexProc<<"].cntChildThreads :: "<<processTable[indexProc].cntChildThreads;
				}
				
				currentThread->Finish();
				
			//	interrupt->Halt();    		  		                       //17/10/2008 - Just to check
			}
			else
			{			
	//			cout<<"\n\n"<<currentThread->getName()<<" : Thread Exiting . . .\n";
		
				cout<<"\nFexit execThreadCount :: "<<execThreadCount;
				cout<<"\nFPexit forkThreadCount :: "<<forkThreadCount;
				
				processCntLock->Release();
				processTableLock->Release();
				
				if(spaceDelFlag1)
				{
					cout<<"processTable["<<indexProc<<"].childEntry["<<indexChild<<"] thread deleting address space . . . \n";
					cout<<"\nprocessTable["<<indexProc<<"].cntChildThreads :: "<<processTable[indexProc].cntChildThreads;
					delete currentThread->space;		  		//       16/10/2008 - Just to check
				}
				
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
			
			processCntLock->Release();
			processTableLock->Release();
			
			return;
		
		}
	}
	
/*	processCntLock->Release();
	processTableLock->Release();					 	  	       
	printf("released processtable lock in exit\n");				  	   */
		
	return;
	
}

/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/



																	2nd Try
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
void Exit_Syscall(int status)
{
	int index=999,threadFoundFlag=0,stackRemovalFlag=0,procFoundFlag=0,indexSpace=0;
	
	int i,j,k;
	
	inExit++;
	
	cout<<"\nIn Exit System Call for "<<inExit<<" time . . .\n"<<endl;
	
	processTableLock->Acquire();				  	 		      //Acquire processTableLock before accessing processTable
	
//	processCnt--;
	
	for(i=0;i<ProcessTableSize;i++)
	{
		if(processTable[i].createdThread==currentThread)
		{			
			if(processTable[i].type==1)
			{
				forkThreadCount--;
				stackRemovalFlag=1;
			}
			else
			{
				execThreadCount--;
				processCnt--;
				procFoundFlag=1;
			}
			index=i;
			break;
		}		
	}
	
	
		
	
	if(index==999)
	{
		cout<<"\n"<<currentThread->getName()<<" :: No Entry found in process table. Wrong Request for Exit . Error . . . \n"<<endl;
		processTableLock->Release();
		return;
	}
	
	for(i=0;i<ProcessTableSize;i++)
	{
		if(processTable[i].mainSpace==currentThread->space)
		{
			indexSpace=i;
			break;
		}
	}
	
	if(i>=ProcessTableSize)
	{
		cout<<"\nNo Parent thread found in Process Table. Error . . . \n"<<endl;
		processTableLock->Release();		
		return;
	}
	
	//Last Process and Last Thread
	if(processCnt==0 && forkThreadCount==0/*processTable[indexSpace].cntChildThreads==1 && processTable[index].cntChildThreads==1*/)
	{
		processTable[index].status=0;
		processTable[index].mainSpace=NULL;
		processTable[index].createdThread=NULL;
		processTable[index].cntChildThreads=0;
		processTable[index].type=-1;
		
		
		
		if(stackRemovalFlag==1)
		{
			RemoveForkStack();
		}
				
		
		
		cout<<"\n"<<currentThread->getName()<<" :: Machine about to Halt . . . interrupt->Halt() is being called . . . \n"<<endl;
		
		processTableLock->Release();
	//	delete[] processTable;
		delete currentThread->space;
		interrupt->Halt();
	}
	
	//If Last Thread
	if(processTable[indexSpace].cntChildThreads==1 && processTable[index].cntChildThreads==1)
	{
		processTable[index].status=0;
		processTable[index].mainSpace=NULL;
		processTable[index].createdThread=NULL;
		
		processTable[indexSpace].cntChildThreads=0;
		processTable[index].type=-1;
		
		
		if(stackRemovalFlag==1)
		{
			RemoveForkStack();
		}
		
		delete currentThread->space;
		
		
		
		processTableLock->Release();
		currentThread->Finish();
	}
	
	//If other  threads are there for the process
	
	
	
	processTable[indexSpace].cntChildThreads--;
	processTable[index].cntChildThreads--;
	
	if(stackRemovalFlag==1)
	{
		RemoveForkStack();
	}
	
	
	
	processTableLock->Release();
	currentThread->Finish();
	
	return;
}

/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/
/****************************************************************************************Old Exit_syscall code*********************************************************************************************************/