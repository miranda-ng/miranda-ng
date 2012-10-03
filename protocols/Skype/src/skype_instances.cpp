#include "skype_proto.h"

int CSkypeProto::SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return this->SendBroadcast(NULL, type, result, hProcess, lParam);
}

LIST<CSkypeProto> CSkypeProto::instanceList(1, CSkypeProto::CompareProtos);

CSkypeProto* CSkypeProto::InitSkypeProto(const char* protoName, const wchar_t* userName)
{
	CSkypeProto *ppro = new CSkypeProto(protoName, userName);
	CSkypeProto::instanceList.insert(ppro);

	return ppro;
}

int CSkypeProto::UninitSkypeProto(CSkypeProto* ppro)
{
	CSkypeProto::instanceList.remove(ppro);
	delete ppro;

	return 0;
}