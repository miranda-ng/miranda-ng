#include <windows.h>
//#include <stdio.h>
#include <tchar.h>
#include <Commctrl.h>
#include <assert.h>
#include "pid2name.h"


#ifdef _DEBUG
#include "debug.h"
#endif

#include "resource.h"


#include <newpluginapi.h>

#include <m_clist.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_langpack.h>
//#include <m_plugins.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_system.h>
//#include <m_updater.h>

#include "ConnectionNotify.h"
#include "netstat.h"
#include "filter.h"
#include "version.h"

#define MAX_SETTING_STR 512
#define PLUGINNAME "ConnectionNotify"

#define STATUS_COUNT 9

HINSTANCE hInst;

//PLUGINLINK *pluginLink=NULL;
HANDLE hOptInit=NULL;
static HWND hTimerWnd = (HWND)NULL;
static UINT TID = (UINT)12021;
//HANDLE hHookModulesLoaded=NULL;
HANDLE hCheckEvent=NULL;
HANDLE hCheckHook=NULL;
HANDLE hHookModulesLoaded=NULL;
HANDLE hHookPreShutdown=NULL;
HANDLE hConnectionCheckThread = NULL;
HANDLE hFilterOptionsThread=NULL;
HANDLE killCheckThreadEvent=NULL;
HANDLE hExceptionsMutex=NULL;
//HANDLE hCurrentEditMutex=NULL;
int hLangpack=0;

DWORD FilterOptionsThreadId;
DWORD ConnectionCheckThreadId;
BYTE settingSetColours=0;
COLORREF settingBgColor;
COLORREF settingFgColor;
int settingInterval=0;
int settingInterval1=0;
BYTE settingResolveIp=0;
BOOL settingStatus[STATUS_COUNT];
int settingFiltersCount=0;
BOOL settingDefaultAction=TRUE;
WORD settingStatusMask=0;

struct CONNECTION *first=NULL;
struct CONNECTION *connExceptions=NULL;
struct CONNECTION *connCurrentEdit;
struct CONNECTION *connExceptionsTmp=NULL;
struct CONNECTION *connCurrentEditModal=NULL;
int currentStatus = ID_STATUS_OFFLINE,diffstat=0;
BOOL bOptionsOpen=FALSE;
TCHAR *tcpStates[]={_T("CLOSED"),_T("LISTEN"),_T("SYN_SENT"),_T("SYN_RCVD"),_T("ESTAB"),_T("FIN_WAIT1"),_T("FIN_WAIT2"),_T("CLOSE_WAIT"),_T("CLOSING"),_T("LAST_ACK"),_T("TIME_WAIT"),_T("DELETE_TCB")};
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	PLUGINNAME,
	PLUGIN_MAKE_VERSION(0,1,0,5),
	"Notify with popup if some connection established",
	"MaKaR",
	"makar@poczta.of.pl",
	"© 2011-2013 MaKaRSoFT",
	"http://maciej.wycik.pl/miranda",
	UNICODE_AWARE,		//not transient
	//	4BB5B4AA-C364-4F23-9746-D5B708A286A5
	{0x4bb5b4aa, 0xc364, 0x4f23, { 0x97, 0x46, 0xd5, 0xb7, 0x8, 0xa2, 0x86, 0xa5 } }
};

extern "C" __declspec(dllexport) const MUUID interfaces[]={MIID_CONNECTIONNOTIFY,MIID_PROTOCOL, MIID_LAST};

// authentication callback futnction from extension manager
// ======================================
//=========================================================================================


