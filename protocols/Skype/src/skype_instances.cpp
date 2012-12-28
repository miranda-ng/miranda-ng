#include "skype_proto.h"

int CSkypeProto::SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return this->SendBroadcast(NULL, type, result, hProcess, lParam);
}

LIST<CSkypeProto> CSkypeProto::instanceList(1, CSkypeProto::CompareProtos);

CSkypeProto* CSkypeProto::InitSkypeProto(const char* protoName, const wchar_t* userName)
{
	if (CSkypeProto::instanceList.getCount() > 0) 
	{
		CSkypeProto::ShowNotification(
			::TranslateT("SkypeKit will only permit you to login to one account at a time. \
						  Adding multiple instances of SkypeKit is prohibited in the licence \
						  agreement and standard distribution terms."), 
			MB_ICONWARNING);
		return NULL;
	}
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