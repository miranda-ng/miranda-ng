#include "Mra.h"

static int MraExtraIconsApplyAll(WPARAM, LPARAM)
{
	for (int i=0; i < g_Instances.getCount(); i++)
		g_Instances[i]->MraExtraIconsApply(0, 0);
	return 0;
}

CMraProto::CMraProto(const char* _module, const TCHAR* _displayName) :
	PROTO<CMraProto>(_module, _displayName),
	m_bLoggedIn(false)
{
	InitializeCriticalSectionAndSpinCount(&csCriticalSectionSend, 0);
	MraSendQueueInitialize(0, &hSendQueueHandle);
	MraFilesQueueInitialize(0, &hFilesQueueHandle);
	MraMPopSessionQueueInitialize(&hMPopSessionQueue);
	MraAvatarsQueueInitialize(&hAvatarsQueueHandle);

	CreateService(PS_SETCUSTOMSTATUSEX,   &CMraProto::MraSetXStatusEx);
	CreateService(PS_GETCUSTOMSTATUSEX,   &CMraProto::MraGetXStatusEx);
	CreateService(PS_GETCUSTOMSTATUSICON, &CMraProto::MraGetXStatusIcon);

	CreateService(PS_SET_LISTENINGTO,     &CMraProto::MraSetListeningTo);

	CreateService(PS_CREATEACCMGRUI,      &CMraProto::MraCreateAccMgrUI);
	CreateService(PS_GETAVATARCAPS,       &CMraProto::MraGetAvatarCaps);
	CreateService(PS_GETAVATARINFOT,      &CMraProto::MraGetAvatarInfo);
	CreateService(PS_GETMYAVATART,        &CMraProto::MraGetMyAvatar);

	CreateService(MS_ICQ_SENDSMS,         &CMraProto::MraSendSMS);
	CreateService(MRA_SEND_NUDGE,         &CMraProto::MraSendNudge);

	if ( ServiceExists(MS_NUDGE_SEND))
		heNudgeReceived = CreateHookableEvent(MS_NUDGE);

	TCHAR name[128];
	mir_sntprintf( name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = name;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	HookEvent(ME_SYSTEM_PRESHUTDOWN, &CMraProto::OnPreShutdown);

	InitContactMenu();

	// xstatus menu
	for (size_t i = 0; i < MRA_XSTATUS_COUNT; i++) {
		char szServiceName[100];
		mir_snprintf(szServiceName, SIZEOF(szServiceName), "/menuXStatus%ld", i);
		CreateServiceParam(szServiceName, &CMraProto::MraXStatusMenu, i);
	}

	mir_snprintf(szNewMailSound, SIZEOF(szNewMailSound), "%s: %s", m_szModuleName, MRA_SOUND_NEW_EMAIL);
	SkinAddNewSoundEx(szNewMailSound, m_szModuleName, MRA_SOUND_NEW_EMAIL);

	HookEvent(ME_CLIST_PREBUILDSTATUSMENU, &CMraProto::MraRebuildStatusMenu);

	hExtraXstatusIcon = ExtraIcon_Register("MRAXstatus", LPGEN("Mail.ru Xstatus"), "mra_xstatus25");
	hExtraInfo = ExtraIcon_Register("MRAStatus", LPGEN("Mail.ru extra info"), "mra_xstatus49");

	bHideXStatusUI = FALSE;
	m_iXStatus = mraGetByte(NULL, DBSETTING_XSTATUSID, MRA_MIR_XSTATUS_NONE);
	if ( !IsXStatusValid(m_iXStatus))
		m_iXStatus = MRA_MIR_XSTATUS_NONE;

	bChatExists = MraChatRegister();
}

CMraProto::~CMraProto()
{
	Netlib_CloseHandle(hNetlibUser);

	if (heNudgeReceived)
		DestroyHookableEvent(heNudgeReceived);

	MraMPopSessionQueueDestroy(hMPopSessionQueue);
	MraFilesQueueDestroy(hFilesQueueHandle);
	MraSendQueueDestroy(hSendQueueHandle);
	DeleteCriticalSection(&csCriticalSectionSend);
}

INT_PTR CMraProto::MraCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (int)CreateDialogParam(masMraSettings.hInstance, MAKEINTRESOURCE(IDD_MRAACCOUNT),
		 (HWND)lParam, DlgProcAccount, LPARAM(this));
}

