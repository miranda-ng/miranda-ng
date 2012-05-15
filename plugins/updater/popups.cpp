#include "common.h"
#include "popups.h"

HWND hwndPop;
HANDLE hEventPop;
bool pop_cancelled;
bool bPopupsEnabled;

typedef struct
{
	unsigned flags;
	HICON hIcon;
} PopupDataType;


LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case UM_INITPOPUP:
			{
				PopupDataType* popup = (PopupDataType*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,0);
				if (popup->flags & POPFLAG_SAVEHWND) hwndPop = hWnd;
			}
			return 0;
		case WMU_CLOSEPOP:
			SetEvent(hEventPop);
			PUDeletePopUp( hWnd );
			return TRUE;

		case WM_COMMAND:
			{
				PopupDataType* popup = (PopupDataType*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,0);
				if (popup->flags & POPFLAG_SAVEHWND) pop_cancelled = false;
			}
			SetEvent(hEventPop);
			PUDeletePopUp( hWnd );
			return TRUE;

		case WM_CONTEXTMENU:
			{
				PopupDataType* popup = (PopupDataType*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,0);
				if (popup->flags & POPFLAG_SAVEHWND) pop_cancelled = true;
			}
			SetEvent(hEventPop);
			PUDeletePopUp( hWnd );
			return TRUE;

		case UM_FREEPLUGINDATA: {
			PopupDataType* popup = (PopupDataType*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);
			if ((INT_PTR)popup != CALLSERVICE_NOTFOUND)
			{
				if (popup->flags & POPFLAG_SAVEHWND) hwndPop = 0;
				if (popup->hIcon) ReleaseIconEx(popup->hIcon);
				mir_free(popup);
			}
			return TRUE;
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void __stdcall sttPopupProc( void* dwParam )
{
	POPUPDATAT* ppd = ( POPUPDATAT* )dwParam;
	CallService( MS_POPUP_ADDPOPUPT, ( WPARAM )ppd, 0 );
	mir_free( ppd );
}

void ShowPopup( HANDLE hContact, const TCHAR* line1, const TCHAR* line2, int flags, int timeout )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !ServiceExists( MS_POPUP_ADDPOPUPT )) {	
		TCHAR title[256];
		mir_sntprintf(title, SIZEOF(title), _T("%s Message"), _T(MODULE));

		if(line1 && line2) {
			size_t len = _tcslen(line1) + _tcslen(line2) + 1;
			TCHAR *message = (TCHAR*)alloca(len * sizeof(TCHAR)); // newline and null terminator
			mir_sntprintf(message, len, _T("%s\n%s"), line1, line2);
			MessageBox( NULL, message, title, MB_OK | MB_ICONINFORMATION );
		} else if(line1) {
			MessageBox( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBox( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	SetEvent(hEventPop);
	ResetEvent(hEventPop);

	if(ServiceExists(MS_POPUP_ADDPOPUPCLASS))
	{
		POPUPDATACLASS d = {sizeof(d), "updaterpopups"};
		d.ptszTitle = line1;
		d.ptszText  = line2;
		d.PluginData = mir_calloc(sizeof(PopupDataType));
		((PopupDataType*)d.PluginData)->flags = flags;
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
	} 
	else 
	{
		POPUPDATAT* ppd = ( POPUPDATAT* )mir_calloc( sizeof( POPUPDATAT ));

		ppd->lchContact = hContact;
		ppd->lchIcon = LoadIconEx(I_CHKUPD);

		if (line1 && line2)
		{
			_tcscpy( ppd->lptzContactName, line1 );
			_tcscpy( ppd->lptzText, line2 );
		} 
		else if (line1) 
			_tcscpy( ppd->lptzText, line1 );
		else if (line2) 
			_tcscpy( ppd->lptzText, line2 );

		if (options.set_colours) 
		{
			ppd->colorBack = options.bkCol;
			ppd->colorText = options.textCol;
		} 
		else
		{
			//ppd->colorText = 0x00FFFFFF;
			ppd->colorText = 0;
			//ppd->colorBack = POPUP_USE_SKINNED_BG;
			ppd->colorBack = 0;
		}

		ppd->iSeconds = timeout;

		ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
		ppd->PluginData = mir_calloc(sizeof(PopupDataType));
		((PopupDataType*)ppd->PluginData)->flags = flags;
		((PopupDataType*)ppd->PluginData)->hIcon = ppd->lchIcon;

		CallFunctionAsync( sttPopupProc , ppd );
	}
}

void ShowWarning(TCHAR *msg)
{
	static const TCHAR title[512] = _T(MODULE) _T(" Warning");

	if(ServiceExists(MS_POPUP_SHOWMESSAGE))
	{
		TCHAR message[1024];
		mir_sntprintf(message, SIZEOF(message), _T("%s: %s"), _T(MODULE), msg);
		ShowPopup(0, title, message, 0, 10);
		//PUShowMessage(message, SM_WARNING);
	} 
	else
		MessageBox(0, msg, title, MB_OK | MB_ICONWARNING);
}

void ShowError(TCHAR *msg) 
{
	static const TCHAR title[512] = _T(MODULE) _T(" Error");
	NLog(msg);

	if(ServiceExists(MS_POPUP_SHOWMESSAGE)) 
	{
		TCHAR message[1024];
		mir_sntprintf(message, SIZEOF(message), _T("%s: %s"), _T(MODULE), msg);
		//PUShowMessage(message, SM_WARNING);
		ShowPopup(0, title, message, 0, 10);
	} 
	else
		MessageBox(0, msg, title, MB_OK | MB_ICONERROR);
}

void ChangePopupText(HWND hwnd, TCHAR *text) 
{
	CallService(MS_POPUP_CHANGETEXTT, (WPARAM)hwnd, (LPARAM)text);
}

bool ArePopupsEnabled() 
{
	return bPopupsEnabled;
}

HICON hPopupIcon;
void InitPopups() {
	bPopupsEnabled = ServiceExists(MS_POPUP_ADDPOPUPT) != 0;

	hEventPop = CreateEvent( NULL, TRUE, FALSE, NULL );

	if (ServiceExists(MS_POPUP_REGISTERCLASS)) 
	{
		hPopupIcon = LoadIconEx(I_CHKUPD);
		POPUPCLASS test = {0};
		test.cbSize = sizeof(test);
		test.flags = PCF_TCHAR;
		test.hIcon = hPopupIcon;
		test.iSeconds = -1;
		test.ptszDescription = TranslateT(MODULE);
		test.pszName = "updaterpopups";
		test.PluginWindowProc = NullWindowProc;
		CallService(MS_POPUP_REGISTERCLASS, 0, (WPARAM)&test);
	}
}

void DeinitPopups()
{
	CloseHandle(hEventPop);
	ReleaseIconEx(hPopupIcon);
}