BOOL strrep(TCHAR *src,TCHAR *needle,TCHAR *newstring)
{
	TCHAR *found,begining[MAX_SETTING_STR],tail[MAX_SETTING_STR];
	int pos=0;

	//strset(begining,' ');
	//strset(tail,' ');
	if(!(found=_tcsstr(src,needle)))
		return FALSE;

	pos=(int)(found-src);
	_tcsncpy_s(begining,_countof(begining),src,pos);
	begining[pos]='\0';

	pos=pos+(int)_tcslen(needle);
	_tcsncpy_s(tail,_countof(tail),src+pos,sizeof(tail));
	begining[pos]='\0';

	pos=mir_sntprintf(src,_tcslen(src),_T("%s%s%s"),begining,newstring,tail);
	return TRUE;
}
/*miranda IM
__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if(mirandaVersion<PLUGIN_MAKE_VERSION(0,1,0,1)) return NULL;
	pluginInfo.cbSize=sizeof(PLUGININFO); // needed as v0.6 does equality check 
	return (PLUGININFO*)&pluginInfo;
}
*/

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.cbSize=sizeof(PLUGININFOEX);
	return &pluginInfo;
}
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}
void saveSettingsConnections(struct CONNECTION *connHead)
{
	char buff[128];
	int i=0;
	struct CONNECTION *tmp=connHead;
	while(tmp!=NULL)
	{

		mir_snprintf(buff,sizeof(buff), "%dFilterIntIp", i);
		db_set_ts(NULL, PLUGINNAME, buff, tmp->strIntIp);
		mir_snprintf(buff,sizeof(buff), "%dFilterExtIp", i);
		db_set_ts(NULL, PLUGINNAME, buff, tmp->strExtIp);
		mir_snprintf(buff,sizeof(buff), "%dFilterPName", i);
		db_set_ws(NULL, PLUGINNAME, buff, tmp->PName);
		mir_snprintf(buff,sizeof(buff), "%dFilterIntPort", i);
		db_set_dw(NULL, PLUGINNAME, buff, tmp->intIntPort);
		mir_snprintf(buff,sizeof(buff), "%dFilterExtPort", i);
		db_set_dw(NULL, PLUGINNAME, buff, tmp->intExtPort);
		mir_snprintf(buff,sizeof(buff), "%dFilterAction", i);
		db_set_dw(NULL, PLUGINNAME, buff, tmp->Pid);
		i++;
		tmp=tmp->next;
	}
	settingFiltersCount=i;
	db_set_dw(NULL, PLUGINNAME, "FiltersCount", settingFiltersCount );
	
}
//load filters from db
struct CONNECTION* LoadSettingsConnections()
{
	struct CONNECTION *connHead=NULL;
	DBVARIANT dbv;
	char buff[128];
	int i=0;
	for(i=settingFiltersCount-1;i>=0;i--)
	{
		struct CONNECTION *conn=(struct CONNECTION*)mir_alloc(sizeof(struct CONNECTION));
		mir_snprintf(buff,sizeof(buff), "%dFilterIntIp", i);
		if(!db_get_ts(NULL, PLUGINNAME, buff, &dbv))
		wcsncpy(conn->strIntIp,dbv.ptszVal,_countof(conn->strIntIp) );
		db_free(&dbv);
		mir_snprintf(buff,sizeof(buff), "%dFilterExtIp", i);
		if(!db_get_ts(NULL, PLUGINNAME, buff, &dbv))
		wcsncpy(conn->strExtIp,dbv.ptszVal,_countof(conn->strExtIp) );
		db_free(&dbv);
		mir_snprintf(buff,sizeof(buff), "%dFilterPName", i);
		if(!db_get_ts(NULL, PLUGINNAME, buff, &dbv))
		wcsncpy(conn->PName,dbv.ptszVal,_countof(conn->PName) );
		db_free(&dbv);
				
		mir_snprintf(buff,sizeof(buff), "%dFilterIntPort", i);
		conn->intIntPort = db_get_dw(0, PLUGINNAME, buff, -1);
		
		mir_snprintf(buff,sizeof(buff), "%dFilterExtPort", i);
		conn->intExtPort = db_get_dw(0, PLUGINNAME, buff, -1);

		mir_snprintf(buff,sizeof(buff), "%dFilterAction", i);
		conn->Pid = db_get_dw(0, PLUGINNAME, buff, 0);

		conn->next = connHead;
		connHead=conn;
	}
	return connHead;
}
//called to load settings from database
void LoadSettings()
{
	char buff[128];
	int i;
	settingInterval=(INT)db_get_dw(NULL, PLUGINNAME, "Interval", 500);
	settingInterval1=(INT)db_get_dw (NULL, PLUGINNAME, "PopupInterval", 0);
	settingResolveIp=db_get_b  (NULL, PLUGINNAME, "ResolveIp",TRUE);
	settingDefaultAction=db_get_b  (NULL, PLUGINNAME, "FilterDefaultAction",TRUE);
	
	settingSetColours=db_get_b  (NULL, PLUGINNAME, "PopupSetColours",0);
	settingBgColor = (COLORREF)db_get_dw(NULL, PLUGINNAME, "PopupBgColor", (DWORD)0xFFFFFF);
	settingFgColor = (COLORREF)db_get_dw(NULL, PLUGINNAME, "PopupFgColor", (DWORD)0x000000);
	settingFiltersCount=(INT)db_get_dw (NULL, PLUGINNAME, "FiltersCount", 0);
	settingStatusMask=(WORD)db_get_w(NULL, PLUGINNAME, "StatusMask", 16);
	for(i = 0; i < STATUS_COUNT; i++) 
	{
		mir_snprintf(buff,sizeof(buff), "Status%d", i);
		settingStatus[i] = (db_get_b(0, PLUGINNAME, buff, 0) == 1);
	}
	//lookupLotusDefaultSettings();
}