int CMraProto::OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, &CMraProto::MraExtraIconsApply);
	HookEvent(ME_OPT_INITIALISE, &CMraProto::OnOptionsInit);
	HookEvent(ME_DB_CONTACT_DELETED, &CMraProto::MraContactDeleted);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, &CMraProto::MraDbSettingChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, &CMraProto::MraRebuildContactMenu);
	HookEvent(ME_WAT_NEWSTATUS, &CMraProto::MraMusicChanged);

	// всех в offline // тк unsaved values сохран€ютс€ их нужно инициализировать
	for (HANDLE hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
		SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS), -1, -1, 0, 0, ID_STATUS_OFFLINE, NULL, 0, NULL, 0, NULL, 0);

	// unsaved values
	db_set_resident(m_szModuleName, "Status");// NOTE: XStatus cannot be temporary
	db_set_resident(m_szModuleName, "LogonTS");
	db_set_resident(m_szModuleName, "ContactID");
	db_set_resident(m_szModuleName, "GroupID");
	db_set_resident(m_szModuleName, "ContactFlags");
	db_set_resident(m_szModuleName, "ContactSeverFlags");
	db_set_resident(m_szModuleName, "HooksLocked");
	db_set_resident(m_szModuleName, DBSETTING_CAPABILITIES);
	db_set_resident(m_szModuleName, DBSETTING_XSTATUSNAME);
	db_set_resident(m_szModuleName, DBSETTING_XSTATUSMSG);
	db_set_resident(m_szModuleName, DBSETTING_BLOGSTATUSTIME);
	db_set_resident(m_szModuleName, DBSETTING_BLOGSTATUSID);
	db_set_resident(m_szModuleName, DBSETTING_BLOGSTATUS);
	db_set_resident(m_szModuleName, DBSETTING_BLOGSTATUSMUSIC);

	// destroy all chat sessions
	if (bChatExists)
		MraChatSessionDestroy(NULL);

	return 0;
}

int CMraProto::OnPreShutdown(WPARAM, LPARAM)
{
	SetStatus(ID_STATUS_OFFLINE);
	MraAvatarsQueueDestroy(hAvatarsQueueHandle); hAvatarsQueueHandle = NULL;

	if (hThreadWorker) {
		if (IsThreadAlive(hThreadWorker))
			WaitForSingleObjectEx(hThreadWorker, (WAIT_FOR_THREAD_TIMEOUT*1000), FALSE);
		CloseHandle(hThreadWorker);
		hThreadWorker = NULL;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE CMraProto::AddToListByEmail(LPCTSTR plpsEMail, LPCTSTR plpsNick, LPCTSTR plpsFirstName, LPCTSTR plpsLastName, DWORD dwFlags)
{
	if (!plpsEMail)
		return NULL;

	BOOL bAdded;
	HANDLE hContact = MraHContactFromEmail( _T2A(plpsEMail), lstrlen(plpsEMail), TRUE, TRUE, &bAdded);
	if (hContact) {
		if (plpsNick)
			mraSetStringW(hContact, "Nick", plpsNick);
		if (plpsFirstName)
			mraSetStringW(hContact, "FirstName", plpsFirstName);
		if (plpsLastName)
			mraSetStringW(hContact, "LastName", plpsLastName);

		if (dwFlags & PALF_TEMPORARY)
			db_set_b(hContact, "CList", "Hidden", 1);
		else
			db_unset(hContact, "CList", "NotOnList");

		if (bAdded)
			MraUpdateContactInfo(hContact);
	}

	return hContact;
}

HANDLE CMraProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT))
		return 0;

	return AddToListByEmail(psr->email, psr->nick, psr->firstName, psr->lastName, flags);
}

