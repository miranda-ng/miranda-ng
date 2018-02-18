#ifndef _OAUTH_H_
#define _OAUTH_H_

class COAuthDlg : public CDlgBase
{
private:
	CCloudService *m_service;
	const char *m_authUrl;
	pThreadFuncOwner m_requestAccessTokenThread;

	CCtrlButton m_authorize;
	CCtrlEdit m_code;
	CCtrlButton m_ok;

protected:
	void OnInitDialog() override;

	void Auth_OnClick(CCtrlButton*);
	void Code_OnChange(CCtrlBase*);
	void Ok_OnClick(CCtrlButton*);

public:
	COAuthDlg(CCloudService *service, const char *authUrl, pThreadFuncOwner requestAccessTokenThread);
};

#endif //_OAUTH_H_