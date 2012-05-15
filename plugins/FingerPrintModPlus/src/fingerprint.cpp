/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

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
					
//Start of header
#include "global.h"
#include "fingerprints.h"		//Including of fingerprint masks

#define LIB_REG		2
#define LIB_USE		3

//#define USE_MY_SERVER
#define REGISTER_BETA
//#define REGISTER_AUTO

//#define mir_strncpy(a, b, c)		{ strncpy(a, b, c)[c - 1] = 0; }
//#define mir_tcsncpy_s(a, b, c, d)	{ _tcsncpy_s(a, b, c, d); c[d - 1] = 0; }
//#define _qtoupper(_c) (((_c) >= 'a' && (_c) <= 'z') ? ((_c) - 'a' + 'A') : (_c))
//#define _qtoupper(_c) (((_c) >= 'a' && (_c) <= 'z') ? ((_c) & 0x5F) : (_c))

void InitFingerEvents();
void UninitFingerEvents();
void FASTCALL ClearFI();

int OnIconsChanged(WPARAM wParam, LPARAM lParam);
int OnExtraIconClick(WPARAM wParam, LPARAM lParam);
int OnExtraIconListRebuild(WPARAM wParam, LPARAM lParam);
int OnExtraImageApply(WPARAM wParam, LPARAM lParam);
int OnContactSettingChanged(WPARAM wParam, LPARAM lParam);
int OnOptInitialise(WPARAM wParam, LPARAM lParam);
int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int OnPreShutdown(WPARAM wParam, LPARAM lParam);

INT_PTR ServiceSameClientsA(WPARAM wParam, LPARAM lParam);
INT_PTR ServiceGetClientIconA(WPARAM wParam, LPARAM lParam);
#ifdef UNICODE
INT_PTR ServiceSameClientsW(WPARAM wParam, LPARAM lParam);
INT_PTR ServiceGetClientIconW(WPARAM wParam, LPARAM lParam);
#endif // !UNICODE

HICON FASTCALL CreateJoinedIcon(HICON hBottom, HICON hTop);
HBITMAP __inline CreateBitmap32(int cx, int cy);
HBITMAP FASTCALL CreateBitmap32Point(int cx, int cy, LPVOID* bits);
HANDLE FASTCALL GetIconIndexFromFI(LPTSTR szMirVer);

HICON FASTCALL LoadIconFromExternalFile(LPTSTR filename, int nLibrary, LPSTR IconName, int flag, LPTSTR Description, int internalidx, LPBOOL NeedFree);
BOOL FASTCALL WildCompareA(LPSTR name, LPSTR mask);
BOOL FASTCALL WildCompareW(LPWSTR name, LPWSTR mask);
BOOL __inline WildCompareProcA(LPSTR name, LPSTR mask);
BOOL __inline WildCompareProcW(LPWSTR name, LPWSTR mask);

#ifdef UNICODE
	#define WildCompare		WildCompareW
	#define GetIconsIndexes	GetIconsIndexesW
#else
	#define WildCompare		WildCompareA
	#define GetIconsIndexes	GetIconsIndexesA
#endif // !UNICODE

HINSTANCE		g_hInst;
PLUGINLINK*		pluginLink;
MM_INTERFACE	mmi;					// miranda memory interface
int hLangpack;

BOOL g_bExtraIcon_Register_ServiceExist		= FALSE;
BOOL g_bCList_Extra_Set_Icon_ServiceExist	= FALSE;

HANDLE hHeap					= NULL;
HANDLE hExtraImageListRebuild	= NULL;		// hook event handle for ME_CLIST_EXTRA_LIST_REBUILD
HANDLE hExtraImageApply			= NULL;		// hook event handle for ME_CLIST_EXTRA_IMAGE_APPLY
HANDLE hContactSettingChanged	= NULL;		// hook event handle for ME_DB_CONTACT_SETTINGCHANGED
HANDLE hOptInitialise			= NULL;		// hook event handle for ME_OPT_INITIALISE
HANDLE hIconsChanged			= NULL;		// hook event handle for ME_SKIN2_ICONSCHANGED
HANDLE hPreShutdown				= NULL;		// hook event handle for ME_SYSTEM_PRESHUTDOWN
HANDLE hExtraIconClick			= NULL;		// hook event handle for ME_CLIST_EXTRA_CLICK

HANDLE compClientServA			= NULL;
HANDLE getClientIconA			= NULL;
#ifdef UNICODE
HANDLE compClientServW			= NULL;
HANDLE getClientIconW			= NULL;
LPSTR  g_szClientDescription	= NULL;
#endif // !UNICODE
HANDLE hStaticHooks[1]			= { NULL };
HANDLE hExtraIcon				= NULL;
HANDLE hFolderChanged			= NULL,		hIconFolder	= NULL;
TCHAR g_szSkinLib[MAX_PATH];

BYTE bColumn		= EXTRA_ICON_CLIENT;

FOUNDINFO* fiList	= NULL;
int nFICount		= 0;
BYTE gbUnicodeAPI;
UINT g_LPCodePage;

//End of header

// PluginInfo & PluginInfoEx
PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__INTERNAL_NAME_STRING,
	__VERSION_DWORD,
	"Fingerprint Mod+ (client version) icons module set extra icon of your buddies according to their client version.",
	"Artem Shpynov aka FYR, Bio, Faith Healer, HierOS",
	"crazy.hieros@gmail.com",
	__LEGAL_COPYRIGHT_STRING,
	"http://code.google.com/p/fingerprintmod/",
	UNICODE_AWARE,
	0,	
	MIID_THIS_PLUGIN
};

static void LoadDBCheckState(HWND hwndDlg, int idCtrl, LPCSTR szSetting, BYTE bDef)
{
	CheckDlgButton(hwndDlg, idCtrl, DBGetContactSettingByte(NULL, "Finger", szSetting, bDef));
}

static void StoreDBCheckState(HWND hwndDlg, int idCtrl, LPCSTR szSetting)
{
	DBWriteContactSettingByte(NULL, "Finger", szSetting, (BYTE)IsDlgButtonChecked(hwndDlg, idCtrl));
}

static void OptDlgChanged(HWND hwndDlg, BOOL show)
{
	if (show)
		ShowWindow(GetDlgItem(hwndDlg, IDC_OPTCHANGENOTE), SW_SHOW);
	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C"		__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{// Tell Miranda IM about plugin
	if(mirandaVersion < 0x8000)
	{
		MessageBox(NULL, TranslateT("Fingerprint Mod+ plugin requires Miranda IM 0.8.0.0 or later"), TranslateT("Fatal error"), MB_OK);
		return NULL;
	}
	return &pluginInfoEx;
}

extern "C"		__declspec(dllexport) const MUUID* MirandaPluginInterfaces()
{
	static const MUUID interfaces[] = {MIID_THIS_PLUGIN, MIID_FINGERPRINT, MIID_LAST };
	return interfaces;
}

extern "C" int	__declspec(dllexport) Load(PLUGINLINK* link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLP(&pluginInfoEx);

	hStaticHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	compClientServA = CreateServiceFunction(MS_FP_SAMECLIENTS, ServiceSameClientsA);
	getClientIconA = CreateServiceFunction(MS_FP_GETCLIENTICON, ServiceGetClientIconA);
#ifdef UNICODE
	compClientServW = CreateServiceFunction(MS_FP_SAMECLIENTSW, ServiceSameClientsW);
	getClientIconW = CreateServiceFunction(MS_FP_GETCLIENTICONW, ServiceGetClientIconW);
#endif // !UNICODE
	hHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	gbUnicodeAPI = IsWinVerNT();
	return 0;
}

extern "C" int	__declspec(dllexport) Unload()
{
	size_t i;
#ifdef UNICODE
	if(g_szClientDescription != NULL) mir_free(g_szClientDescription);
#endif // !UNICODE
	HeapDestroy(hHeap);
	ClearFI();

	for(i = 0; i < SIZEOF(hStaticHooks); i++)
	{
		UnhookEvent(hStaticHooks[i]);
		hStaticHooks[i] = NULL;
	}
	return 0;
}

/*
*	FreeIcon
*	for mode > 0 it releases icon from iconlib
*	for mode < 0 it destroys icon
*/
void FASTCALL FreeIcon(HICON hIcon, BOOL mode)
{
	if(!mode) return;
	if(mode > 0)
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, (WPARAM)0);
	else
		DestroyIcon(hIcon);
}

