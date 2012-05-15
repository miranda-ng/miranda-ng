/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

#define UN_MOD               "UpdateNotify"
#define UN_ENABLE            "UpdateNotifyEnable"
#define UN_ENABLE_DEF        1
#define UN_LASTCHECK         "UpdateNotifyLastCheck"
#define UN_SERVERPERIOD      "UpdateNotifyPingDelayPeriod"
#define UN_CURRENTVERSION    "UpdateNotifyCurrentVersion"
#define UN_CURRENTVERSIONFND "UpdateNotifyCurrentVersionFound"
#define UN_NOTIFYTYPE        "UpdateNotifyType"
#define UN_NOTIFYTYPE_STABLE 1
#define UN_NOTIFYTYPE_BETA   2
#define UN_NOTIFYTYPE_ALPHA  3
#define UN_NOTIFYTYPE_DEF    UN_NOTIFYTYPE_STABLE
#define UN_CUSTOMXMLURL      "UpdateNotifyCustomXMLURL"
#define UN_URLXML            "http://update.miranda-im.org/update.xml"
#define UN_MINCHECKTIME      60*60 /* Check no more than once an hour */
#define UN_DEFAULTCHECKTIME  60*48*60 /* Default to check once every 48 hours */
#define UN_FIRSTCHECK        15 /* First check 15 seconds after startup */
#define UN_REPEATNOTIFYDLY   24*60*60 /* 24 hours before showing release notification again */

typedef struct {
	int isNew;
	int isManual;
    char version[64];
    char versionReal[16];
    char notesUrl[256];
	char downloadUrl[256];
	DWORD reqTime;
} UpdateNotifyData;

typedef struct {
    DWORD dwVersion;
    char *szVersionPublic;
    char *szVersion;
    char *szDownload;
    char *szNotes;
} UpdateNotifyReleaseData;

static BOOL bModuleInitialized = FALSE;
static HANDLE hNetlibUser = 0, hHookModules, hHookPreShutdown;
static UINT_PTR updateTimerId;
static HANDLE dwUpdateThreadID = 0;
static HWND hwndUpdateDlg = 0, hwndManualUpdateDlg = 0;
static XML_API xun;

static int UpdateNotifyOptInit(WPARAM wParam, LPARAM lParam);
static INT_PTR UpdateNotifyMenuCommand(WPARAM wParam, LPARAM lParam);
static VOID CALLBACK UpdateNotifyTimerCheck(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static int UpdateNotifyMakeRequest(UpdateNotifyData *und);
static void UpdateNotifyPerform(void *m);
static INT_PTR CALLBACK UpdateNotifyProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK UpdateNotifyOptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static int UpdateNotifyModulesLoaded(WPARAM, LPARAM) {
	NETLIBUSER nlu;

	ZeroMemory(&nlu, sizeof(nlu));
	nlu.cbSize = sizeof(nlu);
	nlu.flags =  NUF_OUTGOING|NUF_HTTPCONNS;
	nlu.szSettingsModule = UN_MOD;
	nlu.szDescriptiveName = Translate("Update notification");
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	return 0;
}

static int UpdateNotifyPreShutdown(WPARAM, LPARAM) {
	if (IsWindow(hwndUpdateDlg)) {
		SendMessage(hwndUpdateDlg, WM_COMMAND, MAKELONG(IDOK, 0), 0);
	}
	if (IsWindow(hwndManualUpdateDlg)) {
		SendMessage(hwndManualUpdateDlg, WM_COMMAND, MAKELONG(IDOK, 0), 0);
	}
	return 0;
}

int LoadUpdateNotifyModule(void) {
	CLISTMENUITEM mi = { 0 };
	
	bModuleInitialized = TRUE;
	
    // Upgrade Routine
    if (DBGetContactSettingByte(NULL, UN_MOD, "UpdateNotifyNotifyAlpha", 0)) {
        DBDeleteContactSetting(NULL, UN_MOD, "UpdateNotifyNotifyAlpha");
        DBWriteContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_ALPHA);
    }
    // Ene Upgrade Routine
    
	CreateServiceFunction("UpdateNotify/UpdateCommand", UpdateNotifyMenuCommand);
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_EMPTYBLOB);
	mi.pszPopupName = LPGEN("&Help");
	mi.position = 2000030000;
	mi.pszName = LPGEN("Check for Update");
	mi.pszService = "UpdateNotify/UpdateCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	
	hHookModules = HookEvent(ME_SYSTEM_MODULESLOADED, UpdateNotifyModulesLoaded);
	hHookPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, UpdateNotifyPreShutdown);
	HookEvent(ME_OPT_INITIALISE, UpdateNotifyOptInit);
	updateTimerId = SetTimer(NULL, 0, 1000*UN_FIRSTCHECK, UpdateNotifyTimerCheck);
    mir_getXI(&xun);
	return 0;
}

