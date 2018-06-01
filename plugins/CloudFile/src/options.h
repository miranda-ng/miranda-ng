#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptionsMainDlg : public CDlgBase
{
private:
	CCtrlCombo m_defaultService;

	CCtrlCheck m_doNothingOnConflict;
	CCtrlCheck m_renameOnConflict;
	CCtrlCheck m_repalceOnConflict;

	CCtrlCheck m_urlAutoSend;
	CCtrlCheck m_urlPasteToMessageInputArea;
	CCtrlCheck m_urlCopyToClipboard;

protected:
	void OnInitDialog();
	void OnApply();

public:
	COptionsMainDlg();
};

/////////////////////////////////////////////////////////////////////////////////

class CAccountManagerDlg : public CProtoDlgBase<CCloudService>
{
private:
	CCtrlButton m_requestAccess;
	CCtrlButton m_revokeAccess;

protected:
	void OnInitDialog();

	void RequestAccess_OnClick(CCtrlButton*);
	void RevokeAccess_OnClick(CCtrlButton*);

public:
	CAccountManagerDlg(CCloudService *service);
};

#endif //_OPTIONS_H_