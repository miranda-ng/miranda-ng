#include "commonheaders.h"


//=====================================================
// Name : timerProc
// Parameters: none
// Returns : void
// Description : called when the timer interval occurs
//=====================================================
void __cdecl timerFunc(LPVOID di) 
{
	HANDLE hContact;
	char text[512], fn[16], szFileName[MAX_PATH], temp[MAX_PATH];
	int i, timer;
	int timerCount = DBGetContactSettingWord(NULL, modname, "timerCount",1)+1;

//	killTimer();
	if (LCStatus == ID_STATUS_OFFLINE) 
	{
		killTimer();
		return;
	}
	DBWriteContactSettingWord(NULL, modname, "timerCount", (WORD)timerCount);
	/* update the web pages*/
	for (i=0; ;i++)
	{
		sprintf(fn, "fn%d", i);
		if (!DBGetContactSettingString(NULL, modname, fn, text))
			break;
		if (!strncmp("http://", text, strlen("http://")))
		{
			strcat(fn, "_timer");
			timer = DBGetContactSettingWord(NULL,modname, fn, 60);
			if (timer && !(timerCount % timer))
			{
				if (!InternetDownloadFile(text))
				{
					wsprintf(szFileName,"%s\\plugins\\fn%d.html",getMimDir(temp), i);
					savehtml(szFileName);
				}
			}
		}
	}
	/* update all the contacts */
	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hContact) 
	{
		if(!lstrcmp(modname,(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0)) ) 
		{
			timer = DBGetContactSettingWord(hContact, modname, "Timer", 15);
			if (timer && !(timerCount % timer))
			{
				if (DBGetContactSettingString(hContact, modname, "Name", text))
					replaceAllStrings(hContact);
			}
		}
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}
//	startTimer(TIMER);
}


void CALLBACK timerProc()
{
		
// new thread for the timer...
	forkthread(timerFunc, 0, 0); 
//	timerFunc(0);
	
}

//=====================================================
// Name : startTimer
// Parameters: int interval
// Returns : int
// Description : starts the timer
//=====================================================
int startTimer(int interval)
{
	timerId = SetTimer(NULL, 0, interval, timerProc);
	return 0;
}

//=====================================================
// Name : killTimer
// Parameters: none
// Returns : int
// Description : stops the timer
//=====================================================
int killTimer()
{
	DBWriteContactSettingWord(NULL, modname, "timerCount",0);
	KillTimer(NULL,timerId);
	return 0;
}