#include "stdafx.h"

CMLuaOptionsMain::CMLuaOptionsMain(CMPlugin &plugin)
	: CDlgBase(plugin, IDD_OPTIONSMAIN),
	m_plugin(plugin),
	isScriptListInit(false),
	m_popupOnError(this, IDC_POPUPONERROR),
	m_popupOnObsolete(this, IDC_POPUPONOBSOLETE),
	m_scriptsList(this, IDC_SCRIPTS),
	m_reload(this, IDC_RELOAD)
{
	CreateLink(m_popupOnError, "PopupOnError", DBVT_BYTE, 1);
	CreateLink(m_popupOnObsolete, "PopupOnObsolete", DBVT_BYTE, 1);

	m_scriptsList.OnClick = Callback(this, &CMLuaOptionsMain::OnScriptListClick);
	m_reload.OnClick = Callback(this, &CMLuaOptionsMain::OnReload);
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

	return 0;
}

void CMLuaOptionsMain::LoadScripts()
{
	for (auto &script : m_plugin.m_scripts.rev_iter()) {
		int iIcon = ScriptStatusToIcon(script->GetStatus());
		int iItem = m_scriptsList.AddItem(script->GetName(), iIcon, (LPARAM)script);
		m_scriptsList.SetCheckState(iItem, script->IsEnabled());
		m_scriptsList.SetItem(iItem, 1, TranslateT("Open"), 2);
		m_scriptsList.SetItem(iItem, 2, TranslateT("Reload"), 3);
		if (!script->IsBinary())
			m_scriptsList.SetItem(iItem, 3, TranslateT("Compile"), 4);
	}
}

bool CMLuaOptionsMain::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_scriptsList.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);

	HIMAGELIST hImageList = m_scriptsList.CreateImageList(LVSIL_SMALL);
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_LOADED));
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_FAILED));
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_OPEN));
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_RELOAD));
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_COMPILE));

	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathW(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));

	wchar_t relativeScriptDir[MAX_PATH];
	PathToRelativeW(scriptDir, relativeScriptDir, nullptr);

	wchar_t header[MAX_PATH + 100];
	mir_snwprintf(header, L"%s (%s)", TranslateT("Common scripts"), relativeScriptDir);

	m_scriptsList.AddColumn(0, L"Script", 346);
	m_scriptsList.AddColumn(1, nullptr, 34 - GetSystemMetrics(SM_CXVSCROLL));
	m_scriptsList.AddColumn(2, nullptr, 36 - GetSystemMetrics(SM_CXVSCROLL));
	m_scriptsList.AddColumn(3, nullptr, 36 - GetSystemMetrics(SM_CXVSCROLL));

	LoadScripts();

	isScriptListInit = true;
	return true;
}

bool CMLuaOptionsMain::OnApply()
{
	int count = m_scriptsList.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++) {
		CMLuaScript *script = (CMLuaScript*)m_scriptsList.GetItemData(iItem);
		if (!m_scriptsList.GetCheckState(iItem))
			script->Disable();
		else
			script->Enable();
	}
	return true;
}

INT_PTR CMLuaOptionsMain::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr = (LPNMHDR)lParam;
		if (lpnmHdr->idFrom == (UINT_PTR)m_scriptsList.GetCtrlId() && lpnmHdr->code == LVN_ITEMCHANGED) {
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

void CMLuaOptionsMain::OnScriptListClick(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = {};
	lvi.iItem = evt->nmlvia->iItem;
	if (lvi.iItem == -1)
		return;

	lvi.pszText = (LPTSTR)mir_calloc(MAX_PATH * sizeof(wchar_t));
	lvi.cchTextMax = MAX_PATH;
	lvi.mask = LVIF_GROUPID | LVIF_TEXT | LVIF_PARAM;
	m_scriptsList.GetItem(&lvi);
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
		m_scriptsList.SetItem(&lvi);
		m_scriptsList.Update(lvi.iItem);
		break;

	case 3:
		if (script->IsBinary())
			break;
		script->Compile();
		lvi.mask = LVIF_IMAGE;
		lvi.iSubItem = 0;
		lvi.iImage = ScriptStatusToIcon(script->GetStatus());
		m_scriptsList.SetItem(&lvi);
		m_scriptsList.Update(lvi.iItem);
		break;
	}

	mir_free(lvi.pszText);
}

void CMLuaOptionsMain::OnReload(CCtrlBase*)
{
	isScriptListInit = false;
	m_scriptsList.DeleteAllItems();
	m_plugin.ReloadLuaScripts();
	LoadScripts();
	isScriptListInit = true;
}

/***********************************************/

CMLuaEvaluateOptions::CMLuaEvaluateOptions(CMPlugin &plugin)
	: CDlgBase(plugin, IDD_OPTIONSEVALUATE),
	m_script(this, IDC_SCRIPTTEXT),
	m_result(this, IDC_SCRIPTRESULT),
	m_autoEval(this, IDC_AUTOEVAL),
	m_evaluate(this, IDC_EVALUATE)
{
	this->L = lua_newthread(plugin.L);
	threadRef = luaL_ref(L, LUA_REGISTRYINDEX);

	CreateLink(m_autoEval, "AutoEval", DBVT_BYTE, 0);

	m_autoEval.OnChange = Callback(this, &CMLuaEvaluateOptions::OnAutoEvalChange);
	m_evaluate.OnClick = Callback(this, &CMLuaEvaluateOptions::OnEvaluate);
}

CMLuaEvaluateOptions::~CMLuaEvaluateOptions()
{
	luaL_unref(L, LUA_REGISTRYINDEX, threadRef);
	threadRef = 0;
}

bool CMLuaEvaluateOptions::OnInitDialog()
{
	m_script.SetText(m_pPlugin.getMStringW("Script"));
	if (m_autoEval.IsChecked()) {
		m_script.OnChange = Callback(this, &CMLuaEvaluateOptions::OnEvaluate);
		m_evaluate.Click();
	}
	return true;
}

bool CMLuaEvaluateOptions::OnApply()
{
	m_pPlugin.setWString("Script", m_script.GetText());
	return true;
}

void CMLuaEvaluateOptions::OnAutoEvalChange(CCtrlBase*)
{
	if (m_autoEval.IsChecked()) {
		m_script.OnChange = Callback(this, &CMLuaEvaluateOptions::OnEvaluate);
		m_evaluate.Click();
	}
	else m_script.OnChange = CCallback<CCtrlBase>();
}

void CMLuaEvaluateOptions::OnEvaluate(CCtrlBase*)
{
	ptrW script(m_script.GetText());

	CMLuaEnvironment env(L);
	env.Eval(script);
	m_result.SetText(ptrW(mir_utf8decodeW(lua_tostring(L, -1))));
	lua_pop(L, 1);
	env.Unload();
}

/***********************************************/
