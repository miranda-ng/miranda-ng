#ifndef _SKYPE_OPTIONS_H_
#define _SKYPE_OPTIONS_H_

class CSkypeOptionsMain : public CSkypeDlgBase
{
private:
	typedef CSkypeDlgBase CSuper;

	CCtrlEdit m_skypename;
	CCtrlEdit m_password;
	CCtrlEdit m_group;

protected:
	CSkypeOptionsMain(CSkypeProto *proto, int idDialog, HWND hwndParent = NULL);

	void OnInitDialog();
	void OnApply();

public:
	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CSkypeOptionsMain *page = new CSkypeOptionsMain((CSkypeProto*)param, IDD_ACCOUNT_MANAGER, owner);
		page->Show();
	}

	static CDlgBase *CreateMainOptionsPage(void *param) { return new CSkypeOptionsMain((CSkypeProto*)param, IDD_OPTIONS_MAIN); }
};

#endif //_SKYPE_OPTIONS_H_