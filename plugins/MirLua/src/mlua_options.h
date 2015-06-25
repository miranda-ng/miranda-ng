#ifndef _LUA_OPTIONS_H_
#define _LUA_OPTIONS_H_

class CCtrlScriptList : public CCtrlListView
{
private:
	typedef CCtrlListView CSuper;

protected:
	BOOL OnNotify(int idCtrl, NMHDR *pnmh);

public:
	CCtrlScriptList(CDlgBase* dlg, int ctrlId);

	CCallback<TEventInfo> OnClick;
};


class CLuaOptions : public CDlgBase
{
private:
	bool isScriptListInit;
	CCtrlScriptList m_scripts;
	CCtrlButton m_reload;

	void LoadScripts(const TCHAR *scriptDir, int iGroup = -1);
	void LoadScripts();

protected:
	void OnInitDialog();
	void OnApply();

	void OnScriptListClick(CCtrlListView::TEventInfo *evt);
	void OnReload(CCtrlBase*);

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CLuaOptions(int idDialog);

	void CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue);
	void CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue);

	static int OnOptionsInit(WPARAM wParam, LPARAM);

	template<class T>
	__inline void CreateLink(CCtrlData &ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	static CDlgBase *CreateOptionsPage() { return new CLuaOptions(IDD_OPTIONS); }
};

#endif //_LUA_OPTIONS_H_