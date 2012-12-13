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

		if ((loginStatus == CAccount::LOGGED_OUT || loginStatus == CAccount::LOGGED_OUT_AND_PWD_SAVED))
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
			
				this->ShowNotification(NULL, CSkypeProto::LogoutReasons[whyLogout - 1]);
			}
		}
		break;

	default :
		OnProfileChanged(prop);
	}
}

bool CSkypeProto::IsOnline()
{
	return this->m_iStatus > ID_STATUS_OFFLINE;
}