HANDLE CMraProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent)
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) != -1) {
		dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
		if ( db_event_get(hDbEvent, &dbei) == 0 &&
				!strcmp(dbei.szModule, m_szModuleName) &&
				(dbei.eventType == EVENTTYPE_AUTHREQUEST || dbei.eventType == EVENTTYPE_CONTACTS)) {

			char *nick = (char*)(dbei.pBlob + sizeof(DWORD)*2);
			char *firstName = nick + strlen(nick) + 1;
			char *lastName = firstName + strlen(firstName) + 1;
			char *email = lastName + strlen(lastName) + 1;
			return AddToListByEmail( _A2T(email), _A2T(nick), _A2T(firstName), _A2T(lastName), 0);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Stubs

HANDLE CMraProto::ChangeInfo(int, void*) { return NULL; }
int    CMraProto::FileResume(HANDLE, int*, const TCHAR**) { return 1; }
int    CMraProto::RecvAwayMsg(HANDLE, int, PROTORECVEVENT*) { return 1; }
int    CMraProto::RecvUrl(HANDLE, PROTORECVEVENT*) { return 1; }
int    CMraProto::SendAwayMsg(HANDLE, HANDLE, const char* ) { return 1; }
int    CMraProto::SendUrl(HANDLE, int, const char*) { return 1; }

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::Authorize(HANDLE hDBEvent)
{
	if (!m_bLoggedIn)	return 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDBEvent)) == -1)
		return 1;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if ( db_event_get(hDBEvent, &dbei))  return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)                         return 1;
	if ( strcmp(dbei.szModule, m_szModuleName))                          return 1;

	LPSTR lpszNick = (LPSTR)(dbei.pBlob + sizeof(DWORD)*2);
	LPSTR lpszFirstName = lpszNick + lstrlenA(lpszNick) + 1;
	LPSTR lpszLastName = lpszFirstName + lstrlenA(lpszFirstName) + 1;
	LPSTR lpszEMail = lpszLastName + lstrlenA(lpszLastName) + 1;
	MraAuthorize(lpszEMail, lstrlenA(lpszEMail));
	return 0;
}

int CMraProto::AuthDeny(HANDLE hDBEvent, const TCHAR* szReason)
{
	if (!m_bLoggedIn) return 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDBEvent)) == -1)
		return 1;

	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if ( db_event_get(hDBEvent, &dbei))  return 1;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)                         return 1;
	if ( strcmp(dbei.szModule, m_szModuleName))                          return 1;

	LPSTR lpszNick = (LPSTR)(dbei.pBlob + sizeof(DWORD)*2);
	LPSTR lpszFirstName = lpszNick + lstrlenA(lpszNick) + 1;
	LPSTR lpszLastName = lpszFirstName + lstrlenA(lpszFirstName) + 1;
	LPSTR lpszEMail = lpszLastName + lstrlenA(lpszLastName) + 1;

	MraMessageW(FALSE, NULL, 0, 0, lpszEMail, lstrlenA(lpszEMail), szReason, lstrlen(szReason), NULL, 0);
	return 0;
}

int CMraProto::AuthRecv(HANDLE hContact, PROTORECVEVENT* pre)
{
	Proto_AuthRecv(m_szModuleName, pre);
	return 0;
}

int CMraProto::AuthRequest(HANDLE hContact, const TCHAR *lptszMessage)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE CMraProto::FileAllow(HANDLE hContact, HANDLE hTransfer, const TCHAR *szPath)
{
	if (szPath != NULL)
		if ( MraFilesQueueAccept(hFilesQueueHandle, (DWORD)hTransfer, szPath, lstrlen(szPath)) == NO_ERROR)
			return hTransfer; // Success

	return NULL;
}

int CMraProto::FileCancel(HANDLE hContact, HANDLE hTransfer)
{
	if (hContact && hTransfer) {
		MraFilesQueueCancel(hFilesQueueHandle, (DWORD)hTransfer, TRUE);
		return 0; // Success
	}

	return 1;
}

int CMraProto::FileDeny(HANDLE hContact, HANDLE hTransfer, const TCHAR*)
{
	return FileCancel(hContact, hTransfer);
}

/////////////////////////////////////////////////////////////////////////////////////////

DWORD_PTR CMraProto::GetCaps(int type, HANDLE hContact)
{
	switch ( type ) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_MODEMSG | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_ADDED | PF1_VISLIST | PF1_INVISLIST |
			    PF1_INDIVSTATUS | PF1_PEER2PEER | PF1_CHAT | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_CANRENAMEFILE | PF1_FILERESUME |
				 PF1_ADDSEARCHRES | PF1_CONTACT | PF1_SEARCHBYEMAIL | PF1_USERIDISEMAIL | PF1_SEARCHBYNAME | PF1_EXTSEARCHUI;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDUTF;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("E-mail address");

	case PFLAG_MAXCONTACTSPERPACKET:
		return MRA_MAXCONTACTSPERPACKET;

	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR)"e-mail";

	case PFLAG_MAXLENOFMESSAGE:
		return MRA_MAXLENOFMESSAGE;

	default:
		return 0;
	}
}

