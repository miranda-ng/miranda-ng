#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class CFeedEditor : public CDlgBase
{
	friend class CAuthRequest;

private:
	typedef CDlgBase CSuper;

	int m_iItem;
	CCtrlListView *m_list;
	MCONTACT m_hContact;

	CCtrlEdit m_feedtitle;
	CCtrlEdit m_feedurl;
	CCtrlEdit m_checktime; 
	CCtrlSpin m_checktimespin;
	CCtrlButton m_checkfeed;
	CCtrlEdit m_tagedit;
	CCtrlButton m_reset;
	CCtrlButton m_help;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void OnCheckFeed(CCtrlBase*);
	void OnReset(CCtrlBase*);
	void OnHelp(CCtrlBase*);
	void OnUseAuth(CCtrlBase*);

public:
	CCtrlCheck m_useauth;
	CCtrlEdit m_login;
	CCtrlEdit m_password;

	CFeedEditor(int iItem, CCtrlListView *m_list, MCONTACT Contact);

	__inline MCONTACT getContact() const { return m_hContact; }
};

class CAuthRequest : public CDlgBase
{
	CFeedEditor *m_pDlg;
	MCONTACT m_hContact;

	CCtrlBase m_feedname;
	CCtrlEdit m_username;
	CCtrlEdit m_password;

public:
	CAuthRequest(CFeedEditor *pDlg, MCONTACT hContact);

	bool OnInitDialog() override;
	bool OnApply() override;
};

#endif //_OPTIONS_H_