/*
*	Prepare
*	prepares upperstring masks and registers them in IcoLib
*/
void FASTCALL Prepare(KN_FP_MASK* mask)
{
	if(!mask->szMask) return;
	size_t iMaskLen;
	LPTSTR pszNewMask;
	LPTSTR pszTranslatedMask;

	if(mask == &def_kn_fp_mask[UNKNOWN_MASK_NUMBER])
		pszTranslatedMask = TranslateTS(mask->szMask);
	else
		pszTranslatedMask = mask->szMask;

	iMaskLen = _tcslen(pszTranslatedMask) + 1;
	pszNewMask = (LPTSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, iMaskLen * sizeof(TCHAR));

	_tcscpy_s(pszNewMask, iMaskLen, pszTranslatedMask);
	_tcsupr_s(pszNewMask, iMaskLen);
	mask->szMask = pszNewMask;
	LoadIconFromExternalFile(mask->szIconFileName, LIB_REG, mask->szIconName, mask->iSectionFlag, mask->szClientDescription, mask->iIconIndex, NULL);
}

/*
*	OnModulesLoaded
*	Hook necessary events here
*/
int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int i;

	Update update = { 0 };
	char szVersion[16];

	g_LPCodePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	g_bExtraIcon_Register_ServiceExist		= ServiceExists(MS_EXTRAICON_REGISTER);
	g_bCList_Extra_Set_Icon_ServiceExist	= ServiceExists(MS_CLIST_EXTRA_SET_ICON);

	//Hook necessary events
	hIconsChanged				= HookEvent(ME_SKIN2_ICONSCHANGED, OnIconsChanged);
	hContactSettingChanged		= HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	hOptInitialise				= HookEvent(ME_OPT_INITIALISE, OnOptInitialise);

	if(g_bExtraIcon_Register_ServiceExist)
	{
		EXTRAICON_INFO ico = { 0 };

		ico.cbSize = sizeof(ico);
		ico.type = EXTRAICON_TYPE_CALLBACK;
		ico.RebuildIcons = OnExtraIconListRebuild;
		ico.ApplyIcon = OnExtraImageApply;
		ico.OnClick = (MIRANDAHOOKPARAM)OnExtraIconClick;
		ico.name = "Client";
		ico.description= "Fingerprint";
		ico.descIcon = "client_Miranda_Unknown";
		hExtraIcon = (HANDLE)CallService(MS_EXTRAICON_REGISTER, (WPARAM)&ico, 0);
	}
	else
	{
		InitFingerEvents();
	}

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hIconFolder = FoldersRegisterCustomPathT("Fingerprint", "Icons", _T(MIRANDA_PATH) _T("\\") DEFAULT_SKIN_FOLDER);
		FoldersGetCustomPathT(hIconFolder, g_szSkinLib, SIZEOF(g_szSkinLib), _T(""));
	}
	else
		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)DEFAULT_SKIN_FOLDER, (LPARAM)g_szSkinLib);

#ifdef USE_MY_SERVER

	update.cbSize = sizeof(Update);

	update.szComponentName = pluginInfoEx.shortName;
	update.pbVersion = (LPBYTE)CreateVersionString(__VERSION_DWORD, szVersion);
	update.cpbVersion = (int)strlen((LPSTR)update.pbVersion);

	update.szUpdateURL = "http://fingerprintmod.googlecode.com/svn/branches/updater/" __BETA_FILE_TITLE ".zip";
	update.szVersionURL = "http://fingerprintmod.googlecode.com/svn/branches/updater/" __BETA_FILE_TITLE ".txt";
	update.pbVersionPrefix = (LPBYTE)__INTERNAL_NAME_STRING " version ";
	update.cpbVersionPrefix = (int)strlen((LPSTR)update.pbVersionPrefix);

	CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
#else //!USE_MY_SERVER

#ifdef REGISTER_BETA

	update.cbSize = sizeof(Update);

	update.szComponentName = pluginInfoEx.shortName;
	update.pbVersion = (LPBYTE)CreateVersionString(__VERSION_DWORD, szVersion);
	update.cpbVersion = (int)strlen((LPSTR)update.pbVersion);
	update.szBetaChangelogURL = "http://code.google.com/p/fingerprintmod/source/list";

#ifdef REGISTER_AUTO

	update.szUpdateURL = UPDATER_AUTOREGISTER;

#else //!REGISTER_AUTO

	update.szUpdateURL = "http://addons.miranda-im.org/download.php?dlfile=" __PLUGIN_ID_STR;
	update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=" __PLUGIN_ID_STR;
	update.pbVersionPrefix = (LPBYTE)"<span class=\"fileNameHeader\">" __INTERNAL_NAME_STRING " ";
	update.cpbVersionPrefix = (int)strlen((LPSTR)update.pbVersionPrefix);

#endif //REGISTER_AUTO

	update.szBetaUpdateURL = "http://fingerprintmod.googlecode.com/svn/branches/updater/" __BETA_FILE_TITLE ".zip";
	update.szBetaVersionURL = "http://fingerprintmod.googlecode.com/svn/branches/updater/" __BETA_FILE_TITLE ".txt";
	update.pbBetaVersionPrefix = (LPBYTE)__INTERNAL_NAME_STRING " version ";
	update.cpbBetaVersionPrefix = (int)strlen((LPSTR)update.pbBetaVersionPrefix);

	CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

#else // !REGISTER_BETA

	CallService(MS_UPDATE_REGISTERFL, (WPARAM)__PLUGIN_ID, (LPARAM)&pluginInfoEx);

#endif // REGISTER_BETA
#endif // USE_MY_SERVER

	// prepare masks
	KN_FP_MASK* mask;

	for(i = 0; i < DEFAULT_KN_FP_MASK_COUNT; i++)
	{
		mask = &def_kn_fp_mask[i];
		if(mask) Prepare(mask);
	}
	if(DBGetContactSettingByte(NULL, "Finger", "Overlay1", 1))
	{
		for(i = 0; i < DEFAULT_KN_FP_OVERLAYS_COUNT; i++)
		{
			mask = &def_kn_fp_overlays_mask[i];
			if(mask) Prepare(mask);
		}
	}
	if(DBGetContactSettingByte(NULL, "Finger", "Overlay2", 1))
	{
		if (DBGetContactSettingByte(NULL, "Finger", "ShowVersion", 0))
		{
			for(i = 0; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
			{
				mask = &def_kn_fp_overlays2_mask[i];
				if(mask) Prepare(mask);
			}
		}
		else
		{
			for(i = 0; i < DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT; i++)
			{
				mask = &def_kn_fp_overlays2_mask[i];
				if(mask) Prepare(mask);
			}
		}
	}
	if(DBGetContactSettingByte(NULL, "Finger", "Overlay3", 1))
	{
		for(i = 0; i < DEFAULT_KN_FP_OVERLAYS3_COUNT; i++)
		{
			mask = &def_kn_fp_overlays3_mask[i];
			if(mask) Prepare(mask);
		}
	}
	return 0;
}

int OnExtraIconClicked(WPARAM wParam, LPARAM lParam)
{
	if(lParam == bColumn)
		CallService(MS_USERINFO_SHOWDIALOG, wParam, NULL);
	return 0;
}

/*
*	 OnPreShutdown
*	 Unhook events here (this is valid place to unhook all events to prevent crash on exiting)
*/
int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	//Unhook events
	UnhookEvent(hContactSettingChanged);
	UnhookEvent(hPreShutdown);
	UnhookEvent(hIconsChanged);
	UnhookEvent(hOptInitialise);
	if(!g_bExtraIcon_Register_ServiceExist) UninitFingerEvents();
	UnhookEvent(hFolderChanged);
	DestroyServiceFunction(compClientServA);
	DestroyServiceFunction(getClientIconA);
#ifdef UNICODE
	DestroyServiceFunction(compClientServW);
	DestroyServiceFunction(getClientIconW);
#endif // !UNICODE
	return 0;
}

/*	ApplyFingerprintImage
*	 1)Try to find appropriate mask
*	 2)Register icon in extraimage list if not yet registered (0xFF)
*	 3)Set ExtraImage for contact
*/
int FASTCALL ApplyFingerprintImage(HANDLE hContact, LPTSTR szMirVer)
{
	HANDLE hImage = INVALID_HANDLE_VALUE;

	if(szMirVer)
		hImage = GetIconIndexFromFI(szMirVer);

	if(hContact == NULL) return 0;

	if(g_bCList_Extra_Set_Icon_ServiceExist && !g_bExtraIcon_Register_ServiceExist)
	{
		IconExtraColumn iec;
		WORD bColumn = DBGetContactSettingWord(NULL, "Finger", "Column", EXTRA_ICON_CLIENT);

		if(bColumn <= 0 || bColumn > EXTRA_ICON_COUNT)
		{
			bColumn = EXTRA_ICON_CLIENT;
		}

		iec.cbSize = sizeof(IconExtraColumn);
		iec.hImage = hImage;
		iec.ColumnType = bColumn;

		CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)hContact, (LPARAM)&iec);
	}
	else if(g_bExtraIcon_Register_ServiceExist && hExtraIcon != INVALID_HANDLE_VALUE && hExtraIcon != NULL)
	{
		EXTRAICON ei = { 0 };

		ei.cbSize = sizeof(ei);
		ei.hExtraIcon = hExtraIcon;
		ei.hContact = hContact;
		ei.hImage = hImage;

		CallService(MS_EXTRAICON_SET_ICON, (WPARAM)&ei, 0);
	}
	return 0;
}

