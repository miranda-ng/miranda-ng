#include "stdafx.h"

CCtrlScriptList::CCtrlScriptList(CDlgBase* dlg, int ctrlId)
	: CCtrlListView(dlg, ctrlId)
{
}

BOOL CCtrlScriptList::OnNotify(int idCtrl, NMHDR *pnmh)
{
	if (pnmh->code == NM_CLICK)
	{
		TEventInfo evt = { this, pnmh };
		OnClick(&evt);
		return TRUE;
	}
	return CCtrlListView::OnNotify(idCtrl, pnmh);
}

/****************************************/

CMLuaOptions::CMLuaOptions(int idDialog)
	: CPluginDlgBase(g_hInstance, idDialog, MODULE),
	m_popupOnError(this, IDC_POPUPONERROR),
	m_popupOnObsolete(this, IDC_POPUPONOBSOLETE),
	isScriptListInit(false), m_scripts(this, IDC_SCRIPTS),
	m_reload(this, IDC_RELOAD)
{
	CreateLink(m_popupOnError, "PopupOnError", DBVT_BYTE, 1);
	CreateLink(m_popupOnObsolete, "PopupOnObsolete", DBVT_BYTE, 1);

	m_scripts.OnClick = Callback(this, &CMLuaOptions::OnScriptListClick);
	m_reload.OnClick = Callback(this, &CMLuaOptions::OnReload);
}

void CMLuaOptions::LoadScripts()
{
	for (int i = 0; i < g_mLua->Scripts.getCount(); i++)
	{
		CMLuaScript *script = g_mLua->Scripts[i];
		wchar_t *fileName = NEWWSTR_ALLOCA(script->GetFileName());
		int iIcon = script->GetStatus() - 1;
		int iItem = m_scripts.AddItem(fileName, iIcon, (LPARAM)script);
		if (db_get_b(NULL, MODULE, _T2A(fileName), 1))
			m_scripts.SetCheckState(iItem, TRUE);
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

	wchar_t scriptDir[MAX_PATH], relativeScriptDir[MAX_PATH], header[MAX_PATH + 100];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));
	PathToRelativeW(scriptDir, relativeScriptDir, NULL);
	mir_snwprintf(header, L"%s (%s)", TranslateT("Common scripts"), relativeScriptDir);

	m_scripts.AddColumn(0, L"Script", 380);
	m_scripts.AddColumn(1, NULL, 32 - GetSystemMetrics(SM_CXVSCROLL));
	m_scripts.AddColumn(2, NULL, 32 - GetSystemMetrics(SM_CXVSCROLL));

	LoadScripts();

	isScriptListInit = true;
}

void CMLuaOptions::OnApply()
{
	int count = m_scripts.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++)
	{
		wchar_t fileName[MAX_PATH];
		m_scripts.GetItemText(iItem, 0, fileName, _countof(fileName));
		if (!m_scripts.GetCheckState(iItem))
			db_set_b(NULL, MODULE, _T2A(fileName), 0);
		else
			db_unset(NULL, MODULE, _T2A(fileName));
	}
}

INT_PTR CMLuaOptions::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr = (LPNMHDR)lParam;
		if (lpnmHdr->idFrom == (UINT_PTR)m_scripts.GetCtrlId() && lpnmHdr->code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uChanged & LVIF_STATE && pnmv->uNewState & LVIS_STATEIMAGEMASK)
			{
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
	LVITEM lvi = { 0 };
	lvi.iItem = evt->nmlvia->iItem;
	if (lvi.iItem == -1) return;
	lvi.pszText = (LPTSTR)mir_calloc(MAX_PATH * sizeof(wchar_t));
	lvi.cchTextMax = MAX_PATH;
	lvi.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_PARAM;
	evt->treeviewctrl->GetItem(&lvi);
	lvi.iSubItem = evt->nmlvia->iSubItem;

	CMLuaScript *script = (CMLuaScript*)lvi.lParam;

	switch (lvi.iSubItem)
	{
	case 1:
		ShellExecute(m_hwnd, L"Open", script->GetFilePath(), NULL, NULL, SW_SHOWNORMAL);
		break;

	case 2:
		lvi.lParam = (LPARAM)new CMLuaScript(*script);
		delete script;
		script = (CMLuaScript*)lvi.lParam;
		script->Load();

		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = 0;
		lvi.iImage = script->GetStatus() - 1;
		ListView_SetItem(m_scripts.GetHwnd(), &lvi);
		m_scripts.Update(evt->nmlvia->iItem);
		break;
	}

	mir_free(lvi.pszText);
}

void CMLuaOptions::OnReload(CCtrlBase*)
{
	isScriptListInit = false;
	m_scripts.DeleteAllItems();
	g_mLua->Unload();
	g_mLua->Load();
	LoadScripts();
	isScriptListInit = true;
}

/****************************************/

int CMLuaOptions::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInstance;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = L"Lua";
	odp.szTab.w = LPGENW("Scripts");
	odp.pDialog = CMLuaOptions::CreateOptionsPage();
	Options_AddPage(wParam, &odp);

	return 0;
}