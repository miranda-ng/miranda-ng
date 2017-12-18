#ifndef _TOX_PROFILE_H_
#define _TOX_PROFILE_H_

class CToxPasswordEditor : public CToxDlgBase
{
private:
	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

	CCtrlButton m_ok;

protected:
	void OnChange(CCtrlBase*);
	void OnOk(CCtrlButton*);

public:
	CToxPasswordEditor(CToxProto *proto);

	wchar_t* GetPassword();
};

#endif //_TOX_PROFILE_H_