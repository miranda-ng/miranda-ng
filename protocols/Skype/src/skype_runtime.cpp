#include "skype_proto.h"

void CSkypeProto::InitSkype()
{
	wchar_t *profileName = ::Utils_ReplaceVarsT(L"%miranda_profilename%");
	wchar_t *dbPath = ::Utils_ReplaceVarsT(L"%miranda_userdata%\\SkypeKit\\");	

	this->skype = CSkype::GetInstance(g_hInstance, profileName, dbPath);
	this->skype->SetOnMessageCallback(
		(CSkype::OnMessaged)&CSkypeProto::OnMessage, 
		this);
}

void CSkypeProto::UninitSkype()
{
	this->skype->stop();
	delete this->skype;
}