#include "skype_proto.h"

LIST<void> CSkypeProto::hookList(1);

void CSkypeProto::InitHookList()
{
	CSkypeProto::hookList.insert(
		::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSkypeProto::PrebuildContactMenu));
}

void CSkypeProto::UninitHookList()
{
	for (int i = 0; i < CSkypeProto::hookList.getCount(); i++)
	{
		::UnhookEvent(CSkypeProto::hookList[i]);
	}
}

void CSkypeProto::InitInstanceHookList()
{
	this->instanceHookList.insert(
		this->HookEvent(ME_OPT_INITIALISE,		&CSkypeProto::OnOptionsInit));
	this->instanceHookList.insert(
		this->HookEvent(ME_USERINFO_INITIALISE, &CSkypeProto::OnUserInfoInit));

	this->instanceHookList.insert(
		this->HookEvent(ME_MSG_PRECREATEEVENT, &CSkypeProto::OnMessagePreCreate));

	this->instanceHookList.insert(
		this->HookEvent(ME_MSG_BUTTONPRESSED, &CSkypeProto::OnTabSRMMButtonPressed)); 
}

void CSkypeProto::UninitInstanceHookList()
{
	for (int i = 0; i < this->instanceHookList.getCount(); i++)
	{
		::UnhookEvent(this->instanceHookList[i]);
	}
}