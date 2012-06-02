/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sessions.h"

HINSTANCE hinstance = NULL;

WNDPROC mainProc;

HANDLE hEventDbWindowEvent,hEventDbOkToExit, hEventDbOptionsInit, hEventDbPluginsLoaded,
hEventDbPreShutdown,hServiceOpenManager,hServiceShowFavMenu,hServiceCloseCurrentSession,hServiceSaveUserSession,
hServiceLoadLastSession,hmSaveCurrentSession,
hmLoadLastSession,hmLoadSession,hmSessionsManager,hibSessionsLoad,hibSessionsSave,hibSessionsLoadLast,hibChecked,hibNotChecked	;

HICON hiChecked,hiNotChecked,hiSessions,hiSessionsLoad ,hiSessionsSave,hiSessionsLoadLast;


HANDLE hmTBButton[2],hiTBbutton[2],iTBbutton[2];

BOOL g_hghostw;

HWND hClistControl;

unsigned int ses_limit;
unsigned int g_ses_count;
BOOL g_bExclHidden;	
BOOL g_bWarnOnHidden;
BOOL g_bOtherWarnings;
BOOL g_bCrashRecovery;
BOOL g_bIncompletedSave;

struct MM_INTERFACE mmi;																					   
HWND g_hDlg;
HWND g_hSDlg;
BOOL DONT = FALSE;
BOOL StartUp,isLastTRUE = FALSE,g_mode,bSC = FALSE;

DWORD session_list[255] = {0};
DWORD user_session_list[255] = {0};
DWORD session_list_recovered[255];

int count = 0;
unsigned int ses_count = 0;

PLUGINLINK *pluginLink;
int hLangpack;

int OptionsInit(WPARAM,LPARAM);

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,	
#ifdef _UNICODE
	// {60558872-2AAB-45aa-888D-097691C9B683}
		{ 0x60558872, 0x2aab, 0x45aa, { 0x88, 0x8d, 0x9, 0x76, 0x91, 0xc9, 0xb6, 0x83 } }
#else
	// {F929B07B-DC4D-43d6-A6F3-3AA51F9FF81A}
		{ 0xf929b07b, 0xdc4d, 0x43d6, { 0xa6, 0xf3, 0x3a, 0xa5, 0x1f, 0x9f, 0xf8, 0x1a } }
#endif
};


