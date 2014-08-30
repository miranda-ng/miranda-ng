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

	if (wParam == PRAC_ADDED && !strcmp(account->szModuleName, m_szModuleName))
	{
		UninitToxCore();
		DialogBoxParam(
			g_hInstance,
			MAKEINTRESOURCE(IDD_PROFILE_MANAGER),
			account->hwndAccMgrUI,
			CToxProto::ToxProfileManagerProc,
			(LPARAM)this);
		InitToxCore();
	}

	return 0;
}