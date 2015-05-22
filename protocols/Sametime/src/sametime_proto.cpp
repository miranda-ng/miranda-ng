#include "StdAfx.h"
#include "sametime.h"

CSametimeProto::CSametimeProto(const char* pszProtoName, const TCHAR* tszUserName) :
	PROTO<CSametimeProto>(pszProtoName, tszUserName),
	is_idle(false), idle_status(false),
	first_online(true),
	idle_timerid(0),
	session(NULL),
	my_login_info(NULL),
	my_conference(NULL),
	service_places(NULL),
	service_conference(NULL),
	server_connection(0)
{
	// Register m_hNetlibUser user
	TCHAR name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);
	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_TCHAR | NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = name;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	RegisterGLibLogger();
	debugLog(_T("CSametimeProto::CSametimeProto() start  m_szModuleName=[%s], m_tszUserName=[%s]"), _A2T(m_szModuleName), m_tszUserName);

	SametimeInitIcons();

	CreateProtoService(PS_GETNAME, &CSametimeProto::GetName);
	CreateProtoService(PS_GETSTATUS, &CSametimeProto::GetStatus);
	CreateProtoService(PS_LOADICON, &CSametimeProto::SametimeLoadIcon);

	HookProtoEvent(ME_SYSTEM_MODULESLOADED, &CSametimeProto::OnModulesLoaded);
	HookProtoEvent(ME_SYSTEM_PRESHUTDOWN, &CSametimeProto::OnPreShutdown);
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CSametimeProto::OnWindowEvent);
	HookProtoEvent(ME_IDLE_CHANGED, &CSametimeProto::OnIdleChanged);
	HookProtoEvent(ME_DB_CONTACT_DELETED, &CSametimeProto::OnSametimeContactDeleted);
	HookProtoEvent(ME_OPT_INITIALISE, &CSametimeProto::OptInit);

	// Initialize temporary DB settings
	db_set_resident(m_szModuleName, "Status");
	db_set_resident(m_szModuleName, "IdleTS");

	RegisterPopups();
	InitAwayMsg();

	mir_snprintf(szProtoGroups, SIZEOF(szProtoGroups), "%s_GROUPS", m_szModuleName);

	m_iStatus = ID_STATUS_OFFLINE;
	previous_status = ID_STATUS_OFFLINE;
	SetAllOffline();

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0);

	InitConferenceMenu();
	InitSessionMenu();

	LoadOptions();

	debugLog(_T("CSametimeProto::CSametimeProto() end"));
}

CSametimeProto::~CSametimeProto()
{
	debugLog(_T("CSametimeProto::~CSametimeProto() start"));

	DeinitSessionMenu();
	DeinitConferenceMenu();

	DeinitAwayMsg();
	UnregisterPopups();

	debugLog(_T("CSametimeProto::~CSametimeProto() end"));

	UnRegisterGLibLogger();
	Netlib_CloseHandle(m_hNetlibUser);
}



MCONTACT CSametimeProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	MYPROTOSEARCHRESULT* sr = (MYPROTOSEARCHRESULT*)psr;
	debugLog(_T("CSametimeProto::AddToList()  flags=[%d]"), flags);
	return AddSearchedUser(sr, flags & PALF_TEMPORARY);
}

HANDLE CSametimeProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	debugLog(_T("CSametimeProto::FileAllow()  hContact=[%x], szPath=[%s]"), hContact, szPath);
	char* szPathA = mir_t2a(szPath);
	HANDLE res = AcceptFileTransfer(hContact, hTransfer, szPathA);
	mir_free(szPathA);
	return res;
}

int CSametimeProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	debugLog(_T("CSametimeProto::FileCancel()  hContact=[%x]"), hContact);
	CancelFileTransfer(hTransfer);
	return 0;
}

int CSametimeProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason)
{
	debugLog(_T("CSametimeProto::FileDeny()  hContact=[%x], szReason=[%s]"), hContact, szReason);
	RejectFileTransfer(hTransfer);
	return 0;
}

int CSametimeProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename)
{
	debugLog(_T("CSametimeProto::FileResume() action=[%d]"), &action);
	return 0;
}


DWORD_PTR CSametimeProto::GetCaps(int type, MCONTACT hContact)
{
	int ret = 0;
	switch (type) {
	case PFLAGNUM_1:
		ret = PF1_IM | PF1_BASICSEARCH | PF1_EXTSEARCHUI | PF1_ADDSEARCHRES | PF1_MODEMSG | PF1_FILE | PF1_CHAT;
		break;
	case PFLAGNUM_2:
		ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_LIGHTDND;
		break;
	case PFLAGNUM_3:
		ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_LIGHTDND;
		break;
	case PFLAGNUM_4:
		ret = PF4_SUPPORTTYPING;
		break;
	case PFLAG_UNIQUEIDTEXT:
		ret = (DWORD_PTR)Translate("ID");
		break;
	case PFLAG_MAXLENOFMESSAGE:
		ret = MAX_MESSAGE_SIZE;
		break;
	case PFLAG_UNIQUEIDSETTING:
		ret = (DWORD_PTR) "stid";
		break;
	}
	return ret;
}

int CSametimeProto::GetInfo(MCONTACT hContact, int infoType)
{
	// GetInfo - retrieves a contact info
	debugLog(_T("CSametimeProto::GetInfo()  hContact=[%x], infoType=[%d]"), hContact, infoType);

	if (getByte(hContact, "ChatRoom", 0))
		return 1;

	if (!session)
		return 1;

	///TODO unimplemented - getting contact info

	TFakeAckParams* tfap = (TFakeAckParams*)mir_alloc(sizeof(TFakeAckParams));
	tfap->proto = this;
	tfap->hContact = hContact;
	tfap->lParam = NULL;
	mir_forkthread(sttFakeAckInfoSuccessThread, (void*)tfap);

	return 0;
}

HANDLE CSametimeProto::SearchBasic(const PROTOCHAR* id)
{
	debugLog(_T("CSametimeProto::SearchBasic()  id:len=[%d]"), id == NULL ? -1 : mir_tstrlen(id));
	char* id_utf8 = mir_utf8encodeT(id);
	int ret = SearchForUser(id_utf8, FALSE);
	mir_free(id_utf8);
	return (HANDLE)ret;
	///TODO - add timeout (like at GGPROTO::searchthread)
}

HWND CSametimeProto::SearchAdvanced(HWND owner)
{
	TCHAR buf[512];
	int ret = 0;
	if (GetDlgItemText(owner, IDC_EDIT1, buf, SIZEOF(buf))) {
		debugLog(_T("CSametimeProto::SearchAdvanced()  buf:len=[%d]"), buf == NULL ? -1 : mir_tstrlen(buf));
		char* buf_utf8 = mir_utf8encodeT(buf);
		ret = SearchForUser(buf_utf8, TRUE);
		mir_free(buf_utf8);
	}
	return (HWND)ret;
}

HWND CSametimeProto::CreateExtendedSearchUI(HWND owner)
{
	debugLog(_T("CSametimeProto::CreateExtendedSearchUI() start"));
	return CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_USERSEARCH), owner, SearchDialogFunc, (LPARAM)this);
}


int CSametimeProto::RecvFile(MCONTACT hContact, PROTOFILEEVENT* pre)
{
	debugLog(_T("CSametimeProto::RecvFile()  hContact=[%x]"), hContact);

	db_unset(hContact, "CList", "Hidden");
	db_unset(hContact, "CList", "NotOnList");

	return Proto_RecvFile(hContact, pre);
}

int CSametimeProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	debugLog(_T("CSametimeProto::RecvMsg() hContact=[%x]"), hContact);

	db_unset(hContact, "CList", "Hidden");
	db_unset(hContact, "CList", "NotOnList");

	return Proto_RecvMessage(hContact, pre);
}