INT_PTR CALLBACK ExitDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{	
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hdlg);
			LoadPosition(hdlg, "ExitDlg");
			ShowWindow(hdlg,SW_SHOW);
		}break;

	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
			case IDOK:
			{
				SavePosition(hdlg, "ExitDlg");
				SaveSessionDate();
				SaveSessionHandles(0,0);
				DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastempty", 0);
				DestroyWindow(hdlg);
			}break;
			
			case IDCANCEL:
			{
				SavePosition(hdlg, "ExitDlg");
				DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastempty", 1);
				DestroyWindow(hdlg);
			}break;
		}

	case WM_CLOSE:
		DestroyWindow(hdlg);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK SaveSessionDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{	
	g_hSDlg = hdlg;
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			HWND hList;
			hList = GetDlgItem(hdlg, IDC_LIST);
			TranslateDialogDefault(hdlg);
			LoadSessionToCombobox(hdlg, 1, 5, "UserSessionDsc", 0);

			LoadPosition(hdlg, "SaveDlg");
			ShowWindow(hdlg, SW_SHOW);
		}break;

		case WM_CLOSE:
			DestroyWindow(hdlg);
			g_hSDlg = 0;
			break;

		case WM_NOTIFY:
		{
			switch (((LPNMHDR) lparam)->idFrom)
			{
				case IDC_CLIST:
					switch (((LPNMHDR)lparam)->code)
					{
						case CLN_CHECKCHANGED:
						{
							bSC = TRUE;
							memcpy(user_session_list, session_list, SIZEOF(user_session_list));
						}break;

					}
			}break;
		}break;

		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case IDC_SELCONTACTS:
				{
					HANDLE hItem;
					RECT rWnd;
					int i=0, x=0,y=0,dy=0,dx=0,dd=0;

					GetWindowRect(hdlg, &rWnd);
					x=rWnd.right-rWnd.left;
					y=rWnd.bottom-rWnd.top;
					if(IsDlgButtonChecked(hdlg,IDC_SELCONTACTS))
					{
						EnableWindow(GetDlgItem(hdlg,IDC_SANDCCHECK),FALSE);
						dy=20;
						dx=150;
						dd=5;
						hClistControl = CreateWindowEx(WS_EX_CLIENTEDGE,CLISTCONTROL_CLASS, _T(""), 
						WS_TABSTOP |WS_VISIBLE | WS_CHILD , 
						x, y, dx, dy, hdlg, (HMENU)IDC_CLIST, hinstance, 0);

						SetWindowLongPtr(hClistControl, GWL_STYLE,
							GetWindowLongPtr(hClistControl, GWL_STYLE)|CLS_CHECKBOXES|CLS_HIDEEMPTYGROUPS|CLS_USEGROUPS|CLS_GREYALTERNATE|CLS_GROUPCHECKBOXES);
						SendMessage(hClistControl, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP|CLS_EX_TRACKSELECT, 0);
					}
					else
					{
						EnableWindow(GetDlgItem(hdlg,IDC_SANDCCHECK),TRUE);
						dy=-20;
						dx=-150;
						dd=5;
						DestroyWindow(hClistControl);
					}

					SetWindowPos(hdlg, NULL, rWnd.left,rWnd.top, x+dx, y+(dx/3), SWP_NOZORDER | SWP_NOOWNERZORDER|SWP_NOMOVE);

					SetWindowPos(hClistControl,0,x-dd,dd,dx-dd,y+(dx/12),SWP_NOZORDER/*|SWP_NOSIZE|SWP_SHOWWINDOW*/);
					SendMessage(hClistControl,WM_TIMER,TIMERID_REBUILDAFTER,0);

					for ( i = 0; session_list[i] > 0; i++)
					{
						hItem=(HANDLE)SendMessage(hClistControl,CLM_FINDCONTACT, (WPARAM)session_list[i], 0);
						SendMessage(hClistControl, CLM_SETCHECKMARK, (WPARAM)hItem,1);
					}

					OffsetWindow(hdlg, GetDlgItem(hdlg,IDC_LIST), 0, dy);
					OffsetWindow(hdlg, GetDlgItem(hdlg,IDC_STATIC), 0, dy);
					OffsetWindow(hdlg, GetDlgItem(hdlg,IDC_SANDCCHECK), 0, dy);
					OffsetWindow(hdlg, GetDlgItem(hdlg,IDOK), 0, dy);
					OffsetWindow(hdlg, GetDlgItem(hdlg,IDCANCEL), 0, dy);

				}break;

				case IDOK:
				{
					DWORD session_list_temp[255]={0};
					int lenght,i;
					TCHAR szUserSessionName[MAX_PATH];
					lenght = GetWindowTextLength(GetDlgItem(hdlg, IDC_LIST));
					SavePosition(hdlg, "SaveDlg");
					if (lenght>0)
					{
						GetWindowText(GetDlgItem(hdlg, IDC_LIST), szUserSessionName, SIZEOF(szUserSessionName));
						szUserSessionName[lenght+1]='\0';
						if(IsDlgButtonChecked(hdlg,IDC_SELCONTACTS)&&bSC)
						{
							HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
							for (i=0; hContact; hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
							{
								BYTE res =(BYTE)SendMessage(hClistControl, CLM_GETCHECKMARK, SendMessage(hClistControl, CLM_FINDCONTACT, (WPARAM)hContact, 0), 0);
								if (res)
								{
									user_session_list[i]=(DWORD)hContact;
									i++;
								}
							}
							memcpy(session_list_temp,session_list,SIZEOF(session_list_temp));
							memcpy(session_list,user_session_list,SIZEOF(session_list));
							SaveSessionHandles(0,1);
							SaveUserSessionName(szUserSessionName);
							memcpy(session_list,session_list_temp,SIZEOF(session_list));
							DestroyWindow(hdlg);
							g_hSDlg=0;
						}
						else if (!SaveUserSessionName(szUserSessionName))
						{	
							SaveSessionHandles(0,1);

							if(IsDlgButtonChecked(hdlg,IDC_SANDCCHECK))
								CloseCurrentSession(0,0);
							DestroyWindow(hdlg);
							g_hSDlg=0;
						}
						else
							MessageBox(NULL,TranslateT("Current session is empty!"),TranslateT("Sessions Manager"),MB_OK|MB_ICONWARNING);
					}
					else
						MessageBox(NULL,TranslateT("Session name is empty, enter the name and try again"),TranslateT("Sessions Manager"),MB_OK|MB_ICONWARNING); 
				}break;
			
				case IDCANCEL:
				{
					SavePosition(hdlg, "SaveDlg");
					DestroyWindow(hdlg);
					g_hSDlg=0;
				}break;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK LoadSessionDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{	
	g_hDlg=hdlg;
	switch(msg)
	{
		case WM_INITDIALOG:
		{	 
			int iDelay=DBGetContactSettingWord(NULL, __INTERNAL_NAME, "StartupModeDelay", 1500);
			if(g_hghostw==TRUE)
				SetTimer(hdlg, TIMERID_LOAD, iDelay, NULL);
			else
			{
				ses_count=0;
				TranslateDialogDefault(hdlg);
				if ((ses_count=LoadSessionToCombobox(hdlg,0,ses_limit,"SessionDate",0))==ses_limit) 
					EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),TRUE);

				if(LoadSessionToCombobox(hdlg,0,255,"UserSessionDsc",ses_limit)==0 && ses_count!=0) 
					ses_count=0; 

				if(session_list_recovered[0])
					ses_count=256;

				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, (WPARAM)0, 0);
				LoadPosition(hdlg, "LoadDlg");
				if(StartUp)
					SetTimer(hdlg, TIMERID_SHOW, iDelay, NULL);
				else
					ShowWindow(g_hDlg,SW_SHOW);
			}
		}break;

		case WM_TIMER:
			if(wparam == TIMERID_SHOW)
			{
				KillTimer(hdlg, TIMERID_SHOW);
				ShowWindow(hdlg,SW_SHOW);
				StartUp=FALSE;
			}
			else
			{
				KillTimer(hdlg, TIMERID_LOAD);
				LoadSession(0,0);
				g_hghostw=StartUp=FALSE;
				DestroyWindow(hdlg);
				g_hDlg=0;
			}
			break;

		case WM_CLOSE:
			SavePosition(hdlg, "LoadDlg");
			DestroyWindow(hdlg);
			g_hDlg=0;
			break;

		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case IDC_LIST:
				{
					switch(HIWORD(wparam))
					{
						case CBN_SELCHANGE:
						{
							HWND hCombo = GetDlgItem(hdlg, IDC_LIST);
							int index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
							if(index != CB_ERR)
								ses_count = SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)index, 0);
						}break;
					}break;
				}break;

				case IDC_SESSDEL:
				{
					if(session_list_recovered[0]&&ses_count==256)
					{
						int i=0;
						while(session_list_recovered[i])
						{
							DBWriteContactSettingByte((HANDLE)session_list_recovered[i], __INTERNAL_NAME, "wasInLastSession", 0);
							i++;
						}
						ZeroMemory(session_list_recovered,SIZEOF(session_list_recovered));
						g_bIncompletedSave=0;

						EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),FALSE);
						SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);

						if ((ses_count=LoadSessionToCombobox(hdlg,1,ses_limit,"SessionDate",0))==ses_limit) 
							EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),TRUE);

						if(LoadSessionToCombobox(hdlg,1,255,"UserSessionDsc",ses_limit)==0 && ses_count!=0) 
							ses_count=0;

						SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, (WPARAM)0, 0);

					}
					else if (ses_count>=ses_limit)
					{
						ses_count-=ses_limit;
						DelUserDefSession(ses_count);
						EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),FALSE);
						SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);

						if ((ses_count=LoadSessionToCombobox(hdlg,0,ses_limit,"SessionDate",0))==ses_limit)
							EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),TRUE);

						if(LoadSessionToCombobox(hdlg,0,255,"UserSessionDsc",ses_limit)==0 && ses_count!=0)
							ses_count=0;

						if(session_list_recovered[0]) ses_count=256;

						SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, (WPARAM)0, 0);
					}
					else 
					{
						DeleteAutoSession(ses_count);
						EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),FALSE);
						SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);
						if ((ses_count=LoadSessionToCombobox(hdlg,0,ses_limit,"SessionDate",0))==ses_limit) 
							EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),TRUE);

						if(LoadSessionToCombobox(hdlg,0,255,"UserSessionDsc",ses_limit)==0 && ses_count!=0)
							ses_count=0;

						if(session_list_recovered[0]) ses_count=256;

						SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, (WPARAM)0, 0);
					}
					if (SendDlgItemMessage(hdlg, IDC_LIST, CB_GETCOUNT, 0, 0))
						EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),TRUE);
					else
						EnableWindow(GetDlgItem(hdlg,IDC_SESSDEL),FALSE);
				}break;

				case IDOK:
				{	
					if (!LoadSession(0,ses_count))
					{
						SavePosition(hdlg, "LoadDlg");
						DestroyWindow(hdlg);
						g_hDlg=0;
					}
				}break;

				case IDCANCEL:
				{
					SavePosition(hdlg, "LoadDlg");
					DestroyWindow(hdlg);
					g_hDlg=0;
				}break;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

