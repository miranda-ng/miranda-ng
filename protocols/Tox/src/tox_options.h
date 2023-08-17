#ifndef _TOX_OPTIONS_H_
#define _TOX_OPTIONS_H_

class CToxOptionsMain : public CToxDlgBase
{
private:
	typedef CToxDlgBase CSuper;

	CCtrlEdit m_toxAddress;
	CCtrlButton m_toxAddressCopy;
	CCtrlButton m_profileCreate;
	CCtrlButton m_profileImport;
	CCtrlButton m_profileExport;

	CCtrlEdit m_nickname;
	CCtrlEdit m_group;

	CCtrlButton m_passwordCreate;
	CCtrlButton m_passwordChange;
	CCtrlButton m_passwordRemove;

	CCtrlCheck m_enableUdp;
	CCtrlCheck m_enableUdpHolePunching;
	CCtrlCheck m_enableIPv6;
	CCtrlCheck m_enableLocalDiscovery;

protected:
	bool OnInitDialog() override;

	void PasswordCreate_OnClick(CCtrlButton*);
	void PasswordChange_OnClick(CCtrlButton*);
	void PasswordRemove_OnClick(CCtrlButton*);

	void EnableUdp_OnClick(CCtrlBase*);

	void ToxAddressCopy_OnClick(CCtrlButton*);
	void ProfileCreate_OnClick(CCtrlButton*);
	void ProfileImport_OnClick(CCtrlButton*);
	void ProfileExport_OnClick(CCtrlButton*);

	bool OnApply() override;

public:
	CToxOptionsMain(CToxProto *proto, int idDialog);

	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CToxOptionsMain *page = new CToxOptionsMain((CToxProto*)param, IDD_ACCOUNT_MANAGER);
		page->SetParent(owner);
		page->Show();
		return page;
	}

	static CDlgBase *CreateOptionsPage(void *param) { return new CToxOptionsMain((CToxProto*)param, IDD_OPTIONS_MAIN); }
};

/****************************************/

class CToxOptionsNodeList : public CToxDlgBase
{
private:
	typedef CToxDlgBase CSuper;

	CCtrlListView m_nodes;
	CCtrlButton m_addNode;
	CCtrlButton m_updateNodes;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;

	void ReloadNodeList();

	void OnAddNode(CCtrlBase*);
	void OnUpdateNodes(CCtrlBase*);
	void OnNodeListDoubleClick(CCtrlBase*);
	void OnNodeListClick(CCtrlListView::TEventInfo *evt);
	void OnNodeListKeyDown(CCtrlListView::TEventInfo *evt);

public:
	CToxOptionsNodeList(CToxProto *proto);

	static CDlgBase *CreateOptionsPage(void *param) { return new CToxOptionsNodeList((CToxProto*)param); }
};

#endif //_TOX_OPTIONS_H_