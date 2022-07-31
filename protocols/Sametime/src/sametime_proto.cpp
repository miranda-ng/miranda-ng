#include "StdAfx.h"
#include "sametime.h"

CSametimeProto::CSametimeProto(const char* pszProtoName, const wchar_t* tszUserName) :
	PROTO<CSametimeProto>(pszProtoName, tszUserName),
	is_idle(false), idle_status(false),
	first_online(true),
	idle_timerid(0),
	session(nullptr),
	my_login_info(nullptr),
	my_conference(nullptr),
	service_places(nullptr),
	service_conference(nullptr),
	server_connection(nullptr)
{
	// Register m_hNetlibUser user
	NETLIBUSER nlu = {};
	nlu.flags = NUF_UNICODE | NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	RegisterGLibLogger();
	debugLogW(L"CSametimeProto::CSametimeProto() start  m_szModuleName=[%S], m_tszUserName=[%s]", m_szModuleName, m_tszUserName);

	SametimeInitIcons();

	CreateProtoService(PS_GETNAME, &CSametimeProto::GetName);
	CreateProtoService(PS_LOADICON, &CSametimeProto::SametimeLoadIcon);

	HookProtoEvent(ME_MSG_WINDOWEVENT, &CSametimeProto::OnWindowEvent);
	HookProtoEvent(ME_IDLE_CHANGED, &CSametimeProto::OnIdleChanged);
	HookProtoEvent(ME_DB_CONTACT_DELETED, &CSametimeProto::OnSametimeContactDeleted);
	HookProtoEvent(ME_OPT_INITIALISE, &CSametimeProto::OptInit);

	// Initialize temporary DB settings
	db_set_resident(m_szModuleName, "IdleTS");

	RegisterPopups();
	InitAwayMsg();

	mir_snprintf(szProtoGroups, "%s_GROUPS", m_szModuleName);

	m_iStatus = ID_STATUS_OFFLINE;
	previous_status = ID_STATUS_OFFLINE;
	SetAllOffline();

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0);

	InitConferenceMenu();
	InitSessionMenu();

	LoadOptions();

	// register with chat module
	GCREGISTER gcr = {};
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = MAX_MESSAGE_SIZE;
	Chat_Register(&gcr);

	debugLogW(L"CSametimeProto::CSametimeProto() end");
}

CSametimeProto::~CSametimeProto()
{
	debugLogW(L"CSametimeProto::~CSametimeProto() start");

	DeinitAwayMsg();
	UnregisterPopups();

	debugLogW(L"CSametimeProto::~CSametimeProto() end");

	UnRegisterGLibLogger();
}

MCONTACT CSametimeProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	MYPROTOSEARCHRESULT* sr = (MYPROTOSEARCHRESULT*)psr;
	debugLogW(L"CSametimeProto::AddToList()  flags=[%d]", flags);
	return AddSearchedUser(sr, flags & PALF_TEMPORARY);
}

HANDLE CSametimeProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szPath)
{
	debugLogW(L"CSametimeProto::FileAllow()  hContact=[%x], szPath=[%s]", hContact, szPath);
	char* szPathA = mir_u2a(szPath);
	HANDLE res = AcceptFileTransfer(hContact, hTransfer, szPathA);
	mir_free(szPathA);
	return res;
}

int CSametimeProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	debugLogW(L"CSametimeProto::FileCancel()  hContact=[%x]", hContact);
	CancelFileTransfer(hTransfer);
	return 0;
}

int CSametimeProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szReason)
{
	debugLogW(L"CSametimeProto::FileDeny()  hContact=[%x], szReason=[%s]", hContact, szReason);
	RejectFileTransfer(hTransfer);
	return 0;
}

INT_PTR CSametimeProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_EXTSEARCHUI | PF1_ADDSEARCHRES | PF1_MODEMSG | PF1_FILE | PF1_CHAT;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_LIGHTDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_LIGHTDND;
	case PFLAGNUM_4:
		return PF4_SUPPORTTYPING;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("ID");
	case PFLAG_MAXLENOFMESSAGE:
		return MAX_MESSAGE_SIZE;
	default:
		return 0;
	}
}

int CSametimeProto::GetInfo(MCONTACT hContact, int infoType)
{
	// GetInfo - retrieves a contact info
	debugLogW(L"CSametimeProto::GetInfo()  hContact=[%x], infoType=[%d]", hContact, infoType);

	if (Contact::IsGroupChat(hContact))
		return 1;

	if (!session)
		return 1;

	ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0);
	return 0;
}

