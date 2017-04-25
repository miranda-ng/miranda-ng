#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptionsMain : public CPluginDlgBase
{
private:
	CCtrlCheck m_urlAutoSend;
	CCtrlCheck m_urlPasteToMessageInputArea;
	CCtrlCheck m_urlCopyToClipboard;

	bool isServiceListInit;
	CCtrlListView m_services;

protected:
	void OnInitDialog();
	void OnApply();

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	COptionsMain();
};

#endif //_OPTIONS_H_