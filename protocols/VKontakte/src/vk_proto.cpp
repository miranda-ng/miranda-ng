/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static int sttCompareProtocols(const CVkProto *p1, const CVkProto *p2)
{
	return lstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CVkProto> vk_Instances(1, sttCompareProtocols);

static COLORREF sttColors[] = { 0, 1, 2, 3, 4, 5, 6 };

CVkProto::CVkProto(const char *szModuleName, const TCHAR *ptszUserName) :
	PROTO<CVkProto>(szModuleName, ptszUserName),
	m_arRequestsQueue(10),
	m_sendIds(3, PtrKeySortT),
	m_cookies(5),
	m_msgId(1),
	m_chats(1, NumericKeySortT)
{
	InitQueue();

	CreateProtoService(PS_CREATEACCMGRUI, &CVkProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETAVATARINFOT, &CVkProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CVkProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATART, &CVkProto::SvcGetMyAvatar);
	CreateProtoService(PS_SET_LISTENINGTO, &CVkProto::SvcSetListeningTo);

	HookProtoEvent(ME_OPT_INITIALISE, &CVkProto::OnOptionsInit);

	TCHAR descr[512];
	mir_sntprintf(descr, SIZEOF(descr), TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {sizeof(nlu)};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	m_defaultGroup = getTStringA("ProtoGroup");
	if (m_defaultGroup == NULL)
		m_defaultGroup = mir_tstrdup(_T("VKontakte"));
	Clist_CreateGroup(NULL, m_defaultGroup);

	db_set_resident(m_szModuleName, "Status");
	m_bOne = true;

	m_bServerDelivery = getBool("ServerDelivery", true);
	m_bHideChats = getBool("HideChats", true);
	m_bMesAsUnread = getBool("MesAsUnread", false);
	m_bAutoSyncHistory = getBool("AutoSyncHistory", true);
	m_bUseLocalTime = getBool("LocalTime", false);
	m_bReportAbuse = getBool("ReportAbuseOnBanUser", false);
	m_bClearServerHistory = getBool("ClearServerHistoryOnBanUser", false);
	m_bRemoveFromFrendlist = getBool("RemoveFromFrendlistOnBanUser", false);
	m_bRemoveFromClist = getBool("RemoveFromClistOnBanUser", false);
	m_bPopUpSyncHistory = getBool("PopUpSyncHistory", false);
	m_iMarkMessageReadOn = getByte("MarkMessageReadOn", 0);
	m_bAddImgBbc = getBool("AddImgBbc", false);
	m_bStikersAsSmyles = getBool("StikersAsSmyles", false);
	m_bUserForceOnlineOnActivity = getBool("UserForceOnlineOnActivity", false);
	m_bAudioStatusOnly = getBool("AudioStatusOnly", false);

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses(ID_STATUS_OFFLINE);
	vk_Instances.insert(this);
}

CVkProto::~CVkProto()
{
	Netlib_CloseHandle(m_hNetlibUser); m_hNetlibUser = NULL;
	UninitQueue();
	UnInitMenus();
	if (m_hPopupClassError)
		Popup_UnregisterClass(m_hPopupClassError);
	if (m_hPopupClassNotification)
		Popup_UnregisterClass(m_hPopupClassNotification);
	vk_Instances.remove(this);
}

int CVkProto::OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// Chats
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	gcr.nColors = SIZEOF(sttColors);
	gcr.pColors = sttColors;
	CallServiceSync(MS_GC_REGISTER, NULL, (LPARAM)&gcr);
	CreateProtoService(PS_LEAVECHAT, &CVkProto::OnLeaveChat);
	CreateProtoService(PS_JOINCHAT, &CVkProto::OnJoinChat);
	HookProtoEvent(ME_GC_EVENT, &CVkProto::OnChatEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &CVkProto::OnGcMenuHook);
	// Other hooks
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CVkProto::OnProcessSrmmEvent);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CVkProto::OnDbEventRead);

	InitPopups();
	InitMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menu support 

