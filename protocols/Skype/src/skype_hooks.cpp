#include "skype.h"

void CSkypeProto::InitHookList()
{
	::HookEvent(ME_SYSTEM_MODULESLOADED, &CSkypeProto::OnModulesLoaded);
	::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSkypeProto::PrebuildContactMenu);	
}

void CSkypeProto::InitInstanceHookList()
{
	this->HookEvent(ME_OPT_INITIALISE,		&CSkypeProto::OnOptionsInit);
	this->HookEvent(ME_USERINFO_INITIALISE, &CSkypeProto::OnUserInfoInit);

	this->HookEvent(ME_MSG_PRECREATEEVENT,	&CSkypeProto::OnMessagePreCreate);

	this->HookEvent(ME_MSG_BUTTONPRESSED,	&CSkypeProto::OnTabSRMMButtonPressed); 
}