INT_PTR CloseCurrentSession(WPARAM wparam,LPARAM lparam)
{ 
	HWND hWnd;
	int i=0;
	MessageWindowInputData  mwid;
	MessageWindowData  mwd;

	while(session_list[0]!=0)
	{ 
		mwid.cbSize = sizeof(MessageWindowInputData);
		mwid.hContact=(HANDLE)session_list[i];
		mwid.uFlags=MSG_WINDOW_UFLAG_MSG_BOTH;

		mwd.cbSize = sizeof(MessageWindowData);
		mwd.hContact = mwid.hContact;
		mwd.uFlags=MSG_WINDOW_UFLAG_MSG_BOTH;
		CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid,(LPARAM)&mwd);

		if (g_mode)
		{
			hWnd=GetAncestor(mwd.hwndWindow,GA_ROOT);
			SendMessage(hWnd,WM_CLOSE,0,1);
		}
		else
			SendMessage(mwd.hwndWindow, WM_CLOSE, 0, 0);
	}
	ZeroMemory(session_list,SIZEOF(session_list));
	return 0;
}

int SaveSessionHandles(WPARAM wparam,LPARAM lparam)
{
	int k=0;

	if(session_list[0]!=0)
	{
		for (HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact;
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
		{	
			if ((k=CheckForDuplicate(session_list,(DWORD)hContact))!=-1
				&&!(g_bExclHidden&&!CheckContactVisibility(hContact))) 
			{
				AddSessionMark((HANDLE)hContact,lparam,'1');
				AddInSessionOrder((HANDLE)hContact,lparam,k,1);
			}
			else
			{
				AddSessionMark((HANDLE)hContact,lparam,'0');
				AddInSessionOrder((HANDLE)hContact,lparam,0,0);	
			}
		}
		if(lparam==1)
		{
			g_ses_count++;
			DBWriteContactSettingByte(0, __INTERNAL_NAME, "UserSessionsCount", (BYTE)g_ses_count);
		}
		return 0;
	}
	else
		return 1;
}

INT_PTR SaveUserSessionHandles(WPARAM wparam,LPARAM lparam)
{
	if (g_hSDlg) 
	{
		ShowWindow(g_hSDlg,SW_SHOW);
		return 1;
	} 
	else
		g_hSDlg=CreateDialog(hinstance,MAKEINTRESOURCE(IDD_SAVEDIALOG), 0, SaveSessionDlgProc);

	return 0;
}

INT_PTR OpenSessionsManagerWindow(WPARAM wparam,LPARAM lparam)
{ 
	if (g_hDlg)
	{
		ShowWindow(g_hDlg,SW_SHOW);
		return 0;
	}
	if(g_bIncompletedSave||DBGetStringT(NULL, __INTERNAL_NAME, "SessionDate_0")||DBGetStringT(NULL, __INTERNAL_NAME, "UserSessionDsc_0")) 
	{ 
		g_hDlg=CreateDialog(hinstance,MAKEINTRESOURCE(IDD_WLCMDIALOG), 0, LoadSessionDlgProc);
		return 0;
	}
	if (g_bOtherWarnings)
		MessageBox(NULL, TranslateT("No sessions to open"), TranslateT("Sessions Manager"), MB_OK|MB_ICONWARNING);
	return 1;
}

int SaveSessionDate()
{
	if(session_list[0]!=0)
	{
		int TimeSize = GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, NULL, NULL, NULL, 0);
		TCHAR *szTimeBuf = (TCHAR*)mir_alloc((TimeSize+1)*sizeof(TCHAR));

		GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, NULL, NULL, szTimeBuf, TimeSize);

		int DateSize = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, NULL, 0);
		TCHAR *szDateBuf = (TCHAR*)mir_alloc((DateSize+1)*sizeof(TCHAR));

		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, szDateBuf, DateSize );
		TCHAR *szSessionTime = (TCHAR*)mir_alloc((DateSize+TimeSize+5)*sizeof(TCHAR));
		int lenn = (int)_tcslen(szSessionTime);
		mir_sntprintf(szSessionTime, lenn, _T("%s - %s"), szTimeBuf, szDateBuf);

		char szSessionDate[256];
		DBVARIANT  dbv = {0};
		mir_snprintf(szSessionDate, SIZEOF(szSessionDate), "%s_%d", "SessionDate", 0);
		DBGetContactSettingTString(NULL, __INTERNAL_NAME, szSessionDate, &dbv);
		TCHAR *szSessionDateBuf_1 = mir_tstrdup(dbv.ptszVal);
		DBFreeVariant(&dbv);

		DBWriteContactSettingTString(NULL, __INTERNAL_NAME, szSessionDate, szSessionTime);
		mir_free(szSessionTime);
		ResaveSettings("SessionDate", 1, ses_limit, szSessionDateBuf_1);

		if(szTimeBuf)
			mir_free(szTimeBuf);
		if(szDateBuf)
			mir_free(szDateBuf);
	}
	if(g_bCrashRecovery)
		DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastSaveCompleted", 1);
	return 0;
}

