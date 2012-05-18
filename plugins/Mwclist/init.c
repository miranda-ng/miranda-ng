/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project,
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

HINSTANCE g_hInst = 0;
PLUGINLINK * pluginLink;
CLIST_INTERFACE* pcli = NULL;
int hLangpack;

struct LIST_INTERFACE li;
struct MM_INTERFACE mmi;
static HANDLE hCListShutdown = 0;

HMENU BuildGroupPopupMenu( struct ClcGroup* group );

void  CalcEipPosition( struct ClcData *dat, struct ClcContact *contact, struct ClcGroup *group, POINT *result);
void  CheckPDNCE(ClcCacheEntryBase*);
void  CluiProtocolStatusChanged( int, const char* );
int   CompareContacts( const struct ClcContact *contact1, const struct ClcContact *contact2 );
void  FreeDisplayNameCacheItem(ClcCacheEntryBase*);
void  GetDefaultFontSetting(int i,LOGFONT *lf,COLORREF *colour);
void  RebuildEntireList(HWND hwnd,struct ClcData *dat);
void  RecalcScrollBar(HWND hwnd,struct ClcData *dat);

struct ClcGroup* ( *saveAddGroup )(HWND hwnd,struct ClcData *dat,const TCHAR *szName,DWORD flags,int groupId,int calcTotalMembers);
struct ClcGroup* ( *saveRemoveItemFromGroup )(HWND hwnd,struct ClcGroup *group,struct ClcContact *contact,int updateTotalCount);

void (*saveFreeContact)(struct ClcContact *p);
void (*saveFreeGroup)(struct ClcGroup *p);

LRESULT ( CALLBACK *saveContactListControlWndProc )( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ( CALLBACK *saveContactListWndProc )(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ( *saveLoadClcOptions )(HWND hwnd,struct ClcData *dat);

int ( *saveAddItemToGroup )( struct ClcGroup *group, int iAboveItem );
int AddItemToGroup(struct ClcGroup *group, int iAboveItem);

int ( *saveAddInfoItemToGroup)(struct ClcGroup *group,int flags,const TCHAR *pszText);
int AddInfoItemToGroup(struct ClcGroup *group,int flags,const TCHAR *pszText);

void ( *saveSortCLC )(HWND hwnd,struct ClcData *dat,int useInsertionSort);
void SortCLC(HWND hwnd,struct ClcData *dat,int useInsertionSort);

INT_PTR ( *saveTrayIconProcessMessage )(WPARAM wParam,LPARAM lParam);
INT_PTR TrayIconProcessMessage(WPARAM wParam,LPARAM lParam);

int  (*saveIconFromStatusMode)(const char *szProto,int nStatus, HANDLE hContact);
int  cli_IconFromStatusMode(const char *szProto,int nStatus, HANDLE hContact);


//from bgrcfg
extern int BGModuleLoad();
extern int BGModuleUnload();

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	#if defined( _UNICODE )
		"MultiWindow Contact List Unicode",
	#else
		"MultiWindow Contact List",
	#endif
	PLUGIN_MAKE_VERSION(0,9,0,0),
	"Display contacts, event notifications, protocol status with MW modifications",
	"",
	"bethoven@mailgate.ru" ,
	"Copyright 2000-2010 Miranda-IM project ["__DATE__" "__TIME__"]",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	DEFMOD_CLISTALL,
	#if defined( _UNICODE )
		{0x2a417ab9, 0x16f2, 0x472d, { 0x9a, 0xe3, 0x41, 0x51, 0x3, 0xc7, 0x8a, 0x64 }} //{2A417AB9-16F2-472d-9AE3-415103C78A64}
	#else
		{0x7ab05d31, 0x9972, 0x4406, { 0x82, 0x3e, 0xe, 0xd7, 0x45, 0xef, 0x7c, 0x56 }} //{7AB05D31-9972-4406-823E-0ED745EF7C56}
	#endif
};

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION(0,8,0,9) )
		return NULL;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_CLIST, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

int LoadContactListModule(void);
int LoadCLCModule(void);
int LoadCLUIModule();
int InitSkinHotKeys();

static int systemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	__try	{
		int *disableDefaultModule = 0;
		disableDefaultModule=(int*)CallService(MS_PLUGINS_GETDISABLEDEFAULTARRAY,0,0);
		if(!disableDefaultModule[DEFMOD_UICLUI]) if( LoadCLUIModule()) return 1;
	}
	__except (exceptFunction(GetExceptionInformation()) )
	{
		return 0;
	}

	InitSkinHotKeys();

	return 0;
}

INT_PTR SetDrawer(WPARAM wParam,LPARAM lParam)
{
	pDrawerServiceStruct DSS=(pDrawerServiceStruct)wParam;
	if (DSS->cbSize!=sizeof(*DSS)) return -1;
	if (DSS->PluginName==NULL) return -1;
	if (DSS->PluginName==NULL) return -1;
	if (!ServiceExists(DSS->GetDrawFuncsServiceName)) return -1;


	SED.cbSize=sizeof(SED);
	SED.PaintClc=(void (__cdecl *)(HWND,struct ClcData *,HDC,RECT *,int ,ClcProtoStatus *,HIMAGELIST))CallService(DSS->GetDrawFuncsServiceName,CLUI_EXT_FUNC_PAINTCLC,0);
	if (!SED.PaintClc) return -1;
	return 0;
}