void fillExceptionsListView(HWND hwndDlg)
{
	HWND hwndList=NULL;
	LVITEM lvI={0};

	int i=0;
	struct CONNECTION *tmp=connExceptionsTmp;
	hwndList = GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS);
	ListView_DeleteAllItems(hwndList);

	// Some code to create the list-view control.
	// Initialize LVITEM members that are common to all
	// items. 
	lvI.mask = LVIF_TEXT;
	i=0;
	while(tmp)
	{
		TCHAR tmpAddress[25];
		lvI.iItem=i++;
		lvI.iSubItem=0;
		lvI.pszText=tmp->PName;
		ListView_InsertItem(hwndList, &lvI);
		lvI.iSubItem=1;
		if(tmp->intIntPort==-1)
			mir_sntprintf(tmpAddress,_countof(tmpAddress),_T("%s:*"),tmp->strIntIp);
		else
			mir_sntprintf(tmpAddress,_countof(tmpAddress),_T("%s:%d"),tmp->strIntIp,tmp->intIntPort);
		lvI.pszText=tmpAddress;
		ListView_SetItem(hwndList, &lvI);
		lvI.iSubItem=2;
		if(tmp->intExtPort==-1)
			mir_sntprintf(tmpAddress,_countof(tmpAddress),_T("%s:*"),tmp->strExtIp);
		else
			mir_sntprintf(tmpAddress,_countof(tmpAddress),_T("%s:%d"),tmp->strExtIp,tmp->intExtPort);
		lvI.pszText=tmpAddress;
		ListView_SetItem(hwndList, &lvI);
		lvI.iSubItem=3;
		lvI.pszText=mir_a2t(tmp->Pid?"Show":"Hide");
		ListView_SetItem(hwndList, &lvI);

		tmp=tmp->next;
	}
			
}
//filter editor dialog box procedure opened modally from options dialog
static int CALLBACK FilterEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
		case WM_INITDIALOG:
			{
			struct CONNECTION *conn=(struct CONNECTION*)lParam;
			TranslateDialogDefault(hWnd);
			connCurrentEditModal=conn;
			SetDlgItemText(hWnd, ID_TEXT_NAME,conn->PName);
			SetDlgItemText(hWnd, ID_TXT_LOCAL_IP,conn->strIntIp);
			SetDlgItemText(hWnd, ID_TXT_REMOTE_IP,conn->strExtIp);

			if(conn->intIntPort==-1)
				SetDlgItemText(hWnd, ID_TXT_LOCAL_PORT,_T("*"));
			else
				SetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT,conn->intIntPort,FALSE);

			if(conn->intExtPort==-1)
				SetDlgItemText(hWnd, ID_TXT_REMOTE_PORT,_T("*"));
			else
				SetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT,conn->intExtPort,FALSE);

			SendDlgItemMessage(hWnd, ID_CBO_ACTION , CB_ADDSTRING, 0, (LPARAM)TranslateT("Always show popup"));
			SendDlgItemMessage(hWnd, ID_CBO_ACTION , CB_ADDSTRING, 0, (LPARAM)TranslateT("Never show popup"));
			SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_SETCURSEL ,conn->Pid==0?1:0,0);
			return TRUE;
			}
		case WM_COMMAND:
			switch(LOWORD(wParam))
				{
				case ID_OK:
					{
					TCHAR tmpPort[6];
					GetDlgItemText(hWnd, ID_TXT_LOCAL_PORT, tmpPort, sizeof(tmpPort));
					if (tmpPort[0]=='*')
						connCurrentEditModal->intIntPort=-1;
					else
						connCurrentEditModal->intIntPort=GetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT, NULL, FALSE);
					GetDlgItemText(hWnd, ID_TXT_REMOTE_PORT, tmpPort, sizeof(tmpPort));
					if (tmpPort[0]=='*')
						connCurrentEditModal->intExtPort=-1;
					else
						connCurrentEditModal->intExtPort = GetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT, NULL, FALSE);
					
					GetDlgItemText(hWnd, ID_TXT_LOCAL_IP, connCurrentEditModal->strIntIp, sizeof(connCurrentEditModal->strIntIp));
					GetDlgItemText(hWnd, ID_TXT_REMOTE_IP, connCurrentEditModal->strExtIp, sizeof(connCurrentEditModal->strExtIp));
					GetDlgItemText(hWnd, ID_TEXT_NAME, connCurrentEditModal->PName, sizeof(connCurrentEditModal->PName));

					connCurrentEditModal->Pid=!(BOOL)SendMessage(GetDlgItem(hWnd,ID_CBO_ACTION),CB_GETCURSEL, 0, 0);
					
					connCurrentEditModal=NULL;
					EndDialog(hWnd,IDOK);
					return TRUE;
					}
				case ID_CANCEL:
					connCurrentEditModal=NULL;
					EndDialog(hWnd,IDCANCEL);
					return TRUE;
				}
			return FALSE;
			break;
		case WM_CLOSE:
		{
			connCurrentEditModal=NULL;
			EndDialog(hWnd,IDCANCEL);
			break;
		}
	}
	return FALSE;
}