void InitFingerEvents()
{
	hExtraIconClick =			HookEvent(ME_CLIST_EXTRA_CLICK, OnExtraIconClicked);
	hExtraImageListRebuild =	HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, OnExtraIconListRebuild);
	hExtraImageApply =			HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, OnExtraImageApply);
}

void UninitFingerEvents()
{
	UnhookEvent(hExtraImageListRebuild);
	UnhookEvent(hExtraImageApply);
	UnhookEvent(hExtraIconClick);
}

int OnExtraIconClick(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_USERINFO_SHOWDIALOG, wParam, NULL);
	return 0;
}

/*
*	OnExtraIconListRebuild
*	Set all registered indexes in array to 0xFF (unregistered icon)
*/
int OnExtraIconListRebuild(WPARAM wParam, LPARAM lParam)
{
	ClearFI();
	return 0;
}

/*
*	OnIconsChanged
*/
int OnIconsChanged(WPARAM wParam, LPARAM lParam)
{
	ClearFI();
	return 0;
}

/*
*	 OnExtraImageApply
*	 Try to get MirVer value from db for contact and if success calls ApplyFingerprintImage
*/
int OnExtraImageApply(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;

	if(hContact == NULL) return 0;
	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);

	if(szProto!=NULL)
	{
		DBVARIANT dbvMirVer = { 0 };

		if(!DBGetContactSettingTString(hContact, szProto, "MirVer", &dbvMirVer))
		{
			ApplyFingerprintImage(hContact, dbvMirVer.ptszVal);
			DBFreeVariant(&dbvMirVer);
		}
		else
			ApplyFingerprintImage(hContact, NULL);
	}
	else
		ApplyFingerprintImage(hContact, NULL);
	return 0;
}

/*
*	 OnContactSettingChanged
*	 if contact settings changed apply new image or remove it
*/
int OnContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if((HANDLE)wParam == NULL)	return 0;
	{
		DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
		if(cws && cws->szSetting && !strcmp(cws->szSetting, "MirVer"))
		{
			if(cws->value.type == DBVT_UTF8)
			{
#ifdef UNICODE
				LPWSTR wszVal = NULL;
				int iValLen;

				iValLen = MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, NULL, 0);
				if(iValLen > 0)
				{
					wszVal = (LPWSTR)mir_alloc(iValLen * sizeof(WCHAR));
					MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, wszVal, iValLen);
				}
				ApplyFingerprintImage((HANDLE)wParam, wszVal);
				mir_free(wszVal);
#else
				LPSTR szVal = NULL;
				int iValLenW;
				int iValLenA;

				iValLenW = MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, NULL, 0);
				if(iValLenW > 0)
				{
					LPWSTR wszVal = (LPWSTR)mir_alloc(iValLenW * sizeof(WCHAR));
					MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, wszVal, iValLenW);
					mir_free(wszVal);
					iValLenA = WideCharToMultiByte(g_LPCodePage, 0, wszVal, -1, NULL, 0, NULL, NULL);
					if(iValLenA > 0)
					{
						szVal = (LPSTR)mir_alloc(iValLenA * sizeof(CHAR));
						WideCharToMultiByte(g_LPCodePage, 0, wszVal, -1, szVal, iValLenA, NULL, NULL);
					}
				}
				ApplyFingerprintImage((HANDLE)wParam, szVal);
				mir_free(szVal);
#endif // !UNICODE
			}
			else if(cws->value.type == DBVT_ASCIIZ)
			{
#ifdef UNICODE
				LPWSTR wszVal = NULL;
				int iValLen;

				iValLen = MultiByteToWideChar(g_LPCodePage, 0, cws->value.pszVal, -1, NULL, 0);
				if(iValLen > 0)
				{
					wszVal = (LPWSTR)mir_alloc(iValLen * sizeof(WCHAR));
					MultiByteToWideChar(g_LPCodePage, 0, cws->value.pszVal, -1, wszVal, iValLen);
				}
				ApplyFingerprintImage((HANDLE)wParam, wszVal);
				mir_free(wszVal);
#else
				ApplyFingerprintImage((HANDLE)wParam, cws->value.pszVal);
#endif // !UNICODE
			}
			else if(cws->value.type == DBVT_WCHAR)
			{
#ifdef UNICODE
				ApplyFingerprintImage((HANDLE)wParam, cws->value.pwszVal);
#else
				LPSTR szVal = NULL;
				int iValLen;

				iValLen = WideCharToMultiByte(g_LPCodePage, 0, cws->value.pwszVal, -1, NULL, 0, NULL, NULL);
				if(iValLen > 0)
				{
					szVal = (LPSTR)mir_alloc(iValLen * sizeof(CHAR));
					 WideCharToMultiByte(g_LPCodePage, 0, cws->value.pwszVal, -1, szVal, iValLen, NULL, NULL);
				}
				ApplyFingerprintImage((HANDLE)wParam, szVal);
				mir_free(szVal);
#endif // !UNICODE
			}
			else
				ApplyFingerprintImage((HANDLE)wParam, NULL);
		}
	}
	return 0;
}

/*
*	 LoadIconFromExternalFile
*	 If iconlib module presents register icon there
*	 Register and return icon within iconlib
*	 or from resourse
*/
//	 TO DO: Extracting icons from clienticons.dll or other external files require futher
//	 destroying of icon... need to add field to list, modify it and remove icon on unload
//	 Otherwise it will cause gdi resources leaking.
//	 So nowtime it is commented out

HICON FASTCALL LoadIconFromExternalFile(LPTSTR filename, int nLibrary, LPSTR IconName, int flag, LPTSTR Description, int internalidx, LPBOOL NeedFree)
{
	HICON hIcon = NULL;
	TCHAR destfile[MAX_PATH];

	if(IconName == NULL) return NULL;

	if (filename == _T(""))
		GetModuleFileName(g_hInst,destfile,MAX_PATH);
	else
	{
		mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, filename);
		struct _stat64i32 stFileInfo;

		if (_tstat(destfile, &stFileInfo) == -1)
			return hIcon;
	}

	if(nLibrary == LIB_USE)
	{
		hIcon = ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IconName));
		if(!hIcon)
		{
			ExtractIconEx(destfile, -internalidx, NULL, &hIcon, 1);
			return hIcon;
		}
	}
	else
	{
		SKINICONDESC sid;
		LPTSTR SectName = NULL;

		switch(flag)
		{
			#include "finger_groups.h"
		}

		if(SectName == NULL)
			return hIcon;

		ZeroMemory(&sid, sizeof(sid));

		sid.cbSize = sizeof(sid);
		sid.flags = SIDF_ALL_TCHAR;
		sid.ptszSection = SectName;
		sid.pszName = IconName;
		sid.ptszDescription = Description;
		sid.ptszDefaultFile = destfile;
		sid.iDefaultIndex = -internalidx;
		sid.cx = sid.cy = 16;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		hIcon = ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IconName));
	}

	if(NeedFree)
		*NeedFree = (BOOL)hIcon;
	return hIcon;
}

/*
*	WildCompareA
*	Compare 'name' string with 'mask' strings.
*	Masks can contain '*' or '?' wild symbols
*	Asterics '*' symbol covers 'empty' symbol too e.g WildCompare("Tst","T*st*"), returns TRUE
*	In order to handle situation 'at least one any sybol' use "?*" combination:
*	e.g WildCompare("Tst","T?*st*"), returns FALSE, but both WildCompare("Test","T?*st*") and
*	WildCompare("Teeest","T?*st*") return TRUE.
*
*	Function is case sensitive! so convert input or modify func to use _qtoupper()
*
*	Mask can contain several submasks. In this case each submask (including first)
*	should start from '|' e.g: "|first*submask|second*mask".
*
*	Dec 25, 2006 by FYR:
*	Added Exception to masks: the mask "|^mask3|mask2|mask1" means: 
*	if NOT according to mask 3 AND (mask1 OR mask2)
*	EXCEPTION should be BEFORE main mask:
*		IF Exception match - the comparing stops as FALSE
*		IF Exception does not match - comparing continue
*		IF Mask match - comparing stops as TRUE
*		IF Mask does not not match comparing continue
*/
BOOL FASTCALL WildCompareA(LPSTR szName, LPSTR szMask)
{
	if(*szMask != '|') return WildCompareProcA(szName, szMask);
	{
		size_t s = 1, e = 1;
//		static char temp[100];		//lets made temp static local var - should be faster than dynamic
		LPSTR szTemp = (LPSTR)_alloca(strlen(szMask) * sizeof(CHAR) + sizeof(CHAR));
		BOOL bExcept;

		while(szMask[e] != '\0')
		{
			s = e;
			while(szMask[e] != '\0' && szMask[e] != '|') e++;

			// exception mask
			bExcept = (*(szMask + s) == '^');
			if(bExcept) s++;

			memcpy(szTemp, szMask + s, (e - s) * sizeof(CHAR));
			szTemp[e - s] = '\0';

			if(WildCompareProcA(szName, szTemp))
				return !bExcept;

			if(szMask[e] != '\0')
				e++;
			else
				return FALSE;
		}
		return FALSE;
	}
}

