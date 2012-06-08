
#include "common.h"
#pragma hdrstop

#define TTBI_GROUPSHOWHIDE					"TTBInternal/GroupShowHide"
#define TTBI_SOUNDSONOFF					"TTBInternal/SoundsOnOFF"
#define TTBI_OPTIONSBUTT					"TTBInternal/OptionsBUTT"
#define TTBI_MAINMENUBUTT					"TTBInternal/MainMenuBUTT"
#define TTBI_MINIMIZEBUTT					"TTBInternal/MinimizeBUTT"
#define TTBI_FINDADDBUTT					"TTBInternal/FindAddBUTT"

int LoadInternalButtons();
int UnLoadInternalButtons();
extern HINSTANCE hInst;

HANDLE hSettingChangedHook;

static HBITMAP OnlineUp,OnlineDn,GroupsUp,GroupsDn,SoundsUp;
static HBITMAP SoundsDn,hbOptUp,hbOptDn,testsearch;
static HBITMAP MainMenuUp,MainMenuDn;
static HBITMAP MinimizeUp,MinimizeDn;
static HBITMAP FindUserUp,FindUserDn;

static int ShowOnline,ShowGroups,SoundsEnabled;
static HANDLE hOnlineBut,hGroupBut,hSoundsBut,hOptionsBut,hMainMenuBut;
static HANDLE hMinimizeBut;
static HANDLE hFindUsers;

static HANDLE OnSettingChg;

static HWND hwndContactTree;

int OnSettingChanging(WPARAM wParam,LPARAM lParam)
{
	if (wParam!=0){return(0);};
	{
		DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING *)lParam;
		if (dbcws==NULL){return(0);};
		
		if (!strcmp(dbcws->szModule,"CList"))
		{

			if (!strcmp(dbcws->szSetting,"HideOffline"))
			{
				int val=dbcws->value.bVal;

			//OutputDebugStr("==>TopToolBar HideOffline set it\r\n");

			CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hOnlineBut,(LPARAM)(val)?TTBST_PUSHED:TTBST_RELEASED);		

			CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hOnlineBut),
			(LPARAM)((!val)?Translate("Hide Offline Users"):Translate("Show All Users")));
				
				return(0);
			};

			if (!strcmp(dbcws->szSetting,"UseGroups"))
			{
				int val=dbcws->value.bVal;
				//int val=GetWindowLong(hwndContactTree,GWL_STYLE)&CLS_USEGROUPS;

				CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hGroupBut,(LPARAM)((val)?TTBST_PUSHED:TTBST_RELEASED));
				
				return(0);
			};
		};
	if (!strcmp(dbcws->szModule,"Skin"))
		{
			if (!strcmp(dbcws->szSetting,"UseSound"))
			{
				int val=dbcws->value.bVal;

				CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hSoundsBut,(LPARAM)(val)?TTBST_PUSHED:TTBST_RELEASED);		
				
				return(0);
			};

		};
	}
	return(0);
};
INT_PTR TTBInternalFindAddButt(WPARAM wParam,LPARAM lParam)
{
	//Sleep(100);
	
	CallService("FindAdd/FindAddCommand",0,0);
	CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hFindUsers,TTBST_RELEASED);
	return(0);
};


INT_PTR TTBInternalMinimizeButt(WPARAM wParam,LPARAM lParam)
{
	Sleep(30);
	CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hMinimizeBut,TTBST_RELEASED);
	Sleep(30);
	CallService(MS_CLIST_SHOWHIDE,0,0);
	return(0);
};

INT_PTR TTBInternalMainMenuButt(WPARAM wParam,LPARAM lParam)
{
	POINT pt;
	//CallService("Options/OptionsCommand",0,0);
	HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
//	hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);

	GetCursorPos(&pt);
	TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,(HWND)CallService(MS_CLUI_GETHWND,0,0),NULL);	
	
	Sleep(100);
	CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hMainMenuBut,TTBST_RELEASED);
	return(0);
};