void UnloadUpdateNotifyModule()
{
	if (!bModuleInitialized) return;
	UnhookEvent(hHookModules);
	UnhookEvent(hHookPreShutdown);
}

static int UpdateNotifyOptInit(WPARAM wParam, LPARAM) {
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hMirandaInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_UPDATENOTIFY);
	odp.pszGroup = LPGEN("Events");
	odp.pszTitle = LPGEN("Update Notify");
	odp.pfnDlgProc = UpdateNotifyOptsProc;
	odp.flags = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

static INT_PTR UpdateNotifyMenuCommand(WPARAM, LPARAM) {
	UpdateNotifyData und;
	
	if (IsWindow(hwndManualUpdateDlg)) {
		SetForegroundWindow(hwndManualUpdateDlg);
		return 0;
	}
	ZeroMemory(&und, sizeof(und));
	UpdateNotifyMakeRequest(&und);
	if (und.isNew) {
		DBWriteContactSettingString(NULL, UN_MOD, UN_CURRENTVERSION, und.versionReal);
		DBWriteContactSettingDword(NULL, UN_MOD, UN_CURRENTVERSIONFND, und.reqTime);
	}
	und.isManual = 1;
	DialogBoxParam(hMirandaInst, MAKEINTRESOURCE(IDD_UPDATE_NOTIFY), 0, UpdateNotifyProc,(LPARAM)&und);
	hwndManualUpdateDlg = 0;
	return 0;	
}

static VOID CALLBACK UpdateNotifyTimerCheck(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(NULL, updateTimerId);
	if (!DBGetContactSettingByte(NULL, UN_MOD, UN_ENABLE, UN_ENABLE_DEF))
		return;
	if (dwUpdateThreadID!=0) {
		Netlib_Logf(hNetlibUser, "Update notification already running, ignoring attempt");
		return;
	}
	{
		DWORD lastCheck = 0;

		if (!hNetlibUser)
			return;
		lastCheck = DBGetContactSettingDword(NULL, UN_MOD, UN_LASTCHECK, 0);
		if (!lastCheck) { // never checked for update before
			Netlib_Logf(hNetlibUser, "Running update notify check for the first time.");
			dwUpdateThreadID = mir_forkthread(UpdateNotifyPerform, 0);
		}
		else {
			DWORD dwNow = time(NULL), dwTimeDiff;
			DWORD dwServerPing = DBGetContactSettingDword(NULL, UN_MOD, UN_SERVERPERIOD, UN_DEFAULTCHECKTIME);

			if (lastCheck>dwNow) {
				// time for last check is after the current date so reset lastcheck and quit
				DBWriteContactSettingDword(NULL, UN_MOD, UN_LASTCHECK, dwNow);
				return;
			}
			dwTimeDiff = dwNow - lastCheck;
			if (dwServerPing<UN_MINCHECKTIME)
				dwServerPing = UN_MINCHECKTIME;
			if (dwTimeDiff>dwServerPing)
				dwUpdateThreadID = mir_forkthread(UpdateNotifyPerform, 0);
		}
		updateTimerId = SetTimer(NULL, 0, 1000*UN_MINCHECKTIME, UpdateNotifyTimerCheck);
	}
}

static DWORD UpdateNotifyMakeVersion(char *str) {
    DWORD a1,a2,a3,a4;
    if (!str)
        return 0;
    sscanf(str, "%u.%u.%u.%u", &a1, &a2, &a3, &a4);
    return PLUGIN_MAKE_VERSION(a1, a2, a3, a4);
}

static int UpdateNotifyIsNewer(DWORD dwCurrent, DWORD dwTest) {
    if (dwTest>dwCurrent) 
        return 1;
    return 0;
}

