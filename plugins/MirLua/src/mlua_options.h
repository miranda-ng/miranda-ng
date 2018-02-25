#ifndef _LUA_OPTIONS_H_
#define _LUA_OPTIONS_H_

#include <m_gui.h>

class CMLuaOptions : public CPluginDlgBase
{
private:
	CCtrlCheck m_popupOnError;
	CCtrlCheck m_popupOnObsolete;

	bool isScriptListInit;
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
	CMLuaOptions(int idDialog);

	static int OnOptionsInit(WPARAM wParam, LPARAM);
	static CDlgBase *CreateOptionsPage() { return new CMLuaOptions(IDD_OPTIONS); }
};

#endif //_LUA_OPTIONS_H_