#ifndef _OAUTH_H_
#define _OAUTH_H_

class COAuthDlg : public CDlgBase
{
	CCloudService *m_service;
	CCloudService::MyThreadFunc m_requestAccessTokenThread;

	CCtrlButton m_ok;
	CCtrlHyperlink m_authorize;
	CCtrlEdit m_code;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;

	void Code_OnChange(CCtrlBase*);

public:
	COAuthDlg(CCloudService *service, const char *authUrl, CCloudService::MyThreadFunc requestAccessTokenThread);
};

#endif //_OAUTH_H_