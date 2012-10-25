#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->HookEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);

	this->login = this->GetSettingString(SKYPE_SETTINGS_LOGIN);
	this->rememberPassword = this->GetSettingByte("RememberPassword");

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	this->SetStatus(ID_STATUS_OFFLINE);

	return 0;
}

int CSkypeProto::OnContactDeleted(WPARAM wParam, LPARAM lParam)
{
	this->RevokeAuth(wParam, lParam);

	return 0;
}
