#include "stdafx.h"

LIST<CSlackProto> CSlackProto::Accounts(1, CSlackProto::CompareAccounts);

int CSlackProto::CompareAccounts(const CSlackProto *p1, const CSlackProto *p2)
{
	return mir_wstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

CSlackProto* CSlackProto::InitAccount(const char *protoName, const wchar_t *userName)
{
	CSlackProto *proto = new CSlackProto(protoName, userName);
	Accounts.insert(proto);
	return proto;
}

int CSlackProto::UninitAccount(CSlackProto *proto)
{
	Accounts.remove(proto);
	delete proto;
	return 0;
}

CSlackProto* CSlackProto::GetContactAccount(MCONTACT hContact)
{
	for (int i = 0; i < Accounts.getCount(); i++)
		if (mir_strcmpi(GetContactProto(hContact), Accounts[i]->m_szModuleName) == 0)
			return Accounts[i];
	return NULL;
}

int CSlackProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CSlackProto::OnOptionsInit);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CSlackProto::OnPreCreateMessage);

	return 0;
}

int CSlackProto::OnAccountDeleted(WPARAM, LPARAM)
{
	ptrA token(getStringA("TokenSecret"));
	SendRequest(new AuthRevokeRequest(token));
	
	return 0;
};

INT_PTR CSlackProto::OnAccountManagerInit(WPARAM, LPARAM)
{
	return NULL;// (INT_PTR)(CSlackOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}