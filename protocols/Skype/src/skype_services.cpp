#include "skype.h"
#include <m_chat.h>

void CSkypeProto::InitServiceList()
{
	::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService);
}

void CSkypeProto::InitInstanceServiceList()
{
	// Message API
	this->CreateProtoService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
	// Chat API
	this->CreateProtoService(PS_JOINCHAT, &CSkypeProto::OnJoinChat);
	this->CreateProtoService(PS_LEAVECHAT, &CSkypeProto::OnLeaveChat);
	// Own info
	this->CreateProtoService(PS_SETMYNICKNAME, &CSkypeProto::SetMyNickName);
	// Avatar API
	this->CreateProtoService(PS_GETAVATARINFOT, &CSkypeProto::GetAvatarInfo);
	this->CreateProtoService(PS_GETAVATARCAPS, &CSkypeProto::GetAvatarCaps);
	this->CreateProtoService(PS_GETMYAVATART, &CSkypeProto::GetMyAvatar);
	this->CreateProtoService(PS_SETMYAVATART, &CSkypeProto::SetMyAvatar);
	// service to get from protocol chat buddy info
	this->CreateProtoService(MS_GC_PROTO_GETTOOLTIPTEXT, &CSkypeProto::SkypeGCGetToolTipText);
}
