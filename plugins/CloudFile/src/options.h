#ifndef _OPTIONS_H_
#define _OPTIONS_H_

class COptionsMain : public CPluginDlgBase
{
private:
	CCtrlCheck m_urlAutoSend;
	CCtrlCheck m_urlPasteToMessageInputArea;
	CCtrlCheck m_urlCopyToClipboard;

protected:
	void OnInitDialog();

public:
	COptionsMain();
};

#endif //_OPTIONS_H_