int SaveUserSessionName(TCHAR *szUSessionName)
{
	int i=0;
	DBVARIANT  dbv={0};
	char szUserSessionNameBuf[256]={0};
	TCHAR *szUserSessionNameBuf_1=NULL;
	TCHAR *szUserSessionName=NULL;
	TCHAR *ptr=NULL;

	if(session_list[0]!=0)
	{	
		szUserSessionName = mir_tstrdup(szUSessionName);
		mir_snprintf(szUserSessionNameBuf, SIZEOF(szUserSessionNameBuf), "%s_%u", "UserSessionDsc", 0);
		if (!DBGetContactSettingTString(NULL, __INTERNAL_NAME, szUserSessionNameBuf, &dbv))
		{
			szUserSessionNameBuf_1 = mir_tstrdup(dbv.ptszVal);
			DBFreeVariant(&dbv);
			ResaveSettings("UserSessionDsc",1,255,szUserSessionNameBuf_1);
		}

		DBWriteContactSettingTString(NULL, __INTERNAL_NAME, szUserSessionNameBuf, szUserSessionName);

		//free(szUserSessionNameBuf_1);
		mir_free(szUserSessionName);
		return 0;
	}

	return 1;
}

INT_PTR LoadLastSession(WPARAM wparam,LPARAM lparam)
{	
	int ret=0;
	if(isLastTRUE)
		ret=LoadSession(wparam,lparam);
	else if (g_bOtherWarnings) 
		MessageBox(NULL, TranslateT("Last Sessions is empty"), TranslateT("Sessions Manager"), MB_OK);
	return ret;
}

