#include "stdafx.h"

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

/////////////////////////////////////////////////////////////////////////////////////////
// main options page

class CMLuaOptionsMain : public CDlgBase
{
	CCtrlCheck m_popupOnError;
	CCtrlCheck m_popupOnObsolete;

	CCtrlListView m_scriptsList;
	CCtrlButton m_reload;

	void LoadScripts()
	{
		for (auto &script : g_plugin.m_scripts) {
			int iIcon = ScriptStatusToIcon(script->GetStatus());
			int iItem = m_scriptsList.AddItem(script->GetName(), iIcon, (LPARAM)script);
			m_scriptsList.SetCheckState(iItem, script->IsEnabled());
			m_scriptsList.SetItem(iItem, 1, TranslateT("Open"), 2);
			m_scriptsList.SetItem(iItem, 2, TranslateT("Reload"), 3);
			if (!script->IsBinary())
				m_scriptsList.SetItem(iItem, 3, TranslateT("Compile"), 4);
		}
	}

public:
	CMLuaOptionsMain() :
		CDlgBase(g_plugin, IDD_OPTIONSMAIN),
		m_popupOnError(this, IDC_POPUPONERROR),
		m_popupOnObsolete(this, IDC_POPUPONOBSOLETE),
		m_scriptsList(this, IDC_SCRIPTS),
		m_reload(this, IDC_RELOAD)
	{
		CreateLink(m_popupOnError, "PopupOnError", DBVT_BYTE, 1);
		CreateLink(m_popupOnObsolete, "PopupOnObsolete", DBVT_BYTE, 1);

		m_scriptsList.OnClick = Callback(this, &CMLuaOptionsMain::onClick_List);
		m_scriptsList.OnDoubleClick = Callback(this, &CMLuaOptionsMain::onDblClick_List);

		m_reload.OnClick = Callback(this, &CMLuaOptionsMain::OnReload);
	}

	bool OnInitDialog() override
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

		m_scriptsList.AddColumn(0, TranslateT("Script"), 346);
		m_scriptsList.AddColumn(1, nullptr, 34 - GetSystemMetrics(SM_CXVSCROLL));
		m_scriptsList.AddColumn(2, nullptr, 36 - GetSystemMetrics(SM_CXVSCROLL));
		m_scriptsList.AddColumn(3, nullptr, 36 - GetSystemMetrics(SM_CXVSCROLL));

		LoadScripts();
		return true;
	}

	bool OnApply() override
	{
		int count = m_scriptsList.GetItemCount();
		for (int iItem = 0; iItem < count; iItem++) {
			CMLuaScript *script = (CMLuaScript *)m_scriptsList.GetItemData(iItem);
			if (!m_scriptsList.GetCheckState(iItem))
				script->Disable();
			else
				script->Enable();
		}
		return true;
	}

	void onClick_List(CCtrlListView::TEventInfo *evt)
	{
		LVITEM lvi = {};
		lvi.iItem = evt->nmlvia->iItem;
		if (lvi.iItem == -1)
			return;

		lvi.mask = LVIF_PARAM;
		m_scriptsList.GetItem(&lvi);
		auto *script = (CMLuaScript *)lvi.lParam;

		switch (evt->nmlvia->iSubItem) {
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
	}

	void onDblClick_List(CCtrlListView::TEventInfo *evt)
	{
		int iItem = evt->nmlv->iItem;
		if (iItem == -1)
			return;

		auto *script = (CMLuaScript *)m_scriptsList.GetItemData(iItem);
		ShellExecute(m_hwnd, L"open", script->GetFilePath(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	void OnReload(CCtrlBase *)
	{
		m_scriptsList.SetSilent(true);
		m_scriptsList.DeleteAllItems();
		g_plugin.ReloadLuaScripts();
		LoadScripts();
		m_scriptsList.SetSilent(false);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// evaluate options page

class CMLuaEvaluateOptions : public CDlgBase
{
	lua_State *L = nullptr;
	int threadRef = 0;

	CCtrlEdit m_script;
	CCtrlEdit m_result;

	CCtrlCheck m_autoEval;
	CCtrlButton m_evaluate;

	void OnAutoEvalChange(CCtrlBase *)
	{
		if (m_autoEval.IsChecked()) {
			m_script.OnChange = Callback(this, &CMLuaEvaluateOptions::OnEvaluate);
			m_evaluate.Click();
		}
		else m_script.OnChange = CCallback<CCtrlBase>();
	}

	void OnEvaluate(CCtrlBase *)
	{
		ptrW script(m_script.GetText());

		CMLuaEnvironment env(L);
		env.Eval(script);
		m_result.SetText(ptrW(mir_utf8decodeW(lua_tostring(L, -1))));
		lua_pop(L, 1);
		env.Unload();
	}

public:
	CMLuaEvaluateOptions() :
		CDlgBase(g_plugin, IDD_OPTIONSEVALUATE),
		m_script(this, IDC_SCRIPTTEXT),
		m_result(this, IDC_SCRIPTRESULT),
		m_autoEval(this, IDC_AUTOEVAL),
		m_evaluate(this, IDC_EVALUATE)
	{
		this->L = lua_newthread(g_plugin.L);
		threadRef = luaL_ref(L, LUA_REGISTRYINDEX);

		CreateLink(m_autoEval, "AutoEval", DBVT_BYTE, 0);

		m_autoEval.OnChange = Callback(this, &CMLuaEvaluateOptions::OnAutoEvalChange);
		m_evaluate.OnClick = Callback(this, &CMLuaEvaluateOptions::OnEvaluate);
	}

	~CMLuaEvaluateOptions()
	{
		luaL_unref(L, LUA_REGISTRYINDEX, threadRef);
		threadRef = 0;
	}

	bool OnInitDialog() override
	{
		m_script.SetText(m_pPlugin.getMStringW("Script"));
		if (m_autoEval.IsChecked()) {
			m_script.OnChange = Callback(this, &CMLuaEvaluateOptions::OnEvaluate);
			m_evaluate.Click();
		}
		return true;
	}

	bool OnApply() override
	{
		m_pPlugin.setWString("Script", m_script.GetText());
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

int CMPlugin::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = L"Lua";

	odp.szTab.w = LPGENW("Scripts");
	odp.pDialog = new CMLuaOptionsMain();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Evaluate");
	odp.pDialog = new CMLuaEvaluateOptions();
	g_plugin.addOptions(wParam, &odp);

	return 0;
}