HANDLE CSametimeProto::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	debugLog(_T("CSametimeProto::SendFile()  hContact=[%x]"), hContact);
	if (m_iStatus != ID_STATUS_OFFLINE) {
		if (hContact && ppszFiles && szDescription) {
			if (db_get_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
				return SendFilesToUser(hContact, ppszFiles, szDescription);
			}
		}
	}
	return 0; // failure
}

int CSametimeProto::SendMsg(MCONTACT hContact, int, const char* msg)
{
	debugLog(_T("CSametimeProto::SendMsg()  hContact=[%x]"), hContact);

	char *proto = GetContactProto(hContact);
	int ret;

	if (!proto || strcmp(proto, m_szModuleName) != 0 || db_get_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
		TFakeAckParams* tfap = (TFakeAckParams*)mir_alloc(sizeof(TFakeAckParams));
		tfap->proto = this;
		tfap->hContact = hContact;
		tfap->lParam = 0;
		mir_forkthread(sttFakeAckMessageFailedThread, (void*)tfap);
		return 0;
	}

	if (!msg)
		return 0;

	ret = (int)SendMessageToUser(hContact, msg);

	TFakeAckParams *tfap = (TFakeAckParams*)mir_alloc(sizeof(TFakeAckParams));
	tfap->proto = this;
	tfap->hContact = hContact;
	tfap->lParam = (LPARAM)ret;
	mir_forkthread(sttFakeAckMessageSuccessThread, (void*)tfap);

	return ret;
}

int CSametimeProto::SetStatus(int iNewStatus)
{
	debugLog(_T("CSametimeProto::SetStatus()  m_iStatus=[%d], m_iDesiredStatus=[%d], iNewStatus=[%d]"), m_iStatus, m_iDesiredStatus, iNewStatus);
	m_iDesiredStatus = iNewStatus;
	if (iNewStatus != ID_STATUS_OFFLINE) {
		if (m_iStatus == ID_STATUS_OFFLINE)
			LogIn(iNewStatus, m_hNetlibUser);
		else
			SetSessionStatus(iNewStatus);
	}
	else if (m_iStatus != ID_STATUS_OFFLINE && iNewStatus == ID_STATUS_OFFLINE) {
		LogOut();
	}

	return 0;
}

HANDLE CSametimeProto::GetAwayMsg(MCONTACT hContact)
{
	debugLog(_T("CSametimeProto::GetInfo()  hContact=[%x], m_iStatus=[%d]"), hContact, m_iStatus);
	if (hContact && m_iStatus != ID_STATUS_OFFLINE) {
		TFakeAckParams* tfap;
		tfap = (TFakeAckParams*)malloc(sizeof(TFakeAckParams));
		tfap->proto = this;
		tfap->hContact = hContact;
		mir_forkthread(sttRecvAwayThread, (void*)tfap);
		return (HANDLE)1;
	}
	return NULL;
}

int CSametimeProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt)
{
	debugLog(_T("CSametimeProto::RecvAwayMsg()  hContact=[%x], mode=[%d]"), hContact, mode);

	ptrT pszMsg(mir_utf8decodeT(evt->szMessage));
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)evt->lParam, pszMsg);
	return 0;
}

int CSametimeProto::SetAwayMsg(int iStatus, const PROTOCHAR* msg)
{
	debugLog(_T("CSametimeProto::SetAwayMsg()  iStatus=[%d], msg:len=[%d]"), iStatus, msg == NULL ? -1 : mir_tstrlen(msg));
	SetSessionAwayMessage(iStatus, msg);
	return 0;
}

int CSametimeProto::UserIsTyping(MCONTACT hContact, int type)
{
	debugLog(_T("CSametimeProto::UserIsTyping()  hContact=[%x], type=[%d]"), hContact, type);
	SendTyping(hContact, type == PROTOTYPE_SELFTYPING_ON);
	return 0;
}

int CSametimeProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	switch (iEventType) {
	case EV_PROTO_ONOPTIONS:
		debugLog(_T("CSametimeProto::OnEvent() EV_PROTO_ONOPTIONS"));
		OptInit(wParam, lParam);
		break;
	}

	return TRUE;
}