void CVkProto::InitMenus()
{
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CVkProto::OnPreBuildContactMenu);

	//Contact Menu Services
	CreateProtoService(PS_GETALLSERVERHISTORY, &CVkProto::SvcGetAllServerHistory);
	CreateProtoService(PS_VISITPROFILE, &CVkProto::SvcVisitProfile);
	CreateProtoService(PS_CREATECHAT, &CVkProto::SvcCreateChat);
	CreateProtoService(PS_ADDASFRIEND, &CVkProto::SvcAddAsFriend);
	CreateProtoService(PS_DELETEFRIEND, &CVkProto::SvcDeleteFriend);
	CreateProtoService(PS_BANUSER, &CVkProto::SvcBanUser);
	CreateProtoService(PS_REPORTABUSE, &CVkProto::SvcReportAbuse);
	CreateProtoService(PS_DESTROYKICKCHAT, &CVkProto::SvcDestroyKickChat);
	
	CLISTMENUITEM mi = { sizeof(mi) };
	char szService[100];

	// Proto menu
	mi.flags = CMIF_CHILDPOPUP;
	mi.hParentMenu = MO_GetProtoRootMenu(m_szModuleName);
	
	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_CREATECHAT);
	mi.pszService = szService;
	mi.position = 10009 + PMI_CREATECHAT;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_CHAT_JOIN);
	mi.pszName = LPGEN("Create new chat");
	g_hProtoMenuItems[PMI_CREATECHAT] = Menu_AddProtoMenuItem(&mi);
	
	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_VISITPROFILE);
	mi.pszService = szService;
	mi.position = 10009 + PMI_VISITPROFILE;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_VISITPROFILE));
	mi.pszName = LPGEN("Visit profile");
	g_hProtoMenuItems[PMI_VISITPROFILE] = Menu_AddProtoMenuItem(&mi);

	//Contact Menu Items
	mi.pszContactOwner = m_szModuleName;
	mi.flags = CMIF_TCHAR;

	mi.position = -200001000 + CMI_VISITPROFILE;
	mi.ptszName = LPGENT("Visit profile");
	g_hContactMenuItems[CMI_VISITPROFILE] = Menu_AddContactMenuItem(&mi);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_GETALLSERVERHISTORY);
	mi.position = -200001000 + CMI_GETALLSERVERHISTORY;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.ptszName = LPGENT("Reload all messages from vk.com");
	mi.pszService = szService;
	g_hContactMenuItems[CMI_GETALLSERVERHISTORY] = Menu_AddContactMenuItem(&mi);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_ADDASFRIEND);
	mi.position = -200001000 + CMI_ADDASFRIEND;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_FRIENDADD));
	mi.ptszName = LPGENT("Add as friend");
	mi.pszService = szService;
	g_hContactMenuItems[CMI_ADDASFRIEND] = Menu_AddContactMenuItem(&mi);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_DELETEFRIEND);
	mi.position = -200001000 + CMI_DELETEFRIEND;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_FRIENDDEL));
	mi.ptszName = LPGENT("Delete from friend list");
	mi.pszService = szService;
	g_hContactMenuItems[CMI_DELETEFRIEND] = Menu_AddContactMenuItem(&mi);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_BANUSER);
	mi.position = -200001000 + CMI_BANUSER;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_BAN));
	mi.ptszName = LPGENT("Ban user");
	mi.pszService = szService;
	g_hContactMenuItems[CMI_BANUSER] = Menu_AddContactMenuItem(&mi);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_REPORTABUSE);
	mi.position = -200001000 + CMI_REPORTABUSE;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_ABUSE));
	mi.ptszName = LPGENT("Report abuse");
	mi.pszService = szService;
	g_hContactMenuItems[CMI_REPORTABUSE] = Menu_AddContactMenuItem(&mi);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_DESTROYKICKCHAT);
	mi.position = -200001000 + CMI_DESTROYKICKCHAT;
	mi.icolibItem = Skin_GetIconByHandle(GetIconHandle(IDI_FRIENDDEL));
	mi.ptszName = LPGENT("Destroy room");
	mi.pszService = szService;
	g_hContactMenuItems[CMI_DESTROYKICKCHAT] = Menu_AddContactMenuItem(&mi);
}

