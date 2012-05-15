/*
 * This code implements window handling (connection error)
 *
 * (c) majvan 2002,2004
 */
#include <windows.h>
#include <stdio.h>
#include <newpluginapi.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_popup.h>
#include "../main.h"
#include "m_protoplugin.h"
#include "m_account.h"
#include "../debug.h"
#include "m_messages.h"
#include "../mails/m_mails.h"
#include "m_yamn.h"
#include "../resources/resource.h"
#include "m_browser.h"
#include <win2k.h>

#define BADCONNECTTITLE "%s - connection error"
#define BADCONNECTMSG "An error occured. Error code: %d"

//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

extern YAMN_VARIABLES YAMNVar;

//From synchro.cpp
extern DWORD WINAPI WaitToWriteFcn(PSWMRG SObject,PSCOUNTER SCounter=NULL);
extern void WINAPI WriteDoneFcn(PSWMRG SObject,PSCOUNTER SCounter=NULL);
extern DWORD WINAPI WaitToReadFcn(PSWMRG SObject);
extern void WINAPI ReadDoneFcn(PSWMRG SObject);
extern DWORD WINAPI SCIncFcn(PSCOUNTER SCounter);
extern DWORD WINAPI SCDecFcn(PSCOUNTER SCounter);


extern HICON hYamnIcons[];

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//Window callback procedure for popup window (created by popup plugin)
LRESULT CALLBACK BadConnectPopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) ;

//Dialog callback procedure for bad connection message
LRESULT CALLBACK DlgProcYAMNBadConnection(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);

//BadConnection thread function creates window for BadConnection message
DWORD WINAPI BadConnection(LPVOID Param);

INT_PTR RunBadConnectionSvc(WPARAM wParam,LPARAM lParam);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

LRESULT CALLBACK BadConnectPopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	DWORD PluginParam;
	switch(msg)
	{
		case WM_COMMAND:
			if((HIWORD(wParam)==STN_CLICKED) && (CallService(MS_POPUP_GETPLUGINDATA,(WPARAM)hWnd,(LPARAM)&PluginParam)))	//if clicked and it's new mail popup window
			{
				PROCESS_INFORMATION pi;
				STARTUPINFOW si;
				HACCOUNT ActualAccount;

				ZeroMemory(&si,sizeof(si));
				si.cb=sizeof(si);
				ActualAccount=(HACCOUNT)CallService(MS_POPUP_GETCONTACT,(WPARAM)hWnd,(LPARAM)0);
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"PopUpProc:LEFTCLICK:ActualAccountSO-read wait\n");
#endif
				if(WAIT_OBJECT_0==WaitToReadFcn(ActualAccount->AccountAccessSO))
				{
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"PopUpProc:LEFTCLICK:ActualAccountSO-read enter\n");
#endif
					if(ActualAccount->BadConnectN.App!=NULL)
					{
						WCHAR *Command;
						if(ActualAccount->BadConnectN.AppParam!=NULL)
							Command=new WCHAR[wcslen(ActualAccount->BadConnectN.App)+wcslen(ActualAccount->BadConnectN.AppParam)+6];
						else
							Command=new WCHAR[wcslen(ActualAccount->BadConnectN.App)+6];
	
						if(Command!=NULL)
						{
							lstrcpyW(Command,L"\"");
							lstrcatW(Command,ActualAccount->BadConnectN.App);
							lstrcatW(Command,L"\" ");
							if(ActualAccount->BadConnectN.AppParam!=NULL)
								lstrcatW(Command,ActualAccount->BadConnectN.AppParam);
							CreateProcessW(NULL,Command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
							delete[] Command;
						}
					}
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"PopUpProc:LEFTCLICK:ActualAccountSO-read done\n");
#endif
					ReadDoneFcn(ActualAccount->AccountAccessSO);
				}
#ifdef DEBUG_SYNCHRO
				else
					DebugLog(SynchroFile,"PopUpProc:LEFTCLICK:ActualAccountSO-read enter failed\n");
#endif
				SendMessage(hWnd,UM_DESTROYPOPUP,0,0);
			}
			break;
		case UM_FREEPLUGINDATA:
			//Here we'd free our own data, if we had it.
			return FALSE;
		case UM_INITPOPUP:
			//This is the equivalent to WM_INITDIALOG you'd get if you were the maker of dialog popups.
			break;
		case WM_CONTEXTMENU:
			SendMessage(hWnd,UM_DESTROYPOPUP,0,0);
			break;
		case WM_NOTIFY:
/*			switch(((LPNMHDR)lParam)->code)
			{
				case NM_CLICK:
				{
				}
			}
			break;
*/		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

