#include "skype.h"
#include <m_chat.h>

void CSkypeProto::InitServiceList()
{
	::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService);
}

void CSkypeProto::InitInstanceServiceList()
{
	// Message API
	this->CreateServiceObj(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	// Chat API
	this->CreateServiceObj(PS_JOINCHAT, &CSkypeProto::OnJoinChat);
	this->CreateServiceObj(PS_LEAVECHAT, &CSkypeProto::OnLeaveChat);
	// Own info
	this->CreateServiceObj(PS_SETMYNICKNAME, &CSkypeProto::SetMyNickName);
	// Avatar API
	this->CreateServiceObj(PS_GETAVATARINFOT, &CSkypeProto::GetAvatarInfo);
	this->CreateServiceObj(PS_GETAVATARCAPS, &CSkypeProto::GetAvatarCaps);
	this->CreateServiceObj(PS_GETMYAVATART, &CSkypeProto::GetMyAvatar);
	this->CreateServiceObj(PS_SETMYAVATART, &CSkypeProto::SetMyAvatar);
	// service to get from protocol chat buddy info
	this->CreateServiceObj(MS_GC_PROTO_GETTOOLTIPTEXT, &CSkypeProto::SkypeGCGetToolTipText);
}
