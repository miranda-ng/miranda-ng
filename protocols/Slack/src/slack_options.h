#ifndef _SLACK_OPTIONS_H_
#define _SLACK_OPTIONS_H_

class CSlackOptionsMain : public CProtoDlgBase<CSlackProto>
{
	typedef CProtoDlgBase<CSlackProto> CSuper;

private:
	CCtrlEdit m_team;
	CCtrlEdit m_email;
	CCtrlEdit m_password;
	CCtrlEdit m_group;

protected:
	void OnInitDialog();
	void OnApply();

public:
	CSlackOptionsMain(CSlackProto *proto, int idDialog);

	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CSlackOptionsMain *page = new CSlackOptionsMain((CSlackProto*)param, IDD_ACCOUNT_MANAGER);
		page->SetParent(owner);
		page->Show();
		return page;
	}

	static CDlgBase *CreateOptionsPage(void *param) { return new CSlackOptionsMain((CSlackProto*)param, IDD_OPTIONS_MAIN); }
};

#endif //_SLACK_OPTIONS_H_