int CMraProto::GetInfo(HANDLE hContact, int infoType)
{
	return MraUpdateContactInfo(hContact) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE CMraProto::SearchBasic(const TCHAR *id)
{
	return SearchByEmail(id);
}

HANDLE CMraProto::SearchByEmail(const TCHAR *email)
{
	if (m_bLoggedIn && email)
		return MraWPRequestByEMail(NULL, ACKTYPE_SEARCH, _T2A(email), lstrlen(email));

	return NULL;
}

HANDLE CMraProto::SearchByName(const TCHAR *pszNick, const TCHAR *pszFirstName, const TCHAR *pszLastName)
{
	INT_PTR iRet = 0;

	if (m_bLoggedIn && (*pszNick || *pszFirstName || *pszLastName)) {
		DWORD dwRequestFlags = 0;
		if (*pszNick)      SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_NICKNAME);
		if (*pszFirstName) SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME);
		if (*pszLastName)  SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_LASTNAME);
		return MraWPRequestW(NULL, ACKTYPE_SEARCH, dwRequestFlags, NULL, 0, NULL, 0,
			pszNick, lstrlen(pszNick), pszFirstName, lstrlen(pszFirstName), pszLastName, lstrlen(pszLastName), 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::RecvContacts(HANDLE hContact, PROTORECVEVENT* pre)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_CONTACTS;
	dbei.cbBlob = pre->lParam;
	dbei.pBlob = (PBYTE)pre->szMessage;
	db_event_add(hContact, &dbei);
	return 0;
}

int CMraProto::RecvFile(HANDLE hContact, PROTORECVFILET *pre)
{
	return Proto_RecvFile(hContact, pre);
}

int CMraProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	return Proto_RecvMessage(hContact, pre);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList)
{
	INT_PTR iRet = 0;

	if (m_bLoggedIn && hContact) {
		BOOL bSlowSend;
		CHAR szEMail[MAX_EMAIL_LEN];
		LPWSTR lpwszData, lpwszDataCurrent, lpwszNick;
		size_t dwDataBuffSize, dwEMailSize, dwStringSize, dwNickSize;

		dwDataBuffSize = (nContacts*(MAX_EMAIL_LEN*2));
		lpwszData = (LPWSTR)mir_calloc((dwDataBuffSize*sizeof(WCHAR)));
		if (lpwszData) {
			lpwszDataCurrent = lpwszData;
			if ( mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
				for (int i = 0; i < nContacts; i++) {
					if ( IsContactMra( hContactsList[i] ))
					if ( mraGetStaticStringW(hContactsList[i], "e-mail", lpwszDataCurrent, dwDataBuffSize-(lpwszDataCurrent-lpwszData), &dwStringSize)) {
						lpwszDataCurrent += dwStringSize;
						*lpwszDataCurrent++ = ';';

						lpwszNick = GetContactNameW(hContactsList[i]);
						dwNickSize = lstrlenW(lpwszNick);
						memmove(lpwszDataCurrent, lpwszNick, dwNickSize*sizeof(WCHAR));
						lpwszDataCurrent += dwNickSize;
						*lpwszDataCurrent++ = ';';
					}
				}

				bSlowSend = mraGetByte(NULL, "SlowSend", MRA_DEFAULT_SLOW_SEND);
				iRet = MraMessageW(bSlowSend, hContact, ACKTYPE_CONTACTS, MESSAGE_FLAG_CONTACT, szEMail, dwEMailSize, lpwszData, (lpwszDataCurrent-lpwszData), NULL, 0);
				if (bSlowSend == FALSE)
					ProtoBroadcastAck(hContact, ACKTYPE_CONTACTS, ACKRESULT_SUCCESS, (HANDLE)iRet, 0);
			}
			mir_free(lpwszData);
		}
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_CONTACTS, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send when you are offline.");

	return iRet;
}

HANDLE CMraProto::SendFile(HANDLE hContact, const TCHAR* szDescription, TCHAR** ppszFiles)
{
	INT_PTR iRet = 0;

	if (m_bLoggedIn && hContact && ppszFiles) {
		size_t dwFilesCount;
		for (dwFilesCount = 0; ppszFiles[dwFilesCount]; dwFilesCount++);
		MraFilesQueueAddSend(hFilesQueueHandle, 0, hContact, ppszFiles, dwFilesCount, (DWORD*)&iRet);
	}
	return (HANDLE)iRet;
}

int CMraProto::SendMsg(HANDLE hContact, int flags, const char *lpszMessage)
{
	if (!m_bLoggedIn) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send when you are offline.");
		return 0;
	}

	CHAR szEMail[MAX_EMAIL_LEN];
	DWORD dwFlags = 0;
	LPWSTR lpwszMessage = NULL;
	int iRet = 0;

	if (flags & PREF_UNICODE)
		lpwszMessage = (LPWSTR)(lpszMessage + lstrlenA(lpszMessage)+1 );
	else if (flags & PREF_UTF)
		lpwszMessage = mir_utf8decodeT(lpszMessage);
	else
		lpwszMessage = mir_a2t(lpszMessage);

	if ( !lpwszMessage) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"Cant allocate buffer for convert to unicode.");
		return 0;
	}

	size_t dwEMailSize;
	if ( mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
		BOOL bSlowSend = mraGetByte(NULL, "SlowSend", MRA_DEFAULT_SLOW_SEND);
		if ( mraGetByte(NULL, "RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE) && (MraContactCapabilitiesGet(hContact) & FEATURE_FLAG_RTF_MESSAGE))
			dwFlags |= MESSAGE_FLAG_RTF;

		iRet = MraMessageW(bSlowSend, hContact, ACKTYPE_MESSAGE, dwFlags, szEMail, dwEMailSize, lpwszMessage, lstrlen(lpwszMessage), NULL, 0);
		if (bSlowSend == FALSE)
			ProtoBroadcastAckAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)iRet, 0);
	}

	mir_free(lpwszMessage);
	return iRet;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::SetApparentMode(HANDLE hContact, int mode)
{
	if (!m_bLoggedIn || !hContact)
		return 1;

	// Only 3 modes are supported
	if (hContact && (mode == 0 || mode == ID_STATUS_ONLINE || mode == ID_STATUS_OFFLINE)) {
		DWORD dwOldMode = mraGetWord(hContact, "ApparentMode", 0);

		// Dont send redundant updates
		if (mode != dwOldMode) {
			CHAR szEMail[MAX_EMAIL_LEN], szPhones[MAX_EMAIL_LEN];
			WCHAR wszNick[MAX_EMAIL_LEN];
			DWORD dwID, dwGroupID, dwContactFlag = 0;
			size_t dwEMailSize, dwNickSize, dwPhonesSize;

			GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, wszNick, SIZEOF(wszNick), &dwNickSize, szPhones, SIZEOF(szPhones), &dwPhonesSize);

			dwContactFlag &= ~(CONTACT_FLAG_INVISIBLE | CONTACT_FLAG_VISIBLE);
			switch (mode) {
			case ID_STATUS_OFFLINE:
				dwContactFlag |= CONTACT_FLAG_INVISIBLE;
				break;
			case ID_STATUS_ONLINE:
				dwContactFlag |= CONTACT_FLAG_VISIBLE;
				break;
			}

			if (MraModifyContactW(hContact, dwID, dwContactFlag, dwGroupID, szEMail, dwEMailSize, wszNick, dwNickSize, szPhones, dwPhonesSize)) {
				SetContactBasicInfoW(hContact, 0, SCBIF_FLAG, 0, 0, dwContactFlag, 0, 0, NULL, 0, NULL, 0, NULL, 0);
				return 0; // Success
			}
		}
	}

	return 1;
}

int CMraProto::SetStatus(int iNewStatus)
{
	// remap global statuses to local supported
	switch (iNewStatus) {
	case ID_STATUS_OCCUPIED:
		iNewStatus = ID_STATUS_DND;
		break;
	case ID_STATUS_NA:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		iNewStatus = ID_STATUS_AWAY;
		break;
	}

	// nothing to change
	if (m_iStatus == iNewStatus)
		return 0;

	DWORD dwOldStatusMode;

	//set all contacts to offline
	if ((m_iDesiredStatus = iNewStatus) == ID_STATUS_OFFLINE) {
		m_bLoggedIn = FALSE;
		dwOldStatusMode = InterlockedExchange((volatile LONG*)&m_iStatus, m_iDesiredStatus);

		// всех в offline, только если мы бывали подключены
		if (dwOldStatusMode > ID_STATUS_OFFLINE) {
			// функци€ сама провер€ет принадлежность контакта к MRA
			for (HANDLE hContact = db_find_first();hContact != NULL;hContact = db_find_next(hContact))
				SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS), -1, -1, 0, 0, ID_STATUS_OFFLINE, NULL, 0, NULL, 0, NULL, 0);
		}
		Netlib_CloseHandle(hConnection);
	}
	else {
		// если offline то сразу ставим connecting, но обработка как offline
		dwOldStatusMode = InterlockedCompareExchange((volatile LONG*)&m_iStatus, ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);

		switch (dwOldStatusMode) {
		case ID_STATUS_OFFLINE: // offline, connecting
			if (StartConnect() != NO_ERROR) {
				m_bLoggedIn = FALSE;
				m_iDesiredStatus = ID_STATUS_OFFLINE;
				dwOldStatusMode = InterlockedExchange((volatile LONG*)&m_iStatus, m_iDesiredStatus);
			}
			break;
		case ID_STATUS_ONLINE:// connected, change status
		case ID_STATUS_AWAY:
		case ID_STATUS_DND:
		case ID_STATUS_FREECHAT:
		case ID_STATUS_INVISIBLE:
			MraSendNewStatus(m_iDesiredStatus, m_iXStatus, NULL, 0, NULL, 0);
		case ID_STATUS_CONNECTING:
			// предотвращаем переход в любой статус (кроме offline) из статуса connecting, если он не вызван самим плагином
			if (dwOldStatusMode == ID_STATUS_CONNECTING && iNewStatus != m_iDesiredStatus)
				break;

		default:
			dwOldStatusMode = InterlockedExchange((volatile LONG*)&m_iStatus, m_iDesiredStatus);
			break;
		}
	}
	MraSetContactStatus(NULL, m_iStatus);
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)dwOldStatusMode, m_iStatus);
	return 0;
}

