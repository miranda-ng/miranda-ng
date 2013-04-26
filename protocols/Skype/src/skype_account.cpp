#include "skype_proto.h"
#include "base64/base64.h"

wchar_t *CSkypeProto::LogoutReasons[] =
{
	LPGENW("LOGOUT_CALLED")												/* LOGOUT_CALLED				*/,
	LPGENW("HTTPS_PROXY_AUTH_FAILED")									/* HTTPS_PROXY_AUTH_FAILED		*/,
	LPGENW("SOCKS_PROXY_AUTH_FAILED")									/* SOCKS_PROXY_AUTH_FAILED		*/,
	LPGENW("P2P_CONNECT_FAILED")										/* P2P_CONNECT_FAILED			*/,
	LPGENW("SERVER_CONNECT_FAILED")										/* SERVER_CONNECT_FAILED		*/,
	LPGENW("SERVER_OVERLOADED")											/* SERVER_OVERLOADED			*/,
	LPGENW("DB_IN_USE")													/* DB_IN_USE					*/,
	LPGENW("Invalid skypename")											/* INVALID_SKYPENAME			*/,
	LPGENW("Invalid email")												/* INVALID_EMAIL				*/,
	LPGENW("Unacceptable password")										/* UNACCEPTABLE_PASSWORD		*/,
	LPGENW("SKYPENAME_TAKEN")											/* SKYPENAME_TAKEN				*/,
	LPGENW("REJECTED_AS_UNDERAGE")										/* REJECTED_AS_UNDERAGE			*/,
	LPGENW("NO_SUCH_IDENTITY")											/* NO_SUCH_IDENTITY				*/,
	LPGENW("Incorrect password")										/* INCORRECT_PASSWORD			*/,
	LPGENW("Too many login attempts")									/* TOO_MANY_LOGIN_ATTEMPTS		*/,
	LPGENW("PASSWORD_HAS_CHANGED")										/* PASSWORD_HAS_CHANGED			*/,
	LPGENW("PERIODIC_UIC_UPDATE_FAILED")								/* PERIODIC_UIC_UPDATE_FAILED	*/,
	LPGENW("DB_DISK_FULL")												/* DB_DISK_FULL					*/,
	LPGENW("DB_IO_ERROR")												/* DB_IO_ERROR					*/,
	LPGENW("DB_CORRUPT")												/* DB_CORRUPT					*/,
	LPGENW("DB_FAILURE")												/* DB_FAILURE					*/,
	LPGENW("INVALID_APP_ID")											/* INVALID_APP_ID				*/,
	LPGENW("APP_ID_FAILURE")											/* APP_ID_FAILURE				*/,
	LPGENW("UNSUPPORTED_VERSION")										/* UNSUPPORTED_VERSION			*/,
	LPGENW("ATO (Account TakeOver) detected, account blocked")			/* ATO_BLOCKED					*/,
	LPGENW("Logout from another instance")								/* REMOTE_LOGOUT				*/,
	LPGENW("")															/* ACCESS_TOKEN_RENEWAL_FAILED 	*/
};

wchar_t *CSkypeProto::PasswordChangeReasons[] =
{
	LPGENW("Password change succeeded")									/* PWD_OK						*/,
	LPGENW("")															/* PWD_CHANGING					*/,
	LPGENW("Old password was incorrect")								/* PWD_INVALID_OLD_PASSWORD		*/,
	LPGENW("Failed to verify password. No connection to server")		/* PWD_SERVER_CONNECT_FAILED	*/,
	LPGENW("Password was set but server didn't like it much")			/* PWD_OK_BUT_CHANGE_SUGGESTED	*/,
	LPGENW("New password was exactly the same as old one")				/* PWD_MUST_DIFFER_FROM_OLD		*/,
	LPGENW("The new password was unacceptable")							/* PWD_INVALID_NEW_PWD			*/,
	LPGENW("Account was currently not logged in")						/* PWD_MUST_LOG_IN_TO_CHANGE	*/,
};

bool CSkypeProto::IsOnline()
{
	return this->m_iStatus > ID_STATUS_OFFLINE;
}

bool CSkypeProto::PrepareLogin()
{
	this->login = ::db_get_wsa(NULL, this->m_szModuleName, SKYPE_SETTINGS_LOGIN);
	if ( !this->login || !::wcslen(this->login))
	{
		this->m_iStatus = ID_STATUS_OFFLINE;
		this->SendBroadcast(ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
		this->ShowNotification(
			::TranslateT("You have not entered a Skype name.\n\
						Configure this in Options->Network->Skype and try again."));
		return false;
	}

	return true;
}

bool CSkypeProto::PreparePassword()
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
		else
			::CallService(MS_DB_CRYPT_DECODESTRING, ::strlen(this->password), (LPARAM)this->password);
	}

	return true;
}

bool CSkypeProto::LogIn()
{
	if (this->IsOnline()|| !this->PrepareLogin())
		return false;

	if (g_skype->GetAccount(::mir_u2a(this->login), this->account))
	{
		if ( !this->PreparePassword())
			return false;

		this->account.fetch();
		this->account->SetOnAccountChangedCallback(
			(CAccount::OnAccountChanged)&CSkypeProto::OnAccountChanged,
			this);

		this->InitProxy();

		this->SetAccountSettings();	

		this->account->LoginWithPassword(this->password, false, false);
	}

	return true;
}

void CSkypeProto::LogOut()
{
	if	(this->IsOnline() || this->m_iStatus == ID_STATUS_CONNECTING)
	{
		this->account->SetAvailability(CContact::OFFLINE);
		this->account->Logout(true);

		this->m_iStatus = ID_STATUS_OFFLINE;
		this->SetAllContactStatus(ID_STATUS_OFFLINE);
	}
}

