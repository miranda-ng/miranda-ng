#include "skype.h"

void CSkypeProto::InitHookList()
{
	::HookEvent(ME_SYSTEM_MODULESLOADED, &CSkypeProto::OnModulesLoaded);
	::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSkypeProto::PrebuildContactMenu);	
}

void CSkypeProto::InitInstanceHookList()
{
	this->HookProtoEvent(ME_OPT_INITIALISE,		&CSkypeProto::OnOptionsInit);
	this->HookProtoEvent(ME_USERINFO_INITIALISE, &CSkypeProto::OnUserInfoInit);

	this->HookProtoEvent(ME_MSG_PRECREATEEVENT,	&CSkypeProto::OnMessagePreCreate);

	this->HookProtoEvent(ME_MSG_BUTTONPRESSED,	&CSkypeProto::OnTabSRMMButtonPressed); 
}