/*
*	WildCompareW
*	Compare 'name' string with 'mask' strings.
*	Masks can contain '*' or '?' wild symbols
*	Asterics '*' symbol covers 'empty' symbol too e.g WildCompare("Tst","T*st*"), returns TRUE
*	In order to handle situation 'at least one any sybol' use "?*" combination:
*	e.g WildCompare("Tst","T?*st*"), returns FALSE, but both WildCompare("Test","T?*st*") and
*	WildCompare("Teeest","T?*st*") return TRUE.
*
*	Function is case sensitive! so convert input or modify func to use _qtoupper()
*
*	Mask can contain several submasks. In this case each submask (including first)
*	should start from '|' e.g: "|first*submask|second*mask".
*
*	Dec 25, 2006 by FYR:
*	Added Exception to masks: the mask "|^mask3|mask2|mask1" means: 
*	if NOT according to mask 3 AND (mask1 OR mask2)
*	EXCEPTION should be BEFORE main mask:
*		IF Exception match - the comparing stops as FALSE
*		IF Exception does not match - comparing continue
*		IF Mask match - comparing stops as TRUE
*		IF Mask does not not match comparing continue
*/
BOOL FASTCALL WildCompareW(LPWSTR wszName, LPWSTR wszMask)
{
	if(*wszMask != L'|') return WildCompareProcW(wszName, wszMask);
	{
		size_t s = 1, e = 1;
//		static char temp[100];		//lets made temp static local var - should be faster than dynamic
		LPWSTR wszTemp = (LPWSTR)_alloca(wcslen(wszMask) * sizeof(WCHAR) + sizeof(WCHAR));
		BOOL bExcept;

		while(wszMask[e] != L'\0')
		{
			s = e;
			while(wszMask[e] != L'\0' && wszMask[e] != L'|') e++;

			// exception mask
			bExcept = (*(wszMask + s) == L'^');
			if(bExcept) s++;

			memcpy(wszTemp, wszMask + s, (e - s) * sizeof(WCHAR));
			wszTemp[e - s] = L'\0';

			if(WildCompareProcW(wszName, wszTemp))
				return !bExcept;

			if(wszMask[e] != L'\0')
				e++;
			else
				return FALSE;
		}
		return FALSE;
	}
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			for (int i = 0; i < DEFAULT_SETTINGS_COUNT; i++)
			{
				if (lstrcmpA(settings[i].szSetName, "ShowVersion") == 0)
					LoadDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName, 0);
				else
					LoadDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName, 1);
			}
			LPTSTR CIdComboBox[] =
			{
				_T("Email"),
				_T("Protocol"),
				_T("SMS"),
				_T("Advanced 1"),
				_T("Advanced 2"),
				_T("Web"),
				_T("Client (default)"),
				_T("VisMode"),
				_T("Advanced 3"),
				_T("Advanced 4")
			};

			for(int i = 0; i < SIZEOF(CIdComboBox); i++)
				ComboBoxAddString(GetDlgItem(hwndDlg, IDC_ADVICON), CIdComboBox[i], i);

			SendDlgItemMessage(hwndDlg, IDC_ADVICON, CB_SETCURSEL, (DBGetContactSettingWord(NULL, "Finger", "Column", EXTRA_ICON_CLIENT)) - 1, 0);
			if(g_bExtraIcon_Register_ServiceExist)
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADVICON), FALSE);

			ShowWindow(GetDlgItem(hwndDlg, IDC_OPTCHANGENOTE), SW_HIDE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			BOOL show;
		case IDC_ADVICON:
			if(HIWORD(wParam) == CBN_SELCHANGE)
				OptDlgChanged(hwndDlg, true);
			break;
		case IDC_OVERLAY1:
		case IDC_OVERLAY2:
		case IDC_OVERLAY3:
		case IDC_VERSION:
			show = true;
			OptDlgChanged(hwndDlg, show);
			break;

		case IDC_GROUPMIRANDA:
		case IDC_GROUPMULTI:
		case IDC_GROUPPACKS:
		case IDC_GROUPOTHERS:
		case IDC_GROUPAIM:
		case IDC_GROUPGADU:
		case IDC_GROUPICQ:
		case IDC_GROUPIRC:
		case IDC_GROUPJABBER:
		case IDC_GROUPMAIL:
		case IDC_GROUPMSN:
		case IDC_GROUPQQ:
		case IDC_GROUPRSS:
		case IDC_GROUPSKYPE:
		case IDC_GROUPTLEN:
		case IDC_GROUPVOIP:
		case IDC_GROUPWEATHER:
		case IDC_GROUPYAHOO:
			show = false;
			OptDlgChanged(hwndDlg, show);
			break;
		default:
			return 0;
		}
		break;
	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if (hdr && hdr->code == PSN_APPLY) 
			{
				int i = SendDlgItemMessage(hwndDlg, IDC_ADVICON, CB_GETCURSEL, 0, 0) + 1;
				DBWriteContactSettingWord(NULL, "Finger", "Column", (WORD)i);

				// prepare masks
				KN_FP_MASK* mask;

				for(i = 0; i < DEFAULT_KN_FP_MASK_COUNT; i++)
				{
					mask = &def_kn_fp_mask[i];
					if(mask) CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)mask->szIconName);
				}
				if(DBGetContactSettingByte(NULL, "Finger", "Overlay1", 1))
				{
					for(i = 0; i < DEFAULT_KN_FP_OVERLAYS_COUNT; i++)
					{
						mask = &def_kn_fp_overlays_mask[i];
						if(mask) CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)mask->szIconName);
					}
				}
				if(DBGetContactSettingByte(NULL, "Finger", "Overlay2", 1))
				{
					if (DBGetContactSettingByte(NULL, "Finger", "ShowVersion", 0))
					{
						for(i = 0; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
						{
							mask = &def_kn_fp_overlays2_mask[i];
							if(mask) CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)mask->szIconName);
						}
					}
					else
					{
						for(i = 0; i < DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT; i++)
						{
							mask = &def_kn_fp_overlays2_mask[i];
							if(mask) CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)mask->szIconName);
						}
					}
				}
				if(DBGetContactSettingByte(NULL, "Finger", "Overlay3", 1))
				{
					for(i = 0; i < DEFAULT_KN_FP_OVERLAYS3_COUNT; i++)
					{
						mask = &def_kn_fp_overlays3_mask[i];
						if(mask) CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)mask->szIconName);
					}
				}

				for (int i = 0; i < DEFAULT_SETTINGS_COUNT; i++)
					StoreDBCheckState(hwndDlg, settings[i].idCtrl, settings[i].szSetName);
					
				for(i = 0; i < DEFAULT_KN_FP_MASK_COUNT; i++)
				{
					mask = &def_kn_fp_mask[i];
					if(mask) Prepare(mask);
				}
				if(DBGetContactSettingByte(NULL, "Finger", "Overlay1", 1))
				{
					for(i = 0; i < DEFAULT_KN_FP_OVERLAYS_COUNT; i++)
					{
						mask = &def_kn_fp_overlays_mask[i];
						if(mask) Prepare(mask);
					}
				}
				if(DBGetContactSettingByte(NULL, "Finger", "Overlay2", 1))
				{
					if (DBGetContactSettingByte(NULL, "Finger", "ShowVersion", 0))
					{
						for(i = 0; i < DEFAULT_KN_FP_OVERLAYS2_COUNT; i++)
						{
							mask = &def_kn_fp_overlays2_mask[i];
							if(mask) Prepare(mask);
						}
					}
					else
					{
						for(i = 0; i < DEFAULT_KN_FP_OVERLAYS2_NO_VER_COUNT; i++)
						{
							mask = &def_kn_fp_overlays2_mask[i];
							if(mask) Prepare(mask);
						}
					}
				}
				if(DBGetContactSettingByte(NULL, "Finger", "Overlay3", 1))
				{
					for(i = 0; i < DEFAULT_KN_FP_OVERLAYS3_COUNT; i++)
					{
						mask = &def_kn_fp_overlays3_mask[i];
						if(mask) Prepare(mask);
					}
				}
			}
			break;
		}
	case WM_DESTROY:
		break;
	}
	return FALSE;
}
int OnOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));

	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = g_hInst;
	odp.ptszGroup = LPGENT("Customize");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG);
	odp.ptszTitle = LPGENT("Fingerprint");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