//options page on miranda called
BOOL CALLBACK DlgProcConnectionNotifyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList;
	switch(msg)
	{
		case WM_INITDIALOG://initialize dialog, so set properties from db.
		{

			LVCOLUMN lvc={0};
			LVITEM lvI={0};
			char *strptr;
			char buff[256];
			int i=0;
			struct CONNECTION *tmp=connExceptions;
			bOptionsOpen=TRUE;
			TranslateDialogDefault(hwndDlg);//translate miranda function
			mir_snprintf(buff,sizeof(buff),"%d.%d.%d.%d", HIBYTE(HIWORD(pluginInfo.version)), LOBYTE(HIWORD(pluginInfo.version)), HIBYTE(LOWORD(pluginInfo.version)), LOBYTE(LOWORD(pluginInfo.version)));
#ifdef WIN64
			strncat(buff,"/64",strlen("/64"));
#else
			strncat(buff,"/32",strlen("/32"));
#endif
			SetDlgItemText(hwndDlg, IDC_VERSION, mir_a2t(buff));
			LoadSettings();
			//connExceptionsTmp=LoadSettingsConnections();
			SetDlgItemInt(hwndDlg, IDC_INTERVAL , settingInterval,FALSE);
			SetDlgItemInt(hwndDlg, IDC_INTERVAL1 , settingInterval1,TRUE);
			CheckDlgButton(hwndDlg, IDC_SETCOLOURS, settingSetColours ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, IDC_RESOLVEIP, settingResolveIp ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, ID_CHK_DEFAULTACTION, settingDefaultAction ? TRUE : FALSE);

			SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingBgColor);
			SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)settingFgColor);
			if(!settingSetColours) 
			{
				HWND hwnd = GetDlgItem(hwndDlg, IDC_BGCOLOR);
				CheckDlgButton(hwndDlg, IDC_SETCOLOURS, FALSE);
				EnableWindow(hwnd, FALSE);
				hwnd = GetDlgItem(hwndDlg, IDC_FGCOLOR);
				EnableWindow(hwnd, FALSE);
			}
			SendDlgItemMessage(hwndDlg, ID_ADD, BM_SETIMAGE, (WPARAM)IMAGE_ICON, ( LPARAM )LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON6),IMAGE_ICON,16,16,0));
			SendDlgItemMessage(hwndDlg, ID_DELETE, BM_SETIMAGE, (WPARAM)IMAGE_ICON, ( LPARAM )LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON3),IMAGE_ICON,16,16,0));	
			SendDlgItemMessage(hwndDlg, ID_DOWN, BM_SETIMAGE, (WPARAM)IMAGE_ICON, ( LPARAM )LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON4),IMAGE_ICON,16,16,0));
			SendDlgItemMessage(hwndDlg, ID_UP, BM_SETIMAGE, (WPARAM)IMAGE_ICON, ( LPARAM )LoadImage(hInst,MAKEINTRESOURCE(IDI_ICON5),IMAGE_ICON,16,16,0));
			// initialise and fill listbox
			hwndList = GetDlgItem(hwndDlg, IDC_STATUS);
			ListView_DeleteAllItems(hwndList);
			SendMessage(hwndList,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
			lvc.iSubItem = 0;
			lvc.pszText = mir_a2t(Translate("Status"));	
			lvc.cx = 120;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);
			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT;
			for(i = 0; i < STATUS_COUNT; i++) 
			{
				strptr = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)(ID_STATUS_ONLINE + i), (LPARAM)0);
				//MultiByteToWideChar((int)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, strptr, -1, buff, 256);
				lvI.pszText= mir_a2t(strptr);
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);
				ListView_SetCheckState(hwndList, i, settingStatus[i]);
			}


			connExceptionsTmp=LoadSettingsConnections();
			hwndList = GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS);
			SendMessage(hwndList,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT);

			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
			lvc.iSubItem = 0;
			lvc.cx = 120;     // width of column in pixels
			lvc.pszText = mir_a2t(Translate("Application"));
			ListView_InsertColumn(hwndList, 1, &lvc);
			lvc.pszText = mir_a2t(Translate("Internal socket"));	
			ListView_InsertColumn(hwndList, 2, &lvc);
			lvc.pszText = mir_a2t(Translate("External socket"));
			ListView_InsertColumn(hwndList, 3, &lvc);
			lvc.pszText = mir_a2t(Translate("Action"));
			lvc.cx = 50;
			ListView_InsertColumn(hwndList, 4, &lvc);

			//fill exceptions list
			fillExceptionsListView(hwndDlg);
				
			break;
		}
		case WM_COMMAND://user changed something, so get changes to variables
			PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			switch(LOWORD(wParam))
			{
				case IDC_INTERVAL: settingInterval =GetDlgItemInt(hwndDlg, IDC_INTERVAL, NULL, FALSE); break;
				case IDC_INTERVAL1: settingInterval1 =GetDlgItemInt(hwndDlg, IDC_INTERVAL1, NULL, TRUE); break;
				case IDC_RESOLVEIP: settingResolveIp=(BYTE) IsDlgButtonChecked(hwndDlg, IDC_RESOLVEIP); break;
				case ID_CHK_DEFAULTACTION: settingDefaultAction=(BYTE) IsDlgButtonChecked(hwndDlg, ID_CHK_DEFAULTACTION); break;
				case ID_ADD:
				{
					struct CONNECTION *cur=(struct CONNECTION *)mir_alloc(sizeof(struct CONNECTION));
					memset(cur,0,sizeof(struct CONNECTION));
					cur->intExtPort=-1;
					cur->intIntPort=-1;
					cur->Pid=0;
					cur->PName[0]='*';
					cur->strExtIp[0]='*';
					cur->strIntIp[0]='*';

					if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FILTER_DIALOG), hwndDlg, (DLGPROC)FilterEditProc, (LPARAM)cur)==IDCANCEL)
					{
						mir_free(cur);
						cur=NULL;
					}
					else
					{
						cur->next=connExceptionsTmp;
						connExceptionsTmp=cur;
					}

					fillExceptionsListView(hwndDlg);
					ListView_SetItemState(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS),0,LVNI_FOCUSED|LVIS_SELECTED , LVNI_FOCUSED|LVIS_SELECTED );
					SetFocus(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS));
					break;
				}
				case ID_DELETE: 
				{
					int pos,pos1;
					struct CONNECTION *cur=NULL,*pre=NULL;

					cur=connExceptionsTmp;

					pos=(int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS),-1,LVNI_FOCUSED );
					if(pos==-1)break;
					pos1=pos;
					while(pos--)
					{
						pre=cur;
						cur=cur->next;
					}
					if (pre==NULL)
						connExceptionsTmp=connExceptionsTmp->next;
					else
						(pre)->next=cur->next;
					mir_free(cur);
					fillExceptionsListView(hwndDlg);
					ListView_SetItemState(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS),pos1,LVNI_FOCUSED|LVIS_SELECTED , LVNI_FOCUSED|LVIS_SELECTED );
					SetFocus(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS));
					break;
				}
				case ID_UP: 
				{
					int pos,pos1;
					struct CONNECTION *cur=NULL,*pre=NULL,*prepre=NULL;

					cur=connExceptionsTmp;

					pos=(int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS),-1,LVNI_FOCUSED );
					if(pos==-1)break;
					pos1=pos;
					while(pos--)
					{
						prepre=pre;
						pre=cur;
						cur=cur->next;
					}
					if (prepre!=NULL)
						{
							pre->next=cur->next;
							cur->next=pre;
							prepre->next=cur;
						}
					else if(pre!=NULL)
						{
							pre->next=cur->next;
							cur->next=pre;
							connExceptionsTmp=cur;							
						}
					fillExceptionsListView(hwndDlg);
					ListView_SetItemState(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS),pos1-1,LVNI_FOCUSED|LVIS_SELECTED , LVNI_FOCUSED|LVIS_SELECTED );
					SetFocus(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS));
					break;
				}
				case ID_DOWN: 
				{
					int pos,pos1;
					struct CONNECTION *cur=NULL,*pre=NULL;

					cur=connExceptionsTmp;

					pos=(int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS),-1,LVNI_FOCUSED );
					if(pos==-1)break;
					pos1=pos;
					while(pos--)
					{
						pre=cur;
						cur=cur->next;
					}
					if (cur==connExceptionsTmp&&cur->next!=NULL)
						{
							connExceptionsTmp=cur->next;
							cur->next=cur->next->next;
							connExceptionsTmp->next=cur;
						}
					else if (cur->next!=NULL)
						{
							struct CONNECTION *tmp=cur->next->next;
							pre->next=cur->next;
							cur->next->next=cur;
							cur->next=tmp;
						}
					fillExceptionsListView(hwndDlg);
					ListView_SetItemState(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS),pos1+1,LVNI_FOCUSED|LVIS_SELECTED , LVNI_FOCUSED|LVIS_SELECTED );
					SetFocus(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS));
					break;
				}				
				case IDC_SETCOLOURS:
				{
					HWND hwnd = GetDlgItem(hwndDlg, IDC_BGCOLOR);
					settingSetColours=IsDlgButtonChecked(hwndDlg, IDC_SETCOLOURS);
					EnableWindow(hwnd,settingSetColours );
					hwnd = GetDlgItem(hwndDlg, IDC_FGCOLOR);
					EnableWindow(hwnd, settingSetColours);
					break;
				}
				case IDC_BGCOLOR: settingBgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);break;
				case IDC_FGCOLOR: settingFgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_GETCOLOUR, 0, 0);break;

			}
			break;

			case WM_NOTIFY://apply changes so write it to db
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_RESET:
						LoadSettings();
							deleteConnectionsTable(connExceptionsTmp);
							connExceptionsTmp=LoadSettingsConnections();
							return TRUE;
						case PSN_APPLY:
						{
							char buff[128];
							int i=0;
							db_set_dw (NULL, PLUGINNAME, "Interval", settingInterval );
							db_set_dw (NULL, PLUGINNAME, "PopupInterval", settingInterval1 );
							db_set_b(NULL, PLUGINNAME, "PopupSetColours", settingSetColours);
							db_set_dw(NULL, PLUGINNAME, "PopupBgColor", (DWORD)settingBgColor);
							db_set_dw(NULL, PLUGINNAME, "PopupFgColor", (DWORD)settingFgColor);
							db_set_b(NULL, PLUGINNAME, "ResolveIp", settingResolveIp);
							db_set_b(NULL, PLUGINNAME, "FilterDefaultAction", settingDefaultAction);
								
							for(i = 0; i < STATUS_COUNT ; i++) 
							{
								mir_snprintf(buff,_countof(buff), "Status%d", i);
								settingStatus[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_STATUS), i) ? TRUE : FALSE);
								db_set_b(0, PLUGINNAME, buff, settingStatus[i] ? 1 : 0);
							}
							if( WAIT_OBJECT_0 == WaitForSingleObject( hExceptionsMutex, 100 ) )
							{
								deleteConnectionsTable(connExceptions);
								saveSettingsConnections(connExceptionsTmp);
								connExceptions=connExceptionsTmp;
								connExceptionsTmp=LoadSettingsConnections();
								ReleaseMutex(hExceptionsMutex);
							}
						}//case PSN_APPLY
							

						return TRUE;
						break;
					}//switch ->code
					break;
				}//case 0
			}//id from
			if (GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS)==((LPNMHDR) lParam)->hwndFrom)
			{
				switch (((LPNMHDR) lParam)->code)
				{
					case NM_DBLCLK:
					{
						int pos,pos1;
						struct CONNECTION *cur=NULL;

						cur=connExceptionsTmp;

						pos=(int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS),-1,LVNI_FOCUSED );
						if(pos==-1)break;
						pos1=pos;
						while(pos--)
						{
							cur=cur->next;
						}
						DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FILTER_DIALOG), hwndDlg, (DLGPROC)FilterEditProc, (LPARAM)cur);
						fillExceptionsListView(hwndDlg);
						ListView_SetItemState(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS),pos1,LVNI_FOCUSED|LVIS_SELECTED , LVNI_FOCUSED|LVIS_SELECTED );
						SetFocus(GetDlgItem(hwndDlg,IDC_LIST_EXCEPTIONS));
						break;
					}
				}
			}
			if (GetDlgItem(hwndDlg, IDC_STATUS)==((LPNMHDR) lParam)->hwndFrom)
			{
				switch (((LPNMHDR) lParam)->code)
				{
					case LVN_ITEMCHANGED:
					{
						NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
						if((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK)
						{
							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
						break;
							
					}
					break;
				}
			}
			break;//switch(msg)
		case WM_DESTROY:
			{
				bOptionsOpen=FALSE;
				deleteConnectionsTable(connExceptionsTmp);
				connExceptionsTmp=NULL;
				return TRUE;
			}
		}//end switch(msg)
	return 0;
}
//options page on miranda called
int ConnectionNotifyOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp={0};
	//ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DIALOG);
	odp.ptszTitle = LPGENT(PLUGINNAME);
	odp.ptszGroup = LPGENT("Plugins");
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.pfnDlgProc = (DLGPROC)DlgProcConnectionNotifyOpts;//callback function name
//	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);//add page to options menu pages
	Options_AddPage(wParam, &odp);
	return 0;
}


