/*

  Miranda IM: the free IM client for Microsoft* Windows*
  
  Copyright 2000-2003 Miranda ICQ/IM project, 
  all portions of this codebase are copyrighted to the people 
  listed in contributors.txt.
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"

#define TM_AUTOALPHA  1
#define TM_STATUSBARUPDATE  200
#define MENU_MIRANDAMENU         0xFFFF1234

extern int DefaultImageListColorDepth;

static HMODULE hUserDll;
HMENU hMenuMain;
static HANDLE hContactDraggingEvent,hContactDroppedEvent,hContactDragStopEvent;
UINT hMsgGetProfile=0;

extern boolean canloadstatusbar;
boolean OnModulesLoadedCalled=FALSE;

HANDLE hSettingChangedHook=0;

static int transparentFocus=1;
static byte oldhideoffline;
static int lastreqh=0,requr=0,disableautoupd=1;
HANDLE hFrameContactTree;
BYTE showOpts;//for statusbar

typedef struct{
int IconsCount;
int CycleStartTick;
char *szProto;
int n;
int TimerCreated;
} ProtoTicks,*pProtoTicks;

ProtoTicks CycleStartTick[64];//max 64 protocols 

int CycleTimeInterval=2000;
int CycleIconCount=8;
int DefaultStep=100;

BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);

int CluiOptInit(WPARAM wParam,LPARAM lParam);
int SortList(WPARAM wParam,LPARAM lParam);
int CluiProtocolStatusChanged(WPARAM wParam,LPARAM lParam);

extern void SetAllExtraIcons(HWND hwndList,HANDLE hContact);
extern void ReloadExtraIcons();
extern void LoadExtraImageFunc();
extern HWND CreateStatusBarhWnd(HWND parent);
extern HANDLE CreateStatusBarFrame();
extern int CLUIFramesUpdateFrame(WPARAM wParam,LPARAM lParam);
extern int ExtraToColumnNum(int extra);
extern int ColumnNumToExtra(int column);
extern void DrawDataForStatusBar(LPDRAWITEMSTRUCT dis);
extern void InitGroupMenus();
extern int UseOwnerDrawStatusBar;
extern HANDLE hExtraImageClick;

HICON GetConnectingIconForProto(char *szProto,int b);
HICON GetConnectingIconForProto_DLL(char *szProto,int b);

void RegisterProtoIconsForAllProtoIconLib();

#define M_CREATECLC  (WM_USER+1)
#define M_SETALLEXTRAICONS (WM_USER+2)

static int CluiModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize=MENUITEMINFO_V4_SIZE;
	mii.fMask=MIIM_SUBMENU;
	mii.hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	SetMenuItemInfo(hMenuMain,0,TRUE,&mii);
	mii.hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	SetMenuItemInfo(hMenuMain,1,TRUE,&mii);

	canloadstatusbar=TRUE;
	SendMessage(pcli->hwndContactList,WM_SIZE,0,0);
	CluiProtocolStatusChanged(0,0);
	Sleep(0);
	PostMessage(pcli->hwndContactList,M_CREATECLC,0,0);
	
	OnModulesLoadedCalled=TRUE;	
	pcli->pfnInvalidateDisplayNameCacheEntry(INVALID_HANDLE_VALUE);
	InitGroupMenus();
	RegisterProtoIconsForAllProtoIconLib();
	return 0;
}

pProtoTicks GetProtoTicksByProto(char * szProto)
{
	int i;

	for (i=0;i<64;i++)
	{
		if (CycleStartTick[i].szProto==NULL) break;
		if (strcmp(CycleStartTick[i].szProto,szProto)) continue;
		return(&CycleStartTick[i]);
	}
	for (i=0;i<64;i++)
	{
		if (CycleStartTick[i].szProto==NULL)
		{
			CycleStartTick[i].szProto=mir_strdup(szProto);
			CycleStartTick[i].CycleStartTick=0;
			CycleStartTick[i].n=i;
			return(&CycleStartTick[i]);
		}
	}
	return (NULL);
}

int GetConnectingIconForProtoCount(char *szProto)
{
	char file[MAX_PATH],fileFull[MAX_PATH],szFullPath[MAX_PATH];
	char szPath[MAX_PATH];
	char *str;
	int ret;

	GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
	str=strrchr(szPath,'\\');
	if(str!=NULL) *str=0;
	_snprintf(szFullPath, SIZEOF(szFullPath), "%s\\Icons\\proto_conn_%s.dll", szPath, szProto);
		
	lstrcpynA(file,szFullPath,SIZEOF(file));
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)file, (LPARAM)fileFull);
	ret=ExtractIconExA(fileFull,-1,NULL,NULL,1);
	if (ret==0&&!strcmp(szProto,"ICQ")) ret=8;
	return ret;
}

static HICON ExtractIconFromPath(const char *path)
{
	char *comma;
	char file[MAX_PATH],fileFull[MAX_PATH];
	int n;
	HICON hIcon;
	{
//		char buf[512];
//		sprintf(buf,"LoadIcon %s\r\n",path);
//		OutputDebugStringA(buf);
	}
	lstrcpynA(file,path,SIZEOF(file));
	comma=strrchr(file,',');
	if(comma==NULL) n=0;
	else {n=atoi(comma+1); *comma=0;}
   CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)file, (LPARAM)fileFull);
	
#ifdef _DEBUG
	{
//		char buf[512];
//		sprintf(buf,"LoadIconFull %d %s\r\n",n,fileFull);
//		OutputDebugStringA(buf);
	}
#endif

	hIcon=NULL;
	ExtractIconExA(fileFull,n,NULL,&hIcon,1);
	return hIcon;
}

HICON LoadIconFromExternalFile(char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx,HICON DefIcon)
{
	char szPath[MAX_PATH],szMyPath[MAX_PATH], szFullPath[MAX_PATH],*str;
	HICON hIcon=NULL;
	SKINICONDESC sid={0};

	memset(szMyPath,0,SIZEOF(szMyPath));
	memset(szFullPath,0,SIZEOF(szFullPath));

	if (filename!=NULL)
	{
		GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
		GetModuleFileNameA(g_hInst, szMyPath, MAX_PATH);
		str=strrchr(szPath,'\\');
		if(str!=NULL) *str=0;
		_snprintf(szFullPath, SIZEOF(szFullPath), "%s\\Icons\\%s,%d", szPath, filename, i);
	}

	if (!UseLibrary||!ServiceExists(MS_SKIN2_ADDICON))
	{		
		hIcon=ExtractIconFromPath(szFullPath);
		if (hIcon) return hIcon;
	}
	else
	{
		if (registerit&&IconName!=NULL&&SectName!=NULL)	
		{
			sid.cbSize = sizeof(sid);
			sid.pszSection = SectName;
			sid.pszName=IconName;
			sid.pszDescription = Description;
			if (strlen(szMyPath)!=0)
			{
				sid.pszDefaultFile=szMyPath;
			}

			sid.iDefaultIndex=internalidx;
			sid.hDefaultIcon=DefIcon;

			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			{
				char buf[256];
				sprintf(buf,"Registring Icon %s/%s hDefaultIcon: %p\r\n",SectName,IconName,DefIcon);
				OutputDebugStringA(buf);
			}
		}
		return ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IconName));
	}

	return (HICON)0;
}

void RegisterProtoIcons (char *protoname)
{
	if (ServiceExists(MS_SKIN2_GETICON)&&DBGetContactSettingByte(NULL,"CList","UseProtoIconFromIcoLib",1))
	{
		int i;
		char buf[256];
		char buf2[256];
		for (i=0;i<8;i++)
		{
			sprintf(buf,"%s #%d",protoname,i);
			sprintf(buf2,"Contact List/Connection Icons %s",protoname);
			
			LoadIconFromExternalFile(NULL,i,TRUE,TRUE,buf,buf2,buf,0,GetConnectingIconForProto_DLL(protoname,i));
		}
	}
}

void RegisterProtoIconsForAllProtoIconLib()
{
	int protoCount,i;
	PROTOACCOUNT **accs;
	
	ProtoEnumAccounts( &protoCount, &accs );
	for ( i=0; i < protoCount; i++ )
		if ( IsAccountEnabled( accs[i] ) && CallProtoService( accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0 ))
			RegisterProtoIcons( accs[i]->szModuleName ); 
}

HICON GetConnectingIconForProto_DLL(char *szProto,int b)
{
	char szFullPath[MAX_PATH];
	HICON hIcon=NULL;

	b=b-1;
	_snprintf(szFullPath, SIZEOF(szFullPath), "proto_conn_%s.dll",szProto);
	//		hIcon=ExtractIconFromPath(szFullPath);
	//		if (hIcon) return hIcon;

	hIcon=LoadIconFromExternalFile(szFullPath,b+1,FALSE,FALSE,NULL,NULL,NULL,0,0);
	if (hIcon) return hIcon;

#ifdef _DEBUG
	{
		char buf [256];
		sprintf(buf,"IconNotFound %s %d\r\n",szProto,b);
		//	OutputDebugStringA(buf);
	}
#endif

	if (!strcmp(szProto,"ICQ"))
	{

#ifdef _DEBUG
		char buf [256];
		sprintf(buf,"Icon %d %d\r\n",GetTickCount(),b);
		//OutputDebugStringA(buf);
#endif
		return(LoadIconA(g_hInst,(LPCSTR)(IDI_ICQC1+b)));
	}

	return(hIcon);
}

HICON GetConnectingIconForProto(char *szProto,int b)
{
		if (ServiceExists(MS_SKIN2_GETICON)&&DBGetContactSettingByte(NULL,"CList","UseProtoIconFromIcoLib",1))
		{
			HICON hIcon=0;
			char buf[256];
			sprintf(buf,"%s #%d",szProto,b);
			
			 hIcon=LoadIconFromExternalFile(NULL,b,TRUE,FALSE,buf,"Contact List/Connection Icons",buf,0,NULL);
			 if (hIcon==NULL) return (GetConnectingIconForProto_DLL(szProto,b));
			 return (CopyIcon(hIcon));
		
		}else
		{
			return(GetConnectingIconForProto_DLL(szProto,b));
		}
		return (NULL);
}




//wParam = szProto
INT_PTR GetConnectingIconService(WPARAM wParam,LPARAM lParam)
{
	int b;						
	ProtoTicks *pt=NULL;
	HICON hIcon=NULL;

	char *szProto=(char *)wParam;
	if (!szProto) return 0;

	pt=GetProtoTicksByProto(szProto);

	if (pt!=NULL) {
		if (pt->CycleStartTick!=0&&pt->IconsCount!=0) {					
			b=((GetTickCount()-pt->CycleStartTick)/(DefaultStep))%pt->IconsCount;
			hIcon=GetConnectingIconForProto(szProto,b);
		}
	}

	return (INT_PTR)hIcon;
}

int CreateTimerForConnectingIcon(WPARAM wParam,LPARAM lParam)
{
	int status=(int)wParam;
	char *szProto=(char *)lParam;					
	if (!szProto) return (0);
	if (!status) return (0);
				
	if ((DBGetContactSettingByte(NULL,"CLUI","UseConnectingIcon",1)==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)
	{
		ProtoTicks *pt=NULL;
		int cnt;

		pt=GetProtoTicksByProto(szProto);
		if (pt!=NULL)
		{
			if (pt->CycleStartTick==0) 
			{					
				//	sprintf(buf,"SetTimer %d\r\n",pt->n);
				//	OutputDebugStringA(buf);

				KillTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->n);
				cnt=GetConnectingIconForProtoCount(szProto);
				if (ServiceExists(MS_SKIN2_GETICON)&&DBGetContactSettingByte(NULL,"Clist","UseProtoIconFromIcoLib",1))
				{
					cnt=8;
				}

				if (cnt!=0)
				{
					DefaultStep=DBGetContactSettingWord(NULL,"CLUI","DefaultStepConnectingIcon",100);
					pt->IconsCount=cnt;
					SetTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->n,(int)(DefaultStep)/1,0);
					pt->TimerCreated=1;
					pt->CycleStartTick=GetTickCount();
				}
			}
		}
	}
	return 0;
}
// Restore protocols to the last global status.
// Used to reconnect on restore after standby.

int OnSettingChanging(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING *)lParam;
	if (wParam==0)
	{
		if ((dbcws->value.type==DBVT_BYTE)&&!strcmp(dbcws->szModule,"CLUI"))
		{
			if (!strcmp(dbcws->szSetting,"SBarShow"))
			{	
				showOpts=dbcws->value.bVal;	
				return(0);
			}
		}
	}
	else
	{		
		if (dbcws==NULL){return(0);}

		if (!ServiceExists("ExtraIcon/Register"))
		{
			if (dbcws->value.type==DBVT_ASCIIZ&&!strcmp(dbcws->szSetting,"e-mail"))
			{
				SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
				return(0);
			}
			if (dbcws->value.type==DBVT_ASCIIZ&&!strcmp(dbcws->szSetting,"Cellular"))
			{		
				SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
				return(0);
			}

			if (dbcws->value.type==DBVT_ASCIIZ&&strstr(dbcws->szModule,"ICQ"))
			{
				if (!strcmp(dbcws->szSetting,(HANDLE)"MirVer"))
				{		
					SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
					return(0);
				}
			
			}
			
			if (dbcws->value.type==DBVT_ASCIIZ&&!strcmp(dbcws->szModule,"UserInfo"))
			{
				if (!strcmp(dbcws->szSetting,(HANDLE)"MyPhone0"))
				{		
					SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
					return(0);
				}
				if (!strcmp(dbcws->szSetting,(HANDLE)"Mye-mail0"))
				{	
					SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);	
					return(0);
				}
			}
		}
	}
	return(0);
}

HWND PreCreateCLC(HWND parent)
{
	pcli->hwndContactTree = CreateWindow(CLISTCONTROL_CLASS,_T(""),
		WS_CHILD|WS_CLIPCHILDREN|CLS_CONTACTLIST
		|(DBGetContactSettingByte(NULL,"CList","UseGroups",SETTING_USEGROUPS_DEFAULT)?CLS_USEGROUPS:0)
		|CLS_HIDEOFFLINE
		//|(DBGetContactSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT)?CLS_HIDEOFFLINE:0)
		|(DBGetContactSettingByte(NULL,"CList","HideEmptyGroups",SETTING_HIDEEMPTYGROUPS_DEFAULT)?CLS_HIDEEMPTYGROUPS:0)
		|(DBGetContactSettingByte(NULL,"CList","ShowStatusMessages",1)?CLS_SHOWSTATUSMESSAGES:0)
		|CLS_MULTICOLUMN
		//|DBGetContactSettingByte(NULL,"CLUI","ExtraIconsAlignToLeft",1)?CLS_EX_MULTICOLUMNALIGNLEFT:0
		,0,0,0,0,parent,NULL,g_hInst,NULL);

	return pcli->hwndContactTree;
}

int CreateCLC(HWND parent)
{
	Sleep(0);
	{		
	   // create contact list frame
		CLISTFrame Frame;
		memset(&Frame,0,sizeof(Frame));
		Frame.cbSize=sizeof(CLISTFrame);
		Frame.hWnd=pcli->hwndContactTree;
		Frame.align=alClient;
		Frame.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
			//LoadIcon(hInst,MAKEINTRESOURCE(IDI_MIRANDA));
		Frame.Flags=F_VISIBLE|F_SHOWTB|F_SHOWTBTIP|F_TCHAR;
		Frame.tname=_T("My Contacts");
		Frame.TBtname=TranslateT("My Contacts");
		hFrameContactTree=(HWND)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,(LPARAM)0);
		//free(Frame.name);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,hFrameContactTree),(LPARAM)TranslateT("My Contacts"));	
	}
	
	ReloadExtraIcons();
	{
		lastreqh=0;
		{
			CallService(MS_CLIST_SETHIDEOFFLINE,(WPARAM)oldhideoffline,0);
		}

		{	int state=DBGetContactSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);
			if(state==SETTING_STATE_NORMAL) ShowWindow(pcli->hwndContactList, SW_SHOW);
			else if(state==SETTING_STATE_MINIMIZED) ShowWindow(pcli->hwndContactList, SW_SHOWMINIMIZED);
		}
		
		lastreqh=0;
		disableautoupd=0;
	
	}
	hSettingChangedHook=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,OnSettingChanging);
	return(0);
}

int GetStatsuBarProtoRect(HWND hwnd,char *szProto,RECT *rc)
{
	int nParts,nPanel;
	ProtocolData *PD;
	int startoffset=DBGetContactSettingDword(NULL,"StatusBar","FirstIconOffset",0);
	
	if (!UseOwnerDrawStatusBar) startoffset=0;

	nParts=SendMessage(hwnd,SB_GETPARTS,0,0);
	FillMemory(rc,sizeof(RECT),0);

	for (nPanel=0;nPanel<nParts;nPanel++)
	{
		PD=(ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,(LPARAM)0);
		if ( PD == NULL )
			return(0);
	
		if (!strcmp(szProto,PD->RealName))
		{
			SendMessage(hwnd,SB_GETRECT,(WPARAM)nPanel,(LPARAM)rc);
			rc->left+=startoffset;
			rc->right+=startoffset;
			return(0);
		}
	}
	return (0);
}

extern LRESULT ( CALLBACK *saveContactListWndProc )(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{     
	/*
	This registers a window message with RegisterWindowMessage() and then waits for such a message,
	if it gets it, it tries to open a file mapping object and then maps it to this process space,
	it expects 256 bytes of data (incl. NULL) it will then write back the profile it is using the DB to fill in the answer.
	
	  The caller is expected to create this mapping object and tell us the ID we need to open ours.	
	*/
	if (msg==hMsgGetProfile && wParam != 0) { /* got IPC message */
		HANDLE hMap;
		char szName[MAX_PATH];
		int rc=0;
		_snprintf(szName,SIZEOF(szName),"Miranda::%u", wParam); // caller will tell us the ID of the map
		hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS,FALSE,szName);
		if (hMap != NULL) {
			void *hView=NULL;
			hView=MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH);
			if (hView) {
				char szFilePath[MAX_PATH], szProfile[MAX_PATH];
				CallService(MS_DB_GETPROFILEPATH,MAX_PATH,(LPARAM)&szFilePath);
				CallService(MS_DB_GETPROFILENAME,MAX_PATH,(LPARAM)&szProfile);
				_snprintf(hView,MAX_PATH,"%s\\%s",szFilePath,szProfile);
				UnmapViewOfFile(hView);
				rc=1;
			}
			CloseHandle(hMap);
		}
		return rc;
	}

	switch (msg) {
	case WM_CREATE:
		CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)GetMenu(hwnd),0);
		DrawMenuBar(hwnd);
		showOpts=DBGetContactSettingByte(NULL,"CLUI","SBarShow",1);		

		//create the status wnd
		//pcli->hwndStatus = CreateStatusWindow(WS_CHILD | (DBGetContactSettingByte(NULL,"CLUI","ShowSBar",1)?WS_VISIBLE:0), "", hwnd, 0);	
		CluiProtocolStatusChanged(0,0);
		
		hMsgGetProfile = RegisterWindowMessageA( "Miranda::GetProfile" ); // don't localise
		
		if ( DBGetContactSettingByte( NULL, "CList", "Transparent", 0 )) {
			SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			if ( MySetLayeredWindowAttributes )
				MySetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
		}
		transparentFocus=1;
		return FALSE;

	case M_SETALLEXTRAICONS:
		return TRUE;

	case M_CREATECLC:
		CreateCLC(hwnd);
		return TRUE;

	case WM_SIZE:
	{
		RECT rc;
		if ( wParam != SIZE_MINIMIZED ) {
			if ( pcli->hwndContactList != NULL )
				CLUIFramesOnClistResize((WPARAM)hwnd,(LPARAM)0);

			GetWindowRect(hwnd, &rc);		
			if(!CallService(MS_CLIST_DOCKINGISDOCKED,0,0)) {
				//if docked, dont remember pos (except for width)
				DBWriteContactSettingDword(NULL,"CList","Height",(DWORD)(rc.bottom - rc.top));
				DBWriteContactSettingDword(NULL,"CList","x",(DWORD)rc.left);
				DBWriteContactSettingDword(NULL,"CList","y",(DWORD)rc.top);
			}
			DBWriteContactSettingDword(NULL,"CList","Width",(DWORD)(rc.right - rc.left));		
		}
		if ( wParam == SIZE_MINIMIZED ) {
			if ( DBGetContactSettingByte(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT )) {
				ShowWindow(hwnd, SW_HIDE);
				DBWriteContactSettingByte(NULL,"CList","State",SETTING_STATE_HIDDEN);
			}
			else DBWriteContactSettingByte(NULL,"CList","State",SETTING_STATE_MINIMIZED);
		}
		return 0;
	}
			
	case WM_SETFOCUS:
	{	
		boolean isfloating;
		if ( hFrameContactTree ) {					
			isfloating = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLOATING,hFrameContactTree),0);
			if ( isfloating == FALSE)
				SetFocus(pcli->hwndContactTree);
		}
		return 0;
	}
   case WM_TIMER:
		if ((int)wParam>=TM_STATUSBARUPDATE&&(int)wParam<=TM_STATUSBARUPDATE+64) {
			int status,i;

			ProtoTicks *pt=NULL;
			for (i=0;i<64;i++) {
				pt=&CycleStartTick[i];
				if (pt->szProto!=NULL&&pt->TimerCreated==1) {
					status=CallProtoService(pt->szProto,PS_GETSTATUS,0,0);
					if (!(status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES))
					{													
						pt->CycleStartTick=0;
						KillTimer(hwnd,TM_STATUSBARUPDATE+pt->n);
						pt->TimerCreated=0;
			}	}	}

			pt=&CycleStartTick[wParam-TM_STATUSBARUPDATE];
			{
				RECT rc;
				GetStatsuBarProtoRect(pcli->hwndStatus,pt->szProto,&rc);
				rc.right=rc.left+GetSystemMetrics(SM_CXSMICON)+1;
				rc.top=0;

				if(IsWindowVisible(pcli->hwndStatus)) InvalidateRect(pcli->hwndStatus,&rc,TRUE);
				pcli->pfnTrayIconUpdateBase(pt->szProto);
			}
			//SendMessage(pcli->hwndStatus,WM_PAINT,0,0);
			UpdateWindow(pcli->hwndStatus);
			return TRUE;
		}
		break;

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
		if ( dis->hwndItem == pcli->hwndStatus ) {
			DrawDataForStatusBar(dis);
			return 0;
		}
		if ( dis->CtlType != ODT_MENU )
			return 0;
		break;
	}
	case WM_KEYDOWN:
		CallService(MS_CLIST_MENUPROCESSHOTKEY,wParam,MPCF_MAINMENU|MPCF_CONTACTMENU);
		if (wParam==VK_F5)
			SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
		return TRUE;
				
	case WM_GETMINMAXINFO:
		DefWindowProc(hwnd,msg,wParam,lParam);
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x=18;
		if (requr==0){((LPMINMAXINFO)lParam)->ptMinTrackSize.y=CLUIFramesGetMinHeight();}
		return 0;
				
	//MSG FROM CHILD CONTROL
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == pcli->hwndContactTree) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
				{
					NMCLISTCONTROL *nm=(NMCLISTCONTROL *)lParam;
					if (nm!=NULL) SetAllExtraIcons(pcli->hwndContactTree,nm->hItem );
					return TRUE;
				}
			case CLN_LISTREBUILT:
				SetAllExtraIcons(pcli->hwndContactTree,0);
				return(FALSE);

			case CLN_LISTSIZECHANGE:
				{
					NMCLISTCONTROL *nmc=(NMCLISTCONTROL*)lParam;
					RECT rcWindow,rcTree,rcWorkArea;
					int maxHeight,newHeight;
					int winstyle;

					if ( !disableautoupd && DBGetContactSettingByte( NULL, "CLUI", "AutoSize", 0 )) {
						if ( !CallService(MS_CLIST_DOCKINGISDOCKED,0,0) && hFrameContactTree != 0 ) {
							maxHeight=DBGetContactSettingByte(NULL,"CLUI","MaxSizeHeight",75);
							GetWindowRect(hwnd,&rcWindow);
							GetWindowRect(pcli->hwndContactTree,&rcTree);
							winstyle=GetWindowLong(pcli->hwndContactTree,GWL_STYLE);

							SystemParametersInfo(SPI_GETWORKAREA,0,&rcWorkArea,FALSE);
							lastreqh=nmc->pt.y;
							newHeight=max(nmc->pt.y,3)+1+((winstyle&WS_BORDER)?2:0)+(rcWindow.bottom-rcWindow.top)-(rcTree.bottom-rcTree.top);
							if (newHeight != rcWindow.bottom - rcWindow.top ) {
								if(newHeight>(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100)
									newHeight=(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100;
								if(DBGetContactSettingByte(NULL,"CLUI","AutoSizeUpward",0)) {
									rcWindow.top=rcWindow.bottom-newHeight;
									if(rcWindow.top<rcWorkArea.top) rcWindow.top=rcWorkArea.top;
								}
								else {
									rcWindow.bottom=rcWindow.top+newHeight;
									if(rcWindow.bottom>rcWorkArea.bottom) rcWindow.bottom=rcWorkArea.bottom;
								}
								
								if (requr == 0) {
									requr=1;					
									SetWindowPos(hwnd,0,rcWindow.left,rcWindow.top,rcWindow.right-rcWindow.left,rcWindow.bottom-rcWindow.top,SWP_NOZORDER|SWP_NOACTIVATE);
									GetWindowRect(hwnd,&rcWindow);
									requr=0;
					}	}	}	}
					return TRUE;
				}
			case NM_CLICK:
				{	
					NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
					DWORD hitFlags;
					HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_HITTEST,(WPARAM)&hitFlags,MAKELPARAM(nm->pt.x,nm->pt.y));

					if (hitFlags&CLCHT_ONITEMEXTRA) {					
						if (!IsHContactGroup(hItem)&&!IsHContactInfo(hItem))
						{
							int extra;
							pdisplayNameCacheEntry pdnce; 

							pdnce = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(nm->hItem);
							if (pdnce==NULL) return 0;

							extra = ColumnNumToExtra(nm->iColumn);
							NotifyEventHooks(hExtraImageClick, (WPARAM)nm->hItem, extra);

							if (!ServiceExists("ExtraIcon/Register"))
							{
								int v,e,w;
								v=ExtraToColumnNum(EXTRA_ICON_PROTO);
								e=ExtraToColumnNum(EXTRA_ICON_EMAIL);
								w=ExtraToColumnNum(EXTRA_ICON_ADV1);

								if(nm->iColumn==v)
									CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)nm->hItem,0);

								if(nm->iColumn==e) {
									//CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)nm->hItem,0);
									char *email,buf[4096];
									email=DBGetStringA(nm->hItem,"UserInfo", "Mye-mail0");
									if (email) {
										sprintf(buf,"mailto:%s",email);
										ShellExecuteA(hwnd,"open",buf,NULL,NULL,SW_SHOW);
									}											
								}	
								if(nm->iColumn==w) {
									char *homepage;
									homepage=DBGetStringA(pdnce->hContact,pdnce->szProto, "Homepage");
									if (homepage!=NULL)
										ShellExecuteA(hwnd,"open",homepage,NULL,NULL,SW_SHOW);
					}	}	}	}

					if(hItem) break;
					if((hitFlags&(CLCHT_NOWHERE|CLCHT_INLEFTMARGIN|CLCHT_BELOWITEMS))==0) break;
					if (DBGetContactSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT)) {
						POINT pt;
						pt=nm->pt;
						ClientToScreen(pcli->hwndContactTree,&pt);
						return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE|HTCAPTION,MAKELPARAM(pt.x,pt.y));
					}
					return TRUE;
				}
			}
		}
		else if(((LPNMHDR)lParam)->hwndFrom==pcli->hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK ) {
				int nParts=0, nPanel=0;
				NMMOUSE *nm=(NMMOUSE*)lParam;
				HMENU hMenu;
				RECT rc;
				POINT pt;
				int totcount;
				ProtocolData *PD;
				int menuid;
				int startoffset=DBGetContactSettingDword(NULL,"StatusBar","FirstIconOffset",0);
				int extraspace=DBGetContactSettingDword(NULL,"StatusBar","BkExtraSpace",0);
				boolean UseOwnerDrawStatusBar=DBGetContactSettingByte(NULL,"CLUI","UseOwnerDrawStatusBar",0);
					
				hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
				nParts=SendMessage(pcli->hwndStatus,SB_GETPARTS,0,0);
						
				if (nm->dwItemSpec==0xFFFFFFFE) {
					nPanel=nParts-1;
					SendMessage(pcli->hwndStatus,SB_GETRECT,nPanel,(LPARAM)&rc);
					if (nm->pt.x < rc.left) return FALSE;
				} 
				else { 
					if (!((startoffset)!=0&&UseOwnerDrawStatusBar))
					{								
						nPanel=nm->dwItemSpec; 
						SendMessage(pcli->hwndStatus,SB_GETRECT,nPanel,(LPARAM)&rc);
					}
					else {
						RECT clrc;
						int sectwidth;

						memset(&rc,0,sizeof(RECT));
						GetClientRect(pcli->hwndStatus,&clrc);
						clrc.right-=clrc.left;
						clrc.right-=startoffset;
						sectwidth=clrc.right/nParts;

						for (nPanel=0;nPanel<nParts;nPanel++) {
							PD=(ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,(LPARAM)0);
							if(PD==NULL)
								continue;

							rc.top=0;
							rc.bottom=clrc.bottom;
							rc.left=nPanel*sectwidth+startoffset;
							rc.right=rc.left+sectwidth-1;

							if (PtInRect(&rc,nm->pt))
								break;
				}	}	}

				totcount=DBGetContactSettingDword(0,"Protocols","ProtoCount",0);
				PD=(ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,(LPARAM)0);
				if(PD==NULL){return(0);}
				menuid=nPanel;
						
				if (menuid<0){break;}
				hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
                {
                    int i;
                    unsigned int cpnl = 0;
                    int mcnt = GetMenuItemCount(hMenu);
                    for (i=0; i<mcnt; ++i) {
					    HMENU hMenus = GetSubMenu(hMenu, i);
                        if (hMenus && cpnl++ == menuid) { 
                            hMenu = hMenus; 
                            break; 
                        }
                    }
                }
                {
					char buf[256];
					sprintf(buf,"nPanel: %d, PD->protopos: %d,PD->RealName %s\r\n",nPanel,PD->protopos,PD->RealName);
					OutputDebugStringA(buf);
				}

				if (hMenu != NULL) {						
					GetCursorPos(&pt);
					TrackPopupMenu(hMenu,TPM_BOTTOMALIGN|TPM_LEFTALIGN,pt.x,pt.y,0,hwnd,NULL);
			}	}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		{
			//saving state
			int state = DBGetContactSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);
			
			FreeProtocolData();
			if ( hSettingChangedHook != 0 )
				UnhookEvent(hSettingChangedHook);
			
			if ( state == SETTING_STATE_NORMAL )
				ShowWindow(hwnd,SW_HIDE);

			CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)hFrameContactTree,(LPARAM)0);
			DestroyWindow(pcli->hwndContactTree);
			pcli->hwndContactList=NULL;

			UnLoadCLUIFramesModule();		
			DBWriteContactSettingByte(NULL,"CList","State",(BYTE)state);
       		PostQuitMessage(0);
		}
		break;
	}

	return saveContactListWndProc( hwnd, msg, wParam, lParam );
}

