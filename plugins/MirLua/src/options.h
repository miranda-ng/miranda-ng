#pragma once

class CMLuaOptions : public CDlgBase
{
private:
	bool isScriptListInit;
	
	CCtrlCheck m_popupOnError;
	CCtrlCheck m_popupOnObsolete;

	CCtrlListView m_scripts;
	CCtrlButton m_reload;

	void LoadScripts();

protected:
	bool OnInitDialog() override;
	bool OnApply() override;

	void OnScriptListClick(CCtrlListView::TEventInfo *evt);
	void OnReload(CCtrlBase*);

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

public:
	CMLuaOptions();
};