//gives protocol avainable statuses
int GetCaps(WPARAM wParam,LPARAM lParam)
{
	if(wParam==PFLAGNUM_1)
		return 0;
	if(wParam==PFLAGNUM_2)
		return PF2_ONLINE; // add the possible statuses here.
	if(wParam==PFLAGNUM_3)
		return 0;
	return 0;
}
//gives  name to protocol module
int GetName(WPARAM wParam,LPARAM lParam)
{
	lstrcpynA((char*)lParam,PLUGINNAME,wParam);
	return 0;
}

//gives icon for proto module
int TMLoadIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam & 0xFFFF) {
		case PLI_ONLINE:
		case PLI_PROTOCOL: id=IDI_ICON1; break; // IDI_TM is the main icon for the protocol
		case PLI_OFFLINE: id=IDI_ICON2; break;
		default: return 0;	
	}
	return (int)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON), GetSystemMetrics(wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON), 0);
}
//=======================================================
//SetStatus
//=======================================================
int SetStatus(WPARAM wParam,LPARAM lParam)
{
	if (wParam == ID_STATUS_OFFLINE)
	{
		diffstat=0;
		//PostThreadMessage(ConnectionCheckThreadId,WM_QUIT ,(WPARAM)0, (LPARAM)0);
		SetEvent(killCheckThreadEvent);

	}
	else if (wParam == ID_STATUS_ONLINE)
	{
		diffstat=0;
		ResetEvent(killCheckThreadEvent);
		if(!hConnectionCheckThread)
			hConnectionCheckThread = (HANDLE)mir_forkthreadex(checkthread, 0, (unsigned int*)&ConnectionCheckThreadId);
	}
	else
	{
		int retv=0;
		
		if(settingStatus[wParam - ID_STATUS_ONLINE])
			retv= SetStatus(ID_STATUS_OFFLINE,lParam);
		else
			retv= SetStatus(ID_STATUS_ONLINE,lParam);
		//LNEnableMenuItem(hMenuHandle ,TRUE);
		diffstat=wParam;
		return retv;

		// the status has been changed to unknown  (maybe run some more code)
	}
	//broadcast the message
	
	//oldStatus = currentStatus;
	if(currentStatus!=wParam)
		ProtoBroadcastAck(PLUGINNAME,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)currentStatus,wParam);
	currentStatus = wParam;
	return 0;

}
//=======================================================
//GetStatus
//=======================================================
int GetStatus(WPARAM wParam,LPARAM lParam)
{
	return currentStatus;
	if (diffstat )
		return diffstat;
	else
		return currentStatus ;

}
/*
void updaterRegister()
{

	Update update = {0}; // for c you'd use memset or ZeroMemory...
	//ZeroMemory(&update,sizeof(update));
	TCHAR buff[256];
#ifdef _DEBUG
	_OutputDebugString("Registering updater...");
#endif
	update.cbSize = sizeof(Update);

	update.szComponentName = pluginInfo.shortName;
	mir_snprintf(buff,sizeof(buff),"%d.%d.%d.%d", HIBYTE(HIWORD(pluginInfo.version)), LOBYTE(HIWORD(pluginInfo.version)), HIBYTE(LOWORD(pluginInfo.version)), LOBYTE(LOWORD(pluginInfo.version)));
	update.pbVersion = (BYTE*)buff;
	update.cpbVersion = _tcslen((TCHAR *)update.pbVersion);

	update.szUpdateURL=UPDATER_AUTOREGISTER;

#ifdef WIN64
	update.szBetaUpdateURL = _T("http://maciej.wycik.pl/miranda/getconnectionnotifybeta.php?dl=64");
	update.pbBetaVersionPrefix = (BYTE *)"beta version ";
#else
	update.szBetaUpdateURL = _T("http://maciej.wycik.pl/miranda/getconnectionnotifybeta.php?dl=");
	update.pbBetaVersionPrefix = (BYTE *)"beta version ";
#endif
	update.szBetaVersionURL  = _T("http://maciej.wycik.pl/connectionnotify.php");
	update.cpbBetaVersionPrefix = _tcslen((TCHAR *)update.pbBetaVersionPrefix);
	

	// do the same for the beta versions of the above struct members if you wish to allow beta updates from another URL

	CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	return;
}
*/

