/*
	ClientChangeNotify - Plugin for Miranda IM
	Copyright (c) 2006-2008 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Common.h"
#include "Misc.h"
#include "VersionNo.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_history.h"
#include "m_protocols.h"
#include "m_updater.h"
#include "m_protosvc.h"
#include "m_metacontacts.h"
#include "m_icolib.h"
#include "m_genmenu.h"
#include "m_ContactSettings.h"

HINSTANCE g_hInstance;
HANDLE g_hMainThread;
HANDLE g_hTogglePopupsMenuItem;
PLUGINLINK *pluginLink;
int hLangpack;
MM_INTERFACE mmi;
TMyArray<HANDLE> hHooks, hServices;
COptPage *g_PreviewOptPage; // we need to show popup even for the NULL contact if g_PreviewOptPage is not NULL (used for popup preview)


// my_make_version is required to break up #define PRODUCTVER from VersionNo.h
DWORD my_make_version(const int a, const int b, const int c, const int d)
{
	return PLUGIN_MAKE_VERSION(a, b, c, d);
}

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"ClientChangeNotify ("
#ifdef _DEBUG
	"DEBUG "
#endif
#ifdef _UNICODE
	"Unicode"
#else
	"ANSI"
#endif
	")",
	0, // see VersionNo.h
	"ClientChangeNotify plugin for Miranda IM.  Build #"STRSPECIALBUILD" [ "__DATE__"  "__TIME__
#ifdef _DEBUG
	" DEBUG"
#endif
#ifdef _UNICODE
	" Unicode"
#else
	" ANSI"
#endif
	" ]",
	"Deathdemon",
	"dchervov@yahoo.com",
	"© 2006-2008 Chervov Dmitry",
	"http://deathdemon.int.ru/",
	UNICODE_AWARE,
	0,
#ifdef _UNICODE
	{0xb68a8906, 0x748b, 0x435d, {0x93, 0xe, 0x21, 0xcc, 0x6e, 0x8f, 0x3b, 0x3f}}
// {B68A8906-748B-435d-930E-21CC6E8F3B3F}
#else
	{0x4e8d06c6, 0xde4f, 0x4b72, {0x9c, 0x52, 0xc2, 0x78, 0x72, 0xed, 0x3, 0xb9}}
// {4E8D06C6-DE4F-4b72-9C52-C27872ED03B9}
#endif
};

PLUGININFO oldPluginInfo = {
	sizeof(PLUGININFO),
	pluginInfo.shortName,
	pluginInfo.version,
	pluginInfo.description,
	pluginInfo.author,
	pluginInfo.authorEmail,
	pluginInfo.copyright,
	pluginInfo.homepage,
	pluginInfo.flags,
	pluginInfo.replacesDefaultModule
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInstance = hinstDLL;
	return TRUE;
}

#define MIID_CLIENTCHANGENOTIFY {0xe9d1f0d4, 0xd65d, 0x4840, {0x87, 0xbd, 0x59, 0xd7, 0xb4, 0x70, 0x2c, 0x47}}
// {E9D1F0D4-D65D-4840-87BD-59D7B4702C47}

static const MUUID interfaces[] = {MIID_CLIENTCHANGENOTIFY, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfo.version = my_make_version(PRODUCTVER);
	return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	oldPluginInfo.version = my_make_version(PRODUCTVER);
	return &oldPluginInfo;
}


static int CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_MEASUREITEM:
		{
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		case WM_DRAWITEM:
		{
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static VOID CALLBACK ShowContactMenu(DWORD wParam)
// wParam = hContact
{
	POINT pt;
	HWND hMenuWnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T(MOD_NAME)_T("_MenuWindow"), 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInstance, NULL);
	SetWindowLong(hMenuWnd, GWL_WNDPROC, (LONG)(WNDPROC)MenuWndProc);
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)wParam, 0);
	GetCursorPos(&pt);
	SetForegroundWindow(hMenuWnd);
	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hMenuWnd, NULL), MPCF_CONTACTMENU), (LPARAM)wParam);
	PostMessage(hMenuWnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
	DestroyWindow(hMenuWnd);
}


void Popup_DoAction(HWND hWnd, BYTE Action, PLUGIN_DATA *pdata)
{
	HANDLE hContact = (HANDLE)CallService(MS_POPUP_GETCONTACT, (WPARAM)hWnd, 0);
	switch (Action)
	{
		case PCA_OPENMESSAGEWND: // open message window
		{
			if (hContact && hContact != INVALID_HANDLE_VALUE)
			{
				CallServiceSync(ServiceExists("SRMsg/LaunchMessageWindow") ? "SRMsg/LaunchMessageWindow" : MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
			}
		} break;
		case PCA_OPENMENU: // open contact menu
		{
			if (hContact && hContact != INVALID_HANDLE_VALUE)
			{
				QueueUserAPC(ShowContactMenu, g_hMainThread, (ULONG_PTR)hContact);
			}
		} break;
		case PCA_OPENDETAILS: // open contact details window
		{
			if (hContact != INVALID_HANDLE_VALUE)
			{
				CallServiceSync(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
			}
		} break;
		case PCA_OPENHISTORY: // open contact history
		{
			if (hContact != INVALID_HANDLE_VALUE)
			{
				CallServiceSync(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)hContact, 0);
			}
		} break;
		case PCA_OPENLOG: // open log file
		{
			TCString LogFilePath;
			LS_LOGINFO li = {0};
			li.cbSize = sizeof(li);
			li.szID = LOG_ID;
			li.hContact = hContact;
			li.Flags = LSLI_TCHAR;
			li.tszLogPath = LogFilePath.GetBuffer(MAX_PATH);
			if (!CallService(MS_LOGSERVICE_GETLOGINFO, (WPARAM)&li, 0))
			{
				LogFilePath.ReleaseBuffer();
				ShowLog(LogFilePath);
			} else
			{
				LogFilePath.ReleaseBuffer();
			}
		} break;
		case PCA_CLOSEPOPUP: // close popup
		{
			PUDeletePopUp(hWnd);
		} break;
		case PCA_DONOTHING: // do nothing
			break;
	}
}


static int CALLBACK PopupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);
	if (pdata)
	{
		switch (message)
		{
			case WM_COMMAND:
			{
				if (HIWORD(wParam) == STN_CLICKED) // left mouse button
				{
					Popup_DoAction(hWnd, pdata->PopupLClickAction, pdata);
					return true;
				}
			} break;
			case WM_CONTEXTMENU:	// right mouse button
			{
				Popup_DoAction(hWnd, pdata->PopupRClickAction, pdata);
				return true;
			} break;
			case UM_FREEPLUGINDATA:
			{
				if (pdata->hIcon)
				{
					DestroyIcon(pdata->hIcon);
				}
				free(pdata);
				return false;
			} break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void ShowPopup(SHOWPOPUP_DATA *sd)
{
	TCString PopupText;
	if (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_SHOWPREVCLIENT))
	{
		mir_sntprintf(PopupText.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("changed client to %s (was %s)"), (const TCHAR*)sd->MirVer, (const TCHAR*)sd->OldMirVer);
		PopupText.ReleaseBuffer();
	} else
	{
		mir_sntprintf(PopupText.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("changed client to %s"), (const TCHAR*)sd->MirVer);
		PopupText.ReleaseBuffer();
	}
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)calloc(1, sizeof(PLUGIN_DATA));
	POPUPDATAT ppd = {0};
	ppd.lchContact = sd->hContact;
	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)sd->hContact, 0);
	pdata->hIcon = ppd.lchIcon = (HICON)CallService(MS_FP_GETCLIENTICON, (WPARAM)(const char*)TCHAR2ANSI(sd->MirVer), false);
	_ASSERT(ppd.lchIcon);
	if (!ppd.lchIcon || (DWORD)ppd.lchIcon == CALLSERVICE_NOTFOUND)
	{ // if we didn't succeed retrieving client icon, show the usual status icon instead
		ppd.lchIcon = LoadSkinnedProtoIcon(szProto, DBGetContactSettingWord(sd->hContact, szProto, "Status", ID_STATUS_OFFLINE));
		pdata->hIcon = NULL;
	}
	_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)sd->hContact, GCDNF_TCHAR), lengthof(ppd.lptzContactName) - 1);
	_tcsncpy(ppd.lptzText, PopupText, lengthof(ppd.lptzText) - 1);
	ppd.colorBack = (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_DEFBGCOLOUR) ? 0 : sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_BGCOLOUR));
	ppd.colorText = (sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_DEFTEXTCOLOUR) ? 0 : sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_TEXTCOLOUR));
	ppd.PluginWindowProc = (WNDPROC)PopupWndProc;
	pdata->PopupLClickAction = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_LCLICK_ACTION);
	pdata->PopupRClickAction = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_RCLICK_ACTION);
	ppd.iSeconds = sd->PopupOptPage->GetValue(IDC_POPUPOPTDLG_POPUPDELAY);
	ppd.PluginData = pdata;
	CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
}


int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!lstrcmpA(cws->szSetting, DB_MIRVER))
	{
		HANDLE hContact = (HANDLE)wParam;
		SHOWPOPUP_DATA sd = {0};
		char *szProto = NULL;
		if (g_PreviewOptPage)
		{
			sd.MirVer = _T("Miranda IM 0.6.0.1 (ICQ v0.3.7 alpha)");
		} else
		{
			if (!hContact) // exit if hContact == NULL and it's not a popup preview
			{
				return 0;
			}
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			_ASSERT(szProto);
			if (ServiceExists(MS_MC_GETPROTOCOLNAME) && !strcmp(szProto, (char*)CallService(MS_MC_GETPROTOCOLNAME, 0, 0))) // workaround for metacontacts
			{
				return 0;
			}
			sd.MirVer = DBGetContactSettingString(hContact, szProto, DB_MIRVER, _T(""));
			if (sd.MirVer.IsEmpty())
			{
				return 0;
			}
		}
		sd.OldMirVer = DBGetContactSettingString(hContact, MOD_NAME, DB_OLDMIRVER, _T(""));
		DBWriteContactSettingTString(hContact, MOD_NAME, DB_OLDMIRVER, sd.MirVer); // we have to write it here, because we modify sd.OldMirVer and sd.MirVer to conform our settings later
		if (sd.OldMirVer.IsEmpty())
		{ // looks like it's the right way to do
			return 0;
		}
		COptPage PopupOptPage;
		if (g_PreviewOptPage)
		{
			PopupOptPage = *g_PreviewOptPage;
		} else
		{
			PopupOptPage = g_PopupOptPage;
			PopupOptPage.DBToMem();
		}
		HANDLE hContactOrMeta = (hContact && ServiceExists(MS_MC_GETMETACONTACT)) ? (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0) : hContact;
		if (!hContactOrMeta)
		{
			hContactOrMeta = hContact;
		}
		if (hContact && DBGetContactSettingByte(hContactOrMeta, "CList", "Hidden", 0))
		{
			return 0;
		}
		int PerContactSetting = hContact ? DBGetContactSettingByte(hContact, MOD_NAME, DB_CCN_NOTIFY, NOTIFY_USEGLOBAL) : NOTIFY_ALWAYS; // NOTIFY_ALWAYS for preview
		if (PerContactSetting == NOTIFY_USEGLOBAL && hContactOrMeta != hContact) // subcontact setting has a priority over a metacontact setting
		{
			PerContactSetting = DBGetContactSettingByte(hContactOrMeta, MOD_NAME, DB_CCN_NOTIFY, NOTIFY_USEGLOBAL);
		}
		if (PerContactSetting && (PerContactSetting == NOTIFY_ALMOST_ALWAYS || PerContactSetting == NOTIFY_ALWAYS || !PopupOptPage.GetValue(IDC_POPUPOPTDLG_USESTATUSNOTIFYFLAG) || !(DBGetContactSettingDword(hContactOrMeta, "Ignore", "Mask1", 0) & 0x8))) // check if we need to notify at all
		{
			sd.hContact = hContact;
			sd.PopupOptPage = &PopupOptPage;
			if (!PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY) || !PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWVER))
			{
				CString OldMirVerA = TCHAR2ANSI(sd.OldMirVer);
				CString MirVerA = TCHAR2ANSI(sd.MirVer);
				if (ServiceExists(MS_FP_SAMECLIENTS))
				{
					char *szOldClient = (char*)CallService(MS_FP_SAMECLIENTS, (WPARAM)(const char*)OldMirVerA, (LPARAM)(const char*)OldMirVerA); // remove version from MirVer strings. I know, the way in which MS_FP_SAMECLIENTS is used here is pretty ugly, but at least it gives necessary results
					char *szClient = (char*)CallService(MS_FP_SAMECLIENTS, (WPARAM)(const char*)MirVerA, (LPARAM)(const char*)MirVerA);
					if (szOldClient && szClient)
					{
						if (PerContactSetting != NOTIFY_ALMOST_ALWAYS && PerContactSetting != NOTIFY_ALWAYS && !PopupOptPage.GetValue(IDC_POPUPOPTDLG_VERCHGNOTIFY) && !strcmp(szClient, szOldClient))
						{
							return 0;
						}
						if (!PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWVER))
						{
							sd.MirVer = ANSI2TCHAR(szClient);
							sd.OldMirVer = ANSI2TCHAR(szOldClient);
						}
					}
				}
			}
			if (sd.MirVer == (const TCHAR*)sd.OldMirVer)
			{
				_ASSERT(hContact);
				return 0;
			}
			if (PerContactSetting == NOTIFY_ALWAYS || (PopupOptPage.GetValue(IDC_POPUPOPTDLG_POPUPNOTIFY) && (g_PreviewOptPage || PerContactSetting == NOTIFY_ALMOST_ALWAYS || !PcreCheck(sd.MirVer))))
			{
				ShowPopup(&sd);
				SkinPlaySound(CLIENTCHANGED_SOUND);
			}
		}
		if (hContact)
		{
			TCString ClientName;
			if (PopupOptPage.GetValue(IDC_POPUPOPTDLG_SHOWPREVCLIENT) && sd.OldMirVer.GetLen())
			{
				mir_sntprintf(ClientName.GetBuffer(MAX_MSG_LEN), MAX_MSG_LEN, TranslateT("%s (was %s)"), (const TCHAR*)sd.MirVer, (const TCHAR*)sd.OldMirVer);
				ClientName.ReleaseBuffer();
			} else
			{
				ClientName = sd.MirVer;
			}
			if (ServiceExists(MS_VARS_FORMATSTRING))
			{
				logservice_log(LOG_ID, hContact, ClientName);
			} else
			{
				_ASSERT(szProto);
				TCString szUID(_T(""));
				char *uid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
				if (uid && (int)uid != CALLSERVICE_NOTFOUND)
				{
					szUID = DBGetContactSettingAsString(hContact, szProto, uid, _T(""));
				}
				logservice_log(LOG_ID, hContact, TCString((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)) + _T(" (") + szUID + TranslateT(") changed client to ") + ClientName);
			}
		}
		_ASSERT(sd.MirVer.GetLen()); // save the last known MirVer value even if the new one is empty
	}
	return 0;
}


static int ContactSettingsInit(WPARAM wParam, LPARAM lParam)
{
	CONTACTSETTINGSINIT *csi = (CONTACTSETTINGSINIT*)wParam;
	char *szProto = (csi->Type == CSIT_CONTACT) ? (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)csi->hContact, 0) : NULL;
	if ((csi->Type == CSIT_GROUP) || (szProto && csi->Type == CSIT_CONTACT))
	{
		int Flag1 = (csi->Type == CSIT_CONTACT) ? CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) : PF1_IM; // if it's a group settings dialog, we assume that there are possibly some contacts in the group with PF1_IM capability
		if (Flag1 & (PF1_IMRECV | PF1_URLRECV | PF1_FILERECV)) // I hope, these flags are sufficient to describe which protocols can theoretically have a client
		{
			CONTACTSETTINGSCONTROL csc = {0};
			csc.cbSize = sizeof(csc);
			csc.cbStateSize = sizeof(CSCONTROLSTATE);
			csc.Position = CSPOS_SORTBYALPHABET;
			csc.Flags = CSCF_TCHAR;
			csc.ControlType = CSCT_COMBOBOX;
			csc.ptszTitle = LPGENT("Client change notifications:");
			csc.ptszGroup = CSGROUP_NOTIFICATIONS;
			csc.szModule = MOD_NAME;
			csc.szSetting = DB_CCN_NOTIFY;
			csc.StateNum = 4;
			csc.DefState = 3;
			CSCONTROLSTATE States[] = {CSCONTROLSTATE(LPGENT("Never, ignore client changes for this contact"), (BYTE)NOTIFY_IGNORE), CSCONTROLSTATE(LPGENT("Always except when client change notifications are disabled globally"), (BYTE)NOTIFY_ALMOST_ALWAYS), CSCONTROLSTATE(LPGENT("Always, even when client change notifications are disabled globally"), (BYTE)NOTIFY_ALWAYS), CSCONTROLSTATE(LPGENT("Use global settings (default)"), (BYTE)NOTIFY_USEGLOBAL)};
			csc.pStates = States;
			CallService(MS_CONTACTSETTINGS_ADDCONTROL, wParam, (LPARAM)&csc);
		}
	}
	return 0;
}


static int srvTogglePopups(WPARAM wParam, LPARAM lParam)
{
	g_PopupOptPage.SetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY, !g_PopupOptPage.GetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY));
	return 0;
}


static int PrebuildMainMenu(WPARAM wParam, LPARAM lParam)
{ // we have to use ME_CLIST_PREBUILDMAINMENU instead of updating menu items only on settings change, because "popup_enabled" and "popup_disabled" icons are not always available yet in ModulesLoaded
	if (ServiceExists(MS_POPUP_ADDPOPUPT))
	{
		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(mi);
		mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | CMIM_NAME | CMIM_ICON;
		if (g_PopupOptPage.GetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY))
		{
			mi.ptszName = TranslateT("Disable c&lient change notification");
			mi.hIcon = ServiceExists(MS_SKIN2_GETICON) ? (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"popup_enabled") : NULL;
		} else
		{
			mi.ptszName = TranslateT("Enable c&lient change notification");
			mi.hIcon = ServiceExists(MS_SKIN2_GETICON) ? (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"popup_disabled") : NULL;
		}
		mi.ptszPopupName = TranslateT("PopUps");
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hTogglePopupsMenuItem, (LPARAM)&mi);
	}
	return 0;
}


int CALLBACK CCNErrorDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			return true;
		} break;
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				DestroyWindow(hwndDlg);
			}
		} break;
		case WM_DESTROY:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_DONTREMIND))
			{
				DBWriteContactSettingByte(NULL, MOD_NAME, DB_NO_FINGERPRINT_ERROR, 1);
			}
		} break;
	}
	return 0;
}


int MirandaLoaded(WPARAM wParam, LPARAM lParam)
{
	InitPcre();
	COptPage PopupOptPage(g_PopupOptPage);
	PopupOptPage.DBToMem();
	RecompileRegexps(*(TCString*)PopupOptPage.GetValue(IDC_POPUPOPTDLG_IGNORESTRINGS));
	hHooks.AddElem(HookEvent(ME_OPT_INITIALISE, OptionsDlgInit));
	hHooks.AddElem(HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged));
	hHooks.AddElem(HookEvent(ME_CONTACTSETTINGS_INITIALISE, ContactSettingsInit));
	SkinAddNewSound(CLIENTCHANGED_SOUND, Translate("ClientChangeNotify: Client changed"), "");
// updater plugin support
	Update update = {0};
	char szVersion[16];
	update.cbSize = sizeof(Update);
	update.szComponentName = pluginInfo.shortName;
	update.pbVersion = (BYTE*)CreateVersionString(my_make_version(PRODUCTVER), szVersion);
	update.cpbVersion = strlen((char*)update.pbVersion);
	update.szUpdateURL = "http://deathdemon.int.ru/projects/ClientChangeNotify"
#ifdef _UNICODE
		"W"
#endif
		".zip";
	update.szVersionURL = "http://deathdemon.int.ru/updaterinfo.php";
	update.pbVersionPrefix = (BYTE*)"ClientChangeNotify"
#ifdef _UNICODE
		" Unicode"
#endif
		" version ";
	update.cpbVersionPrefix = strlen((char*)update.pbVersionPrefix);
	CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	if (ServiceExists(MS_POPUP_ADDPOPUPT))
	{
		hServices.AddElem(CreateServiceFunction(MS_CCN_TOGGLEPOPUPS, srvTogglePopups));
		hHooks.AddElem(HookEvent(ME_CLIST_PREBUILDMAINMENU, PrebuildMainMenu));
		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(mi);
		mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		if (g_PopupOptPage.GetDBValueCopy(IDC_POPUPOPTDLG_POPUPNOTIFY))
		{
			mi.ptszName = TranslateT("Disable c&lient change notification");
		} else
		{
			mi.ptszName = TranslateT("Enable c&lient change notification");
		}
		mi.pszService = MS_CCN_TOGGLEPOPUPS;
		mi.ptszPopupName = TranslateT("PopUps");
		g_hTogglePopupsMenuItem = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	}

/*	if (DBGetContactSettingByte(NULL, MOD_NAME, DB_FIRSTRUN, 1)) // DB_SETTINGSVER
	{
		DBWriteContactSettingByte(NULL, MOD_NAME, DB_FIRSTRUN, 0);
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); // copy MirVer to OldMirVer for every contact; well, it won't work for every possible case (we can't detect when user had disabled the plugin for some time and then enabled it again), but at least it'll work once, for the first run ;)
		do
		{
			char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if (szProto)
			{
				TCString MirVer(DBGetContactSettingString(hContact, szProto, DB_MIRVER, _T("")));
				if (MirVer.GetLen())
				{
					DBWriteContactSettingTString(hContact, MOD_NAME, DB_OLDMIRVER, MirVer);
				}
			}
		} while (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0));
	}*/
	if ((!ServiceExists(MS_FP_SAMECLIENTS) || !ServiceExists(MS_FP_GETCLIENTICON)) && !DBGetContactSettingByte(NULL, MOD_NAME, DB_NO_FINGERPRINT_ERROR, 0))
	{ // seems that Fingerprint is not installed
		CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_CCN_ERROR), NULL, CCNErrorDlgProc);
	}
	logservice_register(LOG_ID, LPGENT("ClientChangeNotify"), _T("ClientChangeNotify?puts(p,?dbsetting(%subject%,Protocol,p))?if2(_?dbsetting(,?get(p),?pinfo(?get(p),uidsetting)),).log"), TranslateT("`[`!cdate()-!ctime()`]`  ?cinfo(%subject%,display) (?cinfo(%subject%,id)) changed client to %extratext%"));
	return 0;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI( &mmi );
	mir_getLP( &pluginInfo );

	hHooks.AddElem(HookEvent(ME_SYSTEM_MODULESLOADED, MirandaLoaded));
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &g_hMainThread, THREAD_SET_CONTEXT, false, 0);
	InitOptions();
	if (DBGetContactSettingString(NULL, "KnownModules", MOD_NAME, (char*)NULL) == NULL)
	{
		DBWriteContactSettingString(NULL, "KnownModules", MOD_NAME, MOD_NAME);
	}
	if (DBGetContactSettingByte(NULL, MOD_NAME, DB_SETTINGSVER, 0) < 1)
	{
		TCString Str;
		Str = DBGetContactSettingString(NULL, MOD_NAME, DB_IGNORESUBSTRINGS, _T(""));
		if (Str.GetLen())
		{ // fix incorrect regexp from v0.1.1.0
			DBWriteContactSettingTString(NULL, MOD_NAME, DB_IGNORESUBSTRINGS, Str.Replace(_T("/Miranda[0-9A-F]{8}/"), _T("/[0-9A-F]{8}(\\W|$)/")));
		}
		DBWriteContactSettingByte(NULL, MOD_NAME, DB_SETTINGSVER, 1);
	}
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	CloseHandle(g_hMainThread);
	int I;
	for (I = 0; I < hHooks.GetSize(); I++)
	{
		if (hHooks[I])
		{
			UnhookEvent(hHooks[I]);
		}
	}
	for (I = 0; I < hServices.GetSize(); I++)
	{
		if (hServices[I])
		{
			DestroyServiceFunction(hServices[I]);
		}
	}
	UninitPcre();
	return 0;
}
