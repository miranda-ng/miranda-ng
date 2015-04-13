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
	CCtrlEdit m_password;
	CCtrlEdit m_group;

	CCtrlCheck m_enableUdp;
	CCtrlCheck m_enableIPv6;

protected:
	void OnInitDialog();

	void ToxAddressCopy_OnClick(CCtrlButton*);
	void ProfileCreate_OnClick(CCtrlButton*);
	void ProfileImport_OnClick(CCtrlButton*);
	void ProfileExport_OnClick(CCtrlButton*);

	void OnApply();

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

/////////////////////////////////////////////////////////////////////////////////

class CToxOptionsMultimedia : public CToxDlgBase
{
private:
	typedef CToxDlgBase CSuper;

	CCtrlCombo m_audioInput;
	CCtrlCombo m_audioOutput;

protected:
	void OnInitDialog();

	void AudioInput_OnClick(CCtrlData*);
	void AudioOutput_OnClick(CCtrlData*);

	void OnApply();

public:
	CToxOptionsMultimedia(CToxProto *proto);

	static CDlgBase *CreateOptionsPage(void *param) { return new CToxOptionsMultimedia((CToxProto*)param); }
};

/////////////////////////////////////////////////////////////////////////////////

class CToxNodeEditor : public CDlgBase
{
private:
	typedef CDlgBase CSuper;

	int m_iItem;
	CCtrlListView *m_list;

	CCtrlEdit m_ipv4;
	CCtrlEdit m_ipv6;
	CCtrlEdit m_port;
	CCtrlEdit m_pkey;

	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlBase*);
	void OnClose();

public:
	CToxNodeEditor(int iItem, CCtrlListView *m_list);
};

/****************************************/

class CCtrlNodeList : public CCtrlListView
{
private:
	typedef CCtrlListView CSuper;

protected:
	BOOL OnNotify(int idCtrl, NMHDR *pnmh);

public:
	CCtrlNodeList(CDlgBase* dlg, int ctrlId);

	CCallback<TEventInfo> OnClick;
};

/****************************************/

class CToxOptionsNodeList : public CToxDlgBase
{
private:
	typedef CToxDlgBase CSuper;

	CCtrlNodeList m_nodes;
	CCtrlButton m_addNode;

protected:
	void OnInitDialog();
	void OnApply();

	void OnAddNode(CCtrlBase*);
	void OnNodeListDoubleClick(CCtrlBase*);
	void OnNodeListClick(CCtrlListView::TEventInfo *evt);
	void OnNodeListKeyDown(CCtrlListView::TEventInfo *evt);

public:
	CToxOptionsNodeList(CToxProto *proto);

	static CDlgBase *CreateOptionsPage(void *param) { return new CToxOptionsNodeList((CToxProto*)param); }
};

#endif //_TOX_OPTIONS_H_