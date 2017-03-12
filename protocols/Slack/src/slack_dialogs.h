#ifndef _SLACK_DIALOGS_H_
#define _SLACK_DIALOGS_H_

class CSlackOAuth : public CProtoDlgBase<CSlackProto>
{
	typedef CProtoDlgBase<CSlackProto> CSuper;

private:
	char m_authCode[40];

	CCtrlHyperlink m_authorize;
	CCtrlEdit m_code;
	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

public:
	CSlackOAuth(CSlackProto *proto);

	const char *GetAuthCode();
};

#endif //_SLACK_DIALOGS_H_