INT_PTR TTBInternalOptionsButt(WPARAM wParam,LPARAM lParam)
{
	CallService("Options/OptionsCommand",0,0);
	Sleep(100);
	CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hOptionsBut,TTBST_RELEASED);
	return(0);
};
INT_PTR TTBInternalGroupShowHide(WPARAM wParam,LPARAM lParam)
{
	int newVal=!(GetWindowLong(hwndContactTree,GWL_STYLE)&CLS_USEGROUPS);
	DBWriteContactSettingByte(NULL,"CList","UseGroups",(BYTE)newVal);
	SendMessage(hwndContactTree,CLM_SETUSEGROUPS,newVal,0);

	/*
	CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hGroupBut),
	(!newVal)?Translate("Show Groups"):Translate("Hide Groups"));
	*/
	return(0);
};
//DBGetContactSettingByte(NULL,"Skin","UseSound",1)

INT_PTR TTBInternalSoundsOnOff(WPARAM wParam,LPARAM lParam)
{
	int newVal=!(DBGetContactSettingByte(NULL,"Skin","UseSound",1));
	DBWriteContactSettingByte(NULL,"Skin","UseSound",(BYTE)newVal);
	CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hSoundsBut),
		(LPARAM)((newVal)?Translate("Disable Sounds"):Translate("Enable Sounds")));
	return(0);
};
int UnLoadInternalButtons()
{
	if (hSettingChangedHook){UnhookEvent(hSettingChangedHook);};
	return(0);
};

boolean framesexists=FALSE;
int windhandle;	

LRESULT CALLBACK TestProczzz(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CREATE:
		SetTimer(hwnd,0,100,0);		
		return 0;
	case WM_TIMER:
			InvalidateRect(hwnd,NULL,TRUE);
			RedrawWindow(hwnd,NULL,NULL,0);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT lp;
			HDC hdc;
			hdc=BeginPaint(hwnd,&lp);
			if (hdc)
			{
				char buf[255];
				wsprintf((LPSTR)&buf,"%d",GetTickCount());
				TextOut(hdc,4,4,(LPCTSTR)&buf,strlen(buf));
				EndPaint(hwnd,&lp);
			}
			return(0);
		}

	} 
		
		
	return(DefWindowProc(hwnd, msg, wParam, lParam));
};

boolean first=TRUE;
char pluginname[]="SimpleClassName";

INT_PTR test(WPARAM wParam,LPARAM lParam)
{
if (first)
{
	WNDCLASS wndclass;
	int r;

    wndclass.style         = 0;
    wndclass.lpfnWndProc   = TestProczzz;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);//NULL;//(HBRUSH)(COLOR_3DFACE+1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = pluginname;
	r=RegisterClass(&wndclass);
	first=FALSE;
};

if (framesexists)
{
	CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)windhandle,0);
	windhandle=0;
	framesexists=FALSE;
}else
{
	CLISTFrame Frame;
	//int font;

	HWND pluginwind;
	pluginwind=CreateWindow(pluginname,pluginname,
					WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,
					0,0,0,0,(HWND)CallService(MS_CLUI_GETHWND,0,0),NULL,hInst,NULL);
	
	//font=SendMessage(parent,WM_GETFONT,0,0);
	//SendMessage(pluginwind,WM_SETFONT,font,0);
	
	//
	memset(&Frame,0,sizeof(Frame));
	Frame.name=(char *)malloc(255);
	memset(Frame.name,0,255);
	memcpy(Frame.name,pluginname,sizeof(pluginname));
	Frame.cbSize=sizeof(Frame);
	Frame.hWnd=pluginwind;
	Frame.align=alTop;
	Frame.Flags=F_VISIBLE;
	Frame.height=18;

	windhandle=CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);
	framesexists=TRUE;
	//free(Frame.name);
};
return(0);
};

