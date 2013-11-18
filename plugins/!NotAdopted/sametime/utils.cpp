#include "utils.h"

HICON hProtoIcon;

//LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
static int NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case WM_COMMAND: {
			if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
				PUDeletePopUp( hWnd );
				return TRUE;
			}
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp( hWnd );
			return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CALLBACK sttPopupProcA( ULONG param ) {
//DWORD sttPopupProcA(LPVOID param) {
	POPUPDATAEX* ppd = ( POPUPDATAEX* )param;

	if ( ServiceExists(MS_POPUP_ADDPOPUPEX) )
		CallService( MS_POPUP_ADDPOPUPEX, ( WPARAM )ppd, 0 );
	else 
		if ( ServiceExists(MS_POPUP_ADDPOPUP) )
			CallService( MS_POPUP_ADDPOPUP, ( WPARAM )ppd, 0 );

	free( ppd );

	//return 0;
}

void CALLBACK sttPopupProcW( ULONG param ) {
//DWORD sttPopupProcW(LPVOID param) {
	POPUPDATAW* ppd = ( POPUPDATAW* )param;

	if ( ServiceExists(MS_POPUP_ADDPOPUPW) )
		CallService( MS_POPUP_ADDPOPUPW, ( WPARAM )ppd, 0 );

	free( ppd );

	//return 0;
}

