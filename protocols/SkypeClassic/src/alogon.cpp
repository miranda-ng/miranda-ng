#include "skype.h"
#include "debug.h"

extern char g_szProtoName[];
extern HANDLE SkypeReady;

/**
 * Purpose: Retrieves class name from window
 *
 * Note: This is some sort of hack to return static local variable,
 * but it works :)
 */
static const TCHAR* getClassName(HWND wnd)
{
    static TCHAR className[256];
	
	*className=0;
	GetClassName(wnd, &className[0], sizeof(className)/sizeof(className[0]));
	return className;
}

/**
 * Purpose: Finds a window
 *
 * Note: This function relies on Skype window placement.
 * It should work for Skype 3.x
 */
static HWND findWindow(HWND parent, const TCHAR* childClassName)
{
    // Get child window
    // This window is not combo box or edit box
	HWND wnd = GetWindow(parent, GW_CHILD);
	while(wnd != NULL && _tcscmp(getClassName(wnd), childClassName) != 0)
		wnd = GetWindow(wnd, GW_HWNDNEXT);
    
    return wnd;
}

static  BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwPID;
	const TCHAR *lpszClassName;


	GetWindowThreadProcessId(hWnd,&dwPID);
	if (lParam != 0 && dwPID != (DWORD)lParam) return TRUE;
	lpszClassName = getClassName(hWnd);
	if(_tcscmp(lpszClassName, _T("tSkMainForm.UnicodeClass")) == 0 ||
		_tcscmp(lpszClassName, _T("TLoginForm.UnicodeClass")) == 0)
	{
		HWND loginControl = GetWindow(hWnd, GW_CHILD);

		LOG(("setUserNamePasswordThread: Skype window found!"));

		// Sleep for some time, while Skype is loading
		// It loads slowly :(
		//Sleep(5000);
		LOG (("TLoginControl = %S", getClassName(loginControl)));

		// Check for login control
		if(_tcscmp(getClassName(loginControl), _T("TLoginControl")) == 0)
		{
			// Find user name window
			HWND userName = findWindow(loginControl, _T("TNavigableTntComboBox.UnicodeClass"));
			HWND password = findWindow(loginControl, _T("TNavigableTntEdit.UnicodeClass"));

			LOG (("userName=%08X; password=%08X", userName, password));
			if (userName && password)
			{
				// Set user name and password
				DBVARIANT dbv;

				if(!db_get_ws(NULL,SKYPE_PROTONAME,"LoginUserName",&dbv)) 
				{
					SendMessageW(userName, WM_SETTEXT, 0, (LPARAM)dbv.pwszVal);
					db_free(&dbv);    
				}

				if(!db_get_ws(NULL,SKYPE_PROTONAME,"LoginPassword",&dbv)) 
				{
					SendMessageW(password, WM_SETTEXT, 0, (LPARAM)dbv.pwszVal);
					db_free(&dbv);
					SendMessageW(password, WM_CHAR, 13, 0);
				}


				SendMessageW(hWnd,
							 WM_COMMAND,
							 0x4a8,  // sign-in button; WARNING: This ID can change during newer Skype versions
							 (LPARAM)findWindow(loginControl, _T("TTntButton.UnicodeClass")));
			}
			return FALSE;
		}

	}
	return TRUE;
}

DWORD WINAPI setUserNamePasswordThread(LPVOID lpDummy)
{
	DWORD dwPid = (DWORD)lpDummy;
	HANDLE mutex = CreateMutex(NULL, TRUE, _T("setUserNamePasswordMutex"));

	// Check double entrance
	if(GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	WaitForSingleObject(SkypeReady, 5000);
	EnumWindows (EnumWindowsProc, dwPid);

	ReleaseMutex(mutex);
	CloseHandle(mutex);
	return 0;
}

/**
 * Purpose: Finds Skype window and sets user name and password.
 *
 * Note: This function relies on Skype window placement.
 * It should work for Skype 3.x
 */
void setUserNamePassword(int dwPid)
{
	DWORD threadId;
	CreateThread(NULL, 0, &setUserNamePasswordThread, (LPVOID)dwPid, 0, &threadId);

	// Give time to thread
	Sleep(100);
}