HANDLE CSametimeProto::SearchBasic(const wchar_t* id)
{
	debugLogW(L"CSametimeProto::SearchBasic()  id:len=[%d]", id == nullptr ? -1 : mir_wstrlen(id));
	return (HANDLE)SearchForUser(T2Utf(id), FALSE);
	///TODO - add timeout (like at GGPROTO::searchthread)
}

HWND CSametimeProto::SearchAdvanced(HWND owner)
{
	wchar_t buf[512];
	if (GetDlgItemText(owner, IDC_EDIT1, buf, _countof(buf))) {
		debugLogW(L"CSametimeProto::SearchAdvanced()  buf:len=[%d]", mir_wstrlen(buf));
		return (HWND)SearchForUser(T2Utf(buf), TRUE);
	}
	return nullptr;
}

HWND CSametimeProto::CreateExtendedSearchUI(HWND owner)
{
	debugLogW(L"CSametimeProto::CreateExtendedSearchUI() start");
	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_USERSEARCH), owner, SearchDialogFunc, (LPARAM)this);
}


int CSametimeProto::RecvFile(MCONTACT hContact, PROTORECVFILE* pre)
{
	debugLogW(L"CSametimeProto::RecvFile()  hContact=[%x]", hContact);

	Contact::Hide(hContact, false);
	Contact::PutOnList(hContact);

	return CSuper::RecvFile(hContact, pre);
}

MEVENT CSametimeProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	debugLogW(L"CSametimeProto::RecvMsg() hContact=[%x]", hContact);

	Contact::Hide(hContact, false);
	Contact::PutOnList(hContact);

	return CSuper::RecvMsg(hContact, pre);
}

HANDLE CSametimeProto::SendFile(MCONTACT hContact, const wchar_t* szDescription, wchar_t** ppszFiles)
{
	debugLogW(L"CSametimeProto::SendFile()  hContact=[%x]", hContact);
	if (m_iStatus != ID_STATUS_OFFLINE) {
		if (hContact && ppszFiles && szDescription) {
			if (db_get_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
				return SendFilesToUser(hContact, ppszFiles, szDescription);
			}
		}
	}
	return nullptr; // failure
}

int CSametimeProto::SendMsg(MCONTACT hContact, int, const char* msg)
{
	debugLogW(L"CSametimeProto::SendMsg()  hContact=[%x]", hContact);

	char *proto = Proto_GetBaseAccountName(hContact);
	if (!proto || mir_strcmp(proto, m_szModuleName) != 0 || db_get_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, 0);
		return 0;
	}

	if (!msg)
		return 0;

	int ret = (INT_PTR)SendMessageToUser(hContact, msg);
	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ret);
	return ret;
}

int CSametimeProto::SetStatus(int iNewStatus)
{
	debugLogW(L"CSametimeProto::SetStatus()  m_iStatus=[%d], m_iDesiredStatus=[%d], iNewStatus=[%d]", m_iStatus, m_iDesiredStatus, iNewStatus);
	m_iDesiredStatus = iNewStatus;
	if (iNewStatus != ID_STATUS_OFFLINE) {
		if (m_iStatus == ID_STATUS_OFFLINE)
			LogIn(iNewStatus);
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
	debugLogW(L"CSametimeProto::GetInfo()  hContact=[%x], m_iStatus=[%d]", hContact, m_iStatus);
	if (hContact && m_iStatus != ID_STATUS_OFFLINE) {
		TFakeAckParams* tfap;
		tfap = (TFakeAckParams*)malloc(sizeof(TFakeAckParams));
		tfap->proto = this;
		tfap->hContact = hContact;
		mir_forkThread<TFakeAckParams>(sttRecvAwayThread, tfap);
		return (HANDLE)1;
	}
	return nullptr;
}

int CSametimeProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt)
{
	debugLogW(L"CSametimeProto::RecvAwayMsg()  hContact=[%x], mode=[%d]", hContact, mode);

	ptrW pszMsg(mir_utf8decodeW(evt->szMessage));
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)evt->lParam, pszMsg);
	return 0;
}

int CSametimeProto::SetAwayMsg(int iStatus, const wchar_t* msg)
{
	debugLogW(L"CSametimeProto::SetAwayMsg()  iStatus=[%d], msg:len=[%d]", iStatus, msg == nullptr ? -1 : mir_wstrlen(msg));
	SetSessionAwayMessage(iStatus, msg);
	return 0;
}

int CSametimeProto::UserIsTyping(MCONTACT hContact, int type)
{
	debugLogW(L"CSametimeProto::UserIsTyping()  hContact=[%x], type=[%d]", hContact, type);
	SendTyping(hContact, type == PROTOTYPE_SELFTYPING_ON);
	return 0;
}
