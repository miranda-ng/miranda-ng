#include "commonheaders.h"

UINT_PTR timerId;

//=====================================================
// Name : timerProc
// Parameters: none
// Returns : void
// Description : called when the timer interval occurs
//=====================================================

void timerFunc(void *di) 
{
	char text[512], fn[16], szFileName[MAX_PATH], temp[MAX_PATH];

	int timerCount = db_get_w(NULL, MODNAME, "timerCount",1)+1;

	if (LCStatus == ID_STATUS_OFFLINE) {
		killTimer();
		return;
	}
	db_set_w(NULL, MODNAME, "timerCount", (WORD)timerCount);

	/* update the web pages*/
	for (int i=0; ;i++) {
		mir_snprintf(fn, SIZEOF(fn), "fn%d", i);
		if (!db_get_static(NULL, MODNAME, fn, text))
			break;

		if (!strncmp("http://", text, strlen("http://")) || !strncmp("https://", text, strlen("https://"))) {
			mir_snprintf(fn, SIZEOF(fn), "fn%d_timer", i);
			int timer = db_get_w(NULL, MODNAME, fn, 60);
			if (timer && !(timerCount % timer)) {
				if (!InternetDownloadFile(text)) {
					mir_snprintf(szFileName, SIZEOF(szFileName), "%s\\plugins\\fn%d.html", getMimDir(temp), i);
					savehtml(szFileName);
				}
			}
		}
	}

	/* update all the contacts */
	for (MCONTACT hContact = db_find_first(MODNAME); hContact; hContact = db_find_next(hContact, MODNAME)) {
		int timer = db_get_w(hContact, MODNAME, "Timer", 15);
		if (timer && !(timerCount % timer))
			if (db_get_static(hContact, MODNAME, "Name", text))
				replaceAllStrings(hContact);
	}
}

void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	// new thread for the timer...
	forkthread(timerFunc, 0, 0); 
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
	db_set_w(NULL, MODNAME, "timerCount",0);
	KillTimer(NULL,timerId);
	return 0;
}
