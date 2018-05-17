#include "stdafx.h"

CMLuaOptions::CMLuaOptions(CMLua *mLua)
	: CPluginDlgBase(g_plugin.getInst(), IDD_OPTIONS, MODULE),
	m_mLua(mLua), isScriptListInit(false),
	m_popupOnError(this, IDC_POPUPONERROR),
	m_popupOnObsolete(this, IDC_POPUPONOBSOLETE),
	m_scripts(this, IDC_SCRIPTS), m_reload(this, IDC_RELOAD)
{
	CreateLink(m_popupOnError, "PopupOnError", DBVT_BYTE, 1);
	CreateLink(m_popupOnObsolete, "PopupOnObsolete", DBVT_BYTE, 1);

	m_scripts.OnClick = Callback(this, &CMLuaOptions::OnScriptListClick);
	m_reload.OnClick = Callback(this, &CMLuaOptions::OnReload);
}

void CMLuaOptions::LoadScripts()
{
	for (auto &script : m_mLua->Scripts) {
		wchar_t *fileName = NEWWSTR_ALLOCA(script->GetFileName());
		int iIcon = script->GetStatus() - 1;
		int iItem = m_scripts.AddItem(fileName, iIcon, (LPARAM)script);
		m_scripts.SetCheckState(iItem, script->IsEnabled());
		m_scripts.SetItem(iItem, 1, TranslateT("Open"), 2);
		m_scripts.SetItem(iItem, 2, TranslateT("Reload"), 3);
	}
}

void CMLuaOptions::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_scripts.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);

	HIMAGELIST hImageList = m_scripts.CreateImageList(LVSIL_SMALL);
	ImageList_AddIcon(hImageList, GetIcon(IDI_LOADED));
	ImageList_AddIcon(hImageList, GetIcon(IDI_FAILED));
	ImageList_AddIcon(hImageList, GetIcon(IDI_OPEN));
	ImageList_AddIcon(hImageList, GetIcon(IDI_RELOAD));

	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));
	
	wchar_t relativeScriptDir[MAX_PATH];
	PathToRelativeW(scriptDir, relativeScriptDir, nullptr);

	wchar_t header[MAX_PATH + 100];
	mir_snwprintf(header, L"%s (%s)", TranslateT("Common scripts"), relativeScriptDir);

	m_scripts.AddColumn(0, L"Script", 380);
	m_scripts.AddColumn(1, nullptr, 32 - GetSystemMetrics(SM_CXVSCROLL));
	m_scripts.AddColumn(2, nullptr, 32 - GetSystemMetrics(SM_CXVSCROLL));

	LoadScripts();

	isScriptListInit = true;
}

void CMLuaOptions::OnApply()
{
	int count = m_scripts.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++) {
		CMLuaScript *script = (CMLuaScript*)m_scripts.GetItemData(iItem);
		if (!m_scripts.GetCheckState(iItem))
			script->Disable();
		else
			script->Enable();
	}
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
		lvi.iImage = script->GetStatus() - 1;
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
	m_mLua->Reload();
	LoadScripts();
	isScriptListInit = true;
}
