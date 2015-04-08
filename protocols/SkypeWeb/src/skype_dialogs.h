#ifndef _SKYPE_DIALOGS_H_
#define _SKYPE_DIALOGS_H_

typedef CProtoDlgBase<CSkypeProto> CSkypeDlgBase;

class CSkypePasswordEditor : public CSkypeDlgBase
{
private:
	typedef CSkypeDlgBase CSuper;

	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

	CCtrlButton m_ok;

protected:
	void OnInitDialog()
	{
		SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 20, 0);
	}

	void OnOk(CCtrlButton*)
	{
		/*if (m_savePermanently.Enabled())
			m_proto->setString("Password", m_password.GetTextA());
		if (m_proto->password != NULL)
			mir_free(m_proto->password);
		m_proto->password = mir_utf8encodeW(m_password.GetText());
		*/
		EndDialog(m_hwnd, 1);
	}

public:
	CSkypePasswordEditor(CSkypeProto *proto) :
		CSkypeDlgBase(proto, IDD_PASSWORD_EDITOR, NULL, false), m_ok(this, IDOK),
		m_password(this, IDC_PASSWORD), m_savePermanently(this, IDC_SAVEPERMANENTLY)
	{
		m_ok.OnClick = Callback(this, &CSkypePasswordEditor::OnOk);
	}
};

#endif //_SKYPE_DIALOGS_H_