int LoadInternalButtons(HWND hwnd)
{
	TTBButtonV2 ttb;
	
	hwndContactTree=hwnd;
	hSettingChangedHook=0;
	CreateServiceFunction(TTBI_GROUPSHOWHIDE,TTBInternalGroupShowHide);
	CreateServiceFunction(TTBI_SOUNDSONOFF,TTBInternalSoundsOnOff);

	CreateServiceFunction(TTBI_OPTIONSBUTT,TTBInternalOptionsButt);
	CreateServiceFunction(TTBI_MAINMENUBUTT,TTBInternalMainMenuButt);

	CreateServiceFunction(TTBI_MINIMIZEBUTT,TTBInternalMinimizeButt);
	CreateServiceFunction(TTBI_FINDADDBUTT,TTBInternalFindAddButt);

	CreateServiceFunction("TEST1",test);


	//ShowOnline=(GetWindowLong(hwndContactTree,GWL_STYLE)&CLS_HIDEOFFLINE);
	ShowOnline=DBGetContactSettingByte(NULL,"CList","HideOffline",0);
	//ShowGroups=(GetWindowLong(hwndContactTree,GWL_STYLE)&CLS_USEGROUPS);
	ShowGroups=DBGetContactSettingByte(NULL,"CList","UseGroups",2);
	SoundsEnabled=DBGetContactSettingByte(NULL,"Skin","UseSound",1);

/*
	OnlineDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ONLINEDN));
	OnlineUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ONLINEUP));
	
	hbOptUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CPANELUP));
	hbOptDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CPANELDN));

	MainMenuUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_MENUUP));
	MainMenuDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_MENUDN));
	//MainMenuDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SEP));

	MinimizeUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_MINIMIZEUP));
	MinimizeDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_MINIMIZEDN));

	FindUserUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_FINDUSERUP));
	FindUserDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_FINDUSERDN));


	//OnlineDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TESTBITMAP));
	//OnlineUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_TESTBITMAP));
//	testsearch=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SEARCHTEST));
	
	GroupsDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_GROUPDN));
	GroupsUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_GROUPUP));
	SoundsDn=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SOUNDSDN));
	SoundsUp=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SOUNDSUP));
*/
	//hbOptions=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_OPTIONS));
			
			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapDown=OnlineDn;
			//ttb.hbBitmapUp=OnlineUp;
			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_SHOWONLINEUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_SHOWONLINEDN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);



			ttb.dwFlags=(ShowOnline?TTBBF_PUSHED:0)|TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
			ttb.pszServiceDown=MS_CLIST_SETHIDEOFFLINE;
			ttb.pszServiceUp=MS_CLIST_SETHIDEOFFLINE;
			//ttb.lParam=0;
			ttb.wParamUp=-1;
			ttb.wParamDown=-1;
			ttb.name="Show only Online Users";
			hOnlineBut=(HANDLE)TTBAddButton(&ttb,0);

			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapDown=GroupsDn;
			//ttb.hbBitmapUp=GroupsUp;

			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_GROUPSUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_GROUPSDN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			ttb.dwFlags=(ShowGroups?TTBBF_PUSHED:0)|TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
			ttb.pszServiceDown=TTBI_GROUPSHOWHIDE;
			ttb.pszServiceUp=TTBI_GROUPSHOWHIDE;
			ttb.name="Groups On/Off";
			hGroupBut=(HANDLE)TTBAddButton(&ttb,0);

			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapDown=SoundsDn;
			//ttb.hbBitmapUp=SoundsUp;
			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_SOUNDUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_SOUNDDN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			ttb.dwFlags=(SoundsEnabled?TTBBF_PUSHED:0)|TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
			ttb.pszServiceDown=TTBI_SOUNDSONOFF;
			ttb.pszServiceUp=TTBI_SOUNDSONOFF;
			ttb.name="Sounds Enable/Disable";
			hSoundsBut=(HANDLE)TTBAddButton(&ttb,0);