static struct ClcContact* fnCreateClcContact( void )
{
	return (struct ClcContact*)mir_calloc( sizeof( struct ClcContact ));
}

static ClcCacheEntryBase* fnCreateCacheItem( HANDLE hContact )
{
	pdisplayNameCacheEntry p = (pdisplayNameCacheEntry)mir_calloc( sizeof( displayNameCacheEntry ));
	if ( p )
		p->hContact = hContact;
	return (ClcCacheEntryBase*)p;
}

int __declspec(dllexport) CListInitialise(PLUGINLINK * link)
{
	int rc=0;
	pluginLink=link;
	#ifdef _DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// get the internal malloc/free()
	__try {

		OutputDebugStringA("CListInitialise ClistMW\r\n");

		mir_getMMI( &mmi );
		mir_getLP( &pluginInfo );
		mir_getLI( &li );

		pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);
		if ( (INT_PTR)pcli == CALLSERVICE_NOTFOUND ) {
LBL_Error:
			MessageBoxA( NULL, "This version of plugin requires Miranda IM 0.8.0.9 or later", "Fatal error", MB_OK );
			return 1;
		}
		if ( pcli->version < 6 )
			goto LBL_Error;

		pcli->pfnBuildGroupPopupMenu = BuildGroupPopupMenu;
		pcli->pfnCalcEipPosition = CalcEipPosition;
		pcli->pfnCheckCacheItem = CheckPDNCE;
		pcli->pfnCluiProtocolStatusChanged = CluiProtocolStatusChanged;
		pcli->pfnCompareContacts = CompareContacts;
		pcli->pfnCreateClcContact = fnCreateClcContact;
		pcli->pfnCreateCacheItem = fnCreateCacheItem;
		pcli->pfnFindItem = FindItem;
		pcli->pfnFreeCacheItem = FreeDisplayNameCacheItem;
		pcli->pfnGetDefaultFontSetting = GetDefaultFontSetting;
		pcli->pfnGetRowsPriorTo = GetRowsPriorTo;
		pcli->pfnGetRowByIndex = GetRowByIndex;
		pcli->pfnHitTest = HitTest;
		pcli->pfnPaintClc = PaintClc;
		pcli->pfnRebuildEntireList = RebuildEntireList;
		pcli->pfnRecalcScrollBar = RecalcScrollBar;
		pcli->pfnScrollTo = ScrollTo;

		saveIconFromStatusMode      = pcli->pfnIconFromStatusMode;
		pcli->pfnIconFromStatusMode = cli_IconFromStatusMode;

		saveAddGroup = pcli->pfnAddGroup; pcli->pfnAddGroup = AddGroup;
		saveAddInfoItemToGroup = pcli->pfnAddInfoItemToGroup; pcli->pfnAddInfoItemToGroup = AddInfoItemToGroup;
		saveAddItemToGroup = pcli->pfnAddItemToGroup; pcli->pfnAddItemToGroup = AddItemToGroup;
		saveRemoveItemFromGroup = pcli->pfnRemoveItemFromGroup; pcli->pfnRemoveItemFromGroup = RemoveItemFromGroup;
		saveFreeContact = pcli->pfnFreeContact; pcli->pfnFreeContact = FreeContact;
		saveFreeGroup = pcli->pfnFreeGroup; pcli->pfnFreeGroup = FreeGroup;
		saveContactListControlWndProc = pcli->pfnContactListControlWndProc; pcli->pfnContactListControlWndProc = ContactListControlWndProc;
		saveTrayIconProcessMessage = pcli->pfnTrayIconProcessMessage; pcli->pfnTrayIconProcessMessage = TrayIconProcessMessage;
		saveContactListWndProc = pcli->pfnContactListWndProc; pcli->pfnContactListWndProc = ContactListWndProc;
		saveLoadClcOptions = pcli->pfnLoadClcOptions; pcli->pfnLoadClcOptions = LoadClcOptions;
		saveSortCLC = pcli->pfnSortCLC; pcli->pfnSortCLC = SortCLC;

		memset(&SED,0,sizeof(SED));
		CreateServiceFunction(CLUI_SetDrawerService,SetDrawer);

		rc=LoadContactListModule();
		if (rc==0) rc=LoadCLCModule();

		HookEvent(ME_SYSTEM_MODULESLOADED, systemModulesLoaded);
		BGModuleLoad();

		OutputDebugStringA("CListInitialise ClistMW...Done\r\n");
	}
	__except (exceptFunction(GetExceptionInformation()) )
	{
		return 0;
	}

	return rc;
}

// never called by a newer plugin loader.
int __declspec(dllexport) Load(PLUGINLINK * link)
{
	OutputDebugStringA("Load ClistMW\r\n");
	MessageBoxA(0,"You Running Old Miranda, use >30-10-2004 version!","MultiWindow Clist",0);
	CListInitialise(link);
	return 1;
}

int __declspec(dllexport) Unload(void)
{
	OutputDebugStringA("Unloading ClistMW\r\n");
	if (IsWindow(pcli->hwndContactList)) DestroyWindow(pcli->hwndContactList);
	BGModuleUnload();
	pcli->hwndContactList=0;
	return 0;
}
