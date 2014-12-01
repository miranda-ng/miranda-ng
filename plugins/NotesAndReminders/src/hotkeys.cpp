#include "globals.h"

#define MSG_WND_CLASS _T("MIM_SNMsgWindow")


HWND HKHwnd;


enum KB_ACTIONS {KB_NEW_NOTE = 1, KB_TOGGLE_NOTES, KB_NEW_REMINDER};


void RegisterKeyBindings()
{
	HOTKEYDESC desc;

	memset(&desc, 0, sizeof(desc));
	desc.cbSize = sizeof(desc);
	desc.ptszSection = _T(SECTIONNAME);
	desc.dwFlags = HKD_TCHAR;

	desc.pszName = MODULENAME"/NewNote";
	desc.ptszDescription = LPGENT("New Note");
	desc.lParam = KB_NEW_NOTE;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_INSERT);
	desc.pszService = MODULENAME"/MenuCommandAddNew";
	Hotkey_Register(&desc);

	desc.pszName = MODULENAME"/ToggleNotesVis";
	desc.ptszDescription = LPGENT("Toggle Notes Visibility");
	desc.lParam = KB_TOGGLE_NOTES;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_ADD);
	desc.pszService = MODULENAME"/MenuCommandShowHide";
	Hotkey_Register(&desc);

	desc.pszName = MODULENAME"/BringNotesFront";
	desc.ptszDescription = LPGENT("Bring All Notes to Front");
	desc.lParam = KB_TOGGLE_NOTES;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_HOME);
	desc.pszService = MODULENAME"/MenuCommandBringAllFront";
	Hotkey_Register(&desc);

	desc.pszName = MODULENAME"/NewReminder";
	desc.ptszDescription = LPGENT("New Reminder");
	desc.lParam = KB_NEW_REMINDER;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_SUBTRACT);
	desc.pszService = MODULENAME"/MenuCommandNewReminder";
	Hotkey_Register(&desc);
}


/*int HandleNRShortcuts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	STICKYNOTE* PSN;

	BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
	BOOL isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) && !isAlt;

	int action;
	MSG amsg;
	amsg.hwnd = hwnd;
	amsg.message = msg;
	amsg.wParam = wParam;
	amsg.lParam = lParam;

	switch ( (action = CallService(MS_HOTKEY_CHECK, (WPARAM)&amsg, (LPARAM)SECTIONNAME)) )
	{
	case KB_NEW_NOTE:
		PSN = NewNote(0,0,0,0,NULL,NULL,TRUE,TRUE,0);
		SetFocus(PSN->REHwnd);
		return FALSE;
	case KB_TOGGLE_NOTES:
		ShowHideNotes();
		return FALSE;
	case KB_NEW_REMINDER:
		NewReminder();
		return FALSE;
	}

	return -1;
}*/


LRESULT CALLBACK NotifyHotKeyWndProc(HWND AHwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
	BOOL b;

	switch (Message)
	{
	case WM_TIMER:
		{
			KillTimer(HKHwnd,1026);
			b = CheckRemindersAndStart();
			SetTimer(HKHwnd,1026,b ? REMINDER_UPDATE_INTERVAL_SHORT : REMINDER_UPDATE_INTERVAL,0);

			return FALSE;
		}
	}

	return DefWindowProc(AHwnd,Message,wParam,lParam);
}

void CreateMsgWindow(void)
{
	HWND hParent = NULL;
	WNDCLASSEX TWC = {0};

	if (!GetClassInfoEx(hmiranda,MSG_WND_CLASS,&TWC))
	{
		TWC.style = 0;
		TWC.cbClsExtra = 0;
		TWC.cbWndExtra = 0;
		TWC.hInstance = hmiranda;
		TWC.hIcon = 0;
		TWC.hCursor = 0;
		TWC.hbrBackground = 0;
		TWC.lpszMenuName = NULL;
		TWC.lpszClassName = MSG_WND_CLASS;
		TWC.cbSize = sizeof(TWC);
		TWC.lpfnWndProc = NotifyHotKeyWndProc;
		RegisterClassEx(&TWC);
	}

	hParent = HWND_MESSAGE;

	HKHwnd = CreateWindowEx(WS_EX_TOOLWINDOW, MSG_WND_CLASS, _T("StickyNotes"), 0, 0, 0, 0, 0, hParent, NULL, hmiranda, NULL);
	SetTimer(HKHwnd, 1026, REMINDER_UPDATE_INTERVAL, 0);
}

void DestroyMsgWindow(void)
{
	KillTimer(HKHwnd,1026);
	DestroyWindow(HKHwnd);
}