LRESULT CALLBACK DlgProcYAMNBadConnection(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			BOOL ShowPopUp,ShowMsg,ShowIco;
			HACCOUNT ActualAccount;
			DWORD ErrorCode;
			char *TitleStrA;
			char *Message1A=NULL;
			WCHAR *Message1W=NULL;
			POPUPDATAEX BadConnectPopUp;

			ActualAccount=((struct BadConnectionParam *)lParam)->account;
			ErrorCode=((struct BadConnectionParam *)lParam)->errcode;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read wait\n");
#endif
			if(WAIT_OBJECT_0!=WaitToReadFcn(ActualAccount->AccountAccessSO))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read wait failed\n");
#endif
				return FALSE;
			}
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read enter\n");
#endif
			TitleStrA=new char[strlen(ActualAccount->Name)+strlen(Translate(BADCONNECTTITLE))];
			sprintf(TitleStrA,Translate(BADCONNECTTITLE),ActualAccount->Name);

			ShowPopUp=ActualAccount->BadConnectN.Flags & YAMN_ACC_POP;
			ShowMsg=ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG;
			ShowIco=ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO;

			if(ShowPopUp)
			{
				BadConnectPopUp.lchContact=ActualAccount;
				BadConnectPopUp.lchIcon=hYamnIcons[3];
				BadConnectPopUp.colorBack=ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC ? ActualAccount->BadConnectN.PopUpB : GetSysColor(COLOR_BTNFACE);
				BadConnectPopUp.colorText=ActualAccount->BadConnectN.Flags & YAMN_ACC_POPC ? ActualAccount->BadConnectN.PopUpT : GetSysColor(COLOR_WINDOWTEXT);
				BadConnectPopUp.iSeconds=ActualAccount->BadConnectN.PopUpTime;

				BadConnectPopUp.PluginWindowProc=(WNDPROC)BadConnectPopUpProc;
				BadConnectPopUp.PluginData=0;					//it's bad connect popup
				lstrcpyn(BadConnectPopUp.lpzContactName,ActualAccount->Name,sizeof(BadConnectPopUp.lpzContactName));
			}

			if(ActualAccount->Plugin->Fcn!=NULL && ActualAccount->Plugin->Fcn->GetErrorStringWFcnPtr!=NULL)
			{
				Message1W=ActualAccount->Plugin->Fcn->GetErrorStringWFcnPtr(ErrorCode);
				SendMessageW(GetDlgItem(hDlg,IDC_STATICMSG),WM_SETTEXT,(WPARAM)0,(LPARAM)Message1W);
				WideCharToMultiByte(CP_ACP,0,Message1W,-1,(char *)BadConnectPopUp.lpzText,sizeof(BadConnectPopUp.lpzText),NULL,NULL);
				if(ShowPopUp)
					CallService(MS_POPUP_ADDPOPUPEX,(WPARAM)&BadConnectPopUp,0);
			}
			else if(ActualAccount->Plugin->Fcn!=NULL && ActualAccount->Plugin->Fcn->GetErrorStringAFcnPtr!=NULL)
			{
				Message1A=ActualAccount->Plugin->Fcn->GetErrorStringAFcnPtr(ErrorCode);
				SendMessageA(GetDlgItem(hDlg,IDC_STATICMSG),WM_SETTEXT,(WPARAM)0,(LPARAM)Message1A);
				lstrcpyn(BadConnectPopUp.lpzText,Message1A,sizeof(BadConnectPopUp.lpzText));
				if(ShowPopUp)
					CallService(MS_POPUP_ADDPOPUPEX,(WPARAM)&BadConnectPopUp,0);
			}
			else
			{
				Message1A=Translate("Unknown error");
				SendMessageA(GetDlgItem(hDlg,IDC_STATICMSG),WM_SETTEXT,(WPARAM)0,(LPARAM)Message1A);
				lstrcpyn(BadConnectPopUp.lpzText,Message1A,sizeof(BadConnectPopUp.lpzText));
				if(ShowPopUp)
					CallService(MS_POPUP_ADDPOPUPEX,(WPARAM)&BadConnectPopUp,0);
			}

			if(!ShowMsg && !ShowIco)
				DestroyWindow(hDlg);
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read done\n");
#endif
			ReadDoneFcn(ActualAccount->AccountAccessSO);

			SendMessage(hDlg,WM_SETTEXT,(WPARAM)0,(LPARAM)TitleStrA);
			delete[] TitleStrA;
			if(Message1A!=NULL)
				delete[] Message1A;
			if(ActualAccount->Plugin->Fcn!=NULL && ActualAccount->Plugin->Fcn->DeleteErrorStringFcnPtr!=NULL && Message1A!=NULL)
				ActualAccount->Plugin->Fcn->DeleteErrorStringFcnPtr(Message1A);
			if(ActualAccount->Plugin->Fcn!=NULL && ActualAccount->Plugin->Fcn->DeleteErrorStringFcnPtr!=NULL && Message1W!=NULL)
				ActualAccount->Plugin->Fcn->DeleteErrorStringFcnPtr(Message1W);
			return 0;
		}
		case WM_DESTROY:
		{
			NOTIFYICONDATA nid;

			ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
			nid.cbSize=sizeof(NOTIFYICONDATA);
			nid.hWnd=hDlg;
			nid.uID=0;
			Shell_NotifyIcon(NIM_DELETE,&nid);
			PostQuitMessage(0);
			break;
		}
		case WM_YAMN_NOTIFYICON:
			switch (lParam)
			{
				case WM_LBUTTONDBLCLK:
				          ShowWindow(hDlg,SW_SHOWNORMAL);
				          SetForegroundWindow(hDlg);
				          break;
			}
			return 0;
		case WM_CHAR:
			switch((TCHAR)wParam)
			{
				case 27:
				case 13:
					DestroyWindow(hDlg);
					break;
			}
			break;
		case WM_SYSCOMMAND:
			switch(wParam)
			{
				case SC_CLOSE:
					DestroyWindow(hDlg);
					break;
			}
		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);
			switch(LOWORD(wParam))
			{
				case IDC_BTNOK:
					DestroyWindow(hDlg);
					break;
			}
			break;
		}
	}
	return 0;
}

