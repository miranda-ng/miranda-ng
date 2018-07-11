#ifndef _TOX_PROFILE_H_
#define _TOX_PROFILE_H_

/* ENTER PASSWORD */

class CToxEnterPasswordDlg : public CToxDlgBase
{
private:
	CCtrlEdit m_password;

	CCtrlButton m_ok;

protected:
	bool OnInitDialog() override;
	
	void Password_OnChange(CCtrlBase*);
	void OnOk(CCtrlButton*);

public:
	CToxEnterPasswordDlg(CToxProto *proto);
};

/* CREATE PASSWORD */

class CToxCreatePasswordDlg : public CToxDlgBase
{
private:
	CCtrlEdit m_newPassword;
	CCtrlEdit m_confirmPassword;

	CCtrlBase m_passwordValidation;

	CCtrlButton m_ok;

protected:
	bool OnInitDialog() override;
	
	void Password_OnChange(CCtrlBase*);
	void OnOk(CCtrlButton*);

public:
	CToxCreatePasswordDlg(CToxProto *proto);
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
	bool OnInitDialog() override;
	
	void Password_OnChange(CCtrlBase*);
	void OnOk(CCtrlButton*);

public:
	CToxChangePasswordDlg(CToxProto *proto);
};

#endif //_TOX_PROFILE_H_