#include "stdafx.h"

static UINT_PTR timerId = 0;

//=====================================================
// Name : timerProc
// Parameters: none
// Returns : void
// Description : called when the timer interval occurs
//=====================================================
//
void timerFunc(void*)
{
	char text[512], fn[16], szFileName[MAX_PATH], temp[MAX_PATH];

	int timerCount = g_plugin.getWord("timerCount", 1) + 1;

	if (LCStatus == ID_STATUS_OFFLINE) {
		killTimer();
		return;
	}
	g_plugin.setWord("timerCount", (uint16_t)timerCount);

	/* update the web pages*/
	for (int i = 0;; i++) {
		mir_snprintf(fn, "fn%d", i);
		if (db_get_static(NULL, MODNAME, fn, text, _countof(text)))
			break;

		if (!strncmp("http://", text, mir_strlen("http://")) || !strncmp("https://", text, mir_strlen("https://"))) {
			mir_snprintf(fn, "fn%d_timer", i);
			int timer = g_plugin.getWord(fn, 60);
			if (timer && !(timerCount % timer)) {
				if (!InternetDownloadFile(text)) {
					mir_snprintf(szFileName, "%s\\plugins\\fn%d.html", getMimDir(temp), i);
					savehtml(szFileName);
				}
			}
		}
	}

	/* update all the contacts */
	for (auto &hContact : Contacts(MODNAME)) {
		int timer = g_plugin.getWord(hContact, "Timer", 15);
		if (timer && !(timerCount % timer))
			if (!db_get_static(hContact, MODNAME, "Name", text, _countof(text)))
				replaceAllStrings(hContact);
	}
}

void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	// new thread for the timer...
	mir_forkthread(timerFunc);
}

//=====================================================
// Name : startTimer
// Parameters: int interval
// Returns : int
// Description : starts the timer
//=====================================================
//
int startTimer(int interval)
{
	timerId = SetTimer(nullptr, 0, interval, timerProc);
	return 0;
}

//=====================================================
// Name : killTimer
// Parameters: none
// Returns : int
// Description : stops the timer
//=====================================================
//
int killTimer()
{
	if (timerId != 0) {
		g_plugin.setWord("timerCount", 0);
		KillTimer(nullptr, timerId);
		timerId = 0;
	}
	return 0;
}
