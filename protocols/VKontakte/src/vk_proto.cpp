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

	m_bServerDelivery = getBool("ServerDelivery", true);
	m_bHideChats = getBool("HideChats", true);
	m_bMesAsUnread = getBool("MesAsUnread", false);
	m_bMarkReadOnReply = getBool("MarkReadOnReply", false);
	m_bMarkReadOnTyping = getBool("MarkReadOnTyping", false) && m_bMarkReadOnReply;
	m_bAutoSyncHistory = getBool("AutoSyncHistory", true);

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses(ID_STATUS_OFFLINE);
	vk_Instances.insert(this);
}

CVkProto::~CVkProto()
{
	Netlib_CloseHandle(m_hNetlibUser); m_hNetlibUser = NULL;
	UninitQueue();
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

	CreateProtoService(PS_CREATECHAT, &CVkProto::SvcCreateChat);

	HookProtoEvent(ME_GC_EVENT, &CVkProto::OnChatEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &CVkProto::OnGcMenuHook);

	char szService[100];
	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_CREATECHAT);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_CHILDPOPUP;
	mi.hParentMenu = MO_GetProtoRootMenu(m_szModuleName);
	mi.pszService = szService;
	mi.position = 10009;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_CHAT_JOIN);
	mi.pszName = LPGEN("Create new chat");
	Menu_AddProtoMenuItem(&mi);

	//Server History
	CreateProtoService(PS_GETALLSERVERHISTORY, &CVkProto::SvcGetAllServerHistory);

	mir_snprintf(szService, sizeof(szService), "%s%s", m_szModuleName, PS_GETALLSERVERHISTORY);
	mi.pszService = szService;
	mi.pszContactOwner = m_szModuleName;
	mi.flags = CMIF_TCHAR;
	mi.position = -200001000+1;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
	mi.position = -201001000+102;
	mi.ptszName = LPGENT("Reload all messages from vk.com");
	Menu_AddContactMenuItem(&mi);

	return 0;
}

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
		return PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_MODEMSG;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_FORCEADDED | PF4_IMSENDUTF | PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE;

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
	Proto_RecvMessage(hContact, pre);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

struct TFakeAckParams
{
	__inline TFakeAckParams(MCONTACT _hContact, int _msgid) :
		hContact(_hContact), msgid(_msgid)
		{}

	MCONTACT hContact;
	int msgid;
};

void CVkProto::SendMsgAck(void *param)
{
	TFakeAckParams *ack = (TFakeAckParams*)param;
	Sleep(100);
	ProtoBroadcastAck(ack->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ack->msgid, 0);
	delete ack;
}

int CVkProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{ 
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

	ULONG msgId = ::InterlockedIncrement(&m_msgId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage)
		<< INT_PARAM("user_id", userID)
		<< CHAR_PARAM("message", szMsg)
		<< VER_API;
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->pUserInfo = new CVkSendMsgParam(hContact, msgId); 
	Push(pReq);

	if (!m_bServerDelivery)
		ForkThread(&CVkProto::SendMsgAck, new TFakeAckParams(hContact, msgId));
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
			m_sendIds.insert((HANDLE)mid);
			if (mid>getDword(param->hContact, "lastmsgid", 0))
				setDword(param->hContact, "lastmsgid", mid);
			if (m_bMarkReadOnReply)
				MarkMessagesRead(param->hContact);
			iResult = ACKRESULT_SUCCESS;
		}
	}

	if (m_bServerDelivery)
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, iResult, HANDLE(param->iMsgID), 0);
	delete param;
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::SetStatus(int iNewStatus)
{
	if (m_iDesiredStatus == iNewStatus || iNewStatus == ID_STATUS_IDLE)
		return 0;

	int oldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if ( IsOnline()) {
			SetServerStatus(ID_STATUS_OFFLINE);
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
	else if ( IsOnline())
		SetServerStatus(iNewStatus);
	else 
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

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

HANDLE CVkProto::SearchBasic(const PROTOCHAR* id)
{
	return 0;
}

HANDLE CVkProto::SearchByEmail(const PROTOCHAR* email)
{
	return 0;
}

HANDLE CVkProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName)
{
	return 0;
}

MCONTACT CVkProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return NULL;
}

int CVkProto::AuthRequest(MCONTACT hContact,const PROTOCHAR *message)
{
	return 0;
}

int CVkProto::Authorize(HANDLE hDbEvent)
{
	return 1;
}

int CVkProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR *reason)
{
	return 1;
}

int CVkProto::UserIsTyping(MCONTACT hContact, int type)
{ 
	if (PROTOTYPE_SELFTYPING_ON == type) {
		LONG userID = getDword(hContact, "ID", -1);
		if ((userID == -1)||(!IsOnline()))
			return 1;
		
		if (m_bMarkReadOnTyping)
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

HANDLE CVkProto::FileAllow(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *path)
{
	return NULL;
}

int CVkProto::FileCancel(MCONTACT hContact,HANDLE hTransfer)
{
	return 1;
}

int CVkProto::FileDeny(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *reason)
{
	return 1;
}

int CVkProto::FileResume(HANDLE hTransfer,int *action,const PROTOCHAR **filename)
{
	return 1;
}

int CVkProto::GetInfo(MCONTACT hContact, int infoType)
{
	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1)
		return 1;
	RetrieveUserInfo(userID);
	return 0;
}

HWND CVkProto::SearchAdvanced(HWND owner)
{
	return NULL;
}

HWND CVkProto::CreateExtendedSearchUI(HWND owner)
{
	return NULL;
}

int CVkProto::RecvContacts(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int CVkProto::RecvFile(MCONTACT hContact,PROTORECVFILET *)
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

HANDLE CVkProto::SendFile(MCONTACT hContact,const PROTOCHAR *desc, PROTOCHAR **files)
{
	return NULL;
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