/*
			memset(&ttb,0,sizeof(ttb));
			ttb.hbBitmapDown=hbOptions;
			ttb.hbBitmapUp=hbOptions;
			ttb.dwFlags=TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_DRAWBORDER;
			ttb.pszServiceDown="Options/OptionsCommand";
			ttb.pszServiceUp="Options/OptionsCommand";
			ttb.lParam=0;
			ttb.wParam=0;
//			hOptionsBut=TTBAddButton(&ttb,0);
*/
			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapDown=hbOptDn;
			//ttb.hbBitmapUp=hbOptUp;
			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_OPTIONSUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_OPTIONSDN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			ttb.dwFlags=TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
			ttb.pszServiceDown=TTBI_OPTIONSBUTT;
			ttb.pszServiceUp=TTBI_OPTIONSBUTT;
			ttb.name="Show Options Page";
			hOptionsBut=(HANDLE)TTBAddButton(&ttb,0);

			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapDown=MinimizeDn;
			//ttb.hbBitmapUp=MinimizeUp;
			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_MINIMIZEUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_MINIMIZEDN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			ttb.dwFlags=TTBBF_VISIBLE;
			ttb.pszServiceDown=TTBI_MINIMIZEBUTT;
			ttb.pszServiceUp=TTBI_MINIMIZEBUTT;
			ttb.name="Minimize Button";
			hMinimizeBut=(HANDLE)TTBAddButton(&ttb,0);

			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapDown=FindUserDn;
			//ttb.hbBitmapUp=FindUserUp;
			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_FINDADDUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_FINDADDDN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			ttb.dwFlags=TTBBF_VISIBLE;
			ttb.pszServiceDown=TTBI_FINDADDBUTT;
			ttb.pszServiceUp=TTBI_FINDADDBUTT;
			ttb.name="Find/Add Contacts";
			hFindUsers=(HANDLE)TTBAddButton(&ttb,0);



			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapUp=MainMenuUp;
			//ttb.hbBitmapDown=MainMenuDn;
			ttb.hIconUp=LoadImage(hInst,MAKEINTRESOURCE(IDI_MIRANDAUP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
			ttb.hIconDn=LoadImage(hInst,MAKEINTRESOURCE(IDI_MIRANDADN),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);

			ttb.dwFlags=TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP;
			ttb.pszServiceDown=TTBI_MAINMENUBUTT;
			ttb.pszServiceUp=TTBI_MAINMENUBUTT;
			ttb.name="Show Main Menu";
			hMainMenuBut=(HANDLE)TTBAddButton(&ttb,0);


			memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			//ttb.hbBitmapUp=MainMenuUp;
			//ttb.hbBitmapDown=MainMenuDn;
			ttb.dwFlags=TTBBF_VISIBLE;
			ttb.pszServiceDown="TEST1";
			ttb.pszServiceUp="TEST1";
			ttb.name="Test";
			//TTBAddButton(&ttb,0);

			CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hOnlineBut),
			(LPARAM)((ShowOnline)?Translate("Hide Offline Users"):Translate("Show All Users")));

			CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hGroupBut),
			(LPARAM)((ShowGroups)?Translate("Hide Groups"):Translate("Show Groups")));

			CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hSoundsBut),
			(LPARAM)((SoundsEnabled)?Translate("Disable Sounds"):Translate("Enable Sounds")));

			CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM(TTBO_TIPNAME,hOptionsBut),
			(LPARAM)Translate("Show Options"));

			//OutputDebugStr("==>TopToolBar setted Hook for ME_DB_CONTACT_SETTINGCHANGED\r\n");
			//hSettingChangedHook=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,OnSettingChanging);
			//OutputDebugStr("==>TopToolBar setted Hook for ME_DB_CONTACT_SETTINGCHANGED Done\r\n");

			return(0);

};