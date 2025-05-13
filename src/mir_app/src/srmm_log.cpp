/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team,
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

/////////////////////////////////////////////////////////////////////////////////////////
// SRMM log container

#include "stdafx.h"
#include "chat.h"

static OBJLIST<SrmmLogWindowClass> g_arLogClasses(1, PtrKeySortT);

static CMOption<char *> g_logger(SRMM_MODULE, "Logger", "built-in");
static CMOption<char *> g_loggerGC(SRMM_MODULE, "LoggerGC", "built-in");

void CheckLogOptions()
{
	if (db_get_b(0, SRMM_MODULE, "EnableCustomLogs")) {
		g_loggerGC = (char*)g_logger;
		db_unset(0, SRMM_MODULE, "EnableCustomLogs");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool sttEnableCustomLogs(CMsgDialog *pDlg)
{
	// always enable custom log viewers for private chats
	if (!pDlg)
		return true;

	// check if custom viewers are forbidden for this particular account
	auto *szProto = Proto_GetBaseAccountName(pDlg->m_hContact);
	if (szProto) {
		// hidden setting !!!!!!!!
		CMStringA szProtoList(db_get_sm(0, SRMM_MODULE, "DisableCustomLogsForProto"));

		int iStart = 0;
		while (true) {
			auto forbiddenProto = szProtoList.Tokenize(",; ", iStart);
			if (forbiddenProto.IsEmpty())
				break;

			if (forbiddenProto == szProto)
				return false;
		}
	}
	
	// ok-ok, use that custom viewer
	return true;
}

MIR_APP_DLL(SrmmLogWindowClass *) Srmm_GetWindowClass(CMsgDialog *pDlg)
{
	if (sttEnableCustomLogs(pDlg)) {
		CMStringA szViewerName;
		if (pDlg != nullptr)
			szViewerName = db_get_sm(pDlg->m_hContact, SRMSGMOD, pDlg->isChat() ? "LoggerGC" : "Logger");
		if (szViewerName.IsEmpty())
			szViewerName = pDlg->isChat() ? g_loggerGC : g_logger;

		for (auto &it : g_arLogClasses)
			if (szViewerName == it->szShortName)
				return it;
	}

	for (auto &it : g_arLogClasses)
		if (!mir_strcmp(it->szShortName, "built-in"))
			return it;

	return nullptr; // shall never happen
}

MIR_APP_DLL(bool) Srmm_IsCustomLogUsed(bool forGroupChats)
{
	if (forGroupChats)
		return mir_strcmp(g_loggerGC, "built-in") != 0;

	return mir_strcmp(g_logger, "built-in") != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options dialog

static void CALLBACK OpenOptions(void *)
{
	g_plugin.openOptions(L"Message sessions", L"Log viewer");
}

static void ReloadOptions(void *hWnd)
{
	while (IsWindow((HWND)hWnd))
		Sleep(50);

	CallFunctionAsync(OpenOptions, nullptr);
}

static class CSrmmLogOptionsDlg *pDialog = nullptr;

class CSrmmLogOptionsDlg : public CDlgBase
{
	CCtrlCombo cmbLogger, cmbLoggerGC;

	void PopulateCombo(const char *pszClass, CCtrlCombo &ctrl)
	{
		ctrl.ResetContent();

		for (auto &it : g_arLogClasses) {
			int idx = ctrl.AddString(TranslateW_LP(it->wszScreenName, it->pPlugin), LPARAM(it));
			if (!mir_strcmp(it->szShortName, pszClass))
				ctrl.SetCurSel(idx);
		}
	}

public:
	CSrmmLogOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_SRMMLOG),
		cmbLogger(this, IDC_LOGGER),
		cmbLoggerGC(this, IDC_LOGGER_GC)
	{
	}

	bool OnInitDialog() override
	{
		pDialog = this;
		PopulateCombo(g_logger, cmbLogger);
		PopulateCombo(g_loggerGC, cmbLoggerGC);
		return true;
	}

	bool OnApply() override
	{
		if (auto *pLogger = (SrmmLogWindowClass *)cmbLogger.GetCurData())
			g_logger = pLogger->szShortName;
		else
			return false;

		if (auto *pLogger = (SrmmLogWindowClass *)cmbLoggerGC.GetCurData())
			g_loggerGC = pLogger->szShortName;
		else
			return false;

		if (!m_bExiting) {
			PostMessage(m_hwndParent, WM_CLOSE, 1, 0);
			mir_forkthread(ReloadOptions, m_hwndParent);
		}
		return true;
	}

	void OnDestroy() override
	{
		pDialog = nullptr;
	}

	void Rebuild()
	{
		OnInitDialog();
	}

	void onChange_List(CCtrlListBox *)
	{
		NotifyChange();
	}
};

void SrmmLogOptionsInit(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Log viewer");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CSrmmLogOptionsDlg();
	g_plugin.addOptions(wParam, &odp);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HANDLE) RegisterSrmmLog(CMPlugin *pPlugin, const char *pszShortName, const wchar_t *pwszScreenName, pfnSrmmLogCreator fnBuilder)
{
	if (!pszShortName || !pwszScreenName || !fnBuilder)
		return nullptr;

	auto *p = new SrmmLogWindowClass(pPlugin, pszShortName, pwszScreenName, fnBuilder);
	g_arLogClasses.insert(p);

	if (pDialog)
		pDialog->Rebuild();
	return p;
}

MIR_APP_DLL(void) UnregisterSrmmLog(HANDLE pLogger)
{
	g_arLogClasses.remove((SrmmLogWindowClass *)pLogger);

	if (pDialog)
		pDialog->Rebuild();
}
