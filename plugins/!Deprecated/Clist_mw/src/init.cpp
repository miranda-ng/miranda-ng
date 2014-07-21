/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-04 Miranda ICQ/IM project,
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
CLIST_INTERFACE* pcli = NULL;
int hLangpack;

static HANDLE hCListShutdown = 0;

HMENU BuildGroupPopupMenu( ClcGroup* group );

void  CalcEipPosition( struct ClcData *dat, struct ClcContact *contact, ClcGroup *group, POINT *result);
void  CheckPDNCE(ClcCacheEntry*);
void  CluiProtocolStatusChanged( int, const char* );
int   CompareContacts( const struct ClcContact *contact1, const struct ClcContact *contact2 );
void  FreeDisplayNameCacheItem(ClcCacheEntry*);
void  GetDefaultFontSetting(int i,LOGFONT *lf,COLORREF *colour);
void  RebuildEntireList(HWND hwnd,struct ClcData *dat);
void  RecalcScrollBar(HWND hwnd,struct ClcData *dat);

ClcGroup* ( *saveAddGroup )(HWND hwnd,struct ClcData *dat,const TCHAR *szName,DWORD flags,int groupId,int calcTotalMembers);
ClcGroup* ( *saveRemoveItemFromGroup )(HWND hwnd,ClcGroup *group,struct ClcContact *contact,int updateTotalCount);

void (*saveFreeContact)(struct ClcContact *p);
void (*saveFreeGroup)(ClcGroup *p);

LRESULT ( CALLBACK *saveContactListControlWndProc )( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ( CALLBACK *saveContactListWndProc )(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ( *saveLoadClcOptions )(HWND hwnd,struct ClcData *dat);

int ( *saveAddItemToGroup )( ClcGroup *group, int iAboveItem );
int AddItemToGroup(ClcGroup *group, int iAboveItem);

int ( *saveAddInfoItemToGroup)(ClcGroup *group,int flags,const TCHAR *pszText);
int AddInfoItemToGroup(ClcGroup *group,int flags,const TCHAR *pszText);

void ( *saveSortCLC )(HWND hwnd,struct ClcData *dat,int useInsertionSort);
void SortCLC(HWND hwnd,struct ClcData *dat,int useInsertionSort);

INT_PTR ( *saveTrayIconProcessMessage )(WPARAM wParam, LPARAM lParam);
INT_PTR TrayIconProcessMessage(WPARAM wParam, LPARAM lParam);

int  (*saveIconFromStatusMode)(const char *szProto,int nStatus, MCONTACT hContact);
int  cli_IconFromStatusMode(const char *szProto,int nStatus, MCONTACT hContact);

//from clcfonts
void RegisterCListFonts( void );
//from bgrcfg
extern int BGModuleLoad();
extern int BGModuleUnload();

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
	// {2A417AB9-16F2-472D-9AE3-415103C78A64}
	{0x2a417ab9, 0x16f2, 0x472d, {0x9a, 0xe3, 0x41, 0x51, 0x3, 0xc7, 0x8a, 0x64}}
};

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_CLIST, MIID_LAST};

int LoadContactListModule(void);
int PreloadCLCModule(void);
int LoadCLCModule(void);
int LoadCLUIModule();
int InitSkinHotKeys();

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	if ( LoadCLUIModule())
		return 1;

	InitSkinHotKeys();
	RegisterCListFonts();
	return 0;
}

INT_PTR SetDrawer(WPARAM wParam, LPARAM lParam)
{
	pDrawerServiceStruct DSS = (pDrawerServiceStruct)wParam;
	if (DSS->cbSize != sizeof(*DSS)) return -1;
	if (DSS->PluginName == NULL) return -1;
	if (DSS->GetDrawFuncsServiceName == NULL) return -1;
	if ( !ServiceExists(DSS->GetDrawFuncsServiceName)) return -1;


	SED.cbSize = sizeof(SED);
	SED.PaintClc = (void (__cdecl *)(HWND,struct ClcData *,HDC,RECT *,int ,ClcProtoStatus *,HIMAGELIST))CallService(DSS->GetDrawFuncsServiceName,CLUI_EXT_FUNC_PAINTCLC,0);
	if ( !SED.PaintClc) return -1;
	return 0;
}

static struct ClcContact* fnCreateClcContact( void )
{
	return (struct ClcContact*)mir_calloc( sizeof( struct ClcContact ));
}

static ClcCacheEntry *fnCreateCacheItem(MCONTACT hContact )
{
	ClcCacheEntry *p = (ClcCacheEntry *)mir_calloc( sizeof( ClcCacheEntry ));
	if ( p )
		p->hContact = hContact;
	return p;
}

extern "C" int __declspec(dllexport) CListInitialise()
{
	int rc = 0;

	OutputDebugStringA("CListInitialise ClistMW\r\n");
	mir_getLP( &pluginInfo );
	mir_getCLI();

	PreloadCLCModule();

	pcli->hInst = g_hInst;
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

	saveIconFromStatusMode = pcli->pfnIconFromStatusMode;
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

	rc = LoadContactListModule();
	if (rc == 0)
		rc = LoadCLCModule();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	BGModuleLoad();

	OutputDebugStringA("CListInitialise ClistMW...Done\r\n");

	return rc;
}

// never called by a newer plugin loader.
extern "C" int __declspec(dllexport) Load(void)
{
	OutputDebugStringA("Load ClistMW\r\n");
	CListInitialise();
	return 1;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	OutputDebugStringA("Unloading ClistMW\r\n");
	if (IsWindow(pcli->hwndContactList)) DestroyWindow(pcli->hwndContactList);
	BGModuleUnload();
	pcli->hwndContactList = 0;
	return 0;
}
