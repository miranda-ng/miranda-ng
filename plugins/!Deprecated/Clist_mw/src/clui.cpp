/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

HMENU hMenuMain;
static HANDLE hContactDraggingEvent,hContactDroppedEvent,hContactDragStopEvent;
UINT hMsgGetProfile = 0;

extern boolean canloadstatusbar;
boolean OnModulesLoadedCalled = FALSE;

static int transparentFocus = 1;
static byte oldhideoffline;
static int lastreqh = 0,requr = 0,disableautoupd = 1;
HANDLE hFrameContactTree;
BYTE showOpts;//for statusbar

typedef struct
{
	int IconsCount;
	int CycleStartTick;
	char *szProto;
	int n;
	int TimerCreated;
}
	ProtoTicks,*pProtoTicks;

ProtoTicks CycleStartTick[64];//max 64 protocols

int CycleTimeInterval = 2000;
int CycleIconCount = 8;
int DefaultStep = 100;

int CluiOptInit(WPARAM wParam, LPARAM lParam);
int SortList(WPARAM wParam, LPARAM lParam);

void CluiProtocolStatusChanged(int parStatus, const char* szProto);

extern void ReloadExtraIcons();
extern HWND CreateStatusBarhWnd(HWND parent);
extern HANDLE CreateStatusBarFrame();
extern int CLUIFramesUpdateFrame(WPARAM wParam, LPARAM lParam);
extern void DrawDataForStatusBar(LPDRAWITEMSTRUCT dis);
extern void InitGroupMenus();
extern int UseOwnerDrawStatusBar;

HICON GetConnectingIconForProto(char *szProto,int b);
HICON GetConnectingIconForProto_DLL(char *szProto,int b);

void RegisterProtoIconsForAllProtoIconLib();

static int CluiModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_SUBMENU;
	mii.hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	SetMenuItemInfo(hMenuMain,0,TRUE,&mii);
	mii.hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	SetMenuItemInfo(hMenuMain,1,TRUE,&mii);

	canloadstatusbar = TRUE;
	SendMessage(pcli->hwndContactList,WM_SIZE,0,0);
	CluiProtocolStatusChanged(0,0);
	Sleep(0);
	PostMessage(pcli->hwndContactList,M_CREATECLC,0,0);

	OnModulesLoadedCalled = TRUE;
	pcli->pfnInvalidateDisplayNameCacheEntry(INVALID_CONTACT_ID);
	InitGroupMenus();
	RegisterProtoIconsForAllProtoIconLib();
	return 0;
}

pProtoTicks GetProtoTicksByProto(char * szProto)
{
	int i;

	for (i = 0;i<64;i++)
	{
		if (CycleStartTick[i].szProto == NULL) break;
		if (strcmp(CycleStartTick[i].szProto,szProto)) continue;
		return(&CycleStartTick[i]);
	}
	for (i = 0;i<64;i++)
	{
		if (CycleStartTick[i].szProto == NULL)
		{
			CycleStartTick[i].szProto = mir_strdup(szProto);
			CycleStartTick[i].CycleStartTick = 0;
			CycleStartTick[i].n = i;
			return(&CycleStartTick[i]);
		}
	}
	return NULL;
}

int GetConnectingIconForProtoCount(char *szProto)
{
	char file[MAX_PATH],fileFull[MAX_PATH],szFullPath[MAX_PATH];
	char szPath[MAX_PATH];
	char *str;
	int ret;

	GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
	str = strrchr(szPath,'\\');
	if (str != NULL) *str = 0;
	mir_snprintf(szFullPath, SIZEOF(szFullPath), "%s\\Icons\\proto_conn_%s.dll", szPath, szProto);

	lstrcpynA(file,szFullPath,SIZEOF(file));
	PathToAbsolute(file, fileFull);
	ret = ExtractIconExA(fileFull,-1,NULL,NULL,1);
	if (ret == 0&&!strcmp(szProto,"ICQ")) ret = 8;
	return ret;
}

static HICON ExtractIconFromPath(const char *path)
{
	char *comma;
	char file[MAX_PATH],fileFull[MAX_PATH];
	int n;
	HICON hIcon;
	lstrcpynA(file,path,SIZEOF(file));
	comma = strrchr(file,',');
	if (comma == NULL) n = 0;
	else {n = atoi(comma+1); *comma = 0;}
   PathToAbsolute(file, fileFull);

	hIcon = NULL;
	ExtractIconExA(fileFull,n,NULL,&hIcon,1);
	return hIcon;
}

