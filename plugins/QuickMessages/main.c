/*
Quick Messages plugin for Miranda IM

Copyright (C) 2008 Danil Mozhar

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

#include "quickmessages.h"

HINSTANCE hinstance;

WNDPROC mainProc;

HANDLE hEventCBButtonPressed, hEventCBInit,hEventInputMenu,  hEventDbOptionsInit, hEventDbPluginsLoaded,
hEventDbPreShutdown, hIcon ;	

int g_iButtonsCount=0;
int g_bShutDown=0;   	
int g_bStartup=0;
BOOL g_bRClickAuto=0;
BOOL g_bLClickAuto=0;
BOOL g_bQuickMenu=0;

struct MM_INTERFACE mmi;																					   
struct LIST_INTERFACE li;

PLUGINLINK *pluginLink;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	MODULENAME,
	PLUGIN_MAKE_VERSION(0,0,4,2),
	"Plugin for quick insert pre-defined messages in message input area.",
	"MaD_CLuSTeR",
	"daniok@yandex.ru",
	"© 2008 Danil Mozhar",
	"http://miranda.radicaled.ru/index.php?plugin=3",
	UNICODE_AWARE,
	0,	
#ifdef _UNICODE
	// {37ED754B-6CF9-40ed-9EB6-0FEF8E822475}
		{ 0x37ed754b, 0x6cf9, 0x40ed, { 0x9e, 0xb6, 0xf, 0xef, 0x8e, 0x82, 0x24, 0x75 } }
#else
	// {A15C6605-75F3-4be2-9193-6D3DDD0A13B7} 
		{ 0xa15c6605, 0x75f3, 0x4be2, { 0x91, 0x93, 0x6d, 0x3d, 0xdd, 0xa, 0x13, 0xb7 } }
#endif
	};


int PreShutdown(WPARAM wparam,LPARAM lparam)
	{	 
	g_bShutDown=1;
	DestructButtonsList();

	if(hEventCBButtonPressed)	UnhookEvent(hEventCBButtonPressed);
	if(hEventCBInit)			UnhookEvent(hEventCBInit);
	if(hEventInputMenu)			UnhookEvent(hEventInputMenu);
	UnhookEvent(hEventDbPluginsLoaded);
	UnhookEvent(hEventDbOptionsInit);
	UnhookEvent(hEventDbPreShutdown);

	return 0;
	}
static int InputMenuPopup(WPARAM wParam,LPARAM lParam)
	{
	HMENU hSubMenu=NULL;
	int i=0;
	MessageWindowPopupData * mwpd = (MessageWindowPopupData *)lParam;
	if(mwpd->uFlags==MSG_WINDOWPOPUP_LOG||!g_bQuickMenu||!QuickList->realCount) return 0;

	if(mwpd->uType==MSG_WINDOWPOPUP_SHOWING)
		{
		hSubMenu = CreatePopupMenu();

		InsertMenu((HMENU)mwpd->hMenu,6,MF_STRING|MF_POPUP|MF_BYPOSITION,(UINT_PTR)hSubMenu,TranslateT("Quick Messages"));
		InsertMenu((HMENU)mwpd->hMenu,7,MF_SEPARATOR|MF_BYPOSITION,0,0);
		qsort(QuickList->items,QuickList->realCount,sizeof(QuickData *),sstQuickSortButtons); 
		for(i=0;i<QuickList->realCount;i++)
			{
			QuickData* qd= (QuickData *)QuickList->items[i];
			if(qd->fEntryType&QMF_EX_SEPARATOR)
				AppendMenu(hSubMenu,MF_SEPARATOR,0,NULL);
			else
				AppendMenu(hSubMenu,MF_STRING,qd->dwPos+254,qd->ptszValueName);
			}
		}
	else if(mwpd->uType==MSG_WINDOWPOPUP_SELECTED&&mwpd->selection>=254)
		{
		for(i=0;i<QuickList->realCount;i++)
			{
			QuickData* qd= (QuickData *)QuickList->items[i];
			if ((qd->dwPos+254)==mwpd->selection)
				{
				CHARRANGE cr;
				UINT textlenght=0;
				TCHAR* pszText=NULL;
				TCHAR* ptszQValue=NULL;
				TCHAR* pszCBText=NULL;
				BOOL bIsService=0;

				if(IsClipboardFormatAvailable(CF_TEXT)) {
					if (OpenClipboard(mwpd->hwnd)) {
						HANDLE hData=NULL;
						TCHAR* chBuffer=NULL; 
						int textLength=0;
#ifdef _UNICODE
						hData= GetClipboardData(CF_UNICODETEXT);
#else 
						hData = GetClipboardData(CF_TEXT);
#endif
						chBuffer= (TCHAR*)GlobalLock(hData);
						textLength=(int)_tcslen(chBuffer);
						pszCBText=mir_tstrdup(chBuffer);
						GlobalUnlock(hData);
						CloseClipboard();
						}
					}
				
				SendMessage(mwpd->hwnd, EM_EXGETSEL, 0, (LPARAM)&cr); 
				textlenght=cr.cpMax-cr.cpMin;
				
				if(textlenght)
					{
					pszText=mir_alloc((textlenght+10)*sizeof(TCHAR));
					ZeroMemory(pszText,(textlenght+10)*sizeof(TCHAR));
					SendMessage(mwpd->hwnd,EM_GETSELTEXT, 0, (LPARAM)pszText);
					}
				if(qd->ptszValue){
						ptszQValue=ParseString(mwpd->hContact,qd->ptszValue,pszText?pszText:_T(""),pszCBText?pszCBText:_T(""),(int)_tcslen(qd->ptszValue),textlenght,pszCBText?(int)_tcslen(pszCBText):0);
					if ((bIsService=qd->bIsService)&&ptszQValue)
#ifdef _UNICODE
						CallService(mir_u2a(ptszQValue),(WPARAM)mwpd->hContact,0);
#else
						CallService(ptszQValue,(WPARAM)mwpd->hContact,0);
#endif
					}

				if(ptszQValue)
					SendMessage(mwpd->hwnd, EM_REPLACESEL, TRUE, (LPARAM)ptszQValue);

				if(pszText) mir_free(pszText);
				if(ptszQValue) free(ptszQValue);
				if(pszCBText) mir_free(pszCBText);
				break;
				}
			}
		return 1;
		}
	return 0;
	}

static int CustomButtonPressed(WPARAM wParam,LPARAM lParam)
	{
	CustomButtonClickData *cbcd=(CustomButtonClickData *)lParam;

	CHARRANGE cr;
	HWND hEdit=NULL;
	int i=0;
	BOOL bCTRL=0;
	BOOL bIsService=0;
	TCHAR* pszText=NULL;
	TCHAR* pszCBText=NULL;
	TCHAR* pszFormatedText=NULL;
	TCHAR* ptszQValue=NULL;
	int cblenght=0,qvlenght=0;
	int count=0,mode=0;
	UINT textlenght=0;
	BBButton bbd={0};
	SortedList* sl=NULL;
	int state=0;

	if(strcmp(cbcd->pszModule,PLGNAME)) return 0;

	if (!ButtonsList[cbcd->dwButtonId]) return 1;
	
	sl=ButtonsList[cbcd->dwButtonId]->sl;

	if (!sl) return 1;

	if(IsClipboardFormatAvailable(CF_TEXT)) {
		if (OpenClipboard(cbcd->hwndFrom)) {
			HANDLE hData=NULL;
			TCHAR* chBuffer=NULL; 
			int textLength=0;
#ifdef _UNICODE
			hData= GetClipboardData(CF_UNICODETEXT);
#else 
			hData = GetClipboardData(CF_TEXT);
#endif
			chBuffer= (TCHAR*)GlobalLock(hData);
			textLength=(int)_tcslen(chBuffer);
			pszCBText=mir_tstrdup(chBuffer);
			GlobalUnlock(hData);
			CloseClipboard();
			}
		}




	qsort(sl->items,sl->realCount,sizeof(ButtonData *),sstSortButtons); 

	hEdit=GetDlgItem(cbcd->hwndFrom,IDC_MESSAGE);
	if (!hEdit) hEdit=GetDlgItem(cbcd->hwndFrom,IDC_CHATMESSAGE);

	cr.cpMin = cr.cpMax = 0;
	SendMessage(hEdit, EM_EXGETSEL, 0, (LPARAM)&cr); 

	textlenght=cr.cpMax-cr.cpMin;
	if(textlenght)
		{
		pszText=mir_alloc((textlenght+10)*sizeof(TCHAR));
		ZeroMemory(pszText,(textlenght+10)*sizeof(TCHAR));
		SendMessage(hEdit,EM_GETSELTEXT, 0, (LPARAM)pszText);
		}

	if(cbcd->flags&BBCF_RIGHTBUTTON)
		state=1;
	else if(sl->realCount==1)
		state=2;
	else 
		state=3;


	switch(state)
		{
		case 1:
			if(ButtonsList[cbcd->dwButtonId]->ptszQValue)
				ptszQValue=ParseString(cbcd->hContact,ButtonsList[cbcd->dwButtonId]->ptszQValue,pszText?pszText:_T(""),pszCBText?pszCBText:_T(""),(int)_tcslen(ButtonsList[cbcd->dwButtonId]->ptszQValue),textlenght,pszCBText?(int)_tcslen(pszCBText):0);
			if ((bIsService=ButtonsList[cbcd->dwButtonId]->bIsServName)&&ptszQValue)
#ifdef _UNICODE
				CallService(mir_u2a(ptszQValue),(WPARAM)cbcd->hContact,0);
#else
				CallService(ptszQValue,(WPARAM)cbcd->hContact,0);
#endif	
			break;
		case 2:
			{
			ButtonData * bd=NULL;
			bd=(ButtonData *)sl->items[0];
			if(bd&&bd->pszValue){
				ptszQValue=ParseString(cbcd->hContact,bd->pszValue,pszText?pszText:_T(""),pszCBText?pszCBText:_T(""),(int)_tcslen(bd->pszValue),textlenght,pszCBText?(int)_tcslen(pszCBText):0);
				if ((bIsService=bd->bIsServName)&&ptszQValue)
#ifdef _UNICODE
						CallService(mir_u2a(ptszQValue),(WPARAM)cbcd->hContact,0);
#else
						CallService(ptszQValue,(WPARAM)cbcd->hContact,0);
#endif			
				}
			}
			break;
		case 3:{
			int res=0;
			int menunum;
			ButtonData * bd=NULL;
			HMENU hMenu=NULL,hSubMenu=NULL;
			BOOL bSetPopUpMark=FALSE;

			if(g_iButtonsCount){
				hMenu = CreatePopupMenu();
				}
			else break;
			for(menunum=0;menunum<sl->realCount;menunum++)
				{
				bd=(ButtonData *)sl->items[menunum];
				if(bd->dwOPFlags&QMF_NEW)
					continue;

				bSetPopUpMark=FALSE;
				if(bd->pszValue==0&&bd->fEntryType==0)
					{
					hSubMenu = CreatePopupMenu();
					bSetPopUpMark=TRUE;
					}

				if(bd->pszValue&&bd->fEntryType==0)
					hSubMenu=NULL;

				if(bd->fEntryType&QMF_EX_SEPARATOR)
					AppendMenu((HMENU)((hSubMenu&&!bSetPopUpMark)?hSubMenu:hMenu),MF_SEPARATOR,0,NULL);
				else
					AppendMenu((HMENU)((hSubMenu&&!bSetPopUpMark)?hSubMenu:hMenu),
						MF_STRING|(bSetPopUpMark?MF_POPUP:0),
						(bSetPopUpMark?(UINT_PTR)hSubMenu:(menunum+1)),	bd->pszName);
				}

			res = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, NULL);
			if(res==0) break;

			bd= (ButtonData *)sl->items[res-1];
			bCTRL=(GetKeyState(VK_CONTROL)&0x8000)?1:0;
			if(bd->pszValue){
				ptszQValue=ParseString(cbcd->hContact,bd->pszValue,pszText?pszText:_T(""),pszCBText?pszCBText:_T(""),(int)_tcslen(bd->pszValue),textlenght,pszCBText?(int)_tcslen(pszCBText):0);
					if ((bIsService=bd->bIsServName)&&ptszQValue)
#ifdef _UNICODE
						CallService(mir_u2a(ptszQValue),(WPARAM)cbcd->hContact,0);
#else
						CallService(ptszQValue,(WPARAM)cbcd->hContact,0);
#endif
				}
			}break;
		}


	if(ptszQValue){
		if (!bIsService){
			SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)ptszQValue);

			if ((g_bLClickAuto&&state!=1)||(g_bRClickAuto&&state==1)||cbcd->flags&BBCF_CONTROLPRESSED||bCTRL)
				SendMessage(cbcd->hwndFrom, WM_COMMAND,IDOK,0);
		}
		free(ptszQValue);
		}
		

	if(pszText) mir_free(pszText);
	if(pszCBText) mir_free(pszCBText);
	return 1;
	}


static int PluginInit(WPARAM wparam,LPARAM lparam)
	{
	g_bStartup=1;
	hEventDbOptionsInit=HookEvent(ME_OPT_INITIALISE,OptionsInit);
	hEventCBButtonPressed=HookEvent(ME_MSG_BUTTONPRESSED,CustomButtonPressed); 
	hEventCBInit=HookEvent(ME_MSG_TOOLBARLOADED,RegisterCustomButton);

	hEventInputMenu=HookEvent(ME_MSG_WINDOWPOPUP,InputMenuPopup);


	g_bRClickAuto=DBGetContactSettingByte(NULL,PLGNAME,"RClickAuto",0);
	g_bLClickAuto=DBGetContactSettingByte(NULL,PLGNAME,"LClickAuto",0);
	g_iButtonsCount=DBGetContactSettingByte(NULL, PLGNAME,"ButtonsCount", 0);
	g_bQuickMenu=DBGetContactSettingByte(NULL, PLGNAME,"QuickMenu", 1);

	if(ServiceExists(MS_UPDATE_REGISTER)) {
		char buffer[1024];
		Update update = {0};
		update.cbSize = sizeof(Update);
		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *) CreateVersionString(pluginInfo.version, buffer);
		update.cpbVersion = (int)strlen((char *) update.pbVersion);
		//update.szUpdateURL = UPDATER_AUTOREGISTER;
		update.szBetaVersionURL = QMESSAGES_VERSION_URL;
		update.szBetaUpdateURL = QMESSAGES_UPDATE_URL;
		update.pbBetaVersionPrefix = (BYTE *) QMESSAGES_VERSION_PREFIX;
		update.cpbBetaVersionPrefix = (int)strlen(QMESSAGES_VERSION_PREFIX);
		update.szBetaChangelogURL=QMESSAGES_CHAGELOG_URL;
		CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);
		}

	hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_QICON));

	InitButtonsList();

	g_bStartup=0;
	return 0;
	}


// {651EAD6E-00C6-47e4-94A3-C63789ED89F9}
static const MUUID interfaces[] = {{ 0x651ead6e, 0xc6, 0x47e4, { 0x94, 0xa3, 0xc6, 0x37, 0x89, 0xed, 0x89, 0xf9 } }, MIID_LAST};
const  __declspec(dllexport) MUUID* MirandaPluginInterfaces(void){
	return interfaces;}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

__declspec(dllexport)int Unload(void)
	{
	return 0;
	}

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
	{
	hinstance=hinst;
	return 1;
	}

int __declspec(dllexport)Load(PLUGINLINK *link)
	{	

	pluginLink=link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getLP(&pluginInfo);

	hEventDbPluginsLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,PluginInit);
	hEventDbPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,PreShutdown);
	return 0;
	}
