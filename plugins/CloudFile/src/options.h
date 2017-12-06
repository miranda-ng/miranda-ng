#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptionsMain : public CPluginDlgBase
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
	COptionsMain();
};

#endif //_OPTIONS_H_