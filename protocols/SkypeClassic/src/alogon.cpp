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

	*className = 0;
	GetClassName(wnd, &className[0], sizeof(className) / sizeof(className[0]));
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
	while (wnd != NULL && mir_tstrcmp(getClassName(wnd), childClassName) != 0)
		wnd = GetWindow(wnd, GW_HWNDNEXT);

	return wnd;
}

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);
	if (lParam != 0 && dwPID != (DWORD)lParam) return TRUE;
	const TCHAR *lpszClassName = getClassName(hWnd);
	if (mir_tstrcmp(lpszClassName, _T("tSkMainForm.UnicodeClass")) == 0 ||
		mir_tstrcmp(lpszClassName, _T("TLoginForm.UnicodeClass")) == 0)
	{
		HWND loginControl = GetWindow(hWnd, GW_CHILD);

		LOG(("setUserNamePasswordThread: Skype window found!"));

		// Sleep for some time, while Skype is loading
		// It loads slowly :(
		//Sleep(5000);
		LOG(("TLoginControl = %S", getClassName(loginControl)));

		// Check for login control
		if (mir_tstrcmp(getClassName(loginControl), _T("TLoginControl")) == 0)
		{
			// Find user name window
			HWND userName = findWindow(loginControl, _T("TNavigableTntComboBox.UnicodeClass"));
			HWND password = findWindow(loginControl, _T("TNavigableTntEdit.UnicodeClass"));

			LOG(("userName=%08X; password=%08X", userName, password));
			if (userName && password)
			{
				// Set user name and password
				DBVARIANT dbv;

				if (!db_get_ws(NULL, SKYPE_PROTONAME, "LoginUserName", &dbv))
				{
					SetWindowTextW(userName, dbv.pwszVal);
					db_free(&dbv);
				}

				if (!db_get_ws(NULL, SKYPE_PROTONAME, "LoginPassword", &dbv))
				{
					SetWindowTextW(password, dbv.pwszVal);
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

void __cdecl setUserNamePasswordThread(void *lpDummy)
{
	DWORD dwPid = (DWORD)lpDummy;
	HANDLE mutex = CreateMutex(NULL, TRUE, _T("setUserNamePasswordMutex"));

	// Check double entrance
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return;

	WaitForSingleObject(SkypeReady, 5000);
	EnumWindows(EnumWindowsProc, dwPid);

	ReleaseMutex(mutex);
	CloseHandle(mutex);
}

/**
 * Purpose: Finds Skype window and sets user name and password.
 *
 * Note: This function relies on Skype window placement.
 * It should work for Skype 3.x
 */
void setUserNamePassword(int dwPid)
{
	mir_forkthread(setUserNamePasswordThread, (void*)dwPid);

	// Give time to thread
	Sleep(100);
}