//thread function with connections check loop
static unsigned __stdcall checkthread(void *dummy)
{

#ifdef _DEBUG
	_OutputDebugString(_T("check thread started"));
#endif
	while(1)
	{
		struct CONNECTION* conn=NULL,*connOld=first,*cur=NULL;
#ifdef _DEBUG
	_OutputDebugString(_T("checking connections table..."));
#endif
	if(WAIT_OBJECT_0 == WaitForSingleObject(killCheckThreadEvent,100))
	{
		hConnectionCheckThread=NULL;
		return 0;
	}
	//TranslateMessage(&msg);
		//DispatchMessage(&msg);
	
//		if(currentStatus==ID_STATUS_OFFLINE)
//			continue;//if status lets to check

		conn=GetConnectionsTable();
		cur=conn;
		while(cur!=NULL)
		{	
			if (searchConnection(first,cur->strIntIp,cur->strExtIp,cur->intIntPort,cur->intExtPort,cur->state)==NULL && (settingStatusMask & (1 << (cur->state-1))))
			{
				

#ifdef _DEBUG
	TCHAR msg[1024];
	mir_sntprintf(msg,_countof(msg),_T("%s:%d\n%s:%d"),cur->strIntIp,cur->intIntPort,cur->strExtIp,cur->intExtPort);
	_OutputDebugString(_T("New connection: %s"),msg);
#endif
				pid2name(cur->Pid,cur->PName);
				if( WAIT_OBJECT_0 == WaitForSingleObject( hExceptionsMutex, 100 ) )
				{
					if(checkFilter(connExceptions,cur))
						showMsg(cur->PName,cur->Pid,cur->strIntIp,cur->strExtIp,cur->intIntPort,cur->intExtPort,cur->state);
					ReleaseMutex(hExceptionsMutex);
				}
			}
			cur=cur->next;
		}

		first=conn;
		deleteConnectionsTable(connOld);
		Sleep(settingInterval);
	}
	hConnectionCheckThread=NULL;
	return 1;
}

