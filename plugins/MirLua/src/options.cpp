#include "stdafx.h"

CMLuaOptions::CMLuaOptions()
	: CDlgBase(g_plugin, IDD_OPTIONS),
	isScriptListInit(false),
	m_popupOnError(this, IDC_POPUPONERROR),
	m_popupOnObsolete(this, IDC_POPUPONOBSOLETE),
	m_scripts(this, IDC_SCRIPTS),
	m_reload(this, IDC_RELOAD)
{
	CreateLink(m_popupOnError, "PopupOnError", DBVT_BYTE, 1);
	CreateLink(m_popupOnObsolete, "PopupOnObsolete", DBVT_BYTE, 1);

	m_scripts.OnClick = Callback(this, &CMLuaOptions::OnScriptListClick);
	m_reload.OnClick = Callback(this, &CMLuaOptions::OnReload);
}

static int ScriptStatusToIcon(ScriptStatus status)
{
	switch (status) {
	case ScriptStatus::None:
		return -1;
	case ScriptStatus::Loaded:
		return 0;
	case ScriptStatus::Failed:
		return 1;
	}
}

void CMLuaOptions::LoadScripts()
{
	for (auto &script : g_plugin.Scripts.rev_iter()) {
		int iIcon = ScriptStatusToIcon(script->GetStatus());
		int iItem = m_scripts.AddItem(script->GetName(), iIcon, (LPARAM)script);
		m_scripts.SetCheckState(iItem, script->IsEnabled());
		m_scripts.SetItem(iItem, 1, TranslateT("Open"), 2);
		m_scripts.SetItem(iItem, 2, TranslateT("Reload"), 3);
		if (!script->IsBinary())
			m_scripts.SetItem(iItem, 3, TranslateT("Compile"), 4);
	}
}

bool CMLuaOptions::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_scripts.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);

	HIMAGELIST hImageList = m_scripts.CreateImageList(LVSIL_SMALL);
	ImageList_AddIcon(hImageList, GetIcon(IDI_LOADED));
	ImageList_AddIcon(hImageList, GetIcon(IDI_FAILED));
	ImageList_AddIcon(hImageList, GetIcon(IDI_OPEN));
	ImageList_AddIcon(hImageList, GetIcon(IDI_RELOAD));
	ImageList_AddIcon(hImageList, GetIcon(IDI_COMPILE));

	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));

	wchar_t relativeScriptDir[MAX_PATH];
	PathToRelativeW(scriptDir, relativeScriptDir, nullptr);

	wchar_t header[MAX_PATH + 100];
	mir_snwprintf(header, L"%s (%s)", TranslateT("Common scripts"), relativeScriptDir);

	m_scripts.AddColumn(0, L"Script", 380);
	m_scripts.AddColumn(1, nullptr, 32 - GetSystemMetrics(SM_CXVSCROLL));
	m_scripts.AddColumn(2, nullptr, 32 - GetSystemMetrics(SM_CXVSCROLL));
	m_scripts.AddColumn(3, nullptr, 32 - GetSystemMetrics(SM_CXVSCROLL));

	LoadScripts();

	isScriptListInit = true;
	return true;
}

bool CMLuaOptions::OnApply()
{
	int count = m_scripts.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++) {
		CMLuaScript *script = (CMLuaScript*)m_scripts.GetItemData(iItem);
		if (!m_scripts.GetCheckState(iItem))
			script->Disable();
		else
			script->Enable();
	}
	return true;
}

INT_PTR CMLuaOptions::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr = (LPNMHDR)lParam;
		if (lpnmHdr->idFrom == (UINT_PTR)m_scripts.GetCtrlId() && lpnmHdr->code == LVN_ITEMCHANGED) {
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uChanged & LVIF_STATE && pnmv->uNewState & LVIS_STATEIMAGEMASK) {
				if (isScriptListInit)
					NotifyChange();
			}
		}
	}
	break;
	}
	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CMLuaOptions::OnScriptListClick(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = {};
	lvi.iItem = evt->nmlvia->iItem;
	if (lvi.iItem == -1)
		return;

	lvi.pszText = (LPTSTR)mir_calloc(MAX_PATH * sizeof(wchar_t));
	lvi.cchTextMax = MAX_PATH;
	lvi.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_PARAM;
	m_scripts.GetItem(&lvi);
	lvi.iSubItem = evt->nmlvia->iSubItem;

	CMLuaScript *script = (CMLuaScript*)lvi.lParam;

	switch (lvi.iSubItem) {
	case 1:
		ShellExecute(m_hwnd, L"open", script->GetFilePath(), nullptr, nullptr, SW_SHOWNORMAL);
		break;

	case 2:
		script->Reload();
		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = 0;
		lvi.iImage = ScriptStatusToIcon(script->GetStatus());
		m_scripts.SetItem(&lvi);
		m_scripts.Update(lvi.iItem);
		break;

	case 3:
		if (script->IsBinary())
			break;
		script->Compile();
		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = 0;
		lvi.iImage = ScriptStatusToIcon(script->GetStatus());
		m_scripts.SetItem(&lvi);
		m_scripts.Update(lvi.iItem);
		break;
	}

	mir_free(lvi.pszText);
}

void CMLuaOptions::OnReload(CCtrlBase*)
{
	isScriptListInit = false;
	m_scripts.DeleteAllItems();
	g_plugin.Reload();
	LoadScripts();
	isScriptListInit = true;
}

/***********************************************/

int OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.hInstance = g_plugin.getInst();
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = L"Lua";
	odp.szTab.w = LPGENW("Scripts");
	odp.pDialog = new CMLuaOptions();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}