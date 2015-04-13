#ifndef _STEAM_DIALOGS_H_
#define _STEAM_DIALOGS_H_

typedef CProtoDlgBase<CSteamProto> CSteamDlgBase;

/////////////////////////////////////////////////////////////////////////////////

class CSteamPasswordEditor : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

public:
	CSteamPasswordEditor(CSteamProto *proto);
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamGuardDialog : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	char m_domain[32];

	CCtrlEdit m_text;
	CCtrlButton m_ok;
	CCtrlHyperlink m_link;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

public:
	CSteamGuardDialog(CSteamProto *proto, char *domain);

	char *GetGuardCode();
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamCaptchaDialog : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	BYTE *m_captchaImage;
	int m_captchaImageSize;

	CCtrlEdit m_text;
	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CSteamCaptchaDialog(CSteamProto *proto, BYTE *captchaImage, int captchaImageSize);
	~CSteamCaptchaDialog();

	char *GetCaptchaText();
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamOptionsMain : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	CCtrlEdit m_username;
	CCtrlEdit m_password;
	CCtrlEdit m_group;

	CCtrlCheck m_biggerAvatars;

protected:
	void OnInitDialog();
	void OnApply();

public:
	CSteamOptionsMain(CSteamProto *proto, int idDialog, HWND hwndParent = NULL);

	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CSteamOptionsMain *page = new CSteamOptionsMain((CSteamProto*)param, IDD_ACCMGR, owner);
		page->Show();
		return page;
	}
};

////////////////////////////////////////////////////////////////////////////////

class CSteamOptionsBlockList : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	CCtrlListView m_list;
	CCtrlCombo m_contacts;
	CCtrlButton m_add;

public:
	CSteamOptionsBlockList(CSteamProto *proto);

protected:
	void OnInitDialog();
	void OnBlock(CCtrlButton*);
};

#endif //_STEAM_DIALOGS_H_