int LoadCLUIModule(void)
{
	DBVARIANT dbv;
	TCHAR titleText[256];
	int laster;
	canloadstatusbar=FALSE;
	hFrameContactTree=0;

	hUserDll = LoadLibraryA("user32.dll");
	if (hUserDll) {
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
	}

	HookEvent(ME_SYSTEM_MODULESLOADED,CluiModulesLoaded);
	HookEvent(ME_OPT_INITIALISE,CluiOptInit);
	hContactDraggingEvent=CreateHookableEvent(ME_CLUI_CONTACTDRAGGING);
	hContactDroppedEvent=CreateHookableEvent(ME_CLUI_CONTACTDROPPED);
	hContactDragStopEvent=CreateHookableEvent(ME_CLUI_CONTACTDRAGSTOP);

	CreateServiceFunction("CLUI/GetConnectingIconForProtocol",GetConnectingIconService);

	if(DBGetContactSettingTString(NULL,"CList","TitleText",&dbv))
		lstrcpyn(titleText,_T(MIRANDANAME),SIZEOF(titleText));
	else {
		lstrcpyn(titleText,dbv.ptszVal,SIZEOF(titleText));
		DBFreeVariant(&dbv);
	}

	oldhideoffline=DBGetContactSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT);

	laster=GetLastError();
	PreCreateCLC(pcli->hwndContactList);

	// create status bar frame
	CreateStatusBarhWnd(pcli->hwndContactList);				

	{	//int state=DBGetContactSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);
		hMenuMain=GetMenu(pcli->hwndContactList);
		if(!DBGetContactSettingByte(NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT)) SetMenu(pcli->hwndContactList,NULL);
		SetWindowPos(pcli->hwndContactList, DBGetContactSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}

	lastreqh=0;
	return 0;
}

void UnloadCLUIModule()
{
	int i;

	for ( i=0; i < SIZEOF(CycleStartTick); i++ )
		if ( CycleStartTick[i].szProto != NULL )
			mir_free(CycleStartTick[i].szProto);
}