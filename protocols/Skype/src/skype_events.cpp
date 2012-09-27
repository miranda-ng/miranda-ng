#include "skype_proto.h"

//void IconsLoad();

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	//IconsLoad();
	this->HookEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	this->SetStatus(ID_STATUS_OFFLINE);

	return 0;
}
