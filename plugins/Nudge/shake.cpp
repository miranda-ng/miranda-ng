#include "headers.h"
#include "shake.h"

extern CShake shake;

void CShake::Load(void)
{

	Shaking = false;
	ShakingChat = false;
	nScaleClist = DBGetContactSettingDword(NULL, "Nudge", "ScaleClist", 5);
	nScaleChat = DBGetContactSettingDword(NULL, "Nudge", "ScaleChat", 2);
	nMoveClist = DBGetContactSettingDword(NULL, "Nudge", "MoveClist", 15);
	nMoveChat = DBGetContactSettingDword(NULL, "Nudge", "MoveChat", 15);
}
void CShake::Save(void)
{
	DBWriteContactSettingDword(NULL, "Nudge", "ScaleClist", this->nScaleClist);
	DBWriteContactSettingDword(NULL, "Nudge", "ScaleChat", this->nScaleChat);
	DBWriteContactSettingDword(NULL, "Nudge", "MoveClist", this->nMoveClist);
	DBWriteContactSettingDword(NULL, "Nudge", "MoveChat", this->nMoveChat);
}

DWORD WINAPI ShakeChatWindow(LPVOID Param)
{
	HWND hWnd = (HWND) Param;
	shake.ShakeChat(hWnd);
	return 1;
}

DWORD WINAPI ShakeClistWindow(LPVOID Param)
{
	HWND hWnd = (HWND) Param;
	shake.ShakeClist(hWnd);
	return 0;
}

INT_PTR ShakeClist( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd = (HWND) CallService( MS_CLUI_GETHWND, 0, 0 );

	CreateThread(NULL,0,ShakeClistWindow,(LPVOID) hWnd,0,&tid);
	return 0;
}

INT_PTR ShakeChat( WPARAM wParam, LPARAM lParam )
{
    if(((HANDLE) wParam) == NULL) return -1;
        
	DWORD tid;
	//char srmmName[100];
	MessageWindowData mwd;
	MessageWindowInputData mwid;

	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = Nudge_GethContact((HANDLE) wParam);
	mwd.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = mwd.hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;


	CallService( MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd );
	//CallService(MS_MSG_GETWINDOWCLASS,(WPARAM)srmmName,(LPARAM)100 );

	HWND parent;
	HWND hWnd = mwd.hwndWindow;
	while((parent = GetParent(hWnd)) != 0) hWnd = parent; // ensure we have the top level window (need parent window for scriver & tabsrmm)

	CreateThread(NULL,0,ShakeChatWindow,(LPVOID) hWnd,0,&tid);
	return 0;
}

/*

int TriggerShakeClist( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd;
	int flags;
	flags = (int)wParam;

	if (!flags&ACT_PERFORM)
		return 0;

	hWnd = (HWND) CallService( MS_CLUI_GETHWND, 0, 0 );
	
	CreateThread(NULL,0,ShakeClistWindow,(LPVOID) hWnd,0,&tid);
	return 0;
}

int TriggerShakeChat( WPARAM wParam, LPARAM lParam )
{
	DWORD tid;
	HWND hWnd;
	char srmmName[100];
	MessageWindowData mwd;
	MessageWindowInputData mwid;
	int flags;
	flags = (int)wParam;

	if (!flags&ACT_PERFORM)
		return 0;

	SPECIFICACTIONINFO *sai;
	HANDLE hContact;

	
	sai = (SPECIFICACTIONINFO *) lParam;

	if ( (sai->td != NULL) && (sai->td->dFlags&DF_CONTACT) )
		hContact = sai->td->hContact;
	else
		return 0;

	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = Nudge_GethContact((HANDLE) hContact);
	mwd.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = Nudge_GethContact((HANDLE) hContact);
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	CallService( MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd );
	CallService(MS_MSG_GETWINDOWCLASS,(WPARAM)srmmName,(LPARAM)100 );

	if ( !strnicmp( srmmName,"tabSRMM ", 7 ))
		hWnd = GetParent(GetParent(mwd.hwndWindow));
	
	if ( !strnicmp( srmmName,"SRMM ", 4))
		hWnd = mwd.hwndWindow;

	if ( !strnicmp( srmmName,"Scriver ", 7 ))
		hWnd = GetParent(mwd.hwndWindow);

	CreateThread(NULL,0,ShakeChatWindow,(LPVOID) hWnd,0,&tid);
	return 0;
}
*/

int CShake::ShakeChat(HWND hWnd)
{
	if(!ShakingChat)
	{
		ShakingChat = true;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		for(int i = 0; i < nMoveChat; i++)
		{
			SetWindowPos(hWnd, 0, rect.left - nScaleChat, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top - nScaleChat, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left + nScaleChat, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top + nScaleChat, 0, 0, SWP_NOSIZE);
			Sleep(10);
		}
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE); //SWP_DRAWFRAME
		ShakingChat = false;
	}
	return 0;
}

int CShake::ShakeClist(HWND hWnd)
{
	if(!Shaking)
	{
		Shaking = true;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		for(int i = 0; i < nMoveClist; i++)
		{
			SetWindowPos(hWnd, 0, rect.left - nScaleClist, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top - nScaleClist, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left + nScaleClist, rect.top, 0, 0, SWP_NOSIZE);
			Sleep(10);
			SetWindowPos(hWnd, 0, rect.left, rect.top + nScaleClist, 0, 0, SWP_NOSIZE);
			Sleep(10);
		}
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		Shaking = false;
	}
	return 0;
}