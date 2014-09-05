#include "skype.h"

wchar_t *CSkypeProto::LogoutReasons[] =
{
	LPGENW("")															/* ---							*/,
	LPGENW("LOGOUT_CALLED")												/* LOGOUT_CALLED				*/,
	LPGENW("HTTPS proxy authentication failed")							/* HTTPS_PROXY_AUTH_FAILED		*/,
	LPGENW("SOCKS proxy authentication failed")							/* SOCKS_PROXY_AUTH_FAILED		*/,
	LPGENW("P2P connection failed")										/* P2P_CONNECT_FAILED			*/,
	LPGENW("Connection to server failed. SkypeKit has been deprecated by Microsoft.")	/* SERVER_CONNECT_FAILED		*/,
	LPGENW("Server is overloaded")										/* SERVER_OVERLOADED			*/,
	LPGENW("SkypeKit database already in use")							/* DB_IN_USE					*/,
	LPGENW("Invalid Skype name")										/* INVALID_SKYPENAME			*/,
	LPGENW("Invalid email")												/* INVALID_EMAIL				*/,
	LPGENW("Unacceptable password")										/* UNACCEPTABLE_PASSWORD		*/,
	LPGENW("Skype name is taken")										/* SKYPENAME_TAKEN				*/,
	LPGENW("REJECTED_AS_UNDERAGE")										/* REJECTED_AS_UNDERAGE			*/,
	LPGENW("NO_SUCH_IDENTITY")											/* NO_SUCH_IDENTITY				*/,
	LPGENW("Incorrect password")										/* INCORRECT_PASSWORD			*/,
	LPGENW("Too many login attempts")									/* TOO_MANY_LOGIN_ATTEMPTS		*/,
	LPGENW("Password has changed")										/* PASSWORD_HAS_CHANGED			*/,
	LPGENW("PERIODIC_UIC_UPDATE_FAILED")								/* PERIODIC_UIC_UPDATE_FAILED	*/,
	LPGENW("DB_DISK_FULL")												/* DB_DISK_FULL					*/,
	LPGENW("DB_IO_ERROR")												/* DB_IO_ERROR					*/,
	LPGENW("SkypeKit database is corrupt")								/* DB_CORRUPT					*/,
	LPGENW("DB_FAILURE")												/* DB_FAILURE					*/,
	LPGENW("Invalid application ID")									/* INVALID_APP_ID				*/,
	LPGENW("APP_ID_FAILURE")											/* APP_ID_FAILURE				*/,
	LPGENW("Version is unsupported")									/* UNSUPPORTED_VERSION			*/,
	LPGENW("Account blocked")											/* ATO_BLOCKED					*/,
	LPGENW("Logout from another instance")								/* REMOTE_LOGOUT				*/,
	LPGENW("ACCESS_TOKEN_RENEWAL_FAILED")								/* ACCESS_TOKEN_RENEWAL_FAILED 	*/
};