BOOL __inline WildCompareProcA(LPSTR szName, LPSTR szMask)
{
	LPSTR szLast = NULL;
	for(;; szMask++, szName++)
	{
		if(*szMask != '?' && *szMask != *szName) break;
		if(*szName == '\0') return ((BOOL)!*szMask);
	}
	if(*szMask != '*') return FALSE;
	for(;; szMask++, szName++)
	{
		while(*szMask == '*')
		{
			szLast = szMask++;
			if(*szMask == '\0') return ((BOOL)!*szMask);	/* true */
		}
		if(*szName == '\0') return ((BOOL)!*szMask);		/* *mask == EOS */
		if(*szMask != '?' && *szMask != *szName && szLast != NULL)
		{
			szName -= (size_t)(szMask - szLast) - 1;
			szMask = szLast;
		}
	}
}

BOOL __inline WildCompareProcW(LPWSTR wszName, LPWSTR wszMask)
{
	LPWSTR wszLast = NULL;
	for(;; wszMask++, wszName++)
	{
		if(*wszMask != L'?' && *wszMask != *wszName) break;
		if(*wszName == L'\0') return ((BOOL)!*wszMask);
	}
	if(*wszMask != L'*') return FALSE;
	for(;; wszMask++, wszName++)
	{
		while(*wszMask == L'*')
		{
			wszLast = wszMask++;
			if(*wszMask == L'\0') return ((BOOL)!*wszMask);	/* true */
		}
		if(*wszName == L'\0') return ((BOOL)!*wszMask);		/* *mask == EOS */
		if(*wszMask != L'?' && *wszMask != *wszName && wszLast != NULL)
		{
			wszName -= (size_t)(wszMask - wszLast) - 1;
			wszMask = wszLast;
		}
	}
}

/*	GetIconsIndexesA
*	Retrieves Icons indexes by Mirver
*/
void FASTCALL GetIconsIndexesA(LPSTR szMirVer, short *base, short *overlay,short *overlay2,short *overlay3)
{
	LPTSTR tszMirVerUp;
	int iMirVerUpLen;
	short i = 0, j = -1, k = -1, n = -1;

	if(strcmp(szMirVer, "?") == 0)
	{
		*base = UNKNOWN_MASK_NUMBER;
		*overlay = -1;
		*overlay2 = -1;
		*overlay3 = -1;
		return;
	}

#ifdef UNICODE
	iMirVerUpLen = MultiByteToWideChar(g_LPCodePage, 0, szMirVer, -1, NULL, 0);
#else
	iMirVerUpLen = strlen(szMirVer) + 1;
#endif // !UNICODE
	tszMirVerUp = (LPTSTR)mir_alloc(iMirVerUpLen * sizeof(TCHAR));
#ifdef UNICODE
	MultiByteToWideChar(g_LPCodePage, 0, szMirVer, -1, tszMirVerUp, iMirVerUpLen);
#else
	_tcscpy_s(tszMirVerUp, iMirVerUpLen, szMirVer);	
#endif // !UNICODE
	_tcsupr_s(tszMirVerUp, iMirVerUpLen);

	while(i < DEFAULT_KN_FP_MASK_COUNT)
	{
		if(WildCompare(tszMirVerUp, def_kn_fp_mask[i].szMask))
		{
			if (def_kn_fp_mask[i].szIconFileName != _T(""))
			{
				TCHAR destfile[MAX_PATH];
				mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, def_kn_fp_mask[i].szIconFileName);
				struct _stat64i32 stFileInfo;

				if (_tstat(destfile, &stFileInfo) == -1)
					i = NOTFOUND_MASK_NUMBER;
			}
			break;
		}
		i++;
	}
	if(!def_kn_fp_mask[i].fNotUseOverlay && i < DEFAULT_KN_FP_MASK_COUNT)
	{
		j = 0;
		while(j < DEFAULT_KN_FP_OVERLAYS_COUNT)
		{
			if(WildCompare(tszMirVerUp, def_kn_fp_overlays_mask[j].szMask))
			{
				if (def_kn_fp_overlays_mask[j].szIconFileName == _T("ClientIcons_Packs"))
				{
					TCHAR destfile[MAX_PATH];
					mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, def_kn_fp_overlays_mask[i].szIconFileName);
					struct _stat64i32 stFileInfo;

					if (_tstat(destfile, &stFileInfo) == -1)
						j++;
					else
						break;
				}
				else
					break;
			}
			j++;
		}
		k = 0;
		while(k < DEFAULT_KN_FP_OVERLAYS2_COUNT)
		{
			if(WildCompare(tszMirVerUp, def_kn_fp_overlays2_mask[k].szMask)) break;
			k++;
		}
		n = 0;
		while(n < DEFAULT_KN_FP_OVERLAYS3_COUNT)
		{
			if(WildCompare(tszMirVerUp, def_kn_fp_overlays3_mask[n].szMask)) break;
			n++;
		}
	}
	*base = (i < DEFAULT_KN_FP_MASK_COUNT) ? i : -1;
	*overlay = (j < DEFAULT_KN_FP_OVERLAYS_COUNT) ? j : -1;
	*overlay2 = (k < DEFAULT_KN_FP_OVERLAYS2_COUNT) ? k : -1;
	*overlay3 = (n < DEFAULT_KN_FP_OVERLAYS3_COUNT) ? n : -1;

	mir_free(tszMirVerUp);
}

/*	GetIconsIndexesW
*	Retrieves Icons indexes by Mirver
*/
#ifdef UNICODE
void FASTCALL GetIconsIndexesW(LPWSTR wszMirVer, short *base, short *overlay,short *overlay2,short *overlay3)
{
	LPWSTR wszMirVerUp;
	size_t iMirVerUpLen;
	short i = 0, j = -1, k = -1, n = -1;

	if(wcscmp(wszMirVer, L"?") == 0)
	{
		*base = UNKNOWN_MASK_NUMBER;
		*overlay = -1;
		*overlay2 = -1;
		*overlay3 = -1;
		return;
	}

	iMirVerUpLen = wcslen(wszMirVer) + 1;
	wszMirVerUp = (LPWSTR)mir_alloc(iMirVerUpLen * sizeof(WCHAR));

	wcscpy_s(wszMirVerUp, iMirVerUpLen, wszMirVer);
	_wcsupr_s(wszMirVerUp, iMirVerUpLen);

	while(i < DEFAULT_KN_FP_MASK_COUNT)
	{
		if(WildCompareW(wszMirVerUp, def_kn_fp_mask[i].szMask))
		{
			if (def_kn_fp_mask[i].szIconFileName != _T(""))
			{
				TCHAR destfile[MAX_PATH];
				mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, def_kn_fp_mask[i].szIconFileName);
				struct _stat64i32 stFileInfo;

				if (_tstat(destfile, &stFileInfo) == -1)
					i = NOTFOUND_MASK_NUMBER;
			}
			break;
		}
		i++;
	}
	if(!def_kn_fp_mask[i].fNotUseOverlay && i < DEFAULT_KN_FP_MASK_COUNT)
	{
		j = 0;
		while(j < DEFAULT_KN_FP_OVERLAYS_COUNT)
		{
			if(WildCompare(wszMirVerUp, def_kn_fp_overlays_mask[j].szMask))
			{
				if (def_kn_fp_overlays_mask[j].szIconFileName == _T("ClientIcons_Packs"))
				{
					TCHAR destfile[MAX_PATH];
					mir_sntprintf(destfile, SIZEOF(destfile), _T("%s\\%s.dll"), g_szSkinLib, def_kn_fp_overlays_mask[j].szIconFileName);
					struct _stat64i32 stFileInfo;

					if (_tstat(destfile, &stFileInfo) == -1)
						j++;
					else
						break;
				}
				else
					break;
			}
			j++;
		}
		k = 0;
		while(k < DEFAULT_KN_FP_OVERLAYS2_COUNT)
		{
			if(WildCompareW(wszMirVerUp, def_kn_fp_overlays2_mask[k].szMask)) break;
			k++;
		}
		n = 0;
		while(n < DEFAULT_KN_FP_OVERLAYS3_COUNT)
		{
			if(WildCompareW(wszMirVerUp, def_kn_fp_overlays3_mask[n].szMask)) break;
			n++;
		}
	}
	*base = (i < DEFAULT_KN_FP_MASK_COUNT) ? i : -1;
	*overlay = (j < DEFAULT_KN_FP_OVERLAYS_COUNT) ? j : -1;
	*overlay2 = (k < DEFAULT_KN_FP_OVERLAYS2_COUNT) ? k : -1;
	*overlay3 = (n < DEFAULT_KN_FP_OVERLAYS3_COUNT) ? n : -1;

	mir_free(wszMirVerUp);
}
#endif // !UNICODE

