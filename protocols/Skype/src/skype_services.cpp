#include "skype_proto.h"

LIST<void> CSkypeProto::serviceList(1);

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
}

void CSkypeProto::InitInstanceServiceList()
{
	// Message API
	this->CreateServiceObj(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	// Chat API
	this->CreateServiceObj(PS_JOINCHAT, &CSkypeProto::OnJoinChat);
	this->CreateServiceObj(PS_LEAVECHAT, &CSkypeProto::OnLeaveChat);
	// Avatar API
	this->CreateServiceObj(PS_GETAVATARINFOT, &CSkypeProto::GetAvatarInfo);
	this->CreateServiceObj(PS_GETAVATARCAPS, &CSkypeProto::GetAvatarCaps);
	this->CreateServiceObj(PS_GETMYAVATART, &CSkypeProto::GetMyAvatar);
	this->CreateServiceObj(PS_SETMYAVATART, &CSkypeProto::SetMyAvatar);
}

void CSkypeProto::UninitInstanceServiceList()
{
	for (int i = 0; i < this->instanceServiceList.getCount(); i++)
	{
		::DestroyServiceFunction(this->instanceServiceList[i]);
	}
}