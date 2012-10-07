#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->HookEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	this->SetStatus(ID_STATUS_OFFLINE);

	return 0;
}

int CSkypeProto::OnContactDeleted(WPARAM wParam, LPARAM)
{
	this->RemoveContact((HANDLE)wParam);

	return 0;
}
