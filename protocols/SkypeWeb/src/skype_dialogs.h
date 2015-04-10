/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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