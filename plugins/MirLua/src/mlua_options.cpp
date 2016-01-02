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

CLuaOptions::CLuaOptions(int idDialog)
	: CDlgBase(g_hInstance, idDialog),
	m_popupOnError(this, IDC_POPUPONERROR),
	m_popupOnObsolete(this, IDC_POPUPONOBSOLETE),
	isScriptListInit(false), m_scripts(this, IDC_SCRIPTS),
	m_reload(this, IDC_RELOAD)
{
	CreateLink(m_popupOnError, "PopupOnError", DBVT_BYTE, 1);
	CreateLink(m_popupOnObsolete, "PopupOnObsolete", DBVT_BYTE, 1);

	m_scripts.OnClick = Callback(this, &CLuaOptions::OnScriptListClick);
	m_reload.OnClick = Callback(this, &CLuaOptions::OnReload);
}

void CLuaOptions::CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, type, iValue);
}

void CLuaOptions::CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
{
	ctrl.CreateDbLink(MODULE, szSetting, szValue);
}

int CLuaOptions::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Services");
	odp.ptszTitle = _T(MODULE);
	odp.ptszTab = LPGENT("Scripts");
	odp.pDialog = CLuaOptions::CreateOptionsPage();
	Options_AddPage(wParam, &odp);

	return 0;
}

void CLuaOptions::LoadScripts()
{
	for (int i = 0; i < g_mLua->Scripts.getCount(); i++)
	{
		CMLuaScript* script = g_mLua->Scripts[i];
		TCHAR* fileName = NEWTSTR_ALLOCA(script->GetFileName());
		int iItem = m_scripts.AddItem(fileName, -1, (LPARAM)script);
		if (db_get_b(NULL, MODULE, _T2A(fileName), 1))
			m_scripts.SetCheckState(iItem, TRUE);
		m_scripts.SetItem(iItem, 1, TranslateT("Open"), 0);
	}
}

void CLuaOptions::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_scripts.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);

	HIMAGELIST hImageList = m_scripts.CreateImageList(LVSIL_SMALL);
	ImageList_AddIcon(hImageList, GetIcon(IDI_OPEN));

	TCHAR scriptDir[MAX_PATH], relativeScriptDir[MAX_PATH], header[MAX_PATH + 100];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARST(MIRLUA_PATHT));
	PathToRelativeT(scriptDir, relativeScriptDir, NULL);
	mir_sntprintf(header, _T("%s (%s)"), TranslateT("Common scripts"), relativeScriptDir);

	m_scripts.AddColumn(0, _T("Script"), 420);
	m_scripts.AddColumn(1, NULL, 32 - GetSystemMetrics(SM_CXVSCROLL));

	LoadScripts();

	isScriptListInit = true;
}

void CLuaOptions::OnApply()
{
	int count = m_scripts.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++)
	{
		TCHAR fileName[MAX_PATH];
		m_scripts.GetItemText(iItem, 0, fileName, _countof(fileName));
		if (!m_scripts.GetCheckState(iItem))
			db_set_b(NULL, MODULE, _T2A(fileName), 0);
		else
			db_unset(NULL, MODULE, _T2A(fileName));
	}
}

INT_PTR CLuaOptions::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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

void CLuaOptions::OnScriptListClick(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = { 0 };
	lvi.iItem = evt->nmlvia->iItem;
	if (lvi.iItem == -1) return;
	lvi.pszText = (LPTSTR)mir_calloc(MAX_PATH * sizeof(TCHAR));
	lvi.cchTextMax = MAX_PATH;
	lvi.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_PARAM;
	evt->treeviewctrl->GetItem(&lvi);
	lvi.iSubItem = evt->nmlvia->iSubItem;

	CMLuaScript* script = (CMLuaScript*)lvi.lParam;

	if (lvi.iSubItem == 1)
		ShellExecute(m_hwnd, _T("Open"), script->GetFilePath(), NULL, NULL, SW_SHOWNORMAL);

	mir_free(lvi.pszText);
}

void CLuaOptions::OnReload(CCtrlBase*)
{
	isScriptListInit = false;
	m_scripts.DeleteAllItems();
	g_mLua->Unload();
	g_mLua->Load();
	LoadScripts();
	isScriptListInit = true;
}