int CVkProto::OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bisFriend = getByte(hContact, "Auth", -1)==0;
	
	Menu_ShowItem(g_hContactMenuItems[CMI_GETALLSERVERHISTORY], !isChatRoom(hContact));
	Menu_ShowItem(g_hContactMenuItems[CMI_VISITPROFILE], !isChatRoom(hContact));
	Menu_ShowItem(g_hContactMenuItems[CMI_ADDASFRIEND], !bisFriend&&!isChatRoom(hContact));
	Menu_ShowItem(g_hContactMenuItems[CMI_DELETEFRIEND], bisFriend);
	Menu_ShowItem(g_hContactMenuItems[CMI_BANUSER], !isChatRoom(hContact));
	Menu_ShowItem(g_hContactMenuItems[CMI_REPORTABUSE], !isChatRoom(hContact));
	Menu_ShowItem(g_hContactMenuItems[CMI_DESTROYKICKCHAT], isChatRoom(hContact)&&getBool(hContact, "off", false));
	return 0;
}

void CVkProto::UnInitMenus()
{
	for (int i = 0; i < PMI_COUNT; i++)
		CallService(MO_REMOVEMENUITEM, (WPARAM)g_hProtoMenuItems[i], 0);

	for (int i = 0; i < CMI_COUNT; i++)
		CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[i], 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PopUp support 

void CVkProto::InitPopups(void)
{
	TCHAR desc[256];
	char name[256];
	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;

	mir_sntprintf(desc, SIZEOF(desc), _T("%s %s"), m_tszUserName, TranslateT("Errors"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Error");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = LoadSkinnedIcon(SKINICON_ERROR);
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClassError = Popup_RegisterClass(&ppc);

	mir_sntprintf(desc, SIZEOF(desc), _T("%s %s"), m_tszUserName, TranslateT("Notification"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Notification");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle(IDI_NOTIFICATION));
	ppc.colorBack = RGB(190, 225, 255); //Blue
	ppc.colorText = RGB(255, 255, 255); //White
	ppc.iSeconds = 4;
	m_hPopupClassNotification = Popup_RegisterClass(&ppc);
}

void CVkProto::MsgPopup(MCONTACT hContact, const TCHAR *szMsg, const TCHAR *szTitle, bool err)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		char name[256];

		POPUPDATACLASS ppd = { sizeof(ppd) };
		ppd.ptszTitle = szTitle;
		ppd.ptszText = szMsg;
		ppd.pszClassName = name;
		ppd.hContact = hContact;
		mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, err ? "Error" : "Notification");

		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
	}
	else {
		DWORD mtype = MB_OK | MB_SETFOREGROUND | MB_ICONSTOP;
		MessageBox(NULL, szMsg, szTitle, mtype);
	}
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	m_bTerminated = true;
	SetEvent(m_evRequestsQueue);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR CVkProto::GetCaps(int type, MCONTACT hContact)
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_SEARCHBYNAME | PF1_SEARCHBYEMAIL | PF1_MODEMSG | PF1_FILESEND | PF1_FILERESUME;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return   PF4_IMSENDUTF | PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES;

	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;

	case PFLAG_MAXLENOFMESSAGE:
		return 4096;

	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)"VK ID";

	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"ID";
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{ 
	debugLogA("CVkProto::RecvMsg");
	Proto_RecvMessage(hContact, pre);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

void CVkProto::SendMsgAck(void *param)
{
	debugLogA("CVkProto::SendMsgAck");
	TFakeAckParams *ack = (TFakeAckParams*)param;
	Sleep(100);
	ProtoBroadcastAck(ack->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ack->msgid, 0);
	delete ack;
}

int CVkProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{ 
	debugLogA("CVkProto::SendMsg");
	if (!IsOnline())
		return 0;
	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1)
		return 0;

	ptrA szMsg;
	if (flags & PREF_UTF)
		szMsg = mir_strdup(msg);
	else if (flags & PREF_UNICODE)
		szMsg = mir_utf8encodeW((wchar_t*)&msg[strlen(msg)+1]);
	else
		szMsg = mir_utf8encode(msg);

	int StickerId = 0; 
	ptrA retMsg(GetStickerId(szMsg, StickerId));

	ULONG msgId = ::InterlockedIncrement(&m_msgId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage)
		<< INT_PARAM("user_id", userID)
		<< VER_API;

	if (StickerId != 0)
		pReq << INT_PARAM("sticker_id", StickerId);
	else
		pReq << CHAR_PARAM("message", szMsg);

	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->pUserInfo = new CVkSendMsgParam(hContact, msgId); 
	Push(pReq);

	if (!m_bServerDelivery)
		ForkThread(&CVkProto::SendMsgAck, new TFakeAckParams(hContact, msgId));
	
	if (retMsg){
		int _flags = flags | PREF_UTF;
		Sleep(330);
		SendMsg(hContact, _flags, retMsg);
	}
	return msgId;
}

void CVkProto::OnSendMessage(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	int iResult = ACKRESULT_FAILED;
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;

	debugLogA("CVkProto::OnSendMessage %d", reply->resultCode);
	if (reply->resultCode == 200) {
		JSONROOT pRoot;
		JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
		if (pResponse != NULL) {
			UINT mid = json_as_int(pResponse);
			if (param->iMsgID != -1)
				m_sendIds.insert((HANDLE)mid);
			if (mid>getDword(param->hContact, "lastmsgid", 0))
				setDword(param->hContact, "lastmsgid", mid);
			if (m_iMarkMessageReadOn >= markOnReply)
				MarkMessagesRead(param->hContact);
			iResult = ACKRESULT_SUCCESS;
		}
	}

	if (param->iMsgID == -1){
		CVkFileUploadParam *fup = (CVkFileUploadParam *)param->iCount;
		ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, iResult, (HANDLE)fup, 0);
		delete fup;
		return;
	} 
	else if (m_bServerDelivery)
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, iResult, HANDLE(param->iMsgID), 0);
	delete param;
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::SetStatus(int iNewStatus)
{
	debugLogA("CVkProto::SetStatus iNewStatus = %d,  m_iStatus = %d, m_iDesiredStatus = %d", iNewStatus, m_iStatus, m_iDesiredStatus);
	if (m_iDesiredStatus == iNewStatus || iNewStatus == ID_STATUS_IDLE)
		return 0;

	int oldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (IsOnline()) {
			SetServerStatus(ID_STATUS_OFFLINE);
			debugLogA("CVkProto::SetStatus ShutdownSession");
			ShutdownSession();
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (m_hWorkerThread == NULL && !(m_iStatus >= ID_STATUS_CONNECTING && m_iStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		m_hWorkerThread = ForkThreadEx(&CVkProto::WorkerThread, 0, NULL);
	}
	else if (IsOnline())
		SetServerStatus(iNewStatus);
	else {
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		if (!(m_iStatus >= ID_STATUS_CONNECTING && m_iStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES))
			m_iDesiredStatus = m_iStatus;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch(event) {
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(wParam,lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam,lParam);

	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam,lParam);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////

MCONTACT CVkProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	debugLogA("CVkProto::AddToList");

	int uid = _ttoi(psr->id);
	if (!uid)
		return NULL;

	MCONTACT hContact = FindUser(uid, true);
	RetrieveUserInfo(uid);
	return hContact;
}

int CVkProto::AuthRequest(MCONTACT hContact,const PROTOCHAR* message)
{
	debugLogA("CVkProto::AuthRequest");
	if (!IsOnline())
		return 1;
	bool bIsFriend = getByte(hContact, "Auth", -1)==0;
	LONG userID = getDword(hContact, "ID", -1);
	if ((userID == -1) || !hContact)
		return 1;
	
	TCHAR msg[501] = {0};
	if (message)
		_tcsncpy_s(msg, 500, message, _TRUNCATE);

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.add.json", true, &CVkProto::OnReceiveAuthRequest)
		<< INT_PARAM("user_id", userID)
		<< TCHAR_PARAM("text", msg)
		<< VER_API)->pUserInfo = new CVkSendMsgParam(hContact);

	return 0;
}

void CVkProto::OnReceiveAuthRequest(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveAuthRequest %d", reply->resultCode);
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	if (reply->resultCode == 200){
		JSONROOT pRoot;
		JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
		if (pResponse != NULL) {
			int iRet = json_as_int(pResponse);
			setByte(param->hContact, "Auth", 0);
			if (iRet == 2){
				CMString msg,
					msgformat = TranslateT("User %s added as friend"),
					tszNick = db_get_tsa(param->hContact, m_szModuleName, "Nick");
				if (tszNick.IsEmpty())
					tszNick = TranslateT("(Unknown contact)");
				msg.AppendFormat(msgformat, tszNick.GetBuffer());
				MsgPopup(param->hContact, msg.GetBuffer(), tszNick.GetBuffer());
			}
		} 
		else{
			switch (param->iCount){
			case VKERR_HIMSELF_AS_FRIEND:
				MsgPopup(param->hContact, TranslateT("You cannot add yourself as friend"), TranslateT("Error"), true);
				break;
			case VKERR_YOU_ON_BLACKLIST:
				MsgPopup(param->hContact, TranslateT("Cannot add this user to friends as they have put you on their blacklist"), TranslateT("Error"), true);
				break;
			case VKERR_USER_ON_BLACKLIST:
				MsgPopup(param->hContact, TranslateT("Cannot add this user to friends as you put him on blacklist"), TranslateT("Error"), true);
				break;
			}
		}
	}
	delete param;
}

int CVkProto::Authorize(HANDLE hDbEvent)
{
	debugLogA("CVkProto::Authorize");
	if (!IsOnline())
		return 1;
	MCONTACT hContact = MContactFromDbEvent(hDbEvent);
	if (hContact == -1)
		return 1;
	
	return AuthRequest(hContact, NULL);
}

int CVkProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR *reason)
{
	debugLogA("CVkProto::AuthDeny");
	if (!IsOnline())
		return 1;
	MCONTACT hContact = MContactFromDbEvent(hDbEvent);
	if (hContact == -1)
		return 1;

	return SvcDeleteFriend(hContact,(LPARAM)true);
}

int CVkProto::UserIsTyping(MCONTACT hContact, int type)
{ 
	debugLogA("CVkProto::UserIsTyping");
	if (PROTOTYPE_SELFTYPING_ON == type) {
		LONG userID = getDword(hContact, "ID", -1);
		if ((userID == -1)||(!IsOnline()))
			return 1;
		
		if (m_iMarkMessageReadOn == markOnTyping)
			MarkMessagesRead(hContact);
		
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.setActivity.json", true, &CVkProto::OnReceiveSmth)
			<< INT_PARAM("user_id", userID) 
			<< CHAR_PARAM("type", "typing")
			<< VER_API);
		return 0;
	}
	return 1;
}

MCONTACT CVkProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return NULL;
}

int CVkProto::AuthRecv(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int CVkProto::GetInfo(MCONTACT hContact, int infoType)
{
	debugLogA("CVkProto::GetInfo");
	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1)
		return 1;
	RetrieveUserInfo(userID);
	return 0;
}

int CVkProto::RecvContacts(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int CVkProto::RecvUrl(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int CVkProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	return 1;
}

int CVkProto::SendUrl(MCONTACT hContact,int flags,const char *url)
{
	return 1;
}

int CVkProto::SetApparentMode(MCONTACT hContact,int mode)
{
	return 1;
}

int CVkProto::RecvAwayMsg(MCONTACT hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}

HANDLE CVkProto::GetAwayMsg(MCONTACT hContact)
{
	return 0; // Status messages are disabled
}

int CVkProto::SetAwayMsg(int status, const PROTOCHAR *msg)
{
	return 0; // Status messages are disabled
}
