#include "common.h"

LIST<CToxProto> CToxProto::accounts(1, CToxProto::CompareAccounts);

int CToxProto::CompareAccounts(const CToxProto *p1, const CToxProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

CToxProto* CToxProto::InitAccount(const char *protoName, const wchar_t *userName)
{
	ptrA address(db_get_sa(NULL, protoName, TOX_SETTINGS_ID));
	if (address == NULL)
	{
		DialogBoxParam(
			g_hInstance,
			MAKEINTRESOURCE(IDD_PROFILE_IMPORT),
			GetActiveWindow(),
			CToxProto::ToxProfileImportProc,
			(LPARAM)userName);
	}

	CToxProto *proto = new CToxProto(protoName, userName);
	return proto;
}

int CToxProto::UninitAccount(CToxProto *proto)
{
	accounts.remove(proto);
	delete proto;

	return 0;
}

int CToxProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CToxProto::OnOptionsInit);
	HookProtoEvent(ME_USERINFO_INITIALISE, &CToxProto::OnUserInfoInit);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CToxProto::OnPreCreateMessage);

	return 0;
}

int CToxProto::OnAccountRenamed(WPARAM, LPARAM lParam)
{
	PROTOACCOUNT *account = (PROTOACCOUNT*)lParam;

	std::tstring newPath = GetToxProfilePath();
	TCHAR oldPath[MAX_PATH];
	mir_sntprintf(oldPath, MAX_PATH, _T("%s\\%s.tox"), VARST(_T("%miranda_userdata%")), accountName);
	_trename(oldPath, newPath.c_str());
	mir_free(accountName);
	accountName = mir_tstrdup(m_tszUserName);

	return 0;
}