/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "dbtool.h"

void ProcessingDone(void);
int WorkInitialChecks(int firstTime);
int WorkModuleChain(int firstTime);
int WorkUser(int firstTime);
int WorkContactChain(int firstTime);
int WorkAggressive(int firstTime);
int WorkFinalTasks(int firstTime);

extern HANDLE hEventRun,hEventAbort;
extern int errorCount;
time_t ts;
DBHeader dbhdr;
DWORD spaceProcessed,sourceFileSize;
DWORD spaceUsed, sp;

static int (*Workers[6])(int)=
	{WorkInitialChecks,WorkModuleChain,WorkUser,WorkContactChain,WorkAggressive,WorkFinalTasks};

void __cdecl WorkerThread(void *unused)
{
	int task,ret,firstTime;
	ts=time(NULL);

	AddToStatus(STATUS_MESSAGE,TranslateT("Database worker thread activated"));
	SetFilePointer(opts.hFile,0,NULL,FILE_BEGIN);
	spaceUsed=1; spaceProcessed=0; sp=0;
	firstTime=0;

	for(task=0;;) {
		if (spaceProcessed/(spaceUsed/1000+1) > sp) {
			sp = spaceProcessed/(spaceUsed/1000+1);
			SetProgressBar(sp);
		}
		WaitForSingleObject(hEventRun,INFINITE);
		if(WaitForSingleObject(hEventAbort,0)==WAIT_OBJECT_0) {
			AddToStatus(STATUS_FATAL,TranslateT("Processing aborted by user"));
			break;
		}
		ret=Workers[task](firstTime);
		firstTime=0;
		if(ret==ERROR_NO_MORE_ITEMS) {
			if(++task==sizeof(Workers)/sizeof(Workers[0])) {
				AddToStatus(STATUS_MESSAGE,TranslateT("Elapsed time: %d sec"), time(NULL)-ts);
				if(errorCount) 
					AddToStatus(STATUS_SUCCESS,TranslateT("All tasks completed but with errors (%d)"),errorCount);
				else 
					AddToStatus(STATUS_SUCCESS,TranslateT("All tasks completed successfully"));
				break;
			}
			firstTime=1;
		}
		else if(ret!=ERROR_SUCCESS)
			break;
	}
	ProcessingDone();
}
