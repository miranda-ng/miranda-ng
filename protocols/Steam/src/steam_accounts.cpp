#include "stdafx.h"

LIST<CSteamProto> CSteamProto::Accounts(1, CSteamProto::CompareProtos);

int CSteamProto::CompareProtos(const CSteamProto *p1, const CSteamProto *p2)
{
	return mir_wstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

CSteamProto* CSteamProto::InitAccount(const char* protoName, const wchar_t* userName)
{
	CSteamProto *ppro = new CSteamProto(protoName, userName);
	Accounts.insert(ppro);
	return ppro;
}

int CSteamProto::UninitAccount(CSteamProto* ppro)
{
	Accounts.remove(ppro);
	delete ppro;
	return 0;
}

CSteamProto* CSteamProto::GetContactAccount(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (proto == nullptr)
		return nullptr;

	for (auto &it : Accounts)
		if (!mir_strcmp(proto, it->m_szModuleName))
			return it;

	return nullptr;
}