HICON LoadIconFromExternalFile(char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx,HICON DefIcon)
{
	char szPath[MAX_PATH],szMyPath[MAX_PATH], szFullPath[MAX_PATH],*str;
	HICON hIcon = NULL;

	memset(szMyPath,0,sizeof(szMyPath));
	memset(szFullPath,0,sizeof(szFullPath));

	if (filename != NULL)
	{
		GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
		GetModuleFileNameA(g_hInst, szMyPath, MAX_PATH);
		str = strrchr(szPath,'\\');
		if (str != NULL) *str = 0;
		mir_snprintf(szFullPath, SIZEOF(szFullPath), "%s\\Icons\\%s,%d", szPath, filename, i);
	}

	if ( !UseLibrary) {
		hIcon = ExtractIconFromPath(szFullPath);
		if (hIcon) return hIcon;
	}
	else {
		if (registerit && IconName != NULL && SectName != NULL) {
			SKINICONDESC sid = { sizeof(sid) };
			sid.pszSection = SectName;
			sid.pszName = IconName;
			sid.pszDescription = Description;
			if (szMyPath[0] != 0)
				sid.pszDefaultFile = szMyPath;

			sid.iDefaultIndex = internalidx;
			sid.hDefaultIcon = DefIcon;

			Skin_AddIcon(&sid);
		}
		return Skin_GetIcon(IconName);
	}

	return (HICON)0;
}

void RegisterProtoIcons (char *protoname)
{
	if ( db_get_b(NULL,"CList","UseProtoIconFromIcoLib",1))
	{
		int i;
		char buf[256];
		char buf2[256];
		for (i = 0;i<8;i++)
		{
			mir_snprintf(buf, SIZEOF(buf), "%s #%d", protoname, i);
			mir_snprintf(buf2, SIZEOF(buf2), "Contact list/Connection Icons %s", protoname);

			LoadIconFromExternalFile(NULL,i,TRUE,TRUE,buf,buf2,buf,0,GetConnectingIconForProto_DLL(protoname,i));
		}
	}
}

void RegisterProtoIconsForAllProtoIconLib()
{
	int protoCount,i;
	PROTOACCOUNT **accs;

	ProtoEnumAccounts( &protoCount, &accs );
	for ( i = 0; i < protoCount; i++ )
		if ( IsAccountEnabled( accs[i] ) && CallProtoService( accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0 ))
			RegisterProtoIcons( accs[i]->szModuleName );
}

HICON GetConnectingIconForProto_DLL(char *szProto,int b)
{
	char szFullPath[MAX_PATH];
	HICON hIcon = NULL;

	b = b-1;
	mir_snprintf(szFullPath, SIZEOF(szFullPath), "proto_conn_%s.dll", szProto);
	//		hIcon = ExtractIconFromPath(szFullPath);
	//		if (hIcon) return hIcon;

	hIcon = LoadIconFromExternalFile(szFullPath,b+1,FALSE,FALSE,NULL,NULL,NULL,0,0);
	if (hIcon) return hIcon;

#ifdef _DEBUG
	{
		char buf [256];
		mir_snprintf(buf, SIZEOF(buf), "IconNotFound %s %d\r\n", szProto, b);
		//	OutputDebugStringA(buf);
	}
#endif

	if ( !strcmp(szProto,"ICQ"))
	{

#ifdef _DEBUG
		char buf [256];
		mir_snprintf(buf, SIZEOF(buf), "Icon %d %d\r\n", GetTickCount(), b);
		//OutputDebugStringA(buf);
#endif
		return(LoadIconA(g_hInst,(LPCSTR)(IDI_ICQC1+b)));
	}

	return(hIcon);
}

HICON GetConnectingIconForProto(char *szProto,int b)
{
	if ( db_get_b(NULL,"CList","UseProtoIconFromIcoLib",1)) {
		HICON hIcon = 0;
		char buf[256];
		mir_snprintf(buf, SIZEOF(buf), "%s #%d", szProto, b);

		hIcon = LoadIconFromExternalFile(NULL,b,TRUE,FALSE,buf,"Contact list/Connection icons",buf,0,NULL);
		if (hIcon == NULL) return (GetConnectingIconForProto_DLL(szProto,b));
		return (CopyIcon(hIcon));
	}

	return GetConnectingIconForProto_DLL(szProto, b);
}