HANDLE CMraProto::GetAwayMsg(HANDLE hContact)
{
	if (!m_bLoggedIn || ! hContact)
		return 0;

	TCHAR szStatusDesc[MICBLOG_STATUS_MAX+MICBLOG_STATUS_MAX+MAX_PATH], szBlogStatus[MICBLOG_STATUS_MAX+4], szTime[64];
	DWORD dwTime;
	int iRet = 0;

	if ( mraGetStaticStringW(hContact, DBSETTING_BLOGSTATUS, szBlogStatus, SIZEOF(szBlogStatus), NULL)) {
		SYSTEMTIME tt = {0};
		dwTime = mraGetDword(hContact, DBSETTING_BLOGSTATUSTIME, 0);
		if (dwTime && MakeLocalSystemTimeFromTime32(dwTime, &tt))
			mir_sntprintf(szTime, SIZEOF(szTime), _T("%04ld.%02ld.%02ld %02ld:%02ld: "), tt.wYear, tt.wMonth, tt.wDay, tt.wHour, tt.wMinute);
		else
			szTime[0] = 0;

		mir_sntprintf(szStatusDesc, SIZEOF(szStatusDesc), _T("%s%s"), szTime, szBlogStatus);
		iRet = GetTickCount();
		ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)iRet, (LPARAM)szStatusDesc);
	}
	return (HANDLE)iRet;
}

int CMraProto::SetAwayMsg(int m_iStatus, const TCHAR* msg)
{
	if (!m_bLoggedIn)
		return 1;

	size_t dwStatusDescSize = lstrlen(msg);
	DWORD dwStatus = m_iStatus;
	DWORD dwXStatus = m_iXStatus;

	// не отправл€ем новый статусный текст дл€ хстатусов, дл€ хстатусов только эвей сообщени€
	if (dwStatus != ID_STATUS_ONLINE || IsXStatusValid(dwXStatus) == FALSE) {
		dwStatusDescSize = min(dwStatusDescSize, STATUS_DESC_MAX);
		MraSendNewStatus(dwStatus, dwXStatus, NULL, 0, msg, dwStatusDescSize);
	}
	return 0;
}

int CMraProto::UserIsTyping(HANDLE hContact, int type)
{
	if (!m_bLoggedIn || !hContact || type == PROTOTYPE_SELFTYPING_OFF)
		return 1;

	CHAR szEMail[MAX_EMAIL_LEN];
	size_t dwEMailSize;

	if ( MraGetContactStatus(hContact) != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_INVISIBLE)
	if ( mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
		if ( MraMessageW(FALSE, hContact, 0, MESSAGE_FLAG_NOTIFY, szEMail, dwEMailSize, L" ", 1, NULL, 0))
			return 0;

	return 1;
}

int CMraProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch ( eventType ) {
	case EV_PROTO_ONLOAD:    return OnModulesLoaded( 0, 0 );
	case EV_PROTO_ONEXIT:    return OnPreShutdown( 0, 0 );
	case EV_PROTO_ONOPTIONS: return OnOptionsInit( wParam, lParam );

	case EV_PROTO_ONMENU:
		InitMainMenu();
		break;
	}
	return 1;
}