int LoadSession(WPARAM wparam,LPARAM lparam)
{	 
	int dup=0;
	int hidden[255]={'0'};
	HANDLE hContact;
	DWORD session_list_t[255]={0};
	int mode=0;
	int i=0,j=0;
	if ((UINT)lparam>=ses_limit&&lparam!=256) 
	{
		mode=1;
		lparam-=ses_limit;
	}
	if(session_list_recovered[0]&&lparam==256&&mode==0)
		 memcpy(session_list_t,session_list_recovered,SIZEOF(session_list_t));
	else
		for (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact;
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
		{		
			if(LoadContactsFromMask(hContact,mode,lparam))
			{
				i=GetInSessionOrder(hContact,mode, lparam);
				session_list_t[i]=(DWORD)hContact;
			}
		}
	i=0;
	//TODO: change to "switch"
	while(session_list_t[i]!=0)
	{
		if (CheckForDuplicate(session_list,session_list_t[i])==-1)	
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)session_list_t[i], 0);
		else if(g_bWarnOnHidden)
		{ 
			if (!CheckContactVisibility((HANDLE)session_list_t[i]))
			{
				hidden[j]=i+1;
				j++;
			}
			dup++;
		}
		i++;
	}

	if(i==0)
	{ 
		if(g_bOtherWarnings)
			MessageBox(NULL, TranslateT("No contacts to open"), TranslateT("Sessions Manager"), MB_OK|MB_ICONWARNING);
		return 1;
	}
	else if(dup==i)
	{
		if (!hidden)
		{  
			if(g_bOtherWarnings)
				MessageBox(NULL, TranslateT("This Session already opened"), TranslateT("Sessions Manager"), MB_OK|MB_ICONWARNING);
			return 1;
		}
		else if (!g_bWarnOnHidden&&g_bOtherWarnings)
		{
			MessageBox(NULL, TranslateT("This Session already opened"), TranslateT("Sessions Manager"), MB_OK|MB_ICONWARNING);
			return 1;
		}
		else if(g_bWarnOnHidden)
		{
			if(MessageBox(NULL, TranslateT("This Session already opened(but probably hidden)\nDo you want to show hidden contacts?"), TranslateT("Sessions Manager"), MB_YESNO|MB_ICONQUESTION)==IDYES)
			{
				j=0;
				while(hidden[j]!=0)
				{
					CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)session_list_t[hidden[j]-1], 0);
					j++;
				}
			}
		}
	}

	return 0;
}

int DelUserDefSession(int ses_count)
{
	int i=0;
	HANDLE hContact;
	char szSessionName[256]={0};
	TCHAR *szSessionNameBuf=NULL;

	for (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{		
		RemoveSessionMark(hContact,1,ses_count);
		SetInSessionOrder(hContact,1,ses_count,0);
	}

	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "UserSessionDsc", ses_count);
	DBDeleteContactSetting(NULL, __INTERNAL_NAME, szSessionName);
	
	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "FavUserSession", ses_count);
	DBDeleteContactSetting(NULL, __INTERNAL_NAME, szSessionName);

	for (i=(ses_count+1);;i++)
	{		
		mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "UserSessionDsc", i);

		if ((szSessionNameBuf=DBGetStringT(NULL, __INTERNAL_NAME, szSessionName))/*&&(szSessionHandlesBuf=DBGetStringT(NULL,PLGNAME,szSessionHandles))*/)
		{
			MarkUserDefSession(i-1,IsMarkedUserDefSession(i));
			mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "UserSessionDsc", i-1);
			DBWriteContactSettingTString(NULL, __INTERNAL_NAME, szSessionName, szSessionNameBuf);
		}
		else
		{
			mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "UserSessionDsc", i-1);
			DBDeleteContactSetting(NULL, __INTERNAL_NAME, szSessionName);

			mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "FavUserSession", i-1);
			DBDeleteContactSetting(NULL, __INTERNAL_NAME, szSessionName);
			break;
		}
	}
	g_ses_count--;
	DBWriteContactSettingByte(0, __INTERNAL_NAME, "UserSessionsCount", (BYTE)g_ses_count);
	mir_free(szSessionNameBuf);
	return 0;
}