/*
* CreateIconFromIndexes
* returns hIcon of joined icon by given indexes
*/
HICON FASTCALL CreateIconFromIndexes(short base, short overlay, short overlay2, short overlay3)
{
	HICON hIcon = NULL;	// returned HICON
	HICON hTmp = NULL;
	HICON icMain = NULL;
	HICON icOverlay = NULL;
	HICON icOverlay2 = NULL;
	HICON icOverlay3 = NULL;

	BOOL needFreeBase = FALSE;
	BOOL needFreeOverlay = FALSE;
	BOOL needFreeOverlay2 = FALSE;
	BOOL needFreeOverlay3 = FALSE;

	KN_FP_MASK* mainMask = &(def_kn_fp_mask[base]);
	icMain = LoadIconFromExternalFile(mainMask->szIconFileName, LIB_USE, mainMask->szIconName, mainMask->iSectionFlag, mainMask->szClientDescription, mainMask->iIconIndex, &needFreeBase);

	if(icMain)
	{
		KN_FP_MASK* overlayMask = (overlay != -1) ? &(def_kn_fp_overlays_mask[overlay]) : NULL;
		KN_FP_MASK* overlay2Mask = (overlay2 != -1) ? &(def_kn_fp_overlays2_mask[overlay2]) : NULL;
		KN_FP_MASK* overlay3Mask = (overlay3 != -1) ? &(def_kn_fp_overlays3_mask[overlay3]) : NULL;
		icOverlay = (overlayMask == NULL) ? NULL : LoadIconFromExternalFile(overlayMask->szIconFileName, LIB_USE, overlayMask->szIconName, overlayMask->iSectionFlag, overlayMask->szClientDescription, overlayMask->iIconIndex, &needFreeOverlay);
		icOverlay2 = (overlay2Mask == NULL) ? NULL : LoadIconFromExternalFile(overlay2Mask->szIconFileName, LIB_USE, overlay2Mask->szIconName, overlay2Mask->iSectionFlag, overlay2Mask->szClientDescription, overlay2Mask->iIconIndex, &needFreeOverlay2);
		icOverlay3 = (overlay3Mask == NULL) ? NULL : LoadIconFromExternalFile(overlay3Mask->szIconFileName, LIB_USE, overlay3Mask->szIconName, overlay3Mask->iSectionFlag, overlay3Mask->szClientDescription, overlay3Mask->iIconIndex, &needFreeOverlay3);

		hIcon = icMain;

		if(overlayMask)
		{
			hIcon = CreateJoinedIcon(hIcon, icOverlay);
			hTmp = hIcon;
		}

		if(overlay2Mask)
		{
			hIcon = CreateJoinedIcon(hIcon, icOverlay2);
			if(hTmp) DestroyIcon(hTmp);
			hTmp = hIcon;
		}

		if(overlay3Mask)
		{
			hIcon = CreateJoinedIcon(hIcon, icOverlay3);
			if(hTmp) DestroyIcon(hTmp);
		}
	}

	if(hIcon == icMain)
		hIcon = CopyIcon(icMain);

	FreeIcon(icMain, needFreeBase);
	FreeIcon(icOverlay, needFreeOverlay);
	FreeIcon(icOverlay2, needFreeOverlay2);
	FreeIcon(icOverlay3, needFreeOverlay3);

	return hIcon;
}

/*
*	ServiceGetClientIconA
*	MS_FP_GETCLIENTICON service implementation.
*	wParam - char * MirVer value to get client for.
*	lParam - int noCopy - if wParam is equal to "1"	will return icon handler without copiing icon.
*	ICON IS ALWAYS COPIED!!!
*/
INT_PTR ServiceGetClientIconA(WPARAM wParam, LPARAM lParam)
{
	LPSTR szMirVer = (LPSTR)wParam;			// MirVer value to get client for.
/*
	static HICON hIcon = NULL;	// returned HICON
	if(hIcon)
	{
		DestroyIcon(hIcon);
		hIcon = NULL;
	}
*/

	if(szMirVer == NULL) return 0;

	{
		HICON hIcon = NULL;			// returned HICON
		int NoCopy = (int)lParam;	// noCopy
		short base, overlay, overlay2, overlay3;

		GetIconsIndexesA(szMirVer, &base, &overlay, &overlay2, &overlay3);

		if(base != -1)
		{
			hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3);
		}
/*
		if(hIcon && !NoCopy)
			return (INT_PTR)CopyIcon(hIcon);
*/
		return (INT_PTR)hIcon;
	}
}

/*
 *	 ServiceSameClientA
 *	 MS_FP_SAMECLIENTS service implementation.
 *	 wParam - char * first MirVer value
 *	 lParam - char * second MirVer value
 *	 return pointer to char string - client desription (do not destroy) if clients are same
 */
INT_PTR ServiceSameClientsA(WPARAM wParam, LPARAM lParam)
{
	LPSTR szMirVerFirst = (LPSTR)wParam;	// MirVer value to get client for.
	LPSTR szMirVerSecond = (LPSTR)lParam;	// MirVer value to get client for.
	int firstIndex, secondIndex;
	BOOL Result = FALSE;

	firstIndex = secondIndex = 0;
	if(!szMirVerFirst || !szMirVerSecond) return (INT_PTR)NULL;	//one of its is not null

	{
		LPTSTR tszMirVerFirstUp, tszMirVerSecondUp;
		int iMirVerFirstUpLen, iMirVerSecondUpLen;
#ifdef UNICODE
		iMirVerFirstUpLen = MultiByteToWideChar(g_LPCodePage, 0, szMirVerFirst, -1, NULL, 0);
		iMirVerSecondUpLen = MultiByteToWideChar(g_LPCodePage, 0, szMirVerSecond, -1, NULL, 0);
#else
		iMirVerFirstUpLen = strlen(szMirVerFirst) + 1;
		iMirVerSecondUpLen = strlen(szMirVerSecond) + 1;
#endif // !UNICODE
		tszMirVerFirstUp = (LPTSTR)mir_alloc(iMirVerFirstUpLen * sizeof(TCHAR));
		tszMirVerSecondUp = (LPTSTR)mir_alloc(iMirVerSecondUpLen * sizeof(TCHAR));
#ifdef UNICODE
		MultiByteToWideChar(g_LPCodePage, 0, szMirVerFirst, -1, tszMirVerFirstUp, iMirVerFirstUpLen);
		MultiByteToWideChar(g_LPCodePage, 0, szMirVerSecond, -1, tszMirVerSecondUp, iMirVerSecondUpLen);
#else
		strcpy_s(tszMirVerFirstUp, iMirVerFirstUpLen, szMirVerFirst);
		strcpy_s(tszMirVerSecondUp, iMirVerSecondUpLen, szMirVerSecond);
#endif // !UNICODE
		_tcsupr_s(tszMirVerFirstUp, iMirVerFirstUpLen);
		_tcsupr_s(tszMirVerSecondUp, iMirVerSecondUpLen);

		if(_tcscmp(tszMirVerFirstUp, _T("?")) == 0)
			firstIndex = UNKNOWN_MASK_NUMBER;
		else
			while(firstIndex < DEFAULT_KN_FP_MASK_COUNT)
			{
				if(WildCompare(tszMirVerFirstUp, def_kn_fp_mask[firstIndex].szMask))
					break;
				firstIndex++;
			}
		if(_tcscmp(tszMirVerSecondUp, _T("?")) == 0)
			secondIndex = UNKNOWN_MASK_NUMBER;
		else
			while(secondIndex < DEFAULT_KN_FP_MASK_COUNT)
			{
				if(WildCompare(tszMirVerSecondUp, def_kn_fp_mask[secondIndex].szMask))
					break;
	 			secondIndex++;
			}

		mir_free(tszMirVerFirstUp);
		mir_free(tszMirVerSecondUp);

		if(firstIndex == secondIndex && firstIndex < DEFAULT_KN_FP_MASK_COUNT)
		{
#ifdef UNICODE
			int iClientDescriptionLen = WideCharToMultiByte(g_LPCodePage, 0, def_kn_fp_mask[firstIndex].szClientDescription, -1, NULL, 0, NULL, NULL);
			if(iClientDescriptionLen > 0)
				g_szClientDescription = (LPSTR)mir_realloc(g_szClientDescription, iClientDescriptionLen * sizeof(CHAR));
			else
				return (INT_PTR)NULL;
			WideCharToMultiByte(g_LPCodePage, 0, def_kn_fp_mask[firstIndex].szClientDescription, -1, g_szClientDescription, iClientDescriptionLen, NULL, NULL);
			return (INT_PTR)g_szClientDescription;
#else
			return (INT_PTR)def_kn_fp_mask[firstIndex].szClientDescription;
#endif // !UNICODE
		}
	}
	return (INT_PTR)NULL;
}