void CSkypeProto::SetAccountSettings()
{
	int port = this->GetSettingWord("Port", rand() % 10000 + 10000);
	g_skype->SetInt(SETUPKEY_PORT, port);
	g_skype->SetInt(SETUPKEY_DISABLE_PORT80, (int)!this->GetSettingByte("UseAlternativePorts", 1));

	// Create default group for new contacts
	DBVARIANT dbv = {0};
	if (!db_get_ts(NULL, m_szModuleName, SKYPE_SETTINGS_DEF_GROUP, &dbv) && lstrlen(dbv.ptszVal) > 0)
	{
		CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)dbv.ptszVal);
		db_free(&dbv);
	}
}

void CSkypeProto::InitProxy()
{
	if (this->hNetLibUser)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		::CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)this->hNetLibUser, (LPARAM)&nlus);

		if (nlus.useProxy) 
		{
			char address[MAX_PATH];
			::mir_snprintf(address, MAX_PATH, "%s:%d", nlus.szProxyServer, nlus.wProxyPort);

			switch (nlus.proxyType)
			{
			case PROXYTYPE_HTTP:
			case PROXYTYPE_HTTPS:
				g_skype->SetInt(SETUPKEY_HTTPS_PROXY_ENABLE, 1);
				g_skype->SetInt(SETUPKEY_SOCKS_PROXY_ENABLE, 0);
				g_skype->SetStr(SETUPKEY_HTTPS_PROXY_ADDR, address);
				if (nlus.useProxyAuth)
				{
					char encodedPass[MAX_PATH];
					Base64::Encode(nlus.szProxyAuthPassword, encodedPass, MAX_PATH);

					g_skype->SetStr(SETUPKEY_HTTPS_PROXY_USER,	nlus.szProxyAuthUser);
					g_skype->SetStr(SETUPKEY_HTTPS_PROXY_PWD,	encodedPass);
				}
				break;

			case PROXYTYPE_SOCKS4:
			case PROXYTYPE_SOCKS5:
				g_skype->SetInt(SETUPKEY_HTTPS_PROXY_ENABLE, 0);
				g_skype->SetInt(SETUPKEY_SOCKS_PROXY_ENABLE, 1);
				g_skype->SetStr(SETUPKEY_SOCKS_PROXY_ADDR, address);
				if (nlus.useProxyAuth)
				{
					g_skype->SetStr(SETUPKEY_SOCKS_PROXY_USER,	nlus.szProxyAuthUser);
					g_skype->SetStr(SETUPKEY_SOCKS_PROXY_PWD,	nlus.szProxyAuthPassword);
				}
				break;

			default:
				g_skype->Delete(SETUPKEY_HTTPS_PROXY_ENABLE);
				g_skype->Delete(SETUPKEY_HTTPS_PROXY_ADDR);
				g_skype->Delete(SETUPKEY_HTTPS_PROXY_USER);
				g_skype->Delete(SETUPKEY_HTTPS_PROXY_PWD);
				g_skype->Delete(SETUPKEY_SOCKS_PROXY_ENABLE);
				g_skype->Delete(SETUPKEY_SOCKS_PROXY_ADDR);
				g_skype->Delete(SETUPKEY_SOCKS_PROXY_USER);
				g_skype->Delete(SETUPKEY_SOCKS_PROXY_PWD);
				break;
			}
		}
	}
}

void CSkypeProto::OnLoggedIn()
{
	if ( !this->rememberPassword)
	{
		::mir_free(this->password);
		this->password = NULL;
	}
	else
	{
		::CallService(MS_DB_CRYPT_ENCODESTRING, ::strlen(this->password), (LPARAM)this->password);
	}

	this->LoadOwnInfo(this);
	this->LoadChatList(this);
	this->LoadContactList(this);
	this->LoadAuthWaitList(this);
	

	fetch(this->transferList);

	this->SetStatus(this->m_iDesiredStatus);
}

void CSkypeProto::OnCblUpdated()
{
	// reload our CL after skype CL fully synced
	this->LoadContactList(this);
	this->LoadAuthWaitList(this);
}

void CSkypeProto::OnLoggedOut(CAccount::LOGOUTREASON reason)
{
	this->m_iStatus = ID_STATUS_OFFLINE;
	this->SendBroadcast(
		ACKTYPE_LOGIN, ACKRESULT_FAILED, 
		NULL, this->SkypeToMirandaLoginError(reason));

	this->ShowNotification(CSkypeProto::LogoutReasons[reason - 1]);

	if (this->rememberPassword && reason == CAccount::INCORRECT_PASSWORD)
	{
		this->rememberPassword = false;
		if (this->password)
		{
			::mir_free(this->password);
			this->password = NULL;
		}
	}
}

void CSkypeProto::OnAccountChanged(int prop)
{
	switch(prop)
	{
	case CAccount::P_STATUS:
		CAccount::STATUS loginStatus;
		this->account->GetPropStatus(loginStatus);

		if (loginStatus == CAccount::LOGGED_IN)
		{
			//this->ForkThread(&CSkypeProto::SignInAsync, 0);
			this->OnLoggedIn();
		}

		if (loginStatus == CAccount::LOGGED_OUT)
		{
			CAccount::LOGOUTREASON reason;
			if (!this->account->GetPropLogoutreason(reason))
				break;

			if (reason != CAccount::LOGOUT_CALLED)
			{
				this->OnLoggedOut(reason);
			}
		}
		break;

	case CAccount::P_CBLSYNCSTATUS:
		{
			CAccount::CBLSYNCSTATUS status;
			this->account->GetPropCblsyncstatus(status);
			if (status == CAccount::CBL_IN_SYNC)
			{
				this->OnCblUpdated();
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