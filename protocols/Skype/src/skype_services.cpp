#include "skype.h"
#include <m_chat.h>

void CSkypeProto::InitServiceList()
{
	::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService);
}

void CSkypeProto::InitInstanceServiceList()
{
	// Message API
	this->CreateService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	// Chat API
	this->CreateService(PS_JOINCHAT, &CSkypeProto::OnJoinChat);
	this->CreateService(PS_LEAVECHAT, &CSkypeProto::OnLeaveChat);
	// Own info
	this->CreateService(PS_SETMYNICKNAME, &CSkypeProto::SetMyNickName);
	// Avatar API
	this->CreateService(PS_GETAVATARINFOT, &CSkypeProto::GetAvatarInfo);
	this->CreateService(PS_GETAVATARCAPS, &CSkypeProto::GetAvatarCaps);
	this->CreateService(PS_GETMYAVATART, &CSkypeProto::GetMyAvatar);
	this->CreateService(PS_SETMYAVATART, &CSkypeProto::SetMyAvatar);
	// service to get from protocol chat buddy info
	this->CreateService(MS_GC_PROTO_GETTOOLTIPTEXT, &CSkypeProto::SkypeGCGetToolTipText);
}
