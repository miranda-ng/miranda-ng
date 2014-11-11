#include "StdAfx.h"
#include "sametime.h"


LRESULT CALLBACK PopupWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		PUDeletePopup(hWnd);
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		PopupData* puData = (PopupData*)PUGetPluginData(hWnd);
		if (puData != NULL && puData != (PopupData*)CALLSERVICE_NOTFOUND) {
			mir_free(puData->title);
			mir_free(puData->text);
			mir_free(puData);
		}
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


void CSametimeProto::RegisterPopups()
{
	TCHAR szDescr[256];
	char szName[256];

	debugLog(_T("CSametimeProto::RegisterPopups()"));

	POPUPCLASS puc = { sizeof(puc) };
	puc.PluginWindowProc = PopupWindowProc;
	puc.flags = PCF_TCHAR;
	puc.ptszDescription = szDescr;
	puc.pszName = szName;

	mir_snprintf(szName, SIZEOF(szName), "%s_%s", m_szModuleName, "Notify");
	mir_sntprintf(szDescr, SIZEOF(szDescr), _T("%s/%s"), m_tszUserName, TranslateT("Notification"));
	puc.hIcon = CopyIcon(LoadIconEx("notify", FALSE));
	ReleaseIconEx("notify", FALSE);
	puc.iSeconds = 8;
	puc.colorBack = GetSysColor(COLOR_BTNFACE);
	puc.colorText = GetSysColor(COLOR_WINDOWTEXT);
	hPopupNotify = Popup_RegisterClass(&puc);

	mir_snprintf(szName, SIZEOF(szName), "%s_%s", m_szModuleName, "Error");
	mir_sntprintf(szDescr, SIZEOF(szDescr), _T("%s/%s"), m_tszUserName, TranslateT("Error"));
	puc.hIcon = CopyIcon(LoadIconEx("error", FALSE));
	ReleaseIconEx("error", FALSE);
	puc.iSeconds = 10;
	puc.colorBack = GetSysColor(COLOR_BTNFACE);
	puc.colorText = GetSysColor(COLOR_WINDOWTEXT);
	hPopupError = Popup_RegisterClass(&puc);
}


void CSametimeProto::UnregisterPopups()
{
	debugLog(_T("CSametimeProto::RegisterPopups()"));
	Popup_UnregisterClass(hPopupError);
	Popup_UnregisterClass(hPopupNotify);
}


void CALLBACK sttMainThreadCallback(PVOID dwParam)
{

	PopupData* puData = (PopupData*)dwParam;
	CSametimeProto* proto = puData->proto;

	ErrorDisplay disp = proto->options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if (disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP;
	if (disp == ED_POP && !ServiceExists(MS_POPUP_ADDPOPUPCLASS)) disp = ED_BAL;
	if (disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

	if (disp == ED_POP) {
		POPUPDATACLASS ppd = { sizeof(ppd) };
		char szName[256];
		ppd.ptszTitle = puData->title;
		ppd.ptszText = puData->text;
		if (puData->flag == SAMETIME_POPUP_ERROR)
			mir_snprintf(szName, SIZEOF(szName), "%s_%s", proto->m_szModuleName, "Error");
		else
			mir_snprintf(szName, SIZEOF(szName), "%s_%s", proto->m_szModuleName, "Notify");
		ppd.pszClassName = szName;
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
	}
	else if (disp == ED_BAL) {
		MIRANDASYSTRAYNOTIFY sn = { sizeof(sn) };
		sn.szProto = proto->m_szModuleName;
		sn.tszInfoTitle = puData->title;
		sn.tszInfo = puData->text;
		sn.dwInfoFlags = NIIF_INTERN_UNICODE;
		if (puData->flag == SAMETIME_POPUP_ERROR) {
			sn.dwInfoFlags = sn.dwInfoFlags | NIIF_WARNING;
			sn.uTimeout = 1000 * 10;
		}
		else {
			sn.dwInfoFlags = sn.dwInfoFlags | NIIF_INFO;
			sn.uTimeout = 1000 * 8;
		}
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);
	}
	else { //disp == ED_MB
		if (puData->flag == SAMETIME_POPUP_ERROR)
			MessageBox(NULL, puData->text, puData->title, MB_OK | MB_ICONWARNING);
		else
			MessageBox(NULL, puData->text, puData->title, MB_OK | MB_ICONINFORMATION);
	}

	if (disp != ED_POP) {
		mir_free(puData->title);
		mir_free(puData->text);
		mir_free(puData);
	}
}

void CSametimeProto::showPopup(const TCHAR* msg, SametimePopupEnum flag)
{
	if (Miranda_Terminated()) return;

	PopupData *puData = (PopupData*)mir_calloc(sizeof(PopupData));
	puData->flag = flag;
	puData->title = mir_tstrdup(m_tszUserName);
	puData->text = mir_tstrdup(msg);
	puData->proto = this;

	CallFunctionAsync(sttMainThreadCallback, puData);
}

void CSametimeProto::showPopup(guint32 code)
{
	struct mwReturnCodeDesc *rcDesc = mwGetReturnCodeDesc(code);

	SametimePopupEnum flag = (rcDesc->type == mwReturnCodeError ? SAMETIME_POPUP_ERROR : SAMETIME_POPUP_INFO);
	TCHAR buff[512];
	mir_sntprintf(buff, SIZEOF(buff), TranslateT("%s\n\nSametime error %S\n%s"), TranslateTS(_A2T(rcDesc->name)), rcDesc->codeString, TranslateTS(_A2T(rcDesc->description)));

	showPopup(buff, flag);
	debugLog(buff);

	g_free(rcDesc->codeString);
	g_free(rcDesc->name);
	g_free(rcDesc->description);
	g_free(rcDesc);
}

void LogFromGLib(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
{
	CSametimeProto* proto = (CSametimeProto*)user_data;
	proto->debugLog(_A2T(message));
}

void CSametimeProto::RegisterGLibLogger()
{
	debugLog(_T("CSametimeProto::RegisterGLibLogger"));
	gLogHandler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, LogFromGLib, this);
}

void CSametimeProto::UnRegisterGLibLogger()
{
	debugLog(_T("CSametimeProto::UnRegisterGLibLogger"));
	if (gLogHandler) g_log_remove_handler(G_LOG_DOMAIN, gLogHandler);
}