void ShowPopupA( const char* line1, const char* line2, int flags )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !ServiceExists( MS_POPUP_ADDPOPUP )) {	
		char title[256];
		sprintf(title, "%s Message", PROTO);

		if(line1 && line2) {
			char *message = new char[strlen(line1) + strlen(line2) + 2]; // newline and null terminator
			sprintf(message, "%s\n%s", line1, line2);
			MessageBoxA( NULL, message, title, MB_OK | MB_ICONINFORMATION );
			delete message;
		} else if(line1) {
			MessageBoxA( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBoxA( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	POPUPDATAEX* ppd = ( POPUPDATAEX* )malloc( sizeof( POPUPDATAEX ));
	memset((void *)ppd, 0, sizeof(POPUPDATAEX));

	ppd->lchContact = NULL;
	ppd->lchIcon = hProtoIcon;

	if(line1 && line2) {
		strncpy( ppd->lpzContactName, line1, MAX_CONTACTNAME );
		strncpy( ppd->lpzText, line2, MAX_SECONDLINE );
	} else if(line1) strncpy( ppd->lpzText, line1, MAX_SECONDLINE );
	else if(line2) strncpy( ppd->lpzText, line2, MAX_SECONDLINE );

	//ppd->colorBack = GetSysColor( COLOR_BTNFACE );
	//ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
	//ppd->colorText = 0x00FFFFFF;	// for old popup modules
	//ppd->colorBack = POPUP_USE_SKINNED_BG;

	ppd->iSeconds = 0;

	ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
	ppd->PluginData = NULL;

	QueueUserAPC( sttPopupProcA , mainThread, ( ULONG )ppd );
	//CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)sttPopupProcA, (LPVOID)ppd, 0, 0));
	//sttPopupProcA((ULONG)ppd);
}

void ShowPopupW( const wchar_t* line1, const wchar_t* line2, int flags )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !ServiceExists( MS_POPUP_ADDPOPUPW )) {	
		wchar_t title[256], *ws;
		swprintf(title, L"%s Message", TranslateW(ws = a2w(PROTO)));
		free(ws);

		if(line1 && line2) {
			wchar_t *message = new wchar_t[wcslen(line1) + wcslen(line2) + 2]; // newline and null terminator
			swprintf(message, L"%s\n%s", line1, line2);
			MessageBoxW( NULL, message, title, MB_OK | MB_ICONINFORMATION );
			delete message;
		} else if(line1) {
			MessageBoxW( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBoxW( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	POPUPDATAW* ppd = ( POPUPDATAW* )malloc( sizeof( POPUPDATAW ));
	memset((void *)ppd, 0, sizeof(POPUPDATAW));

	ppd->lchContact = NULL;
	ppd->lchIcon = hProtoIcon;

	if(line1 && line2) {
		wcsncpy( ppd->lpwzContactName, line1, MAX_CONTACTNAME );
		wcsncpy( ppd->lpwzText, line2, MAX_SECONDLINE );
	} else if(line1) wcsncpy( ppd->lpwzText, line1, MAX_SECONDLINE );
	else if(line2) wcsncpy( ppd->lpwzText, line2, MAX_SECONDLINE );

	//ppd->colorBack = GetSysColor( COLOR_BTNFACE );
	//ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
	//ppd->colorText = 0x00FFFFFF;	// for old popup modules
	//ppd->colorBack = POPUP_USE_SKINNED_BG;

	ppd->iSeconds = 0;

	ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
	ppd->PluginData = NULL;

	QueueUserAPC( sttPopupProcW , mainThread, ( ULONG )ppd );
	//CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)sttPopupProcW, (LPVOID)ppd, 0, 0));
	//sttPopupProcW((ULONG)ppd);
}

void ShowWarning(TCHAR *msg) {
	TCHAR buffer[512];
	ErrorDisplay disp = options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP; 
	if(disp == ED_POP && !ServiceExists(MS_POPUP_SHOWMESSAGE)) disp = ED_BAL;
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

#ifdef _UNICODE
	wsprintf(buffer, L"%s Warning", TranslateTS(PROTO));
#else
	sprintf(buffer, "%s Warning", Translate(PROTO));
#endif


	switch(disp) {
		case ED_POP:
			ShowPopup(buffer, msg, 0);
			break;
		case ED_MB:
			MessageBox(0, msg, buffer, MB_OK | MB_ICONWARNING);
			break;
		case ED_BAL:
			{
				MIRANDASYSTRAYNOTIFY sn = {0};
				sn.cbSize = sizeof(sn);
				sn.szProto = PROTO;
				sn.szInfoTitle = t2a(buffer);
				sn.szInfo = t2a(msg);
				sn.dwInfoFlags = NIIF_WARNING;
				sn.uTimeout = 10;

				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);

				free(sn.szInfoTitle);
				free(sn.szInfo);
			}

			break;
	}
}

void ShowError(TCHAR *msg) {
	TCHAR buffer[512];
	ErrorDisplay disp = options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP;
	if(disp == ED_POP && !ServiceExists(MS_POPUP_SHOWMESSAGE)) disp = ED_BAL;
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

#ifdef _UNICODE
	wchar_t *ws = a2w(PROTO);
	wsprintf(buffer, L"%s Error", TranslateW(ws));
	free(ws);
#else
	sprintf(buffer, "%s Error", Translate(PROTO));
#endif


	switch(disp) {
		case ED_POP:
			ShowPopup(buffer, msg, 0);
			break;
		case ED_MB:
			MessageBox(0, msg, buffer, MB_OK | MB_ICONWARNING);
			break;
		case ED_BAL:
			{
				MIRANDASYSTRAYNOTIFY sn = {0};
				sn.cbSize = sizeof(sn);
				sn.szProto = PROTO;
				sn.szInfoTitle = t2a(buffer);
				sn.szInfo = t2a(msg);
				sn.dwInfoFlags = NIIF_ERROR;
				sn.uTimeout = 10;

				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);

				free(sn.szInfoTitle);
				free(sn.szInfo);
			}

			break;
	}
}


void InitUtils() {
	hProtoIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_PROTO), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
	//hProtoIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_PROTO), IMAGE_ICON, SM_CXICON, SM_CYICON, 0);
}

void DeinitUtils() {
	DestroyIcon(hProtoIcon);
}

char *w2u(const wchar_t *ws) {
	if(ws) {
		int size = WideCharToMultiByte(CP_UTF8, 0, ws, -1, 0, 0, 0, 0);
		char *buff = (char *)malloc(size);
		WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, size, 0, 0);
		return buff;
	} else 
		return 0;
}

wchar_t *u2w(const char *utfs) {
	if(utfs) {
		int size = MultiByteToWideChar(CP_UTF8, 0, utfs, -1, 0, 0);
		wchar_t *buff = (wchar_t *)malloc(size * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, utfs, -1, buff, size);
		return buff;
	} else
		return 0;
}

wchar_t *a2w(const char *as) {
	int size = MultiByteToWideChar(code_page, 0, as, -1, 0, 0);
	wchar_t *buff = (wchar_t *)malloc(size * sizeof(wchar_t));
	MultiByteToWideChar(code_page, 0, as, -1, buff, size);
	return buff;
}

char *w2a(const wchar_t *ws) {
	int size = WideCharToMultiByte(code_page, 0, ws, -1, 0, 0, 0, 0);
	char *buff = (char *)malloc(size);
	WideCharToMultiByte(code_page, 0, ws, -1, buff, size, 0, 0);
	return buff;
}

char *t2a(const TCHAR *ts) {
#ifdef _UNICODE
	return w2a(ts);
#else
	return _strdup(ts);
#endif
}

TCHAR *a2t(const char *as) {
#ifdef _UNICODE
	return a2w(as);
#else
	return _strdup(as);
#endif
}

TCHAR *u2t(const char *utfs) {
#ifdef _UNICODE
	return u2w(utfs);
#else
	wchar_t *ws = u2w(utfs);
	char *ret = w2a(ws);
	free(ws);
	return ret;
#endif
}

char *t2u(const TCHAR *ts) {
#ifdef _UNICODE
	return w2u(ts);
#else
	wchar_t *ws = a2w(ts);
	char *ret = w2u(ws);
	free(ws);
	return ret;
#endif
}

char *u2a(const char *utfs) {
	wchar_t *ws = u2w(utfs);
	char *ret = w2a(ws);
	free(ws);
	return ret;
}

char *a2u(const char *as) {
	wchar_t *ws = a2w(as);
	char *ret = w2u(ws);
	free(ws);
	return ret;
}
