#pragma once

class CMLuaOptions : public CPluginDlgBase
{
private:
	bool isScriptListInit;
	
	CCtrlCheck m_popupOnError;
	CCtrlCheck m_popupOnObsolete;

	CCtrlListView m_scripts;
	CCtrlButton m_reload;

	void LoadScripts();

protected:
	void OnInitDialog();
	void OnApply();

	void OnScriptListClick(CCtrlListView::TEventInfo *evt);
	void OnReload(CCtrlBase*);

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CMLuaOptions();
};