static int UpdateNotifyReleaseDataValid(UpdateNotifyReleaseData *d) {
    if (d&&d->szVersionPublic&&d->szVersion&&d->szDownload&&d->szNotes)
        return 1;
    return 0;
}

static void UpdateNotifyFreeReleaseData(UpdateNotifyReleaseData *d) {
    if (!d) 
        return;
    if (d->szVersionPublic) mir_free(d->szVersionPublic);
    if (d->szVersion) mir_free(d->szVersion);
    if (d->szDownload) mir_free(d->szDownload);
    if (d->szNotes) mir_free(d->szNotes);
}

static void UpdateNotifyReleaseLogUpdate(UpdateNotifyReleaseData *d) {
    if (!UpdateNotifyReleaseDataValid(d)) 
        return;
    #ifdef _WIN64
    Netlib_Logf(hNetlibUser, "Update server version: %s [%s] [64-bit]", d->szVersionPublic, d->szVersion);
    #elif defined(_UNICODE)
    Netlib_Logf(hNetlibUser, "Update server version: %s [%s] [Unicode]", d->szVersionPublic, d->szVersion);
    #else
    Netlib_Logf(hNetlibUser, "Update server version: %s [%s] [ANSI]", d->szVersionPublic, d->szVersion);
    #endif
    
}

static void UpdateNotifyReleaseCopyData(UpdateNotifyReleaseData *d, UpdateNotifyData *und) {
    if (!UpdateNotifyReleaseDataValid(d)||!und) 
        return;
    mir_snprintf(und->version, sizeof(und->version), "%s", d->szVersionPublic);
    mir_snprintf(und->versionReal, sizeof(und->versionReal), "%s", d->szVersion);
    mir_snprintf(und->notesUrl, sizeof(und->notesUrl), "%s", d->szNotes);
    mir_snprintf(und->downloadUrl, sizeof(und->downloadUrl), "%s", d->szDownload);
}