wchar_t *CSkypeProto::PasswordChangeReasons[] =
{
	LPGENW("Password successfully changed")								/* PWD_OK						*/,
	LPGENW("Password changing")											/* PWD_CHANGING					*/,
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
	this->login = ::db_get_wsa(NULL, this->m_szModuleName, SKYPE_SETTINGS_SID);
	if ( !this->login || !::wcslen(this->login))
	{
		this->m_iStatus = ID_STATUS_OFFLINE;
		this->SendBroadcast(ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
		
		wchar_t message[512];
		::mir_sntprintf(message, SIZEOF(message), ::TranslateT("You have not entered a Skype name.\nConfigure this in Options->Network->%s and try again."), this->m_tszUserName);
		this->ShowNotification(message);
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
	}

	return true;
}

bool CSkypeProto::LogIn()
{
	if (this->IsOnline() || !this->PrepareLogin())
		return false;

	if (this->GetAccount((char*)_T2A(this->login), this->account))
	{
		if ( !this->PreparePassword())
			return false;

		this->account.fetch();

		this->InitProxy();

		this->SetAccountSettings();	

		this->debugLogW(L"Login in an account");
		this->account->LoginWithPassword(this->password, false, false);
	}

	return true;
}

void CSkypeProto::LogOut()
{
	if	(this->IsOnline() || this->m_iStatus == ID_STATUS_CONNECTING)
	{
		this->account->SetAvailability(Contact::OFFLINE);
		this->debugLogW(L"Logout from account");
		this->account->Logout(true);
	}
}

void CSkypeProto::SetAccountSettings()
{
	int port = this->getWord("Port", rand() % 10000 + 10000);
	this->debugLogW(L"Setting port number to %d", port);
	this->SetInt(SETUPKEY_PORT, port);

	bool useAlternativePorts = this->getByte("UseAlternativePorts", 1) > 0;
	if (useAlternativePorts)
		this->debugLogW(L"Setting listening of alternative ports (80, 443)");
	this->SetInt(SETUPKEY_DISABLE_PORT80, (int)!useAlternativePorts);

	// Create default group for new contacts
	DBVARIANT dbv = {0};
	if ( !getTString(SKYPE_SETTINGS_DEF_GROUP, &dbv) && lstrlen(dbv.ptszVal) > 0)
	{
		this->debugLogW(L"Setting default group for new contacts");
		::Clist_CreateGroup(0, dbv.ptszVal);
		::db_free(&dbv);
	}
}

void CSkypeProto::InitProxy()
{
	if (this->m_hNetlibUser)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		::CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)this->m_hNetlibUser, (LPARAM)&nlus);

		if (nlus.useProxy)
		{
			char address[MAX_PATH];
			::mir_snprintf(address, MAX_PATH, "%s:%d", nlus.szProxyServer, nlus.wProxyPort);

			switch (nlus.proxyType)
			{
			case PROXYTYPE_HTTP:
			case PROXYTYPE_HTTPS:
				this->debugLogW(L"Setting https user proxy config");
				this->SetInt(SETUPKEY_HTTPS_PROXY_ENABLE, 1);
				this->SetInt(SETUPKEY_SOCKS_PROXY_ENABLE, 0);
				this->SetStr(SETUPKEY_HTTPS_PROXY_ADDR, address);
				if (nlus.useProxyAuth)
				{
					this->SetStr(SETUPKEY_HTTPS_PROXY_USER, nlus.szProxyAuthUser);
					ptrA encodedPass(::mir_base64_encode((BYTE*)nlus.szProxyAuthPassword, ::lstrlenA(nlus.szProxyAuthPassword)));
					this->SetStr(SETUPKEY_HTTPS_PROXY_PWD,	(char*)encodedPass);
				}
				break;

			case PROXYTYPE_SOCKS4:
			case PROXYTYPE_SOCKS5:
				this->debugLogW(L"Setting socks user proxy config");
				this->SetInt(SETUPKEY_HTTPS_PROXY_ENABLE, 0);
				this->SetInt(SETUPKEY_SOCKS_PROXY_ENABLE, 1);
				this->SetStr(SETUPKEY_SOCKS_PROXY_ADDR, address);
				if (nlus.useProxyAuth)
				{
					this->SetStr(SETUPKEY_SOCKS_PROXY_USER, nlus.szProxyAuthUser);
					ptrA encodedPass(::mir_base64_encode((BYTE*)nlus.szProxyAuthPassword, ::lstrlenA(nlus.szProxyAuthPassword)));
					this->SetStr(SETUPKEY_SOCKS_PROXY_PWD, (char*)encodedPass);
				}
				break;

			default:
				this->debugLogW(L"Setting automatic proxy detection");
				this->Delete(SETUPKEY_HTTPS_PROXY_ENABLE);
				this->Delete(SETUPKEY_HTTPS_PROXY_ADDR);
				this->Delete(SETUPKEY_HTTPS_PROXY_USER);
				this->Delete(SETUPKEY_HTTPS_PROXY_PWD);
				this->Delete(SETUPKEY_SOCKS_PROXY_ENABLE);
				this->Delete(SETUPKEY_SOCKS_PROXY_ADDR);
				this->Delete(SETUPKEY_SOCKS_PROXY_USER);
				this->Delete(SETUPKEY_SOCKS_PROXY_PWD);
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

	this->SetServerStatus(this->m_iDesiredStatus);

	this->LoadOwnInfo(this);
	this->LoadChatList(this);
	this->LoadContactList(this);
	this->LoadAuthWaitList(this);
	
	fetch(this->transferList);
}

void CSkypeProto::SetServerStatus(int iNewStatus)
{
	if (!this->account)
		return;

	// change status
	if (m_iStatus == iNewStatus)
		return;

	int oldStatus = m_iStatus;
	m_iStatus = iNewStatus;

	CContact::AVAILABILITY availability = CSkypeProto::MirandaToSkypeStatus(iNewStatus);
	if (availability != CContact::UNKNOWN)
	{
		this->debugLogW(L"Setting status to %d", iNewStatus);
		this->account->SetAvailability(availability);
	}

	this->SendBroadcast(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, this->m_iStatus);
}

void CSkypeProto::OnCblUpdated()
{
	// reload our CL after skype CL fully synced
	this->LoadContactList(NULL);
}

void CSkypeProto::OnLoggedOut(CAccount::LOGOUTREASON reason)
{
	this->debugLogW(L"Failed to login: %s", CSkypeProto::LogoutReasons[reason]);

	if (this->m_iStatus == ID_STATUS_CONNECTING)
		this->SendBroadcast(
			ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL,
			CSkypeProto::SkypeToMirandaLoginError(reason));

	this->SetStatus(ID_STATUS_OFFLINE);
	this->ShowNotification(CSkypeProto::LogoutReasons[reason]);

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
			if (this->account->GetPropLogoutreason(reason))
				if (reason != CAccount::LOGOUT_CALLED)
					this->OnLoggedOut(reason);
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
			{
				this->debugLogW(L"Failed to chage password: %s", CSkypeProto::PasswordChangeReasons[status]);
				this->ShowNotification(CSkypeProto::PasswordChangeReasons[status]);
			}
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

/*	case Account::P_AVAILABILITY:
		{
			Contact::AVAILABILITY status;
			this->account->GetPropAvailability(status);
			int mir_status = this->SkypeToMirandaStatus(status);
			if (mir_status != this->m_iStatus && mir_status >= ID_STATUS_OFFLINE)
				this->SetStatus(mir_status);
		}
		break;*/
	}
}