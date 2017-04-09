#ifndef _DROPBOX_OPTIONS_H_
#define _DROPBOX_OPTIONS_H_

class CDropboxOptionsMain : public CPluginDlgBase
{
private:
	CDropbox *m_instance;


	CCtrlHyperlink m_auth;
	CCtrlEdit m_requestCode;
	CCtrlButton m_authorize;
	CCtrlBase m_authStatus;

	CCtrlCheck m_urlIsTemporary;
	CCtrlCheck m_urlAutoSend;
	CCtrlCheck m_urlPasteToMessageInputArea;
	CCtrlCheck m_urlCopyToClipboard;

protected:
	void OnInitDialog();

	void Auth_OnClick(CCtrlBase*);
	void RequestCode_OnChange(CCtrlBase*);
	void Authorize_OnClick(CCtrlBase*);

public:
	CDropboxOptionsMain(CDropbox *instance);

	static CDlgBase *CreateOptionsPage(void *param) { return new CDropboxOptionsMain((CDropbox*)param); }
};

class CDropboxOptionsInterception : public CPluginDlgBase
{
private:
	bool isAccountListInit;
	CCtrlListView m_accounts;

protected:
	void OnInitDialog();
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void OnApply();

public:
	CDropboxOptionsInterception(CDropbox *instance);

	static CDlgBase *CreateOptionsPage(void *param) { return new CDropboxOptionsInterception((CDropbox*)param); }
};

#endif //_DROPBOX_OPTIONS_H_