//popup reactions
static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
		case WM_COMMAND:
			{
				if (HIWORD(wParam) == STN_CLICKED)//client clicked on popup with left mouse button
				{
					struct CONNECTION *conn,*mpd=NULL;

					conn = (struct CONNECTION*)mir_alloc(sizeof(struct CONNECTION));
					mpd=(struct CONNECTION*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
					
					memcpy(conn,mpd,sizeof(struct CONNECTION));
					PUDeletePopup(hWnd);
					PostThreadMessage(FilterOptionsThreadId,WM_ADD_FILTER,(WPARAM)0, (LPARAM)conn);
					

				}
				break;
			}

		case WM_RBUTTONUP:
			{
				PUDeletePopup(hWnd);
				break;
			}		

		case UM_INITPOPUP:
			{
				//struct CONNECTON *conn=NULL;
				//conn = (struct CONNECTION*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)conn);
				//MessageBox(NULL,conn->extIp);
				//PUDeletePopUp(hWnd);
				break;
			}

		case UM_FREEPLUGINDATA: 
			{
				struct CONNECTION * mpd = NULL;
				mpd = (struct CONNECTION*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
				if (mpd > 0) mir_free(mpd);
				return TRUE; //TRUE or FALSE is the same, it gets ignored.
			}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


//show popup
void showMsg(TCHAR *pName, DWORD pid,TCHAR *intIp,TCHAR *extIp,int intPort,int extPort,int state)
{

	POPUPDATAT ppd;
	TCHAR hostName[128];
	//hContact = A_VALID_HANDLE_YOU_GOT_FROM_SOMEWHERE;
	//hIcon = A_VALID_HANDLE_YOU_GOT_SOMEWHERE;
	//char * lpzContactName = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)lhContact,0);
	//99% of the times you'll just copy this line.
	//1% of the times you may wish to change the contact's name. I don't know why you should, but you can.
	//char * lpzText;
	//The text for the second line. You could even make something like: char lpzText[128]; lstrcpy(lpzText, "Hello world!"); It's your choice.

	struct CONNECTION *mpd = (struct CONNECTION*)mir_alloc(sizeof(struct CONNECTION));
	//MessageBox(NULL,"aaa","aaa",1);
	ZeroMemory(&ppd, sizeof(ppd)); //This is always a good thing to do.
	ppd.lchContact = NULL;//(HANDLE)hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	if(settingResolveIp)
		getDnsName(extIp,hostName);

	mir_sntprintf(ppd.lpwzText,_countof(ppd.lpwzText),_T("%s:%d\n%s:%d"),(settingResolveIp?hostName:extIp),extPort,intIp,intPort);
	mir_sntprintf(ppd.lpwzContactName,_countof(ppd.lpwzContactName),_T("%s (%s)"),pName,tcpStates[state-1]);
	//lstrcpy(ppd.lpzText, text);
	if(settingSetColours)
	{
		ppd.colorBack = settingBgColor; 
		ppd.colorText = settingFgColor;
	}
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;

	ppd.iSeconds=settingInterval1;
	//Now the "additional" data.
	mir_sntprintf(mpd->strIntIp,_countof(mpd->strIntIp),_T("%s"),intIp);
	mir_sntprintf(mpd->strExtIp,_countof(mpd->strExtIp),_T("%s"),extIp);
	mir_sntprintf(mpd->PName,_countof(mpd->PName),_T("%s"),pName);
	mpd->intIntPort = intPort;
	mpd->intExtPort = extPort;
	mpd->Pid=pid;


	//mpd->newStatus = ID_STATUS_ONLINE;

	//Now that the plugin data has been filled, we add it to the PopUpData.
	ppd.PluginData = mpd;

	//Now that every field has been filled, we want to see the popup.
	//CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
	PUAddPopupT(&ppd);
}



//called after all plugins loaded.
//all Connection staff will be called, that will not hang miranda on startup
static int modulesloaded(WPARAM wParam,LPARAM lParam)
{
	
#ifdef _DEBUG
	_OutputDebugString(_T("Modules loaded, lets start TN..."));
#endif
//	hConnectionCheckThread = (HANDLE)mir_forkthreadex(checkthread, 0, 0, ConnectionCheckThreadId);

//#ifdef _DEBUG
//	_OutputDebugString("started check thread %d",hConnectionCheckThread);
//#endif
	killCheckThreadEvent=CreateEvent(NULL,FALSE,FALSE,_T("killCheckThreadEvent"));
	hFilterOptionsThread=startFilterThread();
	//updaterRegister();
	
	return 0;
}
//function hooks before unload
static int preshutdown(WPARAM wParam,LPARAM lParam)
{

	deleteConnectionsTable(first);
	deleteConnectionsTable(connExceptions);
	deleteConnectionsTable(connExceptionsTmp);

	PostThreadMessage(ConnectionCheckThreadId,WM_QUIT ,(WPARAM)0, (LPARAM)0);
	PostThreadMessage(FilterOptionsThreadId,WM_QUIT ,(WPARAM)0, (LPARAM)0);
	
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}
extern "C" int __declspec(dllexport) Load(void)
{
	char service[100]={""};

	PROTOCOLDESCRIPTOR pd={ PROTOCOLDESCRIPTOR_V3_SIZE };

#ifdef _DEBUG
	_OutputDebugString(_T("Entering Load dll"));
#endif

	mir_getLP( &pluginInfo );
	//hCurrentEditMutex=CreateMutex(NULL,FALSE,_T("CurrentEditMutex"));
	hExceptionsMutex=CreateMutex(NULL,FALSE,_T("ExceptionsMutex"));

	LoadSettings();
	connExceptions=LoadSettingsConnections();
//create protocol
	//ZeroMemory(&pd,sizeof(pd));
	//pd.cbSize=sizeof(pd);
 	pd.szName=PLUGINNAME;
	pd.type=PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);
	//set all contacts to offline

	for (HANDLE hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
		if(!lstrcmpA(PLUGINNAME,(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0)))
			db_set_w(hContact,PLUGINNAME,"status",ID_STATUS_OFFLINE);

	mir_snprintf(service,sizeof(service), "%s%s", PLUGINNAME, PS_GETCAPS);
	CreateServiceFunction(service, (MIRANDASERVICE)GetCaps);
	mir_snprintf(service,sizeof(service), "%s%s", PLUGINNAME, PS_GETNAME);
	CreateServiceFunction(service, (MIRANDASERVICE)GetName);
	mir_snprintf(service,sizeof(service), "%s%s", PLUGINNAME, PS_LOADICON);
	CreateServiceFunction(service, (MIRANDASERVICE)TMLoadIcon);
	mir_snprintf(service,sizeof(service), "%s%s", PLUGINNAME, PS_SETSTATUS);
	CreateServiceFunction(service, (MIRANDASERVICE)SetStatus);
	mir_snprintf(service,sizeof(service), "%s%s", PLUGINNAME, PS_GETSTATUS);
	CreateServiceFunction(service, (MIRANDASERVICE)GetStatus);


	SkinAddNewSound(PLUGINNAME,Translate("ConnectionNotify: New Connection Notification"),NULL);
	hOptInit = HookEvent(ME_OPT_INITIALISE, ConnectionNotifyOptInit);//register service to hook option call
	assert(hOptInit);
	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, modulesloaded);//hook event that all plugins are loaded
	assert(hHookModulesLoaded);
	hHookPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, preshutdown);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	WaitForSingleObjectEx(hConnectionCheckThread, INFINITE, FALSE);
        if(hConnectionCheckThread )CloseHandle(hConnectionCheckThread);
	if(hCheckEvent)DestroyHookableEvent(hCheckEvent);
	if (hOptInit) UnhookEvent(hOptInit);
	if (hCheckHook)UnhookEvent(hCheckHook);
	if(hHookModulesLoaded)UnhookEvent(hHookModulesLoaded);
	if(hHookPreShutdown)UnhookEvent(hHookPreShutdown);
	if(killCheckThreadEvent)
		CloseHandle(killCheckThreadEvent);
	//if(hCurrentEditMutex) CloseHandle(hCurrentEditMutex);
	if(hExceptionsMutex) CloseHandle(hExceptionsMutex);

#ifdef _DEBUG
	_OutputDebugString(_T("Unloaded"));
#endif
	return 0;
}

