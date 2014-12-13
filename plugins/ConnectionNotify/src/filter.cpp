
#include <Windows.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_langpack.h>

#include "resource.h"
#include "netstat.h"
#include "Filter.h"
#include "ConnectionNotify.h"

HWND filterAddDlg=NULL;
extern HINSTANCE hInst;
extern struct CONNECTION *connExceptions;
extern HANDLE hFilterOptionsThread;
extern DWORD FilterOptionsThreadId;
extern struct CONNECTION *connCurrentEdit;
extern BOOL settingDefaultAction;
extern HANDLE hExceptionsMutex;
extern BOOL bOptionsOpen;
static unsigned __stdcall filterQueue(void *dummy);
static INT_PTR CALLBACK ConnectionFilterEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HANDLE startFilterThread()
{
	return (HANDLE)mir_forkthreadex(filterQueue, 0, (unsigned int*)&FilterOptionsThreadId);
}

static unsigned __stdcall filterQueue(void *dummy)
{
	BOOL bRet;
	MSG msg;
	//while(1)
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{
		if (msg.message==WM_ADD_FILTER )
		{
			struct CONNECTION *conn=(struct CONNECTION *)msg.lParam;
			filterAddDlg=CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILTER_DIALOG), NULL, ConnectionFilterEditProc,(LPARAM)conn);
			ShowWindow(filterAddDlg,SW_SHOW);

		}
		if (NULL == filterAddDlg || !IsDialogMessage(filterAddDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	hFilterOptionsThread=NULL;
	return TRUE;
}

static INT_PTR CALLBACK ConnectionFilterEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
		case WM_INITDIALOG:
			{
			struct CONNECTION *conn=(struct CONNECTION*)lParam;
			TranslateDialogDefault(hWnd);
			
			SetDlgItemText(hWnd, ID_TEXT_NAME,conn->PName);
			SetDlgItemText(hWnd, ID_TXT_LOCAL_IP,conn->strIntIp);
			SetDlgItemText(hWnd, ID_TXT_REMOTE_IP,conn->strExtIp);
			SetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT,conn->intIntPort,FALSE);
			SetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT,conn->intExtPort,FALSE);
			SendDlgItemMessage(hWnd, ID_CBO_ACTION , CB_ADDSTRING, 0, (LPARAM)TranslateT("Always show popup"));
			SendDlgItemMessage(hWnd, ID_CBO_ACTION , CB_ADDSTRING, 0, (LPARAM)TranslateT("Never show popup"));
			SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_SETCURSEL ,0,0);
			mir_free(conn);
			return TRUE;
			}
		case WM_ACTIVATE:
            if (0 == wParam)             // becoming inactive
               filterAddDlg = NULL;
            else                         // becoming active
               filterAddDlg = hWnd;
            return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
				{
				case ID_OK:
					{
					TCHAR tmpPort[6];
					if (bOptionsOpen)
					{
						MessageBox(hWnd,TranslateT("First close options window"),_T("ConnectionNotify"),MB_OK | MB_ICONSTOP);
						break;
					}
						if ( WAIT_OBJECT_0 == WaitForSingleObject( hExceptionsMutex, 100 ))
						{
							if (connCurrentEdit==NULL)
							{
								connCurrentEdit=(struct CONNECTION*)mir_alloc(sizeof(struct CONNECTION));
								connCurrentEdit->next=connExceptions;
								connExceptions=connCurrentEdit;
							}
							GetDlgItemText(hWnd, ID_TXT_LOCAL_PORT, tmpPort, SIZEOF(tmpPort));
							if (tmpPort[0]=='*')
								connCurrentEdit->intIntPort=-1;
							else
								connCurrentEdit->intIntPort=GetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT, NULL, FALSE);
							GetDlgItemText(hWnd, ID_TXT_REMOTE_PORT, tmpPort, SIZEOF(tmpPort));
							if (tmpPort[0]=='*')
								connCurrentEdit->intExtPort=-1;
							else
								connCurrentEdit->intExtPort = GetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT, NULL, FALSE);
					
							GetDlgItemText(hWnd, ID_TXT_LOCAL_IP, connCurrentEdit->strIntIp, SIZEOF(connCurrentEdit->strIntIp));
							GetDlgItemText(hWnd, ID_TXT_REMOTE_IP, connCurrentEdit->strExtIp, SIZEOF(connCurrentEdit->strExtIp));
							GetDlgItemText(hWnd, ID_TEXT_NAME, connCurrentEdit->PName, SIZEOF(connCurrentEdit->PName));

							connCurrentEdit->Pid = !(BOOL)SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_GETCURSEL, 0, 0);
							connCurrentEdit = NULL;
							saveSettingsConnections(connExceptions);
							ReleaseMutex(hExceptionsMutex);
					}
					//EndDialog(hWnd,IDOK);
					DestroyWindow(hWnd);
					return TRUE;
					}
				case ID_CANCEL:
					connCurrentEdit=NULL;
					DestroyWindow(hWnd);
					//EndDialog(hWnd,IDCANCEL);
					return TRUE;
				}
			return FALSE;

			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
		case WM_DESTROY:
			filterAddDlg=NULL;
			connCurrentEdit=NULL;
			//DestroyWindow(hWnd);
			//PostQuitMessage(0);
			break;
	}
	return FALSE;
}

BOOL checkFilter(struct CONNECTION *head,struct CONNECTION *conn)
{
	struct CONNECTION *cur=head;
	while(cur!=NULL)
	{
		if (wildcmp(cur->PName,conn->PName)&&wildcmp(cur->strIntIp,conn->strIntIp)&&wildcmp(cur->strExtIp,conn->strExtIp)&&(cur->intIntPort==-1||cur->intIntPort==conn->intIntPort)&&(cur->intExtPort==-1||cur->intExtPort==conn->intExtPort))
			return cur->Pid;
		cur=cur->next;
	}
	return settingDefaultAction;
}

