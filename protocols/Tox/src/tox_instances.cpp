#include "common.h"

LIST<CToxProto> CToxProto::instanceList(1, CToxProto::CompareProtos);

int CToxProto::CompareProtos(const CToxProto *p1, const CToxProto *p2)
{
	return wcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

CToxProto* CToxProto::InitProtoInstance(const char* protoName, const wchar_t* userName)
{
	CToxProto *ppro = new CToxProto(protoName, userName);
	CToxProto::instanceList.insert(ppro);

	return ppro;
}

int CToxProto::UninitProtoInstance(CToxProto* ppro)
{
	CToxProto::instanceList.remove(ppro);

	delete ppro;

	return 0;
}

void CToxProto::UninitInstances()
{
	CToxProto::instanceList.destroy();
}

CToxProto* CToxProto::GetContactInstance(MCONTACT hContact)
{
	char *proto = (char *)::CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	if (proto == NULL)
		return NULL;

	for (int i = 0; i < CToxProto::instanceList.getCount(); i++)
		if ( !::strcmp(proto, CToxProto::instanceList[i]->m_szModuleName))
			return CToxProto::instanceList[i];

	return NULL;
}