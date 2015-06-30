#ifndef _STEAM_OPTIONS_H_
#define _STEAM_OPTIONS_H_

class CSteamOptionsMain : public CSteamDlgBase
{
private:
	CCtrlEdit m_username;
	CCtrlEdit m_password;
	CCtrlEdit m_group;

	CCtrlCheck m_biggerAvatars;

protected:
	CSteamOptionsMain(CSteamProto *proto, int idDialog, HWND hwndParent = NULL);

	void OnInitDialog();
	void OnApply();

public:
	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CSteamOptionsMain *page = new CSteamOptionsMain((CSteamProto*)param, IDD_ACCMGR, owner);
		page->Show();
		return page;
	}

	static CDlgBase *CreateOptionsPage(void *param) { return new CSteamOptionsMain((CSteamProto*)param, IDD_OPT_MAIN); }
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamOptionsBlockList : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	CCtrlListView m_list;
	CCtrlCombo m_contacts;
	CCtrlButton m_add;

protected:
	void OnInitDialog();
	void OnBlock(CCtrlButton*);

public:
	CSteamOptionsBlockList(CSteamProto *proto);

	static CDlgBase *CreateOptionsPage(void *param) { return new CSteamOptionsBlockList((CSteamProto*)param); }
};

#endif //_STEAM_OPTIONS_H_