//wParam == szProto
INT_PTR GetConnectingIconService(WPARAM wParam, LPARAM lParam)
{
	int b;
	ProtoTicks *pt = NULL;
	HICON hIcon = NULL;

	char *szProto = (char *)wParam;
	if ( !szProto) return 0;

	pt = GetProtoTicksByProto(szProto);

	if (pt != NULL) {
		if (pt->CycleStartTick != 0&&pt->IconsCount != 0) {
			b = ((GetTickCount()-pt->CycleStartTick)/(DefaultStep))%pt->IconsCount;
			hIcon = GetConnectingIconForProto(szProto,b);
		}
	}

	return (INT_PTR)hIcon;
}

int CreateTimerForConnectingIcon(WPARAM wParam, LPARAM lParam)
{
	int status = (int)wParam;
	char *szProto = (char *)lParam;
	if ( !szProto) return 0;
	if ( !status) return 0;

	if (( db_get_b(NULL,"CLUI","UseConnectingIcon",1) == 1) && status >= ID_STATUS_CONNECTING && status <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES) {
		ProtoTicks *pt = NULL;
		int cnt;

		pt = GetProtoTicksByProto(szProto);
		if (pt != NULL) {
			if (pt->CycleStartTick == 0) {
				KillTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->n);
				cnt = GetConnectingIconForProtoCount(szProto);
				if ( db_get_b(NULL,"Clist","UseProtoIconFromIcoLib",1))
					cnt = 8;

				if (cnt != 0) {
					DefaultStep = db_get_w(NULL,"CLUI","DefaultStepConnectingIcon",100);
					pt->IconsCount = cnt;
					SetTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->n,(int)(DefaultStep)/1,0);
					pt->TimerCreated = 1;
					pt->CycleStartTick = GetTickCount();
				}
			}
		}
	}
	return 0;
}

// Restore protocols to the last global status.
// Used to reconnect on restore after standby.

int OnSettingChanging(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
	if (hContact == 0) {
		if ((dbcws->value.type == DBVT_BYTE) && !strcmp(dbcws->szModule,"CLUI")) {
			if (!strcmp(dbcws->szSetting,"SBarShow")) {
				showOpts = dbcws->value.bVal;
				return 0;
			}
		}
	}
	return 0;
}

HWND PreCreateCLC(HWND parent)
{
	pcli->hwndContactTree = CreateWindow( _T(CLISTCONTROL_CLASS),_T(""),
		WS_CHILD|WS_CLIPCHILDREN|CLS_CONTACTLIST
		|( db_get_b(NULL,"CList","UseGroups",SETTING_USEGROUPS_DEFAULT)?CLS_USEGROUPS:0)
		| CLS_HIDEOFFLINE
		//|( db_get_b(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT)?CLS_HIDEOFFLINE:0)
		|( db_get_b(NULL,"CList","HideEmptyGroups",SETTING_HIDEEMPTYGROUPS_DEFAULT)?CLS_HIDEEMPTYGROUPS:0)
		|( db_get_b(NULL,"CList","ShowStatusMessages",1)?CLS_SHOWSTATUSMESSAGES:0)
		|CLS_MULTICOLUMN
		//|db_get_b(NULL,"CLUI","ExtraIconsAlignToLeft",1)?CLS_EX_MULTICOLUMNALIGNLEFT:0
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
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.hWnd = pcli->hwndContactTree;
		Frame.align = alClient;
		Frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
			//LoadIcon(hInst,MAKEINTRESOURCE(IDI_MIRANDA));
		Frame.Flags = F_VISIBLE|F_SHOWTB|F_SHOWTBTIP|F_TCHAR;
		Frame.tname = _T("My contacts");
		Frame.TBtname = TranslateT("My contacts");
		hFrameContactTree = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);
		//free(Frame.name);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,hFrameContactTree),(LPARAM)TranslateT("My Contacts"));
	}

	lastreqh = 0;
	CallService(MS_CLIST_SETHIDEOFFLINE,(WPARAM)oldhideoffline,0);

	int state = db_get_b(NULL,"CList","State",SETTING_STATE_NORMAL);
	if (state == SETTING_STATE_NORMAL) ShowWindow(pcli->hwndContactList, SW_SHOW);
	else if (state == SETTING_STATE_MINIMIZED) ShowWindow(pcli->hwndContactList, SW_SHOWMINIMIZED);

	lastreqh = 0;
	disableautoupd = 0;

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED,OnSettingChanging);
	return 0;
}