int DeleteAutoSession(int ses_count)
{
	int i=0;
	HANDLE hContact;
	char szSessionName[256]={0};

	TCHAR *szSessionNameBuf=NULL;

	for (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{		
		RemoveSessionMark(hContact,0,ses_count);
		SetInSessionOrder(hContact,0,ses_count,0);
	}

	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "SessionDate", ses_count);
	DBDeleteContactSetting(NULL, __INTERNAL_NAME, szSessionName);

	for (i=(ses_count+1);;i++)
	{		
		mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "SessionDate", i);

		if ((szSessionNameBuf=DBGetStringT(NULL, __INTERNAL_NAME, szSessionName))/*&&(szSessionHandlesBuf=DBGetStringT(NULL,PLGNAME,szSessionHandles))*/)
		{
			mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "SessionDate", i-1);
			DBWriteContactSettingTString(NULL, __INTERNAL_NAME, szSessionName, szSessionNameBuf);

		}
		else
		{
			mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "SessionDate", i-1);
			DBDeleteContactSetting(NULL, __INTERNAL_NAME, szSessionName);
			break;
		}
	}
	mir_free(szSessionNameBuf);
	return 0;
}

int SessionPreShutdown(WPARAM wparam,LPARAM lparam)
{	 
	DONT=1;
	if (hEventDbWindowEvent)		UnhookEvent(hEventDbWindowEvent);
	DestroyServiceFunction(hServiceOpenManager);
	DestroyServiceFunction(hServiceCloseCurrentSession);
	DestroyServiceFunction(hServiceShowFavMenu);
	DestroyServiceFunction(hServiceLoadLastSession);
	DestroyServiceFunction(hServiceSaveUserSession);

	if (hEventDbPluginsLoaded)		UnhookEvent(hEventDbPluginsLoaded);

	if (hEventDbOptionsInit)		UnhookEvent(hEventDbOptionsInit);
	if (hEventDbOkToExit)			UnhookEvent(hEventDbOkToExit);
	if (hEventDbPreShutdown)		UnhookEvent(hEventDbPreShutdown);

	if (g_hDlg)						DestroyWindow(g_hDlg);
	if (g_hSDlg)					DestroyWindow(g_hSDlg);
	
	DestroyIcon(hiSessions);
	DestroyIcon(hiSessionsLoad);
	DestroyIcon(hiSessionsSave);
	DestroyIcon(hiSessionsLoadLast);
	DestroyIcon(hiChecked);
	DestroyIcon(hiNotChecked);

	if(g_bIncompletedSave)
	{
		int i=0;
		while(session_list_recovered[i])
		{
			DBWriteContactSettingByte((HANDLE)session_list_recovered[i], __INTERNAL_NAME, "wasInLastSession", 0);
			i++;
		}
	}

	DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastSaveCompleted", 1);
	return 0;
}

int OkToExit(WPARAM wparam,LPARAM lparam)
{	
	int exitmode=DBGetContactSettingByte(NULL, __INTERNAL_NAME, "ShutdownMode", 2);
	DONT=1;
	if(exitmode==2&&session_list[0]!=0)
	{
		SaveSessionDate();
		SaveSessionHandles(0,0);
		DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastempty", 0);
	}
	else if(exitmode==1&&session_list[0]!=0)
	{
		DialogBox(hinstance,MAKEINTRESOURCE(IDD_EXDIALOG), 0, ExitDlgProc);
	}
	else   DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastempty", 1);
	return 0;
}

static int GetContactHandle(WPARAM wparam,LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;

	if(MWeventdata->uType == MSG_WINDOW_EVT_OPEN)
	{
		if (strstr(MWeventdata->szModule,"tabSRMsg")) g_mode=1;
		AddToCurSession((DWORD)MWeventdata->hContact,0);
		if(g_bCrashRecovery) DBWriteContactSettingByte(MWeventdata->hContact, __INTERNAL_NAME, "wasInLastSession", 1);
	}
	else if(MWeventdata->uType == MSG_WINDOW_EVT_CLOSE)
	{
		if (!DONT) DelFromCurSession((DWORD)MWeventdata->hContact,0);
		if(g_bCrashRecovery) DBWriteContactSettingByte(MWeventdata->hContact, __INTERNAL_NAME, "wasInLastSession", 0);
	}

	return 0;
}

INT_PTR BuildFavMenu(WPARAM wparam,LPARAM lparam)
{
	HMENU hMenu=NULL;
	POINT pt;
	int res=0;
	hMenu = CreatePopupMenu();
	GetCursorPos(&pt);
	FillFavoritesMenu(hMenu,g_ses_count);
	res = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_NONOTIFY, pt.x, pt.y, 0, GetActiveWindow(), NULL);
	if(res==0) return 1;
	LoadSession(0,(res-1)+ses_limit);
	return 0;
}