static int UpdateNotifyMakeRequest(UpdateNotifyData *und) {
	NETLIBHTTPREQUEST req;
	NETLIBHTTPREQUEST *resp;
	NETLIBHTTPHEADER headers[1];
	DWORD dwVersion;
	char szVersion[32], szUrl[256], szVersionText[128], szUserAgent[64];
	int isUnicode, isAlphaCheck, isBetaCheck;
	DBVARIANT dbv;
	
	if (!und) 
		return 0;
	und->version[0] = 0;
	und->versionReal[0] = 0;
	und->notesUrl[0] = 0;
	und->downloadUrl[0] = 0;
	und->reqTime = time(NULL);
	
	DBWriteContactSettingDword(NULL, UN_MOD, UN_LASTCHECK, und->reqTime);
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(szVersionText), (LPARAM)szVersionText);
	isUnicode = strstr(szVersionText, "Unicode") != NULL ? 1 : 0;
	isBetaCheck = DBGetContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_DEF)==UN_NOTIFYTYPE_BETA?1:0;
	isAlphaCheck = DBGetContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_DEF)==UN_NOTIFYTYPE_ALPHA?1:0;
	dwVersion = CallService(MS_SYSTEM_GETVERSION, 0, 0);
	mir_snprintf(szVersion, sizeof(szVersion), "%d.%d.%d.%d",
		HIBYTE(HIWORD(dwVersion)), LOBYTE(HIWORD(dwVersion)),
		HIBYTE(LOWORD(dwVersion)), LOBYTE(LOWORD(dwVersion)));
	if (!DBGetContactSettingString(NULL, UN_MOD, UN_CUSTOMXMLURL, &dbv)) {
		mir_snprintf(szUrl, sizeof(szUrl), "%s", dbv.pszVal?dbv.pszVal:UN_URLXML);
		DBFreeVariant(&dbv);
	}
	else mir_snprintf(szUrl, sizeof(szUrl), "%s", UN_URLXML);
	ZeroMemory(&req, sizeof(req));
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = szUrl;
	req.flags = 0;
	headers[0].szName = "User-Agent";
	headers[0].szValue = szUserAgent;
    #ifdef _WIN64
    mir_snprintf(szUserAgent, sizeof(szUserAgent), "Miranda/%s (x64)", szVersion);
    #elif defined(_UNICODE)
    mir_snprintf(szUserAgent, sizeof(szUserAgent), "Miranda/%s (Unicode)", szVersion);
    #else
    mir_snprintf(szUserAgent, sizeof(szUserAgent), "Miranda/%s (ANSI)", szVersion);
    #endif
	req.headersCount = 1;
	req.headers = headers;
	resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&req);
	if (resp) {
		if (resp->resultCode == 200 && resp->dataLength > 0) {
			//int i;
			int resUpdate = 0;
            TCHAR *tXml;
            char *tmp;
            HXML nodeDoc, n;
            
            tXml = mir_a2t(resp->pData);
            nodeDoc = xun.parseString(tXml, 0, _T("miranda"));
            if (nodeDoc) {
                int rdStableValid = 0, rdBetaValid = 0, rdAlphaValid = 0;
                // stable release
                UpdateNotifyReleaseData rdStable;
                ZeroMemory(&rdStable, sizeof(rdStable));
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasestable/versionpublic"), 0)) != NULL && xun.getText(n)) {
                    rdStable.szVersionPublic = mir_t2a(xun.getText(n));
                }
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasestable/versionreal"), 0)) != NULL && xun.getText(n)) {
                    rdStable.szVersion = mir_t2a(xun.getText(n));
                    if (rdStable.szVersion)
                        rdStable.dwVersion = UpdateNotifyMakeVersion(rdStable.szVersion);
                }
                #ifdef _WIN64
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasestable/downloadx64exe"), 0)) != NULL && xun.getText(n)) {
                    rdStable.szDownload = mir_t2a(xun.getText(n));
                }
                #elif defined(_UNICODE)
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasestable/downloadunicodeexe"), 0)) != NULL && xun.getText(n)) {
                    rdStable.szDownload = mir_t2a(xun.getText(n));
                }
                #else
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasestable/downloadansiexe"), 0)) != NULL && xun.getText(n)) {
                    rdStable.szDownload = mir_t2a(xun.getText(n));
                }
                #endif
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasestable/notesurl"), 0)) != NULL && xun.getText(n)) {
                    rdStable.szNotes = mir_t2a(xun.getText(n));
                }
                rdStableValid = UpdateNotifyReleaseDataValid(&rdStable);
                
                // beta release
                UpdateNotifyReleaseData rdBeta;
                ZeroMemory(&rdBeta, sizeof(rdBeta));
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasebeta/versionpublic"), 0)) != NULL && xun.getText(n)) {
                    rdBeta.szVersionPublic = mir_t2a(xun.getText(n));
                }
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasebeta/versionreal"), 0)) != NULL && xun.getText(n)) {
                    rdBeta.szVersion = mir_t2a(xun.getText(n));
                    if (rdBeta.szVersion)
                        rdBeta.dwVersion = UpdateNotifyMakeVersion(rdBeta.szVersion);
                }
                #ifdef _WIN64
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasebeta/downloadx64zip"), 0)) != NULL && xun.getText(n)) {
                    rdBeta.szDownload = mir_t2a(xun.getText(n));
                }
                #elif defined(_UNICODE)
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasebeta/downloadunicodeexe"), 0)) != NULL && xun.getText(n)) {
                    rdBeta.szDownload = mir_t2a(xun.getText(n));
                }
                #else
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasebeta/downloadansiexe"), 0)) != NULL && xun.getText(n)) {
                    rdBeta.szDownload = mir_t2a(xun.getText(n));
                }
                #endif
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasebeta/notesurl"), 0)) != NULL && xun.getText(n)) {
                    rdBeta.szNotes = mir_t2a(xun.getText(n));
                }
                rdBetaValid = UpdateNotifyReleaseDataValid(&rdBeta);
                
                // alpha release
                UpdateNotifyReleaseData rdAlpha;
                ZeroMemory(&rdAlpha, sizeof(rdAlpha));
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasealpha/versionpublic"), 0)) != NULL && xun.getText(n)) {
                    rdAlpha.szVersionPublic = mir_t2a(xun.getText(n));
                }
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasealpha/versionreal"), 0)) != NULL && xun.getText(n)) {
                    rdAlpha.szVersion = mir_t2a(xun.getText(n));
                    if (rdAlpha.szVersion)
                        rdAlpha.dwVersion = UpdateNotifyMakeVersion(rdAlpha.szVersion);
                }
                #ifdef _WIN64
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasealpha/downloadx64zip"), 0)) != NULL && xun.getText(n)) {
                    rdAlpha.szDownload = mir_t2a(xun.getText(n));
                }
                #elif defined(_UNICODE)
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasealpha/downloadunicodezip"), 0)) != NULL && xun.getText(n)) {
                    rdAlpha.szDownload = mir_t2a(xun.getText(n));
                }
                #else
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasealpha/downloadansizip"), 0)) != NULL && xun.getText(n)) {
                    rdAlpha.szDownload = mir_t2a(xun.getText(n));
                }
                #endif
                if ((n = xun.getChildByPath(nodeDoc, _T("releases/releasealpha/notesurl"), 0)) != NULL && xun.getText(n)) {
                    rdAlpha.szNotes = mir_t2a(xun.getText(n));
                }
                rdAlphaValid = UpdateNotifyReleaseDataValid(&rdAlpha);
                
                if (isBetaCheck) {
                    if (!rdBetaValid&&rdStableValid) {
                        UpdateNotifyReleaseLogUpdate(&rdStable);
                        if (UpdateNotifyIsNewer(dwVersion, rdStable.dwVersion)) 
                            resUpdate = 1;
                        UpdateNotifyReleaseCopyData(&rdStable, und);
                    }
                    else if (rdBetaValid&&rdStableValid&&UpdateNotifyIsNewer(rdBeta.dwVersion, rdStable.dwVersion)) {
                        UpdateNotifyReleaseLogUpdate(&rdStable);
                        if (UpdateNotifyIsNewer(dwVersion, UpdateNotifyMakeVersion(rdStable.szVersion))) 
                            resUpdate = 1;
                        UpdateNotifyReleaseCopyData(&rdStable, und);
                    }
                    else if (rdBetaValid) {
                        UpdateNotifyReleaseLogUpdate(&rdBeta);
                        if (UpdateNotifyIsNewer(dwVersion, rdBeta.dwVersion)) 
                            resUpdate = 1;
                        UpdateNotifyReleaseCopyData(&rdBeta, und);
                    }
                }
                else if (isAlphaCheck) {                 
                    if (!rdAlphaValid&&rdStableValid) {
                        if (UpdateNotifyIsNewer(rdStable.dwVersion, rdAlpha.dwVersion)) {
                            UpdateNotifyReleaseLogUpdate(&rdAlpha);
                            if (UpdateNotifyIsNewer(dwVersion, rdAlpha.dwVersion)) 
                                resUpdate = 1;
                            UpdateNotifyReleaseCopyData(&rdAlpha, und);
                        }
                        else {
                            UpdateNotifyReleaseLogUpdate(&rdStable);
                            if (UpdateNotifyIsNewer(dwVersion, rdStable.dwVersion)) 
                                resUpdate = 1;
                            UpdateNotifyReleaseCopyData(&rdStable, und);
                        } 
                    }
                    else if (rdAlphaValid&&rdStableValid&&UpdateNotifyIsNewer(rdAlpha.dwVersion, rdStable.dwVersion)) {
                        UpdateNotifyReleaseLogUpdate(&rdStable);
                        if (UpdateNotifyIsNewer(dwVersion, rdStable.dwVersion)) 
                            resUpdate = 1;
                        UpdateNotifyReleaseCopyData(&rdStable, und);
                    }
                    else if (!rdAlphaValid&&rdBetaValid&&rdStableValid) {
                        if (UpdateNotifyIsNewer(rdStable.dwVersion, rdBeta.dwVersion)) {
                            UpdateNotifyReleaseLogUpdate(&rdBeta);
                            if (UpdateNotifyIsNewer(dwVersion, rdBeta.dwVersion)) 
                                resUpdate = 1;
                            UpdateNotifyReleaseCopyData(&rdBeta, und);
                        }
                        else {
                            UpdateNotifyReleaseLogUpdate(&rdStable);
                            if (UpdateNotifyIsNewer(dwVersion, rdStable.dwVersion)) 
                                resUpdate = 1;
                            UpdateNotifyReleaseCopyData(&rdStable, und);
                        } 
                    }
                    else if (rdAlphaValid) {
                        UpdateNotifyReleaseLogUpdate(&rdAlpha);
                        if (UpdateNotifyIsNewer(dwVersion, rdAlpha.dwVersion)) 
                            resUpdate = 1;
                        UpdateNotifyReleaseCopyData(&rdAlpha, und);
                    }
                }
                else {
                    if (rdStableValid) {
                        UpdateNotifyReleaseLogUpdate(&rdStable);
                        if (UpdateNotifyIsNewer(dwVersion, rdStable.dwVersion)) 
                            resUpdate = 1;
                        UpdateNotifyReleaseCopyData(&rdStable, und);
                    }
                }
              
                UpdateNotifyFreeReleaseData(&rdStable);
                UpdateNotifyFreeReleaseData(&rdBeta);
                UpdateNotifyFreeReleaseData(&rdAlpha);
                // settings
                if ((n = xun.getChildByPath(nodeDoc, _T("settings/ping"), 0)) != NULL && xun.getText(n)) {
                    tmp = mir_t2a(xun.getText(n));
                    if (tmp) {
                        int pingval = atoi(tmp);
                        if ((pingval*60*60)>UN_MINCHECKTIME) {
                            Netlib_Logf(hNetlibUser, "Next update check in %d hours", pingval);
                            DBWriteContactSettingDword(NULL, UN_MOD, UN_SERVERPERIOD, pingval*60*60);
                        }
                        mir_free(tmp);
                    }
                }
                if ((n = xun.getChildByPath(nodeDoc, _T("settings/updateurl"), 0)) != NULL && xun.getText(n)) {
                    tmp = mir_t2a(xun.getText(n));
                    if (tmp) {
                        Netlib_Logf(hNetlibUser, "Update URL has changed (%s)", tmp);
                        DBWriteContactSettingString(NULL, UN_MOD, UN_CUSTOMXMLURL, tmp);
                        mir_free(tmp);
                    }
                }
                if (resUpdate&&und->version&&und->versionReal&&und->notesUrl&&und->downloadUrl) {
                    Netlib_Logf(hNetlibUser, "A new version of Miranda IM is available: %s", und->version);
                    und->isNew = 1;
                }
				xun.destroyNode(nodeDoc);
            }
            mir_free(tXml);
		}
		else Netlib_Logf(hNetlibUser, "Invalid response code from HTTP request");
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	}
	else Netlib_Logf(hNetlibUser, "No response from HTTP request");
	return und->isNew;
}

