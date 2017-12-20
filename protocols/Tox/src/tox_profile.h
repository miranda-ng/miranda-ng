#ifndef _TOX_PROFILE_H_
#define _TOX_PROFILE_H_

/* ENTER PASSWORD */

class CToxEnterPasswordDlg : public CToxDlgBase
{
private:
	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void Password_OnChange(CCtrlBase*);
	void OnOk(CCtrlButton*);

public:
	CToxEnterPasswordDlg(CToxProto *proto);

	wchar_t* GetPassword();
};

/* CHANGE PASSWORD */

class CToxChangePasswordDlg : public CToxDlgBase
{
private:
	CCtrlEdit m_oldPassword;

	CCtrlEdit m_newPassword;
	CCtrlEdit m_confirmPassword;

	CCtrlBase m_passwordValidation;
	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void Password_OnChange(CCtrlBase*);
	void OnOk(CCtrlButton*);

public:
	CToxChangePasswordDlg(CToxProto *proto);

	wchar_t* GetPassword();
};

#endif //_TOX_PROFILE_H_