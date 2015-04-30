#include "stdafx.h"

int CSteamProto::CompareProtos(const CSteamProto *p1, const CSteamProto *p2)
{
	return lstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CSteamProto> CSteamProto::InstanceList(1, CSteamProto::CompareProtos);

CSteamProto* CSteamProto::InitProtoInstance(const char* protoName, const wchar_t* userName)
{
	CSteamProto *ppro = new CSteamProto(protoName, userName);
	InstanceList.insert(ppro);

	return ppro;
}

int CSteamProto::UninitProtoInstance(CSteamProto* ppro)
{
	InstanceList.remove(ppro);
	delete ppro;

	return 0;
}

void CSteamProto::UninitProtoInstances()
{
	for (int i = InstanceList.getCount(); i > 0; i--)
		UninitProtoInstance(InstanceList[i]);
	InstanceList.destroy();
}

CSteamProto* CSteamProto::GetContactProtoInstance(MCONTACT hContact)
{
	char *proto = (char *)::CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);

	if (proto == NULL)
		return NULL;

	for (int i = 0; i < InstanceList.getCount(); i++)
		if (!strcmp(proto, InstanceList[i]->m_szModuleName))
			return InstanceList[i];

	return NULL;
}