DWORD WINAPI BadConnection(LPVOID Param)
{
	MSG msg;
	HWND hBadConnect;
	HACCOUNT ActualAccount;
	struct BadConnectionParam MyParam;
	NOTIFYICONDATA nid;
	TCHAR *NotIconText=Translate(" - connection error");
	TCHAR *src,*dest;
	int i;

	MyParam=*(struct BadConnectionParam *)Param;
	ActualAccount=MyParam.account;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"BadConnect:Incrementing \"using threads\" %x (account %x)\n",ActualAccount->UsingThreads,ActualAccount);
#endif
	SCIncFcn(ActualAccount->UsingThreads);

//	we will not use params in stack anymore
	SetEvent(MyParam.ThreadRunningEV);

	__try
	{
		hBadConnect=CreateDialogParam(YAMNVar.hInst,MAKEINTRESOURCE(IDD_DLGBADCONNECT),NULL,(DLGPROC)DlgProcYAMNBadConnection,(LPARAM)&MyParam);
		SendMessage(hBadConnect,WM_SETICON,ICON_BIG,(LPARAM)hYamnIcons[3]);
		SendMessage(hBadConnect,WM_SETICON,ICON_SMALL,(LPARAM)hYamnIcons[3]);

		ZeroMemory(&nid,sizeof(nid));
		nid.cbSize=sizeof(NOTIFYICONDATA);
		nid.hWnd=hBadConnect;
		nid.hIcon=hYamnIcons[3];
		nid.uID=0;
		nid.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage=WM_YAMN_NOTIFYICON;

#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read wait\n");
#endif
		if(WAIT_OBJECT_0!=WaitToReadFcn(ActualAccount->AccountAccessSO))
		{
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read wait failed\n");
#endif
			return 0;
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read enter\n");
#endif
		for(src=ActualAccount->Name,dest=nid.szTip,i=0;(*src!=(TCHAR)0) && (i+1<sizeof(nid.szTip));*dest++=*src++);
		for(src=NotIconText;(*src!=(TCHAR)0) && (i+1<sizeof(nid.szTip));*dest++=*src++);
		*dest=(TCHAR)0;

		if(ActualAccount->BadConnectN.Flags & YAMN_ACC_SND)
			CallService(MS_SKIN_PLAYSOUND,0,(LPARAM)YAMN_CONNECTFAILSOUND);
		if(ActualAccount->BadConnectN.Flags & YAMN_ACC_MSG)
			ShowWindow(hBadConnect,SW_SHOWNORMAL);
		if(ActualAccount->BadConnectN.Flags & YAMN_ACC_ICO)
			Shell_NotifyIcon(NIM_ADD,&nid);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"BadConnect:ActualAccountSO-read done\n");
#endif
		ReadDoneFcn(ActualAccount->AccountAccessSO);

		UpdateWindow(hBadConnect);
		while(GetMessage(&msg,NULL,0,0))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);  
		}

//	now, write to file. Why? Because we want to write when was new mail last checked
		if((ActualAccount->Plugin->Fcn!=NULL) && (ActualAccount->Plugin->Fcn->WriteAccountsFcnPtr!=NULL) && ActualAccount->AbleToWork)
			ActualAccount->Plugin->Fcn->WriteAccountsFcnPtr();
	}
	__finally
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"BadConnect:Decrementing \"using threads\" %x (account %x)\n",ActualAccount->UsingThreads,ActualAccount);
#endif
		SCDecFcn(ActualAccount->UsingThreads);
	}
	return 0;
}


INT_PTR RunBadConnectionSvc(WPARAM wParam,LPARAM lParam)
{
	DWORD tid;
//an event for successfull copy parameters to which point a pointer in stack for new thread
	HANDLE ThreadRunningEV;
	PYAMN_BADCONNECTIONPARAM Param=(PYAMN_BADCONNECTIONPARAM)wParam;

	if((DWORD)lParam!=YAMN_BADCONNECTIONVERSION)
		return 0;

	if(NULL!=(ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
	{
		HANDLE NewThread;

		Param->ThreadRunningEV=ThreadRunningEV;
		if(NULL!=(NewThread=CreateThread(NULL,0,BadConnection,Param,0,&tid)))
		{
			WaitForSingleObject(ThreadRunningEV,INFINITE);
			CloseHandle(NewThread);
		}
		CloseHandle(ThreadRunningEV);
			
		return 1;
	}
	return 0;
}
