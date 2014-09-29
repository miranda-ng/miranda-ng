#include "common.h"

LIST<CToxProto> CToxProto::accounts(1, CToxProto::CompareAccounts);

int CToxProto::CompareAccounts(const CToxProto *p1, const CToxProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

CToxProto* CToxProto::InitAccount(const char* protoName, const wchar_t* userName)
{
	CToxProto *ppro = new CToxProto(protoName, userName);
	accounts.insert(ppro);

	return ppro;
}

int CToxProto::UninitAccount(CToxProto* ppro)
{
	accounts.remove(ppro);
	delete ppro;

	return 0;
}

int CToxProto::OnAccountListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT* account = (PROTOACCOUNT*)lParam;

	if (!strcmp(account->szModuleName, m_szModuleName))
	{
		switch (wParam)
		{
		case PRAC_ADDED:
			DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_PROFILE_IMPORT),
				account->hwndAccMgrUI,
				CToxProto::ToxProfileManagerProc,
				(LPARAM)this);
			InitToxCore();
			SaveToxProfile();
			break;

		case PRAC_CHANGED:
			std::tstring newPath = GetToxProfilePath();
			TCHAR oldPath[MAX_PATH];
			mir_sntprintf(oldPath, MAX_PATH, _T("%s\\%s.tox"), VARST(_T("%miranda_userdata%")), accountName);
			MoveFileEx(oldPath, newPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
			mir_free(accountName);
			accountName = mir_tstrdup(m_tszUserName);
			break;
		}
	}

	return 0;
}