/*
*	ServiceGetClientIconW
*	MS_FP_GETCLIENTICONW service implementation.
*	wParam - LPWSTR MirVer value to get client for.
*	lParam - int noCopy - if wParam is equal to "1"	will return icon handler without copiing icon.
*	ICON IS ALWAYS COPIED!!!
*/
#ifdef UNICODE
INT_PTR ServiceGetClientIconW(WPARAM wParam, LPARAM lParam)
{
	LPWSTR wszMirVer = (LPWSTR)wParam;			// MirVer value to get client for.
/*
	static HICON hIcon = NULL;	// returned HICON
	if(hIcon)
	{
		DestroyIcon(hIcon);
		hIcon = NULL;
	}
*/

	if(wszMirVer == NULL) return 0;

	{
		HICON hIcon = NULL;			// returned HICON
		int NoCopy = (int)lParam;	// noCopy
		short base, overlay, overlay2, overlay3;

		GetIconsIndexesW(wszMirVer, &base, &overlay, &overlay2, &overlay3);

		if(base != -1)
		{
			hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3);
		}
/*
		if(hIcon && !NoCopy)
			return (INT_PTR)CopyIcon(hIcon);
*/
		return (INT_PTR)hIcon;
	}
}

/*
 *	 ServiceSameClientW
 *	 MS_FP_SAMECLIENTSW service implementation.
 *	 wParam - LPWSTR first MirVer value
 *	 lParam - LPWSTR second MirVer value
 *	 return pointer to char string - client desription (do not destroy) if clients are same
 */
INT_PTR ServiceSameClientsW(WPARAM wParam, LPARAM lParam)
{
	LPWSTR wszMirVerFirst = (LPWSTR)wParam;	// MirVer value to get client for.
	LPWSTR wszMirVerSecond = (LPWSTR)lParam;	// MirVer value to get client for.
	int firstIndex, secondIndex;
	BOOL Result = FALSE;

	firstIndex = secondIndex = 0;
	if(!wszMirVerFirst || !wszMirVerSecond) return (INT_PTR)NULL;	//one of its is not null

	{
		LPWSTR wszMirVerFirstUp, wszMirVerSecondUp;
		size_t iMirVerFirstUpLen, iMirVerSecondUpLen;

		iMirVerFirstUpLen = wcslen(wszMirVerFirst) + 1;
		iMirVerSecondUpLen = wcslen(wszMirVerSecond) + 1;

		wszMirVerFirstUp = (LPWSTR)mir_alloc(iMirVerFirstUpLen * sizeof(WCHAR));
		wszMirVerSecondUp = (LPWSTR)mir_alloc(iMirVerSecondUpLen * sizeof(WCHAR));

		wcscpy_s(wszMirVerFirstUp, iMirVerFirstUpLen, wszMirVerFirst);
		wcscpy_s(wszMirVerSecondUp, iMirVerSecondUpLen, wszMirVerSecond);

		_wcsupr_s(wszMirVerFirstUp, iMirVerFirstUpLen);
		_wcsupr_s(wszMirVerSecondUp, iMirVerSecondUpLen);

		if(wcscmp(wszMirVerFirstUp, L"?") == 0)
			firstIndex = UNKNOWN_MASK_NUMBER;
		else
			while(firstIndex < DEFAULT_KN_FP_MASK_COUNT)
			{
				if(WildCompareW(wszMirVerFirstUp, def_kn_fp_mask[firstIndex].szMask))
					break;
				firstIndex++;
			}
		if(wcscmp(wszMirVerSecondUp, L"?") == 0)
			secondIndex = UNKNOWN_MASK_NUMBER;
		else
			while(secondIndex < DEFAULT_KN_FP_MASK_COUNT)
			{
				if(WildCompareW(wszMirVerSecondUp, def_kn_fp_mask[secondIndex].szMask))
					break;
	 			secondIndex++;
			}

		mir_free(wszMirVerFirstUp);
		mir_free(wszMirVerSecondUp);

		if(firstIndex == secondIndex && firstIndex < DEFAULT_KN_FP_MASK_COUNT)
		{
			return (INT_PTR)def_kn_fp_mask[firstIndex].szClientDescription;
		}
	}
	return (INT_PTR)NULL;
}
#endif // !UNICODE

/******************************************************************************
 *	Futher routines is for creating joined 'overlay' icons.
 ******************************************************************************/

 /*
 *	CreateBitmap32 - Create DIB 32 bitmap with sizes cx*cy
 */

HBITMAP __inline CreateBitmap32(int cx, int cy)
{
	return CreateBitmap32Point(cx, cy, NULL);
}

 /*
 *	CreateBitmap32 - Create DIB 32 bitmap with sizes cx*cy and put reference
 *				to new bitmap pixel image memory area to void ** bits
 */
HBITMAP FASTCALL CreateBitmap32Point(int cx, int cy, LPVOID* bits)
{
	BITMAPINFO bmpi = { 0 };
	LPVOID ptPixels = NULL;
	HBITMAP DirectBitmap;

	if(cx < 0 || cy < 0) return NULL;

	bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpi.bmiHeader.biWidth = cx;
	bmpi.bmiHeader.biHeight = cy;
	bmpi.bmiHeader.biPlanes = 1;
//	bmpi.bmiHeader.biCompression = BI_RGB;
	bmpi.bmiHeader.biBitCount = 32;

	DirectBitmap = CreateDIBSection(NULL,
					&bmpi,
					DIB_RGB_COLORS,
					&ptPixels,
					NULL, 0);
 	GdiFlush();
	if(ptPixels) memset(ptPixels, 0, cx * cy * 4);
	if(bits != NULL) *bits = ptPixels;

	return DirectBitmap;
}

/*
*	 checkHasAlfa - checks if image has at least one BYTE in alpha channel
*				 that is not a 0. (is image real 32 bit or just 24 bit)
*/
BOOL FASTCALL checkHasAlfa(LPBYTE from, int width, int height)
{
	LPDWORD pt = (LPDWORD)from;
	LPDWORD lim = pt + width * height;
	while(pt < lim)
	{
		if(*pt & 0xFF000000)
			return TRUE;
		pt++;
	}

	return FALSE;
}

/*
*	 checkMaskUsed - checks if mask image has at least one that is not a 0.
*	 Not sure is it required or not
*/
BOOL FASTCALL checkMaskUsed(LPBYTE from)
{
	int i;
	for(i = 0; i < 16 * 16 / 8; i++)
	{
		if(from[i] != 0) return TRUE;
	}
	return FALSE;
}

/*
*	 GetMaskBit - return value of apropriate mask bit in line at x position
*/
BOOL __inline GetMaskBit(LPBYTE line, int x)
{
	return ((*(line + (x >> 3))) & (0x01 << (7 - (x & 0x07)))) != 0;
}

/*
*	blend	- alpha blend ARGB values of 2 pixels. X1 - underlaying,
*	 X2 - overlaying points.
*/
DWORD FASTCALL blend(DWORD X1,DWORD X2)
{
	RGBA* q1 = (RGBA*)&X1;
	RGBA* q2 = (RGBA*)&X2;
	BYTE a_1 = ~q1->a;
	BYTE a_2 = ~q2->a;
	WORD am = q1->a * a_2;

	WORD ar = q1->a + ((a_1 * q2->a) / 255);
	// if a2 more than 0 than result should be more
	// or equal (if a1==0) to a2, else in combination
	// with mask we can get here black points

	ar = (q2->a > ar) ? q2->a : ar;

	if(ar == 0) return 0;

	{
		WORD arm = ar * 255;
		WORD rr = ((q1->r * am + q2->r * q2->a * 255)) / arm;
		WORD gr = ((q1->g * am + q2->g * q2->a * 255)) / arm;
		WORD br = ((q1->b * am + q2->b * q2->a * 255)) / arm;
		return (ar << 24) | (rr << 16) | (gr << 8) | br;
	}
}

