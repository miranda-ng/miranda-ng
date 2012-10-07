#include "skype_proto.h"

LIST<void> CSkypeProto::serviceList(0);

void CSkypeProto::InitServiceList()
{
	CSkypeProto::serviceList.insert(
		::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService));
}

void CSkypeProto::UninitServiceList()
{
	for (int i = 0; i < CSkypeProto::serviceList.getCount(); i++)
	{
		::DestroyServiceFunction(CSkypeProto::serviceList[i]);
	}
	CSkypeProto::serviceList.destroy();
}
