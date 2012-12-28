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

			CContact::AVAILABILITY status;
			this->account->GetPropAvailability(status);
			if (status != CContact::CONNECTING && status >= CContact::ONLINE)
				this->SetStatus(this->SkypeToMirandaStatus(status));
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
	}
}

bool CSkypeProto::IsOnline()
{
	return this->m_iStatus > ID_STATUS_OFFLINE;
}