#ifndef _STEAM_OPTIONS_H_
#define _STEAM_OPTIONS_H_

class CSteamOptionsMain : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	CCtrlEdit m_steamname;
	CCtrlEdit m_password;
	CCtrlEdit m_group;

protected:
	CSteamOptionsMain(CSteamProto *proto, int idDialog, HWND hwndParent = NULL);

	void OnInitDialog();
	void OnApply();

public:
	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CSteamOptionsMain *page = new CSteamOptionsMain((CSteamProto*)param, IDD_ACCOUNT_MANAGER, owner);
		page->Show();
	}

	static CDlgBase *CreateMainOptionsPage(void *param) { return new CSteamOptionsMain((CSteamProto*)param, IDD_OPTIONS_MAIN); }
};

#endif //_STEAM_OPTIONS_H_