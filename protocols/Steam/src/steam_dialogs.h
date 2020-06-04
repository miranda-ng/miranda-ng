#ifndef _STEAM_DIALOGS_H_
#define _STEAM_DIALOGS_H_

typedef CProtoDlgBase<CSteamProto> CSteamDlgBase;

#define DIALOG_RESULT_OK 1

/////////////////////////////////////////////////////////////////////////////////

class CSteamPasswordEditor : public CSteamDlgBase
{
	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

public:
	CSteamPasswordEditor(CSteamProto *proto);
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamGuardDialog : public CSteamDlgBase
{
	char m_domain[32];
	char m_guardCode[6];

	CCtrlEdit m_text;
	CCtrlHyperlink m_link;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

public:
	CSteamGuardDialog(CSteamProto *proto, const char *domain);

	const char *GetGuardCode() {
		return m_guardCode;
	}
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamTwoFactorDialog : public CSteamDlgBase
{
	char m_twoFactorCode[6];

	CCtrlEdit m_text;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

public:
	CSteamTwoFactorDialog(CSteamProto *proto);

	const char *GetTwoFactorCode() {
		return m_twoFactorCode;
	}
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamCaptchaDialog : public CSteamDlgBase
{
	char m_captchaText[7];

	uint8_t *m_captchaImage;
	size_t m_captchaImageSize;

	CCtrlEdit m_text;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

public:
	CSteamCaptchaDialog(CSteamProto *proto, const uint8_t *captchaImage, size_t captchaImageSize);
	~CSteamCaptchaDialog();

	const char* GetCaptchaText() {
		return m_captchaText;
	}
};

#endif //_STEAM_DIALOGS_H_