static void UpdateNotifyPerform(void *)
{
	UpdateNotifyData und;
	DBVARIANT dbv;
	
	ZeroMemory(&und, sizeof(und));
	UpdateNotifyMakeRequest(&und);
	if (und.isNew) {
		int notify = 1;
		
		if (!DBGetContactSettingString(NULL, UN_MOD, UN_CURRENTVERSION, &dbv)) {
			if (!strcmp(dbv.pszVal, und.versionReal)) { // already notified of this version
			
				DWORD dwNotifyLast = DBGetContactSettingDword(NULL, UN_MOD, UN_CURRENTVERSIONFND, 0);

				if (dwNotifyLast>und.reqTime) { // fix last check date if time has changed
					DBWriteContactSettingDword(NULL, UN_MOD, UN_CURRENTVERSIONFND, und.reqTime);
					notify = 0;
				}
				else if (und.reqTime-dwNotifyLast<UN_REPEATNOTIFYDLY) {
					notify = 0;
				}
				DBFreeVariant(&dbv);
			}
		}
		if (notify) {
			DBWriteContactSettingString(NULL, UN_MOD, UN_CURRENTVERSION, und.versionReal);
			DBWriteContactSettingDword(NULL, UN_MOD, UN_CURRENTVERSIONFND, und.reqTime);
			DialogBoxParam(hMirandaInst, MAKEINTRESOURCE(IDD_UPDATE_NOTIFY), 0, UpdateNotifyProc,(LPARAM)&und);
			hwndUpdateDlg = 0;
		}
	}
	dwUpdateThreadID = 0;
}

