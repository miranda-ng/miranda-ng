#include "common.h"

CSkypeOptionsMain::CSkypeOptionsMain(CSkypeProto *proto, int idDialog, HWND hwndParent)
	: CSkypeDlgBase(proto, idDialog, hwndParent, false),
	m_skypename(this, IDC_SKYPENAME),
	m_password(this, IDC_PASSWORD),
	m_group(this, IDC_GROUP),
	m_autosync(this, IDC_AUTOSYNC)
{
	CreateLink(m_group, SKYPE_SETTINGS_GROUP, _T("Skype"));
	CreateLink(m_autosync, "AutoSync", DBVT_BYTE, 1);
}

void CSkypeOptionsMain::OnInitDialog()
{
	CSkypeDlgBase::OnInitDialog();

	m_skypename.SetTextA(ptrA(m_proto->getStringA(SKYPE_SETTINGS_ID)));
	m_password.SetTextA(ptrA(m_proto->getStringA("Password")));

	SendMessage(m_skypename.GetHwnd(), EM_LIMITTEXT, 32, 0);
	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 20, 0);
	SendMessage(m_group.GetHwnd(), EM_LIMITTEXT, 64, 0);
}


void CSkypeOptionsMain::OnApply()
{
	m_proto->setString(SKYPE_SETTINGS_ID, m_skypename.GetTextA());
	m_proto->setString("Password", m_password.GetTextA());
	TCHAR *group = m_group.GetText();
	if (mir_tstrlen(group) > 0 && !Clist_GroupExists(group))
		Clist_CreateGroup(0, group);
}

int CSkypeProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	char *title = mir_t2a(m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.flags = ODPF_BOLDGROUPS | ODPF_DONTTRANSLATE;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pfnDlgProc = CSkypeOptionsMain::DynamicDlgProc;
	odp.dwInitParam = (LPARAM)&SkypeMainOptionsParam;
	SkypeMainOptionsParam.create = CSkypeOptionsMain::CreateOptionsPage;
	SkypeMainOptionsParam.param = this;
	Options_AddPage(wParam, &odp);

	mir_free(title);

	return 0;
}