/*
*	CreateJoinedIcon	- creates new icon by drawing hTop over hBottom.
*/
HICON FASTCALL CreateJoinedIcon(HICON hBottom, HICON hTop)
{
	BOOL drawn = FALSE;
	HDC tempDC, tempDC2, tempDC3;
	HICON res = NULL;
	HBITMAP oImage,nImage;
	HBITMAP nMask, hbm, obmp, obmp2;
	LPBYTE ptPixels = NULL;
	ICONINFO iNew = { 0 };
	BYTE p[32] = { 0 };

	tempDC = CreateCompatibleDC(NULL);
	nImage = CreateBitmap32Point(16, 16, (LPVOID*)&ptPixels);
	oImage = (HBITMAP)SelectObject(tempDC, nImage);

//	if(ptPixels) memset(ptPixels, 0, 16 * 16 * 4);

	if(IsWinVerXPPlus())
	{
		ICONINFO iciBottom = { 0 };
		ICONINFO iciTop = { 0 };

		BITMAP bmp_top = { 0 };
		BITMAP bmp_top_mask = { 0 };

		BITMAP bmp_bottom = { 0 };
		BITMAP bmp_bottom_mask = { 0 };

		GetIconInfo(hBottom, &iciBottom);
		GetObject(iciBottom.hbmColor, sizeof(BITMAP), &bmp_bottom);
		GetObject(iciBottom.hbmMask, sizeof(BITMAP), &bmp_bottom_mask);

		GetIconInfo(hTop, &iciTop);
		GetObject(iciTop.hbmColor, sizeof(BITMAP), &bmp_top);
		GetObject(iciTop.hbmMask, sizeof(BITMAP), &bmp_top_mask);

		if(bmp_bottom.bmBitsPixel == 32 && bmp_top.bmBitsPixel == 32)
		{
			LPBYTE BottomBuffer, TopBuffer, BottomMaskBuffer, TopMaskBuffer;
			LPBYTE bb, tb, bmb, tmb;
			LPBYTE db = ptPixels;
			int vstep_d = 16 * 4;
			int vstep_b = bmp_bottom.bmWidthBytes;
			int vstep_t = bmp_top.bmWidthBytes;
			int vstep_bm = bmp_bottom_mask.bmWidthBytes;
			int vstep_tm = bmp_top_mask.bmWidthBytes;

			if(bmp_bottom.bmBits)
				bb = BottomBuffer = (LPBYTE)bmp_bottom.bmBits;
			else
			{
				BottomBuffer = (LPBYTE)_alloca(bmp_bottom.bmHeight * bmp_bottom.bmWidthBytes);
				GetBitmapBits(iciBottom.hbmColor, bmp_bottom.bmHeight * bmp_bottom.bmWidthBytes, BottomBuffer);
				bb = BottomBuffer + vstep_b * (bmp_bottom.bmHeight - 1);
				vstep_b = -vstep_b;
			}
			if(bmp_top.bmBits)
				tb = TopBuffer = (LPBYTE)bmp_top.bmBits;
			else
			{
				TopBuffer = (LPBYTE)_alloca(bmp_top.bmHeight * bmp_top.bmWidthBytes);
				GetBitmapBits(iciTop.hbmColor, bmp_top.bmHeight * bmp_top.bmWidthBytes, TopBuffer);
				tb = TopBuffer + vstep_t * (bmp_top.bmHeight - 1);
				vstep_t = -vstep_t;
			}
			if(bmp_bottom_mask.bmBits)
				bmb = BottomMaskBuffer = (LPBYTE)bmp_bottom_mask.bmBits;
			else
			{
				BottomMaskBuffer = (LPBYTE)_alloca(bmp_bottom_mask.bmHeight * bmp_bottom_mask.bmWidthBytes);
				GetBitmapBits(iciBottom.hbmMask, bmp_bottom_mask.bmHeight * bmp_bottom_mask.bmWidthBytes, BottomMaskBuffer);
				bmb = BottomMaskBuffer + vstep_bm * (bmp_bottom_mask.bmHeight - 1);
				vstep_bm = -vstep_bm;
			}
			if(bmp_top_mask.bmBits)
				tmb = TopMaskBuffer = (LPBYTE)bmp_top_mask.bmBits;
			else
			{
				TopMaskBuffer = (LPBYTE)_alloca(bmp_top_mask.bmHeight * bmp_top_mask.bmWidthBytes);
				GetBitmapBits(iciTop.hbmMask, bmp_top_mask.bmHeight * bmp_top_mask.bmWidthBytes, TopMaskBuffer);
				tmb = TopMaskBuffer + vstep_tm * (bmp_top_mask.bmHeight - 1);
				vstep_tm = -vstep_tm;
			}
			{
				int x; int y;
				BOOL topHasAlpha = checkHasAlfa(TopBuffer, bmp_top.bmWidth, bmp_top.bmHeight);
				BOOL bottomHasAlpha = checkHasAlfa(BottomBuffer, bmp_bottom.bmWidth, bmp_bottom.bmHeight);
				BOOL topMaskUsed = !topHasAlpha && checkMaskUsed(TopMaskBuffer);
				BOOL bottomMaskUsed = !bottomHasAlpha && checkMaskUsed(BottomMaskBuffer);

				for(y = 0; y < 16; y++)
				{
					for(x = 0; x < 16; x++)
					{
						DWORD bottom_d = ((LPDWORD)bb)[x];
						DWORD top_d = ((LPDWORD)tb)[x];

						if(topMaskUsed)
						{
							if(GetMaskBit(tmb, x))
								top_d &= 0x00FFFFFF;
							else
								top_d |= 0xFF000000;
						}
						else if(!topHasAlpha)
							top_d |= 0xFF000000;

						if(bottomMaskUsed)
						{
							if(GetMaskBit(bmb, x))
								bottom_d &= 0x00FFFFFF;
							else
								bottom_d |= 0xFF000000;
						}
						else if(!bottomHasAlpha)
							bottom_d |= 0xFF000000;

						((LPDWORD)db)[x] = blend(bottom_d, top_d);
					}
					bb += vstep_b;
					tb += vstep_t;
					bmb += vstep_bm;
					tmb += vstep_tm;
					db += vstep_d;
				}
			}

			drawn = TRUE;
		}

		DeleteObject(iciBottom.hbmColor);
		DeleteObject(iciTop.hbmColor);
		DeleteObject(iciBottom.hbmMask);
		DeleteObject(iciTop.hbmMask);
	}

	if(!drawn)
	{
		DrawIconEx(tempDC, 0, 0, hBottom, 16, 16, 0, NULL, DI_NORMAL);
		DrawIconEx(tempDC, 0, 0, hTop, 16, 16, 0, NULL, DI_NORMAL);
	}

	nMask = CreateBitmap(16, 16, 1, 1, p);
	tempDC2 = CreateCompatibleDC(NULL);
	tempDC3 = CreateCompatibleDC(NULL);
	hbm = CreateCompatibleBitmap(tempDC3, 16, 16);
	obmp = (HBITMAP)SelectObject(tempDC2, nMask);
	obmp2 = (HBITMAP)SelectObject(tempDC3, hbm);
	DrawIconEx(tempDC2, 0, 0, hBottom, 16, 16, 0, NULL, DI_MASK);
	DrawIconEx(tempDC3, 0, 0, hTop, 16, 16, 0, NULL, DI_MASK);
	BitBlt(tempDC2, 0, 0, 16, 16, tempDC3, 0, 0, SRCAND);

	GdiFlush();

	SelectObject(tempDC2, obmp);
	DeleteDC(tempDC2);

	SelectObject(tempDC3, obmp2);
	DeleteDC(tempDC3);

	SelectObject(tempDC, oImage);
	DeleteDC(tempDC);

	DeleteObject(hbm);

	iNew.fIcon = TRUE;
	iNew.hbmColor = nImage;
	iNew.hbmMask = nMask;
	res = CreateIconIndirect(&iNew);

	DeleteObject(nImage);
	DeleteObject(nMask);

	return res;
}

HANDLE FASTCALL GetIconIndexFromFI(LPTSTR szMirVer)
{
	int i;
	DWORD val;
	HANDLE hFoundImage = INVALID_HANDLE_VALUE;
	short base, overlay, overlay2, overlay3;

	GetIconsIndexes(szMirVer, &base, &overlay, &overlay2, &overlay3);

	if(base == -1 || nFICount == 0xFF) return hFoundImage;

	// MAX: 1024 + 256 + 128 + 128
	val = (base << 22) | ((overlay & 0xFF) << 14) | ((overlay2 & 0x7F) << 7) | (overlay3 & 0x7F);

	for(i = 0; i < nFICount; i++)
	{
		if(fiList[i].dwArray == val)
		{
			hFoundImage = fiList[i].hRegisteredImage;
			break;
		}
	}

	if(hFoundImage == INVALID_HANDLE_VALUE && i == nFICount) //not found - then add
	{

		HICON hIcon = CreateIconFromIndexes(base, overlay, overlay2, overlay3);

		fiList = (FOUNDINFO*)mir_realloc(fiList, sizeof(FOUNDINFO) * (nFICount + 1));
		fiList[nFICount].dwArray = val;

		if(hIcon != NULL)
		{
			fiList[nFICount].hRegisteredImage = (hIcon) ? (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)hIcon, 0) : INVALID_HANDLE_VALUE;

			hFoundImage = fiList[nFICount].hRegisteredImage;

			DestroyIcon(hIcon);
		}
		else
		{
			fiList[nFICount].hRegisteredImage = INVALID_HANDLE_VALUE;
		}
		nFICount++;
	}

	return hFoundImage;
}

VOID FASTCALL ClearFI()
{
	if(fiList != NULL)
		mir_free(fiList);
	fiList = NULL;
	nFICount = 0;
	return;
}
