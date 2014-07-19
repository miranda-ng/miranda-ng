#include "headers.h"

void CShake::Load(void)
{
	Shaking = false;
	ShakingChat = false;
	nScaleClist = db_get_dw(NULL, MODULENAME, "ScaleClist", 5);
	nScaleChat = db_get_dw(NULL, MODULENAME, "ScaleChat", 2);
	nMoveClist = db_get_dw(NULL, MODULENAME, "MoveClist", 15);
	nMoveChat = db_get_dw(NULL, MODULENAME, "MoveChat", 15);
}
void CShake::Save(void)
{
	db_set_dw(NULL, MODULENAME, "ScaleClist", this->nScaleClist);
	db_set_dw(NULL, MODULENAME, "ScaleChat", this->nScaleChat);
	db_set_dw(NULL, MODULENAME, "MoveClist", this->nMoveClist);
	db_set_dw(NULL, MODULENAME, "MoveChat", this->nMoveChat);
}

DWORD WINAPI ShakeChatWindow(LPVOID Param)
{
	HWND hWnd = (HWND)Param;
	shake.ShakeChat(hWnd);
	return 1;
}

DWORD WINAPI ShakeClistWindow(LPVOID Param)
{
	HWND hWnd = (HWND)Param;
	shake.ShakeClist(hWnd);
	return 0;
}

INT_PTR ShakeClist(WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	DWORD tid;
	CreateThread(NULL, 0, ShakeClistWindow, (LPVOID)hWnd, 0, &tid);
	return 0;
}

INT_PTR ShakeChat(WPARAM wParam, LPARAM lParam)
{
	if (((HANDLE)wParam) == NULL) return -1;

	DWORD tid;
	//char srmmName[100];
	MessageWindowData mwd;
	MessageWindowInputData mwid;

	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = db_mc_tryMeta(wParam);
	mwd.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = mwd.hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;


	CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd);
	//CallService(MS_MSG_GETWINDOWCLASS,(WPARAM)srmmName,(LPARAM)100 );

	HWND parent;
	HWND hWnd = mwd.hwndWindow;
	while ((parent = GetParent(hWnd)) != 0) hWnd = parent; // ensure we have the top level window (need parent window for scriver & tabsrmm)

	CreateThread(NULL, 0, ShakeChatWindow, (LPVOID)hWnd, 0, &tid);
	return 0;
}

int CShake::ShakeChat(HWND hWnd)
{
	if (!ShakingChat) {
		ShakingChat = true;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		for (int i = 0; i < nMoveChat; i++) {
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
	if (!Shaking) {
		Shaking = true;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		for (int i = 0; i < nMoveClist; i++) {
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
