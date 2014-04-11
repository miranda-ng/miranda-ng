#include "common.h"

CSteamProto::CSteamProto(const char* protoName, const TCHAR* userName) :
	PROTO<CSteamProto>(protoName, userName),
	hMessageProcess(1)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CSteamProto::OnAccountManagerInit);

	InitializeCriticalSection(&this->contact_search_lock);

	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_sntprintf(sectionName, SIZEOF(sectionName), _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));

	char settingName[100];
	mir_snprintf(settingName, SIZEOF(settingName), "%s_%s", MODULE, "main");

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = filePath;
	sid.pszName = settingName;
	sid.ptszSection = sectionName;
	sid.ptszDescription = LPGENT("Protocol icon");
	sid.iDefaultIndex = -IDI_STEAM;
	Skin_AddIcon(&sid);

	SetAllContactsStatus(ID_STATUS_OFFLINE);

	// Avatar API
	this->CreateProtoService(PS_GETAVATARINFOT, &CSteamProto::GetAvatarInfo);
	this->CreateProtoService(PS_GETAVATARCAPS, &CSteamProto::GetAvatarCaps);
	this->CreateProtoService(PS_GETMYAVATART, &CSteamProto::GetMyAvatar);
	//this->CreateProtoService(PS_SETMYAVATART, &CSteamProto::SetMyAvatar);
}

CSteamProto::~CSteamProto()
{
	DeleteCriticalSection(&this->contact_search_lock);
}

MCONTACT __cdecl CSteamProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->cbSize != sizeof(STEAM_SEARCH_RESULT))
		return 0;

	STEAM_SEARCH_RESULT *ssr = (STEAM_SEARCH_RESULT*)psr;
	return AddContact(ssr->contact);
}

MCONTACT __cdecl CSteamProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent)
{
	return 0;
}

int __cdecl CSteamProto::Authorize(HANDLE hDbEvent)
{
	return 0;
}

int __cdecl CSteamProto::AuthDeny(HANDLE hDbEvent, const TCHAR* szReason)
{
	return 0;
}

int __cdecl CSteamProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT* pre)
{
	return 0;
}

int __cdecl CSteamProto::AuthRequest(MCONTACT hContact, const TCHAR* szMessage)
{
	return 0;
}

HANDLE __cdecl CSteamProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath ) 
{ 
	return 0;
}

int __cdecl CSteamProto::FileCancel(MCONTACT hContact, HANDLE hTransfer ) 
{
	return 0;
}

int __cdecl CSteamProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason )
{
	return 0;
}

int __cdecl CSteamProto::FileResume( HANDLE hTransfer, int* action, const TCHAR** szFilename )
{
	return 0;
}

DWORD_PTR __cdecl CSteamProto:: GetCaps(int type, MCONTACT hContact)
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_SEARCHBYNAME;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_AVATARS;
	case PFLAGNUM_5:
		return PF2_SHORTAWAY | PF2_HEAVYDND | PF2_OUTTOLUNCH;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("SteamID");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"SteamID";
	default:
		return 0;
	}
}

int __cdecl CSteamProto::GetInfo(MCONTACT hContact, int infoType ) { return 0; }

HANDLE __cdecl CSteamProto::SearchBasic(const TCHAR* id)
{
	if (!this->IsOnline())
		return 0;

	ForkThread(&CSteamProto::SearchByIdThread, mir_wstrdup(id));

	return (HANDLE)STEAM_SEARCH_BYID;
}

HANDLE __cdecl CSteamProto::SearchByEmail(const TCHAR* email)
{
	return 0;
}

HANDLE __cdecl CSteamProto::SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName)
{
	if (!this->IsOnline())
		return 0;

	CMString keywords;
	keywords.AppendFormat(L" %s", nick);
	keywords.AppendFormat(L" %s", firstName);
	keywords.AppendFormat(L" %s", lastName);
	keywords.Trim();

	ForkThread(&CSteamProto::SearchByNameThread, mir_wstrdup(keywords));

	return (HANDLE)STEAM_SEARCH_BYNAME;
}

HWND __cdecl CSteamProto::SearchAdvanced( HWND owner ) { return 0; }

HWND __cdecl CSteamProto::CreateExtendedSearchUI( HWND owner ){ return 0; }

int __cdecl CSteamProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre) 
{
	return 0;
}

int __cdecl CSteamProto::RecvFile(MCONTACT hContact, PROTORECVFILET* pre) 
{
	return 0;
}

int __cdecl CSteamProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = lstrlenA(pre->szMessage);
	dbei.pBlob = (BYTE*)pre->szMessage;
	dbei.flags = DBEF_UTF;

	return (INT_PTR)db_event_add(hContact, &dbei);
}

int __cdecl CSteamProto::RecvUrl(MCONTACT hContact, PROTORECVEVENT *) { return 0; }

int __cdecl CSteamProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	return 0;
}

HANDLE __cdecl CSteamProto::SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles)
{
	return 0;
}

int __cdecl CSteamProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	UINT hMessage = InterlockedIncrement(&hMessageProcess);

	SendMessageParam *param = (SendMessageParam*)mir_calloc(sizeof(SendMessageParam));
	param->hContact = hContact;
	param->text = mir_utf8encode(msg);
	param->hMessage = (HANDLE)hMessage;

	ForkThread(&CSteamProto::SendMessageThread, param);

	return hMessage;
}

int __cdecl CSteamProto::SendUrl(MCONTACT hContact, int flags, const char *url) { return 0; }

int __cdecl CSteamProto::SetApparentMode(MCONTACT hContact, int mode) { return 0; }

int CSteamProto::SetStatus(int new_status)
{
	debugLogA("CSteamProto::SetStatus: from %i to %i", m_iStatus, new_status);

	if (new_status == m_iDesiredStatus)
		return 0;

	int old_status = m_iStatus;
	m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE)
	{
		m_bTerminated = true;
		ForkThread(&CSteamProto::LogOutThread, NULL);

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		if (!Miranda_Terminated())
		{
			SetAllContactsStatus(ID_STATUS_OFFLINE);
			//this->CloseAllChatSessions();
		}

		return 0;
	}
	else
	{
		if (old_status == ID_STATUS_OFFLINE/* && !this->IsOnline()*/)
		{
			m_iStatus = ID_STATUS_CONNECTING;
			ForkThread(&CSteamProto::LogInThread, NULL);
		}
		//else
		//{
			//if (IsOnline())
			//{
			//	ForkThread(&CSteamProto::SetServerStatusThread, (void*)new_status);

			//	return 0;
			//}

			//ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

			//return 0;
		//}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	return 0;
}

HANDLE __cdecl CSteamProto::GetAwayMsg(MCONTACT hContact) { return 0; }
int __cdecl CSteamProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt) { return 0; }
int __cdecl CSteamProto::SetAwayMsg(int m_iStatus, const TCHAR *msg) { return 0; }

int __cdecl CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	return 0;
}

int __cdecl CSteamProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType)
	{
	case EV_PROTO_ONLOAD:
		return this->OnModulesLoaded(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return this->OnPreShutdown(wParam, lParam);

	/*case EV_PROTO_ONOPTIONS:
		return this->OnOptionsInit(wParam, lParam);*/

	/*case EV_PROTO_ONCONTACTDELETED:
		return this->OnContactDeleted(wParam, lParam);*/

	/*case EV_PROTO_ONMENU:
		this->OnInitStatusMenu();
		break;*/
	}

	return 1;
}