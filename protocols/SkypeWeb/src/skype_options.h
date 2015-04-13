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

#ifndef _SKYPE_OPTIONS_H_
#define _SKYPE_OPTIONS_H_

class CSkypeOptionsMain : public CSkypeDlgBase
{
private:
	CCtrlEdit m_skypename;
	CCtrlEdit m_password;
	CCtrlEdit m_group;
	CCtrlCheck m_autosync;
	CCtrlCheck m_localtime;

protected:
	CSkypeOptionsMain(CSkypeProto *proto, int idDialog);

	void OnInitDialog();
	void OnApply();

public:
	static CDlgBase *CreateAccountManagerPage(void *param, HWND owner)
	{
		CSkypeOptionsMain *page = new CSkypeOptionsMain((CSkypeProto*)param, IDD_ACCOUNT_MANAGER);
		page->SetParent(owner);
		page->Show();
		return page;
	}

	static CDlgBase *CreateOptionsPage(void *param) { return new CSkypeOptionsMain((CSkypeProto*)param, IDD_OPTIONS_MAIN); }
};

#endif //_SKYPE_OPTIONS_H_