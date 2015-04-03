#ifndef _STEAM_DIALOGS_H_
#define _STEAM_DIALOGS_H_

class CSteamDlgBase : public CProtoDlgBase<CSteamProto>
{
private:
	typedef CProtoDlgBase<CSteamProto> CSuper;

protected:
	__inline CSteamDlgBase(CSteamProto *proto, int idDialog, HWND parent, bool show_label = true) :
		CSuper(proto, idDialog, parent, show_label) { }
};

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

////////////////////////////////////////////////////////////////////////////////

class CSteamOptionsBlockList : public CSteamDlgBase
{
private:
	typedef CSteamDlgBase CSuper;

	CCtrlListView m_list;
	CCtrlCombo m_contacts;
	CCtrlButton m_add;

protected:
	CSteamOptionsBlockList(CSteamProto *proto);

	void OnInitDialog();
	void OnBlock(CCtrlButton*);

public:
	static CDlgBase *CreateOptionsPage(void *param) { return new CSteamOptionsBlockList((CSteamProto*)param); }
};

#endif //_STEAM_DIALOGS_H_