static INT_PTR CALLBACK UpdateNotifyProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetIcon_IcoLib(hwndDlg, SKINICON_OTHER_MIRANDA);
		{
			UpdateNotifyData *und = (UpdateNotifyData*)lParam;
			char szVersion[128], szVersionTmp[128], *p;
			TCHAR szTmp[128];
			
			if (und->isManual)
				hwndManualUpdateDlg = hwndDlg;
			else hwndUpdateDlg = hwndDlg;
			if (und->isNew) {
				TCHAR* ptszVer = mir_a2t( und->version );
				mir_sntprintf(szTmp, SIZEOF(szTmp), TranslateT("Miranda IM %s Now Available"), ptszVer);
				mir_free(ptszVer);
				ShowWindow(GetDlgItem(hwndDlg, IDC_UPDATE), SW_HIDE);
			}
			else {
				mir_sntprintf(szTmp, SIZEOF(szTmp), TranslateT("No Update Available"));
				SetDlgItemText(hwndDlg, IDC_MESSAGE, TranslateT("You are running the latest version of Miranda IM.  No update is available at this time."));
				EnableWindow(GetDlgItem(hwndDlg, IDC_DOWNLOAD), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_VERSION), SW_HIDE);
			}
			SetWindowText(hwndDlg, szTmp);
			CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(szVersion), (LPARAM)szVersion);
			p = strstr(szVersion, "x64 Unicode");
			if (p)
				*p = '\0';
			p = strstr(szVersion, " Unicode");
			if (p)
				*p = '\0';
			SetDlgItemTextA(hwndDlg, IDC_CURRENTVERSION, szVersion);
			mir_snprintf(szVersionTmp, SIZEOF(szVersionTmp), "%s", und->version?und->version:szVersion);
			SetDlgItemTextA(hwndDlg, und->isNew?IDC_VERSION:IDC_UPDATE, szVersionTmp);
			if (und->isNew) {
				HFONT hFont;
				LOGFONT lf;

				hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_GETFONT, 0, 0);
				GetObject(hFont, sizeof(lf), &lf);
				lf.lfWeight = FW_BOLD;
				hFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_SETFONT, (WPARAM)hFont, 0);
				hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_NEWVERSIONLABEL, WM_GETFONT, 0, 0);
				GetObject(hFont, sizeof(lf), &lf);
				lf.lfWeight = FW_BOLD;
				hFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_NEWVERSIONLABEL, WM_SETFONT, (WPARAM)hFont, 0);
			}
			SetFocus(GetDlgItem(hwndDlg, IDOK));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_VERSION:
			{
				UpdateNotifyData *und = (UpdateNotifyData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (und&&und->notesUrl)
					CallService(MS_UTILS_OPENURL, 1, (LPARAM)und->notesUrl);
				break;
			}
		case IDC_DOWNLOAD:
			{
				UpdateNotifyData *und = (UpdateNotifyData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (und&&und->downloadUrl) {
					CallService(MS_UTILS_OPENURL, 1, (LPARAM)und->downloadUrl);
					DestroyWindow(hwndDlg);
				}
				break;
			}
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib( hwndDlg );
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK UpdateNotifyOptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_ENABLEUPDATES, DBGetContactSettingByte(NULL, UN_MOD, UN_ENABLE, UN_ENABLE_DEF) ? BST_CHECKED : BST_UNCHECKED);
		switch (DBGetContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_STABLE)) {
				case UN_NOTIFYTYPE_BETA: CheckDlgButton(hwndDlg, IDC_ENABLEBETA, BST_CHECKED); break;
				case UN_NOTIFYTYPE_ALPHA: CheckDlgButton(hwndDlg, IDC_ENABLEALPHA, BST_CHECKED); break;
				default: CheckDlgButton(hwndDlg, IDC_ENABLESTABLE, BST_CHECKED); break;
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
	case IDC_ENABLEUPDATES:
	case IDC_ENABLEALPHA:
    case IDC_ENABLEBETA:
    case IDC_ENABLESTABLE:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if (hdr&&hdr->code==PSN_APPLY) {
				DBWriteContactSettingByte(NULL, UN_MOD, UN_ENABLE, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ENABLEUPDATES)));
                if (IsDlgButtonChecked(hwndDlg, IDC_ENABLESTABLE))
                    DBWriteContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_STABLE);
                if (IsDlgButtonChecked(hwndDlg, IDC_ENABLEBETA))
                    DBWriteContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_BETA);
                if (IsDlgButtonChecked(hwndDlg, IDC_ENABLEALPHA))
                    DBWriteContactSettingByte(NULL, UN_MOD, UN_NOTIFYTYPE, UN_NOTIFYTYPE_ALPHA);
			}
			break;
		}
	}
	return FALSE;
}
