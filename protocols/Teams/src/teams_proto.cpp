#include "stdafx.h"

CTeamsProto::CTeamsProto(const char *protoName, const wchar_t *userName) :
	PROTO<CTeamsProto>(protoName, userName),
	m_impl(*this),
	m_requests(10),
	m_PopupClasses(1),
	m_OutMessages(3, PtrKeySortT),
	m_bAutoHistorySync(this, "AutoSync", true),
	m_bUseHostnameAsPlace(this, "UseHostName", true),
	m_bUseBBCodes(this, "UseBBCodes", true),
	m_wstrCListGroup(this, SKYPE_SETTINGS_GROUP, L"Skype"),
	m_wstrPlace(this, "Place", L""),
	m_iMood(this, "Mood", 0),
	m_wstrMoodEmoji(this, "MoodEmoji", L""),
	m_wstrMoodMessage(this, "XStatusMsg", L"")
{
	// network
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = m_tszUserName;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateProtoService(PS_GETAVATARINFO, &CTeamsProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CTeamsProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CTeamsProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CTeamsProto::SvcSetMyAvatar);

	CreateProtoService(PS_OFFLINEFILE, &CTeamsProto::SvcOfflineFile);

	CreateProtoService(PS_MENU_REQAUTH, &CTeamsProto::OnRequestAuth);
	CreateProtoService(PS_MENU_GRANTAUTH, &CTeamsProto::OnGrantAuth);

	CreateProtoService(PS_MENU_LOADHISTORY, &CTeamsProto::SvcLoadHistory);
	CreateProtoService(PS_EMPTY_SRV_HISTORY, &CTeamsProto::SvcEmptyHistory);

	HookProtoEvent(ME_OPT_INITIALISE, &CTeamsProto::OnOptionsInit);

	CreateDirectoryTreeW(GetAvatarPath());

	// sounds
	g_plugin.addSound("skype_inc_call", L"SkypeWeb", LPGENW("Incoming call"));
	g_plugin.addSound("skype_call_canceled", L"SkypeWeb", LPGENW("Incoming call canceled"));

	CheckConvert();
	InitGroupChatModule();
}

CTeamsProto::~CTeamsProto()
{
	UninitPopups();
}

void CTeamsProto::OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags)
{
	if (!hContact || !(flags & CDF_DEL_HISTORY))
		return;

	DB::EventInfo dbei(hDbEvent, false);
	if (dbei.szId) {
		auto *pReq = new AsyncHttpRequest(REQUEST_DELETE, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(getId(hContact)) + "/messages/" + dbei.szId);
		pReq->AddAuthentication(this);
		pReq->AddHeader("Origin", "https://web.skype.com");
		pReq->AddHeader("Referer", "https://web.skype.com/");
		PushRequest(pReq);
	}
}

void CTeamsProto::OnEventEdited(MCONTACT hContact, MEVENT, const DBEVENTINFO &dbei)
{
	SendServerMsg(hContact, dbei.pBlob, dbei.iTimestamp);
}

void CTeamsProto::OnModulesLoaded()
{
	setAllContactStatuses(ID_STATUS_OFFLINE, false);

	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CTeamsProto::OnPreCreateMessage);

	InitPopups();
}

void CTeamsProto::OnShutdown()
{
	StopQueue();
}

INT_PTR CTeamsProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH | PF1_MODEMSG | PF1_FILE | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_4:
		return PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SERVERMSGID | PF4_SERVERFORMATTING;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Teams ID");
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CTeamsProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	debugLogA(__FUNCTION__);

	if (psr->id.a == nullptr)
		return NULL;

	MCONTACT hContact;
	if (psr->flags & PSR_UNICODE)
		hContact = AddContact(T2Utf(psr->id.w), T2Utf(psr->nick.w));
	else
		hContact = AddContact(psr->id.a, psr->nick.a);

	return hContact;
}

MCONTACT CTeamsProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);

	DB::EventInfo dbei(hDbEvent);
	if (!dbei)
		return NULL;
	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return NULL;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return NULL;

	DB::AUTH_BLOB blob(dbei.pBlob);
	return AddContact(blob.get_email(), blob.get_nick());
}

int CTeamsProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthAcceptRequest(getId(hContact)));
	return 0;
}

int CTeamsProto::AuthDeny(MEVENT hDbEvent, const wchar_t *)
{
	MCONTACT hContact = GetContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthDeclineRequest(getId(hContact)));
	return 0;
}

int CTeamsProto::AuthRecv(MCONTACT, DB::EventInfo &dbei)
{
	return Proto_AuthRecv(m_szModuleName, dbei);
}

int CTeamsProto::AuthRequest(MCONTACT hContact, const wchar_t *szMessage)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AddContactRequest(getId(hContact), T2Utf(szMessage)));
	return 0;
}

int CTeamsProto::GetInfo(MCONTACT hContact, int)
{
	if (isChatRoom(hContact))
		return 1;

	PushRequest(new GetProfileRequest(this, hContact));
	return 0;
}

int CTeamsProto::SendMsg(MCONTACT hContact, MEVENT, const char *szMessage)
{
	return SendServerMsg(hContact, szMessage);
}

int CTeamsProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	switch (iNewStatus) {
	case ID_STATUS_FREECHAT: iNewStatus = ID_STATUS_ONLINE; break;
	case ID_STATUS_NA:       iNewStatus = ID_STATUS_AWAY;   break;
	case ID_STATUS_OCCUPIED: iNewStatus = ID_STATUS_DND;    break;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		StopQueue();

		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, ID_STATUS_OFFLINE);

		if (!Miranda_IsTerminated())
			setAllContactStatuses(ID_STATUS_OFFLINE, false);
		return 0;
	}

	if (m_iStatus == ID_STATUS_OFFLINE)
		Login();
	else
		PushRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus)));
	return 0;
}

int CTeamsProto::UserIsTyping(MCONTACT hContact, int iState)
{
	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(getId(hContact)) + "/messages");

	JSONNode node;
	node << INT64_PARAM("clientmessageid", getRandomId()) << CHAR_PARAM("contenttype", "text") << CHAR_PARAM("content", "")
		<< CHAR_PARAM("messagetype", (iState == PROTOTYPE_SELFTYPING_ON) ? "Control/Typing" : "Control/ClearTyping");
	pReq->m_szParam = node.write().c_str();

	PushRequest(pReq);
	return 0;
}

int CTeamsProto::RecvContacts(MCONTACT hContact, DB::EventInfo &dbei)
{
	PROTOSEARCHRESULT **isrList = (PROTOSEARCHRESULT **)dbei.pBlob;

	int nCount = dbei.cbBlob;

	uint32_t cbBlob = 0;
	for (int i = 0; i < nCount; i++)
		cbBlob += int(/*mir_wstrlen(isrList[i]->nick.w)*/0 + 2 + mir_wstrlen(isrList[i]->id.w));

	char *pBlob = (char *)mir_calloc(cbBlob);
	char *pCurBlob = pBlob;

	for (int i = 0; i < nCount; i++) {
		pCurBlob += mir_strlen(pCurBlob) + 1;

		mir_strcpy(pCurBlob, _T2A(isrList[i]->id.w));
		pCurBlob += mir_strlen(pCurBlob) + 1;
	}

	dbei.szModule = m_szModuleName;
	dbei.eventType = EVENTTYPE_CONTACTS;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	db_event_add(hContact, &dbei);

	mir_free(pBlob);
	return 0;
}
