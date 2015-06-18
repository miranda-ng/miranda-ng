#ifndef _LUA_OPTIONS_H_
#define _LUA_OPTIONS_H_

class CLuaOptions : public CDlgBase
{
private:
	bool isScriptListInit;
	CCtrlListView m_scripts;
	CCtrlButton m_reload;

	void LoadScripts(const TCHAR *scriptDir, int iGroup = -1);

protected:
	void OnInitDialog();
	void OnApply();

	void OnReload(CCtrlBase*);

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CLuaOptions(int idDialog);

	void CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue);
	void CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue);

	template<class T>
	__inline void CreateLink(CCtrlData &ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	static CDlgBase *CreateOptionsPage() { return new CLuaOptions(IDD_OPTIONS_MAIN); }
};

#endif //_LUA_OPTIONS_H_