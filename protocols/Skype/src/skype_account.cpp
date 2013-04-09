#include "skype_proto.h"

void CSkypeProto::OnAccountChanged(int prop)
{
	switch(prop)
	{
	case CAccount::P_STATUS:
		CAccount::STATUS loginStatus;
		this->account->GetPropStatus(loginStatus);
		
		if (loginStatus == CAccount::LOGGED_IN)
		{
			this->ForkThread(&CSkypeProto::SignInAsync, 0);
			//this->SignInAsync(this);
		}

		if (loginStatus == CAccount::LOGGED_OUT)
		{
			CAccount::LOGOUTREASON whyLogout;
			this->account->GetPropLogoutreason(whyLogout);
			if (whyLogout != CAccount::LOGOUT_CALLED)
			{
				this->m_iStatus = ID_STATUS_OFFLINE;
				this->SendBroadcast(
					ACKTYPE_LOGIN, 
					ACKRESULT_FAILED, 
					NULL, 
					this->SkypeToMirandaLoginError(whyLogout));

				this->ShowNotification(CSkypeProto::LogoutReasons[whyLogout - 1]);

				if (this->rememberPassword && whyLogout == CAccount::INCORRECT_PASSWORD)
				{
					this->rememberPassword = false;
					if (this->password)
					{
						::mir_free(this->password);
						this->password = NULL;
					}
				}
			}
		}
		break;

	case CAccount::P_PWDCHANGESTATUS:
		{
			CAccount::PWDCHANGESTATUS status;
			this->account->GetPropPwdchangestatus(status);
			if (status != CAccount::PWD_CHANGING)
				this->ShowNotification(CSkypeProto::PasswordChangeReasons[status]);
		}
		break;

	//case CAccount::P_AVATAR_IMAGE:
	case CAccount::P_AVATAR_TIMESTAMP:
		this->UpdateProfileAvatar(this->account.fetch());
		break;

	//case CAccount::P_MOOD_TEXT:
	case CAccount::P_MOOD_TIMESTAMP:
		this->UpdateProfileStatusMessage(this->account.fetch());
		break;

	case CAccount::P_PROFILE_TIMESTAMP:
		this->UpdateProfile(this->account.fetch());
		break;

	/*case CAccount::P_AVAILABILITY:
		{
			CContact::AVAILABILITY status;
			this->account->GetPropAvailability(status);
			if (status != CContact::CONNECTING && status >= CContact::ONLINE)
				this->SetStatus(this->SkypeToMirandaStatus(status));
		}
		break;*/
	}
}

bool CSkypeProto::IsOnline()
{
	return this->m_iStatus > ID_STATUS_OFFLINE;
}

void __cdecl CSkypeProto::SignInAsync(void*)
{
	if ( !this->rememberPassword)
	{
		::mir_free(this->password);
		this->password = NULL;
	}
	else
	{
		::CallService(MS_DB_CRYPT_ENCODESTRING, ::strlen(this->password), LPARAM(this->password));
	}

	this->LoadOwnInfo(this);
	this->LoadContactList(this);

	this->SetStatus(this->m_iDesiredStatus);
}

bool CSkypeProto::SignIn(int status)
{
	this->login = ::db_get_sa(NULL, this->m_szModuleName, SKYPE_SETTINGS_LOGIN);	
	if ( !this->login || !::strlen(this->login))
	{
		this->m_iStatus = ID_STATUS_OFFLINE;
		this->SendBroadcast(ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
		this->ShowNotification(
			TranslateT("You have not entered a Skype name.\n\
						Configure this in Options->Network->Skype and try again."));
	}
	else if (this->skype->GetAccount(this->login, this->account))
	{
		if ( !this->rememberPassword)
		{
			if (this->password)
			{
				::mir_free(this->password);
				this->password = NULL;
			}
			this->password = ::db_get_sa(NULL, this->m_szModuleName, SKYPE_SETTINGS_PASSWORD);
			if ( !this->password || !::strlen(this->password))
			{
				if (this->password)
				{
					::mir_free(this->password);
					this->password = NULL;
				}
				PasswordRequestBoxParam param(this->login);
				if ( !this->RequestPassword(param))
				{
					this->SetStatus(ID_STATUS_OFFLINE);
					return false;
				}
				else
				{
					this->password = ::mir_strdup(param.password);
					this->rememberPassword = param.rememberPassword;
				}
			}
			else ::CallService(MS_DB_CRYPT_DECODESTRING, ::strlen(this->password), LPARAM(this->password));
		}

		this->account.fetch();
		this->account->SetOnAccountChangedCallback(
			(CAccount::OnAccountChanged)&CSkypeProto::OnAccountChanged,
			this);

		int port = this->GetSettingWord("Port", rand() % 10000 + 10000);
		this->skype->SetInt(SETUPKEY_PORT, port);
		this->skype->SetInt(SETUPKEY_DISABLE_PORT80, (int)!this->GetSettingByte("UseAlternativePorts", 1));

		this->InitProxy();

		this->account->LoginWithPassword(this->password, false, false);

		return true;
	}

	return false;
}