static int PluginInit(WPARAM wparam,LPARAM lparam)
{
	int startup=0;

	hEventDbWindowEvent=HookEvent(ME_MSG_WINDOWEVENT, GetContactHandle);
	hEventDbOptionsInit=HookEvent(ME_OPT_INITIALISE, OptionsInit);

	hServiceShowFavMenu=CreateServiceFunction(MS_SESSIONS_SHOWFAVORITESMENU, BuildFavMenu);
	hServiceOpenManager=CreateServiceFunction(MS_SESSIONS_OPENMANAGER, OpenSessionsManagerWindow);
	hServiceLoadLastSession=CreateServiceFunction(MS_SESSIONS_RESTORELASTSESSION, LoadLastSession/*LoadSession*/);
	hServiceSaveUserSession=CreateServiceFunction(MS_SESSIONS_SAVEUSERSESSION, SaveUserSessionHandles);
	hServiceCloseCurrentSession=CreateServiceFunction(MS_SESSIONS_CLOSESESSION, CloseCurrentSession);

	g_ses_count=DBGetContactSettingByte(0, __INTERNAL_NAME, "UserSessionsCount", 0);
	if (!g_ses_count)  g_ses_count=DBGetContactSettingByte(0, "Sessions (Unicode)", "UserSessionsCount", 0);
	ses_limit=DBGetContactSettingByte(0, __INTERNAL_NAME, "TrackCount", 10);
	g_bExclHidden=DBGetContactSettingByte(NULL, __INTERNAL_NAME, "ExclHidden", 0);	
	g_bWarnOnHidden=DBGetContactSettingByte(NULL, __INTERNAL_NAME, "WarnOnHidden", 0);
	g_bOtherWarnings=DBGetContactSettingByte(NULL, __INTERNAL_NAME, "OtherWarnings", 1);
	g_bCrashRecovery=DBGetContactSettingByte(NULL, __INTERNAL_NAME, "CrashRecovery", 0);


	if(g_bCrashRecovery)
		g_bIncompletedSave=!DBGetContactSettingByte(NULL, __INTERNAL_NAME, "lastSaveCompleted", 0);

	if(g_bIncompletedSave)
	{
		int i=0;
		HANDLE hContact;
		ZeroMemory(session_list_recovered,255);
	
		for (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact;
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
		{
			if(DBGetContactSettingByte(hContact, __INTERNAL_NAME, "wasInLastSession", 0))
				session_list_recovered[i++]=(DWORD)hContact;
		}
	}
	if (!session_list_recovered[0]) g_bIncompletedSave=FALSE;
	DBWriteContactSettingByte(NULL, __INTERNAL_NAME, "lastSaveCompleted", 0);

	if (!DBGetContactSettingByte(NULL, __INTERNAL_NAME, "lastempty", 1)||g_bIncompletedSave) isLastTRUE=TRUE;

	startup=DBGetContactSettingByte(NULL, __INTERNAL_NAME, "StartupMode", 3);

	if (startup==1||(startup==3&&isLastTRUE==TRUE))
	{ 
		StartUp=TRUE;
		g_hDlg=CreateDialog(hinstance,MAKEINTRESOURCE(IDD_WLCMDIALOG), 0, LoadSessionDlgProc);
	}
	else if (startup==2&&isLastTRUE==TRUE)
	{
		g_hghostw=TRUE;
		g_hDlg=CreateDialog(hinstance,MAKEINTRESOURCE(IDD_WLCMDIALOG), 0, LoadSessionDlgProc);
	}

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.dwFlags = HKD_TCHAR;
	hkd.ptszSection = _T("Sessions");
	hkd.pszName = "OpenSessionsManager";
	hkd.ptszDescription = _T("Open Sessions Manager");
	hkd.pszService = MS_SESSIONS_OPENMANAGER;
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "RestoreLastSession";
	hkd.ptszDescription = _T("Restore last Session");
	hkd.pszService = MS_SESSIONS_RESTORELASTSESSION;
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "SaveSession";
	hkd.ptszDescription = _T("Save Session");
	hkd.pszService = MS_SESSIONS_SAVEUSERSESSION;
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "CloseSession";
	hkd.ptszDescription = _T("Close Session");
	hkd.pszService = MS_SESSIONS_CLOSESESSION;
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hiChecked	   = LoadIcon(hinstance, MAKEINTRESOURCE(IDD_SESSION_CHECKED));
	hiNotChecked   = LoadIcon(hinstance, MAKEINTRESOURCE(IDD_SESSION_UNCHECKED));

	hiSessions	   = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_SESSIONS));
	hiSessionsLoad = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_SESSIONS_LOAD));
	hiSessionsSave = LoadIcon(hinstance, MAKEINTRESOURCE(IDD_SESSIONS_SAVE));
	hiSessionsLoadLast = LoadIcon(hinstance, MAKEINTRESOURCE(IDD_SESSIONS_LOADLAST));

	hibChecked	  =(HANDLE)AddIcon(hiChecked, "SessionMarked", _T("Favorite Session"));
	hibNotChecked =(HANDLE)AddIcon(hiNotChecked, "SessionUnMarked", _T("Not favorite Session"));

	hibSessionsLoad		=(HANDLE)AddIcon(hiSessionsLoad, "SessionsLoad", _T("Load Session"));
	hibSessionsSave		=(HANDLE)AddIcon(hiSessionsSave, "SessionsSave", _T("Save Session"));
	hibSessionsLoadLast	=(HANDLE)AddIcon(hiSessionsLoadLast, "SessionsLoadLast", _T("Load last Session"));

	CLISTMENUITEM cl;
	ZeroMemory(&cl, sizeof(cl));
	cl.cbSize = sizeof(CLISTMENUITEM);
	cl.position = 1000000000;
	cl.flags = CMIM_ALL | CMIF_TCHAR;

	cl.ptszName = _T("Save session...");
	cl.ptszPopupName = _T("Sessions Manager");
	cl.hIcon = hiSessions;
	cl.pszService = MS_SESSIONS_SAVEUSERSESSION;
	hmSaveCurrentSession = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &cl);

	cl.ptszName = _T("Load session...");
	cl.pszService = MS_SESSIONS_OPENMANAGER;
	cl.hIcon = hiSessionsLoad;
	hmLoadLastSession = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &cl);

	cl.ptszName = _T("Close session");
	cl.pszService = MS_SESSIONS_CLOSESESSION;
	cl.hIcon = 0;
	hmLoadSession = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &cl);

	cl.ptszName = _T("Load last session");
	cl.pszService = MS_SESSIONS_RESTORELASTSESSION;
	cl.hIcon = hiSessionsLoadLast;
	cl.position = 10100000;
	hmLoadSession = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &cl);

	ZeroMemory(&cl, sizeof(cl));
	cl.cbSize = sizeof(cl);
	cl.flags = CMIM_ICON;
	cl.hIcon = hiSessionsSave;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmSaveCurrentSession, (LPARAM)&cl);

	if (ServiceExists(MS_TB_ADDBUTTON))
	{
		TBButton button = {0}; 
		button.defPos = 102200; 
		button.tbbFlags = TBBF_SHOWTOOLTIP|TBBF_VISIBLE; 

		button.pszButtonID = MS_SESSIONS_OPENMANAGER;
		button.pszServiceName = MS_SESSIONS_OPENMANAGER; 
		button.pszTooltipUp = button.pszTooltipUp = button.pszButtonName = "Open Sessions Manager";
		button.hSecondaryIconHandle = button.hPrimaryIconHandle = hibSessionsLoad; 
		CallService(MS_TB_ADDBUTTON, 0, (LPARAM)&button); 
		button.defPos++; 

		button.pszButtonID = MS_SESSIONS_SAVEUSERSESSION;
		button.pszServiceName = MS_SESSIONS_SAVEUSERSESSION; 
		button.pszTooltipUp = button.pszTooltipUp = button.pszButtonName = "Save Session";
		button.hSecondaryIconHandle = button.hPrimaryIconHandle = hibSessionsSave; 
		CallService(MS_TB_ADDBUTTON, 0, (LPARAM)&button); 
		button.defPos++; 

		button.pszButtonID = MS_SESSIONS_RESTORELASTSESSION;
		button.pszServiceName = MS_SESSIONS_RESTORELASTSESSION; 
		button.pszTooltipUp = button.pszTooltipUp = button.pszButtonName = "Restore Last Session";
		button.hSecondaryIconHandle = button.hPrimaryIconHandle = hibSessionsLoadLast; 
		CallService(MS_TB_ADDBUTTON, 0, (LPARAM)&button); 
		button.defPos++; 
	
		button.pszButtonID = MS_SESSIONS_SHOWFAVORITESMENU;
		button.pszServiceName = MS_SESSIONS_SHOWFAVORITESMENU; 
		button.pszTooltipUp = button.pszTooltipUp = button.pszButtonName = "Show Favorite Sessions Menu";
		button.hSecondaryIconHandle = button.hPrimaryIconHandle = hibChecked; 
		CallService(MS_TB_ADDBUTTON, 0, (LPARAM)&button); 
	}

	if(ServiceExists(MS_UPDATE_REGISTER))
	{
		char buffer[1024];
		Update update = {0};
		update.cbSize = sizeof(Update);
		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *) CreateVersionString(pluginInfo.version, buffer);
		update.cpbVersion = (int)strlen((char *) update.pbVersion);
		//update.szUpdateURL = UPDATER_AUTOREGISTER;
		update.szBetaVersionURL = SESSIONS_VERSION_URL;
		update.szBetaUpdateURL = SESSIONS_UPDATE_URL;
		update.pbBetaVersionPrefix = (BYTE *) SESSIONS_VERSION_PREFIX;
		update.cpbBetaVersionPrefix = (int)strlen(SESSIONS_VERSION_PREFIX);
		CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);
	}

	return 0;
}

// {28EE6FE2-B005-4073-BA90-148203C807D0}
static const MUUID interfaces[] = {{ 0x28ee6fe2, 0xb005, 0x4073, { 0xba, 0x90, 0x14, 0x82, 0x3, 0xc8, 0x7, 0xd0 } }, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 9, 0, 0))
		return NULL;

	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hinstance=hinst;
	return 1;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{	
	pluginLink=link;
	mir_getMMI(&mmi);
	mir_getLP(&pluginInfo);

	hEventDbPluginsLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,PluginInit);
	hEventDbOkToExit=HookEvent(ME_SYSTEM_OKTOEXIT,OkToExit);
	hEventDbPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,SessionPreShutdown);
	return 0;
}