int GetStatsuBarProtoRect(HWND hwnd,char *szProto,RECT *rc)
{
	int nParts,nPanel;
	ProtocolData *PD;
	int startoffset = db_get_dw(NULL,"StatusBar","FirstIconOffset",0);

	if ( !UseOwnerDrawStatusBar) startoffset = 0;

	nParts = SendMessage(hwnd,SB_GETPARTS,0,0);
	FillMemory(rc,sizeof(RECT),0);

	for (nPanel = 0;nPanel<nParts;nPanel++)
	{
		PD = (ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,0);
		if ( PD == NULL )
			return 0;

		if ( !strcmp(szProto,PD->RealName))
		{
			SendMessage(hwnd,SB_GETRECT,(WPARAM)nPanel,(LPARAM)rc);
			rc->left += startoffset;
			rc->right += startoffset;
			return 0;
		}
	}
	return 0;
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

	if (msg == hMsgGetProfile && wParam != 0) { /* got IPC message */
		HANDLE hMap;
		char szName[MAX_PATH];
		int rc = 0;
		mir_snprintf(szName, SIZEOF(szName), "Miranda::%u", wParam); // caller will tell us the ID of the map
		hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS,FALSE,szName);
		if (hMap != NULL) {
			void *hView = NULL;
			hView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH);
			if (hView) {
				char szFilePath[MAX_PATH], szProfile[MAX_PATH];
				CallService(MS_DB_GETPROFILEPATH,MAX_PATH,(LPARAM)&szFilePath);
				CallService(MS_DB_GETPROFILENAME,MAX_PATH,(LPARAM)&szProfile);
				mir_snprintf((char*)hView, MAX_PATH, "%s\\%s", szFilePath, szProfile);
				UnmapViewOfFile(hView);
				rc = 1;
			}
			CloseHandle(hMap);
		}
		return rc;
	}

	switch (msg) {
	case WM_CREATE:
		TranslateMenu(GetMenu(hwnd));
		DrawMenuBar(hwnd);
		showOpts = db_get_b(NULL,"CLUI","SBarShow",1);

		//create the status wnd
		//pcli->hwndStatus == CreateStatusWindow(WS_CHILD | ( db_get_b(NULL,"CLUI","ShowSBar",1)?WS_VISIBLE:0), "", hwnd, 0);
		CluiProtocolStatusChanged(0,0);

		hMsgGetProfile = RegisterWindowMessageA( "Miranda::GetProfile" ); // don't localise

		if ( db_get_b( NULL, "CList", "Transparent", 0 )) {
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)db_get_b(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
		}
		transparentFocus = 1;
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
				CLUIFramesOnClistResize((WPARAM)hwnd,0);

			GetWindowRect(hwnd, &rc);
			if ( !CallService(MS_CLIST_DOCKINGISDOCKED,0,0)) {
				//if docked, dont remember pos (except for width)
				db_set_dw(NULL,"CList","Height",(DWORD)(rc.bottom - rc.top));
				db_set_dw(NULL,"CList","x",(DWORD)rc.left);
				db_set_dw(NULL,"CList","y",(DWORD)rc.top);
			}
			db_set_dw(NULL,"CList","Width",(DWORD)(rc.right - rc.left));
		}
		if ( wParam == SIZE_MINIMIZED ) {
			if ( db_get_b(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT )) {
				ShowWindow(hwnd, SW_HIDE);
				db_set_b(NULL,"CList","State",SETTING_STATE_HIDDEN);
			}
			else db_set_b(NULL,"CList","State",SETTING_STATE_MINIMIZED);
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

			ProtoTicks *pt = NULL;
			for (i = 0;i<64;i++) {
				pt = &CycleStartTick[i];
				if (pt->szProto != NULL&&pt->TimerCreated == 1) {
					status = CallProtoService(pt->szProto,PS_GETSTATUS,0,0);
					if ( !(status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES))
					{
						pt->CycleStartTick = 0;
						KillTimer(hwnd,TM_STATUSBARUPDATE+pt->n);
						pt->TimerCreated = 0;
			}	}	}

			pt = &CycleStartTick[wParam-TM_STATUSBARUPDATE];
			{
				RECT rc;
				GetStatsuBarProtoRect(pcli->hwndStatus,pt->szProto,&rc);
				rc.right = rc.left+GetSystemMetrics(SM_CXSMICON)+1;
				rc.top = 0;

				if (IsWindowVisible(pcli->hwndStatus)) InvalidateRect(pcli->hwndStatus,&rc,TRUE);
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
			if (dis->hwndItem == pcli->hwndStatus) {
				DrawDataForStatusBar(dis);
				return 0;
			}
			if (dis->CtlType != ODT_MENU)
				return 0;
		}
		break;

	case WM_KEYDOWN:
		CallService(MS_CLIST_MENUPROCESSHOTKEY, wParam, MPCF_MAINMENU | MPCF_CONTACTMENU);
		if (wParam == VK_F5)
			pcli->pfnInitAutoRebuild(pcli->hwndContactTree);
		return TRUE;

	case WM_GETMINMAXINFO:
		DefWindowProc(hwnd,msg,wParam,lParam);
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 18;
		if (requr == 0){((LPMINMAXINFO)lParam)->ptMinTrackSize.y = CLUIFramesGetMinHeight();}
		return 0;

	//MSG FROM CHILD CONTROL
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == pcli->hwndContactTree) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_LISTSIZECHANGE:
				{
					NMCLISTCONTROL *nmc = (NMCLISTCONTROL*)lParam;
					RECT rcWindow,rcTree,rcWorkArea;
					int maxHeight,newHeight;
					int winstyle;

					if ( !disableautoupd && db_get_b( NULL, "CLUI", "AutoSize", 0 )) {
						if ( !CallService(MS_CLIST_DOCKINGISDOCKED,0,0) && hFrameContactTree != 0 ) {
							maxHeight = db_get_b(NULL,"CLUI","MaxSizeHeight",75);
							GetWindowRect(hwnd,&rcWindow);
							GetWindowRect(pcli->hwndContactTree,&rcTree);
							winstyle = GetWindowLongPtr(pcli->hwndContactTree,GWL_STYLE);

							SystemParametersInfo(SPI_GETWORKAREA,0,&rcWorkArea,FALSE);
							lastreqh = nmc->pt.y;
							newHeight = max(nmc->pt.y,3)+1+((winstyle&WS_BORDER)?2:0)+(rcWindow.bottom-rcWindow.top)-(rcTree.bottom-rcTree.top);
							if (newHeight != rcWindow.bottom - rcWindow.top ) {
								if (newHeight>(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100)
									newHeight = (rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100;
								if ( db_get_b(NULL,"CLUI","AutoSizeUpward",0)) {
									rcWindow.top = rcWindow.bottom-newHeight;
									if (rcWindow.top<rcWorkArea.top) rcWindow.top = rcWorkArea.top;
								}
								else {
									rcWindow.bottom = rcWindow.top+newHeight;
									if (rcWindow.bottom>rcWorkArea.bottom) rcWindow.bottom = rcWorkArea.bottom;
								}

								if (requr == 0) {
									requr = 1;
									SetWindowPos(hwnd,0,rcWindow.left,rcWindow.top,rcWindow.right-rcWindow.left,rcWindow.bottom-rcWindow.top,SWP_NOZORDER|SWP_NOACTIVATE);
									GetWindowRect(hwnd,&rcWindow);
									requr = 0;
					}	}	}	}
					return TRUE;
				}
			}
		}
		else if (((LPNMHDR)lParam)->hwndFrom == pcli->hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK ) {
				int nParts = 0, nPanel = 0;
				NMMOUSE *nm = (NMMOUSE*)lParam;
				HMENU hMenu;
				RECT rc;
				POINT pt;
				int totcount;
				ProtocolData *PD;
				int menuid;
				int startoffset = db_get_dw(NULL,"StatusBar","FirstIconOffset",0);
				int extraspace = db_get_dw(NULL,"StatusBar","BkExtraSpace",0);
				boolean UseOwnerDrawStatusBar = db_get_b(NULL,"CLUI","UseOwnerDrawStatusBar",0);

				hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
				nParts = SendMessage(pcli->hwndStatus,SB_GETPARTS,0,0);

				if (nm->dwItemSpec == 0xFFFFFFFE) {
					nPanel = nParts-1;
					SendMessage(pcli->hwndStatus,SB_GETRECT,nPanel,(LPARAM)&rc);
					if (nm->pt.x < rc.left) return FALSE;
				}
				else {
					if ( !((startoffset) != 0 && UseOwnerDrawStatusBar)) {
						nPanel = nm->dwItemSpec;
						SendMessage(pcli->hwndStatus,SB_GETRECT,nPanel,(LPARAM)&rc);
					}
					else {
						RECT clrc = { 0 };
						GetClientRect(pcli->hwndStatus,&clrc);
						clrc.right -= clrc.left;
						clrc.right -= startoffset;
						int sectwidth = clrc.right/nParts;

						for (nPanel = 0;nPanel<nParts;nPanel++) {
							PD = (ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,0);
							if (PD == NULL)
								continue;

							rc.top = 0;
							rc.bottom = clrc.bottom;
							rc.left = nPanel*sectwidth+startoffset;
							rc.right = rc.left+sectwidth-1;

							if (PtInRect(&rc,nm->pt))
								break;
				}	}	}

				totcount = db_get_dw(0,"Protocols","ProtoCount",0);
				PD = (ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,0);
				if (PD == NULL){return 0;}
				menuid = nPanel;

				if (menuid<0){break;}
				hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
				{
					unsigned int cpnl = 0;
					int mcnt = GetMenuItemCount(hMenu);
					for (int i = 0; i<mcnt; ++i) {
						HMENU hMenus = GetSubMenu(hMenu, i);
						if (hMenus && cpnl++ == menuid) {
							hMenu = hMenus;
							break;
						}
					}
				}

				if (hMenu != NULL) {
					GetCursorPos(&pt);
					TrackPopupMenu(hMenu,TPM_BOTTOMALIGN|TPM_LEFTALIGN,pt.x,pt.y,0,hwnd,NULL);
				}
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		//saving state
		int state = db_get_b(NULL,"CList","State",SETTING_STATE_NORMAL);

		FreeProtocolData();
		if (state == SETTING_STATE_NORMAL)
			ShowWindow(hwnd,SW_HIDE);

		CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)hFrameContactTree,0);
		DestroyWindow(pcli->hwndContactTree);
		pcli->hwndContactList = NULL;

		UnLoadCLUIFramesModule();
		db_set_b(NULL, "CList", "State", (BYTE)state);
		PostQuitMessage(0);
		break;
	}

	return saveContactListWndProc( hwnd, msg, wParam, lParam );
}

int LoadCLUIModule(void)
{
	DBVARIANT dbv;
	TCHAR titleText[256];
	canloadstatusbar = FALSE;
	hFrameContactTree = 0;

	HookEvent(ME_SYSTEM_MODULESLOADED,CluiModulesLoaded);
	HookEvent(ME_OPT_INITIALISE,CluiOptInit);
	hContactDraggingEvent = CreateHookableEvent(ME_CLUI_CONTACTDRAGGING);
	hContactDroppedEvent = CreateHookableEvent(ME_CLUI_CONTACTDROPPED);
	hContactDragStopEvent = CreateHookableEvent(ME_CLUI_CONTACTDRAGSTOP);

	CreateServiceFunction("CLUI/GetConnectingIconForProtocol",GetConnectingIconService);

	if (db_get_ts(NULL,"CList","TitleText",&dbv))
		lstrcpyn(titleText,_T(MIRANDANAME),SIZEOF(titleText));
	else {
		lstrcpyn(titleText,dbv.ptszVal,SIZEOF(titleText));
		db_free(&dbv);
	}

	oldhideoffline = db_get_b(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT);

	int laster = GetLastError();
	PreCreateCLC(pcli->hwndContactList);

	// create status bar frame
	CreateStatusBarhWnd(pcli->hwndContactList);

	hMenuMain = GetMenu(pcli->hwndContactList);
	if ( !db_get_b(NULL, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT))
		SetMenu(pcli->hwndContactList, NULL);
	SetWindowPos(pcli->hwndContactList, db_get_b(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	lastreqh = 0;
	return 0;
}

void UnloadCLUIModule()
{
	for (int i = 0; i < SIZEOF(CycleStartTick); i++)
		if ( CycleStartTick[i].szProto != NULL)
			mir_free(CycleStartTick[i].szProto);
}