// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Protocol Interface Implementation
// -----------------------------------------------------------------------------

#include "stdafx.h"

#include "m_icolib.h"

extern PLUGININFOEX pluginInfo;

#pragma warning(disable:4355)

static int CompareConns(const directconnect *p1, const directconnect *p2)
{
	if (p1 < p2)
		return -1;

	return (p1 == p2) ? 0 : 1;
}

static int CompareCookies(const icq_cookie_info *p1, const icq_cookie_info *p2)
{
	if (p1->dwCookie < p2->dwCookie)
		return -1;

	return (p1->dwCookie == p2->dwCookie) ? 0 : 1;
}

static int CompareFT(const filetransfer *p1, const filetransfer *p2)
{
	if (p1->dwCookie < p2->dwCookie)
		return -1;

	return (p1->dwCookie == p2->dwCookie) ? 0 : 1;
}

static int CompareContactsCache(const icq_contacts_cache *p1, const icq_contacts_cache *p2)
{
	if (p1->dwUin < p2->dwUin)
		return -1;

	if (p1->dwUin > p2->dwUin)
		return 1;

	return mir_strcmpi(p1->szUid, p2->szUid);
}

CIcqProto::CIcqProto(const char* aProtoName, const TCHAR* aUserName) :
	PROTO<CIcqProto>(aProtoName, aUserName),
	cookies(10, CompareCookies),
	directConns(10, CompareConns),
	expectedFileRecvs(10, CompareFT),
	contactsCache(10, CompareContactsCache),
	CustomCapList(1),
	cheekySearchId(-1),
	m_arAvatars(5)	
{
	debugLogA("Setting protocol/module name to '%s'", m_szModuleName);

	// Initialize server lists
	HookProtoEvent(ME_CLIST_GROUPCHANGE, &CIcqProto::ServListCListGroupChange);

	// Initialize status message struct
	memset(&m_modeMsgs, 0, sizeof(icq_mode_messages));

	m_modeMsgsEvent = CreateProtoEvent(ME_ICQ_STATUSMSGREQ);

	// Initialize cookies
	wCookieSeq = 2;

	// Initialize temporary DB settings
	db_set_resident(m_szModuleName, "Status"); // NOTE: XStatus cannot be temporary
	db_set_resident(m_szModuleName, "TemporaryVisible");
	db_set_resident(m_szModuleName, "TickTS");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, "AwayTS");
	db_set_resident(m_szModuleName, "LogonTS");
	db_set_resident(m_szModuleName, "DCStatus");
	db_set_resident(m_szModuleName, "CapBuf"); //capabilities bufer
	db_set_resident(m_szModuleName, DBSETTING_STATUS_NOTE_TIME);
	db_set_resident(m_szModuleName, DBSETTING_STATUS_MOOD);

	// Setup services
	CreateProtoService(PS_CREATEACCMGRUI, &CIcqProto::OnCreateAccMgrUI);
	CreateProtoService(MS_ICQ_SENDSMS, &CIcqProto::SendSms);
	CreateProtoService(PS_SETMYNICKNAME, &CIcqProto::SetNickName);

	CreateProtoService(PS_GETMYAWAYMSG, &CIcqProto::GetMyAwayMsg);

	CreateProtoService(PS_GETINFOSETTING, &CIcqProto::GetInfoSetting);

	CreateProtoService(PSS_ADDED, &CIcqProto::SendYouWereAdded);

	// ChangeInfo API
	CreateProtoService(PS_CHANGEINFOEX, &CIcqProto::ChangeInfoEx);

	// Avatar API
	CreateProtoService(PS_GETAVATARINFO, &CIcqProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CIcqProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CIcqProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CIcqProto::SetMyAvatar);

	// Custom Status API
	CreateProtoService(PS_SETCUSTOMSTATUSEX, &CIcqProto::SetXStatusEx);
	CreateProtoService(PS_GETCUSTOMSTATUSEX, &CIcqProto::GetXStatusEx);
	CreateProtoService(PS_GETCUSTOMSTATUSICON, &CIcqProto::GetXStatusIcon);
	CreateProtoService(PS_GETADVANCEDSTATUSICON, &CIcqProto::RequestAdvStatusIconIdx);
	CreateProtoService(PS_ICQ_REQUESTCUSTOMSTATUS, &CIcqProto::RequestXStatusDetails);

	CreateProtoService(MS_ICQ_ADDSERVCONTACT, &CIcqProto::AddServerContact);

	CreateProtoService(MS_REQ_AUTH, &CIcqProto::RequestAuthorization);
	CreateProtoService(MS_GRANT_AUTH, &CIcqProto::GrantAuthorization);
	CreateProtoService(MS_REVOKE_AUTH, &CIcqProto::RevokeAuthorization);

	CreateProtoService(MS_XSTATUS_SHOWDETAILS, &CIcqProto::ShowXStatusDetails);

	// Custom caps
	CreateProtoService(PS_ICQ_ADDCAPABILITY, &CIcqProto::IcqAddCapability);
	CreateProtoService(PS_ICQ_CHECKCAPABILITY, &CIcqProto::IcqCheckCapability);

	// Reset a bunch of session specific settings
	UpdateGlobalSettings();
	ResetSettingsOnLoad();

	// Initialize Contacts Cache
	InitContactsCache();

	// Startup Auto Info-Update thread
	icq_InitInfoUpdate();

	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &CIcqProto::OnPreBuildStatusMenu);

	// Register netlib users
	NETLIBUSER nlu = { 0 };
	TCHAR szBuffer[MAX_PATH + 64];
	mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s server connection"), m_tszUserName);
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.ptszDescriptiveName = szBuffer;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szHttpGatewayHello = "http://http.proxy.icq.com/hello";
	nlu.szHttpGatewayUserAgent = NETLIB_USER_AGENT;
	nlu.pfnHttpGatewayInit = icq_httpGatewayInit;
	nlu.pfnHttpGatewayBegin = icq_httpGatewayBegin;
	nlu.pfnHttpGatewayWrapSend = icq_httpGatewayWrapSend;
	nlu.pfnHttpGatewayUnwrapRecv = icq_httpGatewayUnwrapRecv;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	char szP2PModuleName[MAX_PATH];
	mir_snprintf(szP2PModuleName, SIZEOF(szP2PModuleName), "%sP2P", m_szModuleName);
	mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s client-to-client connections"), m_tszUserName);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_TCHAR;
	nlu.ptszDescriptiveName = szBuffer;
	nlu.szSettingsModule = szP2PModuleName;
	nlu.minIncomingPorts = 1;
	m_hDirectNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	// Register custom database events
	DBEVENTTYPEDESCR eventType = { sizeof(eventType) };
	eventType.eventType = ICQEVENTTYPE_MISSEDMESSAGE;
	eventType.module = m_szModuleName;
	eventType.descr = "Missed message notifications";
	eventType.textService = ICQ_DB_GETEVENTTEXT_MISSEDMESSAGE;
	eventType.flags = DETF_HISTORY | DETF_MSGWINDOW;
	// for now keep default "message" icon
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&eventType);

	// Protocol instance is ready
	debugLogA("%s: Protocol instance '%s' created.", ICQ_PROTOCOL_NAME, m_szModuleName);
}

CIcqProto::~CIcqProto()
{
	m_bXStatusEnabled = 10; // block clist changing
	m_bMoodsEnabled = 10;

	// Serv-list update board clean-up
	FlushServerIDs();
	/// TODO: make sure server-list handler thread is not running
	/// TODO: save state of server-list update board to DB
	servlistPendingFlushOperations();
	SAFE_FREE((void**)&servlistQueueList);

	// Finalize avatars
	for (int i = 0; i < m_arAvatars.getCount(); i++)
		delete m_arAvatars[i];

	// NetLib clean-up
	NetLib_SafeCloseHandle(&m_hDirectNetlibUser);
	NetLib_SafeCloseHandle(&m_hNetlibUser);

	// Destroy hookable events
	if (m_modeMsgsEvent)
		DestroyHookableEvent(m_modeMsgsEvent);

	// Clean-up remaining protocol instance members
	UninitContactsCache();

	SAFE_FREE(&m_modeMsgs.szOnline);
	SAFE_FREE(&m_modeMsgs.szAway);
	SAFE_FREE(&m_modeMsgs.szNa);
	SAFE_FREE(&m_modeMsgs.szOccupied);
	SAFE_FREE(&m_modeMsgs.szDnd);
	SAFE_FREE(&m_modeMsgs.szFfc);

	debugLogA("%s: Protocol instance '%s' destroyed.", ICQ_PROTOCOL_NAME, m_szModuleName);
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoadedEx - performs hook registration

int CIcqProto::OnModulesLoaded(WPARAM, LPARAM)
{
	char pszP2PName[MAX_PATH];
	char pszGroupsName[MAX_PATH];
	char pszSrvGroupsName[MAX_PATH];
	char* modules[5] = { 0, 0, 0, 0, 0 };

	mir_snprintf(pszP2PName, SIZEOF(pszP2PName), "%sP2P", m_szModuleName);
	mir_snprintf(pszGroupsName, SIZEOF(pszGroupsName), "%sGroups", m_szModuleName);
	mir_snprintf(pszSrvGroupsName, SIZEOF(pszSrvGroupsName), "%sSrvGroups", m_szModuleName);
	modules[0] = m_szModuleName;
	modules[1] = pszP2PName;
	modules[2] = pszGroupsName;
	modules[3] = pszSrvGroupsName;
	CallService("DBEditorpp/RegisterModule", (WPARAM)modules, (LPARAM)4);

	HookProtoEvent(ME_OPT_INITIALISE, &CIcqProto::OnOptionsInit);
	HookProtoEvent(ME_USERINFO_INITIALISE, &CIcqProto::OnUserInfoInit);
	HookProtoEvent(ME_IDLE_CHANGED, &CIcqProto::OnIdleChanged);

	// Init extra optional modules
	ModuleLoad(0, 0);
	InitXStatusItems(FALSE);

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		DWORD bXStatus = getContactXStatus(hContact);
		if (bXStatus > 0)
			setContactExtraIcon(hContact, bXStatus);
	}

	return 0;
}

int CIcqProto::OnPreShutdown(WPARAM, LPARAM)
{
	// signal info update thread to stop
	icq_InfoUpdateCleanup();

	// Make sure all connections are closed
	CloseContactDirectConns(NULL);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_AddToList - adds a contact to the contact list

MCONTACT CIcqProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr == NULL) return 0;

	char szUid[MAX_PATH];
	if (psr->cbSize == sizeof(ICQSEARCHRESULT)) {
		ICQSEARCHRESULT *isr = (ICQSEARCHRESULT*)psr;
		if (isr->uin)
			return AddToListByUIN(isr->uin, flags);
		
		// aim contact
		if (isr->hdr.flags & PSR_UNICODE)
			unicode_to_ansi_static((WCHAR*)isr->hdr.id.t, szUid, MAX_PATH);
		else
			null_strcpy(szUid, (char*)isr->hdr.id.t, MAX_PATH);

		return (szUid[0] == 0) ? 0 : AddToListByUID(szUid, flags);
	}

	if (psr->flags & PSR_UNICODE)
		unicode_to_ansi_static((WCHAR*)psr->id.t, szUid, MAX_PATH);
	else
		null_strcpy(szUid, (char*)psr->id.t, MAX_PATH);

	if (szUid[0] == 0)
		return 0;
	if (IsStringUIN(szUid))
		return AddToListByUIN(atoi(szUid), flags);
	return AddToListByUID(szUid, flags);
}

MCONTACT __cdecl CIcqProto::AddToListByEvent(int flags, int iContact, MEVENT hDbEvent)
{
	DWORD uin = 0;
	uid_str uid = { 0 };

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
		return 0;

	dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob + 1);
	dbei.pBlob[dbei.cbBlob] = '\0';

	if (db_event_get(hDbEvent, &dbei))
		return 0; // failed to get event

	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return 0; // this event is not ours

	switch (dbei.eventType) {
	case EVENTTYPE_CONTACTS:
		{
			char *pbOffset = (char*)dbei.pBlob;
			char *pbEnd = pbOffset + dbei.cbBlob;
			for (int i = 0; i <= iContact; i++) {
				pbOffset += mir_strlen(pbOffset) + 1;  // Nick
				if (pbOffset >= pbEnd) break;
				if (i == iContact) { // we found the contact, get uid
					if (IsStringUIN((char*)pbOffset))
						uin = atoi((char*)pbOffset);
					else {
						uin = 0;
						mir_strcpy(uid, (char*)pbOffset);
					}
				}
				pbOffset += mir_strlen(pbOffset) + 1;  // Uin
				if (pbOffset >= pbEnd) break;
			}
		}
		break;

	case EVENTTYPE_AUTHREQUEST:
	case EVENTTYPE_ADDED:
		if (!getContactUid(DbGetAuthEventContact(&dbei), &uin, &uid))
			break;

	default:
		return 0;
	}

	if (uin != 0)
		return AddToListByUIN(uin, flags); // Success

	// add aim contact
	if (mir_strlen(uid))
		return AddToListByUID(uid, flags); // Success

	return NULL; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_AuthAllow - processes the successful authorization

int CIcqProto::Authorize(MEVENT hDbEvent)
{
	if (icqOnline() && hDbEvent) {
		MCONTACT hContact = HContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		DWORD uin;
		uid_str uid;
		if (getContactUid(hContact, &uin, &uid))
			return 1;

		icq_sendAuthResponseServ(uin, uid, 1, _T(""));

		delSetting(hContact, "Grant");

		return 0; // Success
	}

	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_AuthDeny - handles the unsuccessful authorization

int CIcqProto::AuthDeny(MEVENT hDbEvent, const TCHAR* szReason)
{
	if (icqOnline() && hDbEvent) {
		MCONTACT hContact = HContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		DWORD uin;
		uid_str uid;
		if (getContactUid(hContact, &uin, &uid))
			return 1;

		icq_sendAuthResponseServ(uin, uid, 0, szReason);

		if (db_get_b(hContact, "CList", "NotOnList", 0))
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);

		return 0; // Success
	}

	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int __cdecl CIcqProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT* pre)
{
	setContactHidden(hContact, 0);
	ICQAddRecvEvent(NULL, EVENTTYPE_AUTHREQUEST, pre, pre->lParam, (PBYTE)pre->szMessage, 0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CIcqProto::AuthRequest(MCONTACT hContact, const TCHAR* szMessage)
{
	if (!icqOnline())
		return 1;

	if (hContact) {
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(hContact, &dwUin, &szUid))
			return 1; // Invalid contact

		if (dwUin) {
			char *utf = tchar_to_utf8(szMessage);
			icq_sendAuthReqServ(dwUin, szUid, utf);
			SAFE_FREE(&utf);
			return 0; // Success
		}
	}

	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_FileAllow - starts a file transfer

HANDLE __cdecl CIcqProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath)
{
	DWORD dwUin;
	uid_str szUid;
	if (getContactUid(hContact, &dwUin, &szUid))
		return 0; // Invalid contact

	if (icqOnline() && hContact && szPath && hTransfer) { // approve old fashioned file transfer
		basic_filetransfer *ft = (basic_filetransfer *)hTransfer;

		if (!IsValidFileTransfer(ft))
			return 0; // Invalid transfer

		if (dwUin && ft->ft_magic == FT_MAGIC_ICQ) {
			filetransfer *ft = (filetransfer *)hTransfer;
			ft->szSavePath = tchar_to_utf8(szPath);
			{
				mir_cslock l(expectedFileRecvMutex);
				expectedFileRecvs.insert(ft);
			}

			// Was request received thru DC and have we a open DC, send through that
			if (ft->bDC && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0))
				icq_sendFileAcceptDirect(hContact, ft);
			else
				icq_sendFileAcceptServ(dwUin, ft, 0);

			return hTransfer; // Success
		}
		else if (ft->ft_magic == FT_MAGIC_OSCAR) { // approve oscar file transfer
			return oftFileAllow(hContact, hTransfer, szPath);
		}
	}

	return 0; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_FileCancel - cancels a file transfer

int __cdecl CIcqProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	DWORD dwUin;
	uid_str szUid;
	if (getContactUid(hContact, &dwUin, &szUid))
		return 1; // Invalid contact

	if (hContact && hTransfer) {
		basic_filetransfer *ft = (basic_filetransfer *)hTransfer;

		if (!IsValidFileTransfer(ft))
			return 1; // Invalid transfer

		if (dwUin && ft->ft_magic == FT_MAGIC_ICQ) { // cancel old fashioned file transfer
			icq_CancelFileTransfer((filetransfer*)hTransfer);
			return 0; // Success
		}
		else if (ft->ft_magic == FT_MAGIC_OSCAR) { // cancel oscar file transfer
			return oftFileCancel(hContact, hTransfer);
		}
	}

	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_FileDeny - denies a file transfer

int __cdecl CIcqProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason)
{
	int nReturnValue = 1;
	basic_filetransfer *ft = (basic_filetransfer*)hTransfer;

	DWORD dwUin;
	uid_str szUid;
	if (getContactUid(hContact, &dwUin, &szUid))
		return 1; // Invalid contact

	if (icqOnline() && hTransfer && hContact) {
		if (!IsValidFileTransfer(hTransfer))
			return 1; // Invalid transfer

		if (dwUin && ft->ft_magic == FT_MAGIC_ICQ) { // deny old fashioned file transfer
			filetransfer *ft = (filetransfer*)hTransfer;
			char *szReasonUtf = tchar_to_utf8(szReason);
			// Was request received thru DC and have we a open DC, send through that
			if (ft->bDC && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0))
				icq_sendFileDenyDirect(hContact, ft, szReasonUtf);
			else
				icq_sendFileDenyServ(dwUin, ft, szReasonUtf, 0);
			SAFE_FREE(&szReasonUtf);

			nReturnValue = 0; // Success
		}
		else if (ft->ft_magic == FT_MAGIC_OSCAR) { // deny oscar file transfer
			return oftFileDeny(hContact, hTransfer, szReason);
		}
	}
	// Release possible orphan structure
	SafeReleaseFileTransfer((void**)&ft);

	return nReturnValue;
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_FileResume - processes file renaming etc

int __cdecl CIcqProto::FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename)
{
	if (icqOnline() && hTransfer) {
		basic_filetransfer *ft = (basic_filetransfer *)hTransfer;

		if (!IsValidFileTransfer(ft))
			return 1; // Invalid transfer

		if (ft->ft_magic == FT_MAGIC_ICQ) {
			char *szFileNameUtf = tchar_to_utf8(*szFilename);
			icq_sendFileResume((filetransfer *)hTransfer, *action, szFileNameUtf);
			SAFE_FREE(&szFileNameUtf);
		}
		else if (ft->ft_magic == FT_MAGIC_OSCAR) {
			oftFileResume((oscar_filetransfer *)hTransfer, *action, *szFilename);
		}
		else
			return 1; // Failure

		return 0; // Success
	}

	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

DWORD_PTR __cdecl CIcqProto::GetCaps(int type, MCONTACT hContact)
{
	DWORD_PTR nReturn = 0;

	switch (type) {

	case PFLAGNUM_1:
		nReturn = PF1_IM | PF1_URL | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES |
			PF1_VISLIST | PF1_INVISLIST | PF1_MODEMSG | PF1_FILE | PF1_EXTSEARCH |
			PF1_EXTSEARCHUI | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME |
			PF1_ADDED | PF1_CONTACT;
		if (!m_bAimEnabled)
			nReturn |= PF1_NUMERICUSERID;
		if (m_bSsiEnabled && getByte("ServerAddRemove", DEFAULT_SS_ADDSERVER))
			nReturn |= PF1_SERVERCLIST;
		break;

	case PFLAGNUM_2:
		nReturn = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND |
			PF2_FREECHAT | PF2_INVISIBLE;
		if (m_bAimEnabled)
			nReturn |= PF2_ONTHEPHONE;
		break;

	case PFLAGNUM_3:
		nReturn = PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND |
			PF2_FREECHAT | PF2_INVISIBLE;
		break;

	case PFLAGNUM_4:
		nReturn = PF4_SUPPORTIDLE | PF4_IMSENDOFFLINE | PF4_INFOSETTINGSVC;
		if (m_bAvatarsEnabled)
			nReturn |= PF4_AVATARS;
#ifdef DBG_CAPMTN
		nReturn |= PF4_SUPPORTTYPING;
#endif
		break;

	case PFLAGNUM_5:
		nReturn = PF2_FREECHAT;
		if (m_bAimEnabled)
			nReturn |= PF2_ONTHEPHONE;
		break;

	case PFLAG_UNIQUEIDTEXT:
		nReturn = (DWORD_PTR)Translate("User ID");
		break;

	case PFLAG_UNIQUEIDSETTING:
		nReturn = (DWORD_PTR)UNIQUEIDSETTING;
		break;

	case PFLAG_MAXCONTACTSPERPACKET:
		if (hContact) { // determine per contact
			BYTE bClientId = getByte(hContact, "ClientID", CLID_GENERIC);

			if (bClientId == CLID_MIRANDA) {
				if (CheckContactCapabilities(hContact, CAPF_CONTACTS) && getContactStatus(hContact) != ID_STATUS_OFFLINE)
					nReturn = 0x100; // limited only by packet size
				else
					nReturn = MAX_CONTACTSSEND;
			}
			else if (bClientId == CLID_ICQ6) {
				if (CheckContactCapabilities(hContact, CAPF_CONTACTS))
					nReturn = 1; // crapy ICQ6 cannot handle multiple contacts in the transfer
				else
					nReturn = 0; // this version does not support contacts transfer at all
			}
			else
				nReturn = MAX_CONTACTSSEND;
		}
		else // return generic limit
			nReturn = MAX_CONTACTSSEND;
		break;

	case PFLAG_MAXLENOFMESSAGE:
		nReturn = MAX_MESSAGESNACSIZE - 102;
	}

	return nReturn;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int __cdecl CIcqProto::GetInfo(MCONTACT hContact, int infoType)
{
	if (icqOnline()) {
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(hContact, &dwUin, &szUid))
			return 1; // Invalid contact

		DWORD dwCookie;
		if (dwUin)
			dwCookie = icq_sendGetInfoServ(hContact, dwUin, (infoType & SGIF_ONOPEN) != 0);
		else // TODO: this needs something better
			dwCookie = icq_sendGetAimProfileServ(hContact, szUid);

		return (dwCookie) ? 0 : 1;
	}

	return 1; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by UID

void CIcqProto::CheekySearchThread(void*)
{
	char szUin[UINMAXLEN];
	ICQSEARCHRESULT isr = { 0 };
	isr.hdr.cbSize = sizeof(isr);

	if (cheekySearchUin) {
		_itoa(cheekySearchUin, szUin, 10);
		isr.hdr.id.t = (TCHAR*)szUin;
	}
	else {
		isr.hdr.id.t = (TCHAR*)cheekySearchUid;
	}
	isr.uin = cheekySearchUin;

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)cheekySearchId, (LPARAM)&isr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)cheekySearchId, 0);
	cheekySearchId = -1;
}


HANDLE __cdecl CIcqProto::SearchBasic(const TCHAR *pszSearch)
{
	if (mir_wstrlen(pszSearch) == 0)
		return 0;

	char pszUIN[255];
	int nHandle = 0;
	size_t i, j;

	if (!m_bAimEnabled) {
		for (i = j = 0; (i < mir_wstrlen(pszSearch)) && (j < 255); i++) { // we take only numbers
			if ((pszSearch[i] >= 0x30) && (pszSearch[i] <= 0x39)) {
				pszUIN[j] = pszSearch[i];
				j++;
			}
		}
	}
	else {
		for (i = j = 0; (i < mir_wstrlen(pszSearch)) && (j < 255); i++) { // we remove spaces and slashes
			if ((pszSearch[i] != 0x20) && (pszSearch[i] != '-')) {
				if (pszSearch[i] >= 0x80) continue;
				pszUIN[j] = pszSearch[i];
				j++;
			}
		}
	}
	pszUIN[j] = 0;

	if (mir_strlen(pszUIN)) {
		DWORD dwUin;
		if (IsStringUIN(pszUIN))
			dwUin = atoi(pszUIN);
		else
			dwUin = 0;

		// Cheeky instant UIN search
		if (!dwUin || GetKeyState(VK_CONTROL) & 0x8000) {
			cheekySearchId = GenerateCookie(0);
			cheekySearchUin = dwUin;
			cheekySearchUid = null_strdup(pszUIN);
			ForkThread(&CIcqProto::CheekySearchThread, 0); // The caller needs to get this return value before the results
			nHandle = cheekySearchId;
		}
		else if (icqOnline()) {
			nHandle = SearchByUin(dwUin);
		}

		// Success
		return (HANDLE)nHandle;
	}

	// Failure
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE __cdecl CIcqProto::SearchByEmail(const TCHAR *email)
{
	if (email && icqOnline() && mir_wstrlen(email) > 0) {
		char *szEmail = tchar_to_ansi(email);

		// Success
		DWORD dwSearchId = SearchByMail(szEmail);
		DWORD dwSecId = (dwSearchId == 0 && m_bAimEnabled) ? icq_searchAimByEmail(szEmail, dwSearchId) : 0;

		SAFE_FREE(&szEmail);

		if (dwSearchId)
			return (HANDLE)dwSearchId;
		else
			return (HANDLE)dwSecId;
	}

	return 0; // Failure
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SearchByName - searches the contact by its first or last name, or by a nickname

HANDLE __cdecl CIcqProto::SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName)
{
	if (icqOnline()) {
		if (nick || firstName || lastName) {
			char *nickUtf = tchar_to_utf8(nick);
			char *firstNameUtf = tchar_to_utf8(firstName);
			char *lastNameUtf = tchar_to_utf8(lastName);

			// Success
			HANDLE dwCookie = (HANDLE)SearchByNames(nickUtf, firstNameUtf, lastNameUtf, 0);

			SAFE_FREE(&nickUtf);
			SAFE_FREE(&firstNameUtf);
			SAFE_FREE(&lastNameUtf);

			return dwCookie;
		}
	}

	return 0; // Failure
}


HWND __cdecl CIcqProto::CreateExtendedSearchUI(HWND parent)
{
	if (parent && hInst)
		return CreateDialog(hInst, MAKEINTRESOURCE(IDD_ICQADVANCEDSEARCH), parent, AdvancedSearchDlgProc);

	return NULL; // Failure
}

HWND __cdecl CIcqProto::SearchAdvanced(HWND hwndDlg)
{
	if (icqOnline() && IsWindow(hwndDlg)) {
		size_t nDataLen;
		BYTE* bySearchData;

		if (bySearchData = createAdvancedSearchStructure(hwndDlg, &nDataLen)) {
			int result = icq_sendAdvancedSearchServ(bySearchData, nDataLen);
			SAFE_FREE((void**)&bySearchData);
			return (HWND)result; // Success
		}
	}

	return NULL; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// RecvContacts

int __cdecl CIcqProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre)
{
	ICQSEARCHRESULT **isrList = (ICQSEARCHRESULT**)pre->szMessage;
	int i;
	size_t cbBlob = 0;
	DWORD flags = DBEF_UTF;

	for (i = 0; i < pre->lParam; i++) {
		cbBlob += mir_strlen((char*)isrList[i]->hdr.nick.t) + 2; // both trailing zeros
		if (isrList[i]->uin)
			cbBlob += getUINLen(isrList[i]->uin);
		else
			cbBlob += mir_strlen((char*)isrList[i]->hdr.id.t);
	}
	PBYTE pBlob = (PBYTE)_alloca(cbBlob), pCurBlob;
	for (i = 0, pCurBlob = pBlob; i < pre->lParam; i++) {
		mir_strcpy((char*)pCurBlob, (char*)isrList[i]->hdr.nick.t);
		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
		if (isrList[i]->uin) {
			char szUin[UINMAXLEN];
			_itoa(isrList[i]->uin, szUin, 10);
			mir_strcpy((char*)pCurBlob, szUin);
		}
		else // aim contact
			mir_strcpy((char*)pCurBlob, (char*)isrList[i]->hdr.id.t);

		pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	}

	ICQAddRecvEvent(hContact, EVENTTYPE_CONTACTS, pre, cbBlob, pBlob, flags);
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

int __cdecl CIcqProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	size_t cbBlob = mir_strlen(pre->szMessage) + 1;
	ICQAddRecvEvent(hContact, EVENTTYPE_MESSAGE, pre, cbBlob, (PBYTE)pre->szMessage, DBEF_UTF);

	// stop contact from typing - some clients do not sent stop notify
	if (CheckContactCapabilities(hContact, CAPF_TYPING))
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CIcqProto::SendContacts(MCONTACT hContact, int, int nContacts, MCONTACT *hContactsList)
{
	if (hContact && hContactsList) {
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(hContact, &dwUin, &szUid)) // Invalid contact
			return ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "The receiver has an invalid user ID.");

		WORD wRecipientStatus = getContactStatus(hContact);

		// Failures
		DWORD dwCookie;
		if (!icqOnline())
			dwCookie = ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "You cannot send messages when you are offline.");
		else if (!hContactsList || (nContacts < 1) || (nContacts > MAX_CONTACTSSEND))
			dwCookie = ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "Bad data (internal error #1)");
		else { // OK
			if (CheckContactCapabilities(hContact, CAPF_CONTACTS) && wRecipientStatus != ID_STATUS_OFFLINE) { // Use the new format if possible
				struct icq_contactsend_s* contacts = NULL;

				// Format the data part and the names part
				// This is kinda messy, but there is no simple way to do it. First
				// we need to calculate the length of the packet.
				contacts = (struct icq_contactsend_s*)_alloca(sizeof(struct icq_contactsend_s)*nContacts);
				memset(contacts, 0, (sizeof(struct icq_contactsend_s) * nContacts));

				size_t nDataLen = 0, nNamesLen = 0;
				int i;
				for (i = 0; i < nContacts; i++) {
					uid_str szContactUid;

					if (!IsICQContact(hContactsList[i]))
						break; // Abort if a non icq contact is found
					if (getContactUid(hContactsList[i], &contacts[i].uin, &szContactUid))
						break; // Abort if invalid contact
					contacts[i].uid = contacts[i].uin ? NULL : null_strdup(szContactUid);
					contacts[i].szNick = NickFromHandleUtf(hContactsList[i]);
					nDataLen += getUIDLen(contacts[i].uin, contacts[i].uid) + 4;
					nNamesLen += mir_strlen(contacts[i].szNick) + 8;
				}

				if (i == nContacts) {
					debugLogA("Sending contacts to %s.", strUID(dwUin, szUid));

					// Do not calculate the exact size of the data packet - only the maximal size (easier)
					// Sumarize size of group information
					// - we do not utilize the full power of the protocol and send all contacts with group "General"
					//   just like ICQ6 does
					nDataLen += 9;
					nNamesLen += 9;

					// Create data structures
					icq_packet mData, mNames;
					mData.wPlace = 0;
					mData.pData = (LPBYTE)SAFE_MALLOC(nDataLen);
					mData.wLen = WORD(nDataLen);
					mNames.wPlace = 0;
					mNames.pData = (LPBYTE)SAFE_MALLOC(nNamesLen);

					// pack Group Name
					packWord(&mData, 7);
					packBuffer(&mData, (LPBYTE)"General", 7);
					packWord(&mNames, 7);
					packBuffer(&mNames, (LPBYTE)"General", 7);

					// all contacts in one group
					packWord(&mData, WORD(nContacts));
					packWord(&mNames, WORD(nContacts));
					for (i = 0; i < nContacts; i++) {
						uid_str szContactUid;
						if (contacts[i].uin)
							strUID(contacts[i].uin, szContactUid);
						else
							mir_strcpy(szContactUid, contacts[i].uid);

						// prepare UID
						size_t wLen = mir_strlen(szContactUid);
						packWord(&mData, WORD(wLen));
						packBuffer(&mData, (LPBYTE)szContactUid, wLen);

						// prepare Nick
						wLen = mir_strlen(contacts[i].szNick);
						packWord(&mNames, WORD(wLen + 4));
						packTLV(&mNames, 0x01, wLen, (LPBYTE)contacts[i].szNick);
					}

					// Cleanup temporary list
					for (i = 0; i < nContacts; i++) {
						SAFE_FREE(&contacts[i].szNick);
						SAFE_FREE(&contacts[i].uid);
					}

					// Rate check
					if (IsServerOverRate(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, RML_LIMIT)) { // rate is too high, the message will not go thru...
						SAFE_FREE((void**)&mData.pData);
						SAFE_FREE((void**)&mNames.pData);

						return ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "The message could not be delivered. You are sending too fast. Wait a while and try again.");
					}

					// Set up the ack type
					cookie_message_data *pCookieData = CreateMessageCookieData(MTYPE_CONTACTS, hContact, dwUin, FALSE);

					// AIM clients do not send acknowledgement
					if (!dwUin && pCookieData->nAckType == ACKTYPE_CLIENT)
						pCookieData->nAckType = ACKTYPE_SERVER;
					// Send the message
					dwCookie = icq_SendChannel2Contacts(dwUin, szUid, hContact, (char*)mData.pData, mData.wPlace, (char*)mNames.pData, mNames.wPlace, pCookieData);

					// This will stop the message dialog from waiting for the real message delivery ack
					if (pCookieData->nAckType == ACKTYPE_NONE) {
						SendProtoAck(hContact, dwCookie, ACKRESULT_SUCCESS, ACKTYPE_CONTACTS, NULL);
						// We need to free this here since we will never see the real ack
						// The actual cookie value will still have to be returned to the message dialog though
						ReleaseCookie(dwCookie);
					}
					// Release our buffers
					SAFE_FREE((void**)&mData.pData);
					SAFE_FREE((void**)&mNames.pData);
				}
				else {
					dwCookie = ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "Bad data (internal error #2)");
				}

				for (i = 0; i < nContacts; i++) {
					SAFE_FREE(&contacts[i].szNick);
					SAFE_FREE(&contacts[i].uid);
				}
			}
			else if (dwUin) { // old format is only understood by ICQ clients
				char szContactUin[UINMAXLEN];
				char szCount[17];
				struct icq_contactsend_s* contacts = NULL;
				uid_str szContactUid;
				int i;

				// Format the body
				// This is kinda messy, but there is no simple way to do it. First
				// we need to calculate the length of the packet.
				contacts = (struct icq_contactsend_s*)_alloca(sizeof(struct icq_contactsend_s)*nContacts);
				memset(contacts, 0, (sizeof(struct icq_contactsend_s) * nContacts));
				{
					size_t nBodyLength = 0;
					for (i = 0; i < nContacts; i++) {
						if (!IsICQContact(hContactsList[i]))
							break; // Abort if a non icq contact is found
						if (getContactUid(hContactsList[i], &contacts[i].uin, &szContactUid))
							break; // Abort if invalid contact
						contacts[i].uid = contacts[i].uin ? NULL : null_strdup(szContactUid);
						contacts[i].szNick = NickFromHandle(hContactsList[i]);
						// Compute this contact's length
						nBodyLength += getUIDLen(contacts[i].uin, contacts[i].uid) + 1;
						nBodyLength += mir_strlen(contacts[i].szNick) + 1;
					}

					if (i == nContacts) {
						debugLogA("Sending contacts to %d.", dwUin);

						// Compute count record's length
						_itoa(nContacts, szCount, 10);
						nBodyLength += mir_strlen(szCount) + 1;

						// Finally we need to copy the contact data into the packet body
						char *pBody, *pBuffer = pBody = (char *)SAFE_MALLOC(nBodyLength);
						null_strcpy(pBuffer, szCount, nBodyLength - 1);
						pBuffer += mir_strlen(pBuffer);
						*pBuffer++ = -2;
						for (i = 0; i < nContacts; i++) {
							if (contacts[i].uin) {
								_itoa(contacts[i].uin, szContactUin, 10);
								mir_strcpy(pBuffer, szContactUin);
							}
							else
								mir_strcpy(pBuffer, contacts[i].uid);
							pBuffer += mir_strlen(pBuffer);
							*pBuffer++ = -2;
							mir_strcpy(pBuffer, contacts[i].szNick);
							pBuffer += mir_strlen(pBuffer);
							*pBuffer++ = -2;
						}

						for (i = 0; i < nContacts; i++) { // release memory
							SAFE_FREE(&contacts[i].szNick);
							SAFE_FREE(&contacts[i].uid);
						}

						// Set up the ack type
						cookie_message_data *pCookieData = CreateMessageCookieData(MTYPE_CONTACTS, hContact, dwUin, TRUE);

						if (m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0)) {
							int iRes = icq_SendDirectMessage(hContact, pBody, nBodyLength, pCookieData, NULL);
							if (iRes) {
								SAFE_FREE((void**)&pBody);
								return iRes; // we succeded, return
							}
						}

						// Rate check
						if (IsServerOverRate(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, RML_LIMIT)) { // rate is too high, the message will not go thru...
							SAFE_FREE((void**)&pCookieData);
							SAFE_FREE((void**)&pBody);

							return ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "The message could not be delivered. You are sending too fast. Wait a while and try again.");
						}

						// Select channel and send
						WORD wPriority;
						if (wRecipientStatus == ID_STATUS_ONLINE || wRecipientStatus == ID_STATUS_FREECHAT)
							wPriority = 0x0001;
						else
							wPriority = 0x0021;

						dwCookie = icq_SendChannel2Message(dwUin, hContact, pBody, nBodyLength, wPriority, pCookieData, NULL);

						// This will stop the message dialog from waiting for the real message delivery ack
						if (pCookieData->nAckType == ACKTYPE_NONE) {
							SendProtoAck(hContact, dwCookie, ACKRESULT_SUCCESS, ACKTYPE_CONTACTS, NULL);
							// We need to free this here since we will never see the real ack
							// The actual cookie value will still have to be returned to the message dialog though
							ReleaseCookie(dwCookie);
						}
						SAFE_FREE((void**)&pBody);
					}
					else dwCookie = ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "Bad data (internal error #2)");
				}
			}
			else dwCookie = ReportGenericSendError(hContact, ACKTYPE_CONTACTS, "The reciever does not support receiving of contacts.");
		}
		return dwCookie;
	}

	// Exit with Failure
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE __cdecl CIcqProto::SendFile(MCONTACT hContact, const TCHAR* szDescription, TCHAR** ppszFiles)
{
	if (!icqOnline())
		return 0;

	if (hContact && szDescription && ppszFiles) {
		DWORD dwUin;
		uid_str szUid;
		if (getContactUid(hContact, &dwUin, &szUid))
			return 0; // Invalid contact

		if (getContactStatus(hContact) != ID_STATUS_OFFLINE) {
			if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE))
				return oftInitTransfer(hContact, dwUin, szUid, (LPCTSTR*)ppszFiles, szDescription);

			if (dwUin) {
				WORD wClientVersion = getWord(hContact, "Version", 7);

				if (wClientVersion < 7)
					debugLogA("IcqSendFile() can't send to version %u", wClientVersion);
				else {
					int i;
					filetransfer* ft;
					struct _stat statbuf;

					// Initialize filetransfer struct
					ft = CreateFileTransfer(hContact, dwUin, (wClientVersion == 7) ? 7 : 8);

					for (ft->dwFileCount = 0; ppszFiles[ft->dwFileCount]; ft->dwFileCount++);
					ft->pszFiles = (char **)SAFE_MALLOC(sizeof(char *)* ft->dwFileCount);
					ft->dwTotalSize = 0;
					for (i = 0; i < (int)ft->dwFileCount; i++) {
						ft->pszFiles[i] = (ppszFiles[i]) ? tchar_to_utf8(ppszFiles[i]) : NULL;

						if (_tstat(ppszFiles[i], &statbuf))
							debugLogA("IcqSendFile() was passed invalid filename(s)");
						else
							ft->dwTotalSize += statbuf.st_size;
					}
					ft->szDescription = tchar_to_utf8(szDescription);
					ft->dwTransferSpeed = 100;
					ft->sending = 1;
					ft->fileId = -1;
					ft->iCurrentFile = 0;
					ft->dwCookie = AllocateCookie(CKT_FILE, 0, hContact, ft);
					ft->hConnection = NULL;

					// Send file transfer request
					debugLogA("Init file send");

					char *pszFiles, szFiles[64];
					if (ft->dwFileCount == 1) {
						pszFiles = strchr(ft->pszFiles[0], '\\');
						if (pszFiles)
							pszFiles++;
						else
							pszFiles = ft->pszFiles[0];
					}
					else {
						char tmp[64];
						mir_snprintf(szFiles, SIZEOF(szFiles), ICQTranslateUtfStatic("%d Files", tmp, SIZEOF(tmp)), ft->dwFileCount);
						pszFiles = szFiles;
					}

					// Send packet
					if (ft->nVersion == 7) {
						if (m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0)) {
							int iRes = icq_sendFileSendDirectv7(ft, pszFiles);
							if (iRes) return ft; // Success
						}
						debugLogA("Sending v%u file transfer request through server", 7);
						icq_sendFileSendServv7(ft, pszFiles);
					}
					else {
						if (m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0)) {
							int iRes = icq_sendFileSendDirectv8(ft, pszFiles);
							if (iRes)
								return ft; // Success
						}
						debugLogA("Sending v%u file transfer request through server", 8);
						icq_sendFileSendServv8(ft, pszFiles, ACKTYPE_NONE);
					}

					return ft; // Success
				}
			}
		}
	}

	return 0; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_SendMessage - sends a message

int __cdecl CIcqProto::SendMsg(MCONTACT hContact, int, const char* pszSrc)
{
	if (hContact == NULL || pszSrc == NULL)
		return NULL;

	DWORD dwCookie;
	char* puszText = (char*)pszSrc;
	int bNeedFreeU = 0;

	// Invalid contact
	DWORD dwUin;
	uid_str szUID;
	if (getContactUid(hContact, &dwUin, &szUID))
		return ReportGenericSendError(hContact, ACKTYPE_MESSAGE, "The receiver has an invalid user ID.");

	WORD wRecipientStatus = getContactStatus(hContact);

	BOOL plain_ascii = IsUSASCII(puszText, mir_strlen(puszText));
	BOOL oldAnsi = plain_ascii || !CheckContactCapabilities(hContact, CAPF_UTF) || !getByte(hContact, "UnicodeSend", 1);

	if (m_bTempVisListEnabled && m_iStatus == ID_STATUS_INVISIBLE)
		makeContactTemporaryVisible(hContact);  // make us temporarily visible to contact

	// Failure scenarios
	if (!icqOnline())
		dwCookie = ReportGenericSendError(hContact, ACKTYPE_MESSAGE, "You cannot send messages when you are offline.");
	else if ((wRecipientStatus == ID_STATUS_OFFLINE) && (mir_strlen(puszText) > 4096))
		dwCookie = ReportGenericSendError(hContact, ACKTYPE_MESSAGE, "Messages to offline contacts must be shorter than 4096 characters.");
	// Looks OK
	else {
		debugLogA("Send %smessage - Message cap is %u", puszText ? "unicode " : "", CheckContactCapabilities(hContact, CAPF_SRV_RELAY));
		debugLogA("Send %smessage - Contact status is %u", puszText ? "unicode " : "", wRecipientStatus);

		if (dwUin && m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0)) { // send thru direct
			char *dc_msg = puszText;
			char *dc_cap = plain_ascii ? NULL : CAP_UTF8MSGS;
			char *szUserAnsi = NULL;

			if (!plain_ascii && oldAnsi) {
				szUserAnsi = ConvertMsgToUserSpecificAnsi(hContact, puszText);
				if (szUserAnsi) {
					dc_msg = szUserAnsi;
					dc_cap = NULL;
				}
			}

			// Set up the ack type
			cookie_message_data *pCookieData = CreateMessageCookieData(MTYPE_PLAIN, hContact, dwUin, TRUE);
			pCookieData->nAckType = ACKTYPE_CLIENT;
			dwCookie = icq_SendDirectMessage(hContact, dc_msg, mir_strlen(dc_msg), pCookieData, dc_cap);

			SAFE_FREE(&szUserAnsi);
			if (dwCookie) // free the buffers if alloced
				return dwCookie; // we succeded, return

			// on failure, fallback to send thru server
		}

		/// TODO: add support for RTL & user customizable font
		{
			char *mng = MangleXml(puszText, mir_strlen(puszText));
			size_t len = mir_strlen(mng);
			mng = (char*)SAFE_REALLOC(mng, len + 28);
			memmove(mng + 12, mng, len + 1);
			memcpy(mng, "<HTML><BODY>", 12);
			mir_strcat(mng, "</BODY></HTML>");
			puszText = mng;
			bNeedFreeU = 1;
		}

		WCHAR *pwszText = plain_ascii ? NULL : make_unicode_string(puszText);
		if ((plain_ascii ? mir_strlen(puszText) : mir_wstrlen(pwszText) * sizeof(WCHAR)) > MAX_MESSAGESNACSIZE) { // max length check // TLV(2) is currently limited to 0xA00 bytes in online mode
			// only limit to not get disconnected, all other will be handled by error 0x0A
			dwCookie = ReportGenericSendError(hContact, ACKTYPE_MESSAGE, "The message could not be delivered, it is too long.");

			// free the buffers if alloced
			SAFE_FREE((void**)&pwszText);
			if (bNeedFreeU) SAFE_FREE(&puszText);

			return dwCookie;
		}
		// Rate check
		if (IsServerOverRate(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, RML_LIMIT)) { // rate is too high, the message will not go thru...
			dwCookie = ReportGenericSendError(hContact, ACKTYPE_MESSAGE, "The message could not be delivered. You are sending too fast. Wait a while and try again.");

			// free the buffers if alloced
			SAFE_FREE((void**)&pwszText);
			if (bNeedFreeU) SAFE_FREE(&puszText);

			return dwCookie;
		}

		cookie_message_data *pCookieData = CreateMessageCookieData(MTYPE_PLAIN, hContact, dwUin, FALSE);

		if (plain_ascii)
			dwCookie = icq_SendChannel1Message(dwUin, szUID, hContact, puszText, pCookieData);
		else
			dwCookie = icq_SendChannel1MessageW(dwUin, szUID, hContact, pwszText, pCookieData);
		// free the unicode message
		SAFE_FREE((void**)&pwszText);

		// This will stop the message dialog from waiting for the real message delivery ack
		if (pCookieData && pCookieData->nAckType == ACKTYPE_NONE) {
			SendProtoAck(hContact, dwCookie, ACKRESULT_SUCCESS, ACKTYPE_MESSAGE, NULL);
			// We need to free this here since we will never see the real ack
			// The actual cookie value will still have to be returned to the message dialog though
			ReleaseCookie(dwCookie);
		}
	}
	// free the buffers if alloced
	if (bNeedFreeU) SAFE_FREE(&puszText);

	return dwCookie; // Success
}


////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int __cdecl CIcqProto::SendUrl(MCONTACT hContact, int, const char* url)
{
	if (hContact == NULL || url == NULL)
		return 0;

	DWORD dwUin;
	if (getContactUid(hContact, &dwUin, NULL)) // Invalid contact
		return ReportGenericSendError(hContact, ACKTYPE_URL, "The receiver has an invalid user ID.");

	WORD wRecipientStatus = getContactStatus(hContact);

	// Failure
	if (!icqOnline()) {
		ReportGenericSendError(hContact, ACKTYPE_URL, "You cannot send messages when you are offline.");
		return 0;
	}

	// Set up the ack type
	cookie_message_data *pCookieData = CreateMessageCookieData(MTYPE_URL, hContact, dwUin, TRUE);

	// Format the body
	size_t nUrlLen = mir_strlen(url);
	char *szDesc = (char *)url + nUrlLen + 1;
	size_t nDescLen = mir_strlen(szDesc);
	size_t nBodyLen = nUrlLen + nDescLen + 2;
	char *szBody = (char *)_alloca(nBodyLen);
	mir_strcpy(szBody, szDesc);
	szBody[nDescLen] = -2; // Separator
	mir_strcpy(szBody + nDescLen + 1, url);

	if (m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0)) {
		int iRes = icq_SendDirectMessage(hContact, szBody, nBodyLen, pCookieData, NULL);
		if (iRes)
			return iRes; // we succeded, return
	}

	// Rate check
	if (IsServerOverRate(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND, RML_LIMIT)) { // rate is too high, the message will not go thru...
		SAFE_FREE((void**)&pCookieData);
		return ReportGenericSendError(hContact, ACKTYPE_URL, "The message could not be delivered. You are sending too fast. Wait a while and try again.");
	}

	// Select channel and send
	WORD wPriority;
	if (wRecipientStatus == ID_STATUS_ONLINE || wRecipientStatus == ID_STATUS_FREECHAT)
		wPriority = 0x0001;
	else
		wPriority = 0x0021;

	DWORD dwCookie = icq_SendChannel2Message(dwUin, hContact, szBody, nBodyLen, wPriority, pCookieData, NULL);

	// This will stop the message dialog from waiting for the real message delivery ack
	if (pCookieData->nAckType == ACKTYPE_NONE) {
		SendProtoAck(hContact, dwCookie, ACKRESULT_SUCCESS, ACKTYPE_URL, NULL);
		// We need to free this here since we will never see the real ack
		// The actual cookie value will still have to be returned to the message dialog though
		ReleaseCookie(dwCookie);
	}

	return dwCookie; // Success
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_SetApparentMode - sets the visibility status

int __cdecl CIcqProto::SetApparentMode(MCONTACT hContact, int mode)
{
	DWORD uin;
	uid_str uid;
	if (getContactUid(hContact, &uin, &uid))
		return 1; // Invalid contact

	if (hContact) {
		// Only 3 modes are supported
		if (mode == 0 || mode == ID_STATUS_ONLINE || mode == ID_STATUS_OFFLINE) {
			int oldMode = getWord(hContact, "ApparentMode", 0);

			// Don't send redundant updates
			if (mode != oldMode) {
				setWord(hContact, "ApparentMode", WORD(mode));

				// Not being online is only an error when in SS mode. This is not handled
				// yet so we just ignore this for now.
				if (icqOnline()) {
					if (oldMode != 0) { // Remove from old list
						if (oldMode == ID_STATUS_OFFLINE && getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0)) { // Need to remove Ignore item as well
							icq_removeServerPrivacyItem(hContact, uin, uid, getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0), SSI_ITEM_IGNORE);

							setWord(hContact, DBSETTING_SERVLIST_IGNORE, 0);
						}
						icq_sendChangeVisInvis(hContact, uin, uid, oldMode == ID_STATUS_OFFLINE, 0);
					}
					if (mode != 0) { // Add to new list
						if (mode == ID_STATUS_OFFLINE && getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0))
							return 0; // Success: offline by ignore item

						icq_sendChangeVisInvis(hContact, uin, uid, mode == ID_STATUS_OFFLINE, 1);
					}
				}

				return 0; // Success
			}
		}
	}

	return 1; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PrepareStatusNote - returns correct status note for given status

char* CIcqProto::PrepareStatusNote(int nStatus)
{
	char *szStatusNote = NULL;
	BYTE bXStatus = getContactXStatus(NULL);

	// use custom status message as status note
	if (bXStatus)
		szStatusNote = getSettingStringUtf(NULL, DBSETTING_XSTATUS_MSG, NULL);

	// get standard status message (no custom status defined)
	if (szStatusNote == NULL) {
		mir_cslock l(m_modeMsgsMutex);

		char **pszStatusNote = MirandaStatusToAwayMsg(nStatus);
		if (pszStatusNote)
			szStatusNote = null_strdup(*pszStatusNote);
	}

	if (szStatusNote == NULL) // nothing available. set empty status note
		szStatusNote = null_strdup("");

	return szStatusNote;
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_SetStatus - sets the protocol status

int __cdecl CIcqProto::SetStatus(int iNewStatus)
{
	int nNewStatus = MirandaStatusToSupported(iNewStatus);

	// check if netlib handles are ready
	if (!m_hNetlibUser)
		return 0;

	if (m_bTempVisListEnabled && icqOnline()) // remove temporary visible users
		sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_REMOVETEMPVISIBLE, BUL_TEMPVISIBLE);

	if (nNewStatus == m_iStatus)
		return 0;

	// clear custom status on status change
	if (getByte("XStatusReset", DEFAULT_XSTATUS_RESET))
		setXStatusEx(0, 0);

	// New status is OFFLINE
	if (nNewStatus == ID_STATUS_OFFLINE) { // for quick logoff
		if (icqOnline()) { // set offline status note (otherwise the old will remain)
			char *szOfflineNote = PrepareStatusNote(nNewStatus);
			SetStatusNote(szOfflineNote, 0, FALSE);
			SAFE_FREE(&szOfflineNote);
		}

		m_iDesiredStatus = nNewStatus;

		if (hServerConn) { // Connected, Send disconnect packet
			icq_sendCloseConnection();
			icq_serverDisconnect();

			m_bConnectionLost = false;
			debugLogA("Logged off.");
		}
	}
	else {
		switch (m_iStatus) {

			// We are offline and need to connect
		case ID_STATUS_OFFLINE:
			// Update user connection settings
			UpdateGlobalSettings();

			// Read UIN from database
			m_dwLocalUIN = getContactUin(NULL);
			if (m_dwLocalUIN == 0) {
				SetCurrentStatus(ID_STATUS_OFFLINE);
				ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
				icq_LogMessage(LOG_FATAL, LPGEN("You have not entered an ICQ number.\nConfigure this in Options->Network->ICQ and try again."));
				return 0;
			}

			// Set status to 'Connecting'
			m_iDesiredStatus = nNewStatus;
			SetCurrentStatus(ID_STATUS_CONNECTING);

			// Read password from database
			{
				char *pszPwd = GetUserPassword(FALSE);
				if (pszPwd)
					icq_login(pszPwd);
				else
					RequestPassword();
			}
			break;

			// We are connecting... We only need to change the going online status
		case ID_STATUS_CONNECTING:
			m_iDesiredStatus = nNewStatus;
			break;

			// We are already connected so we should just change status
		default:
			SetCurrentStatus(nNewStatus);

			char *szStatusNote = PrepareStatusNote(nNewStatus);

			//! This is a bit tricky, we do trigger status note change thread and then
			// change the status note right away (this spares one packet) - so SetStatusNote()
			// will only change User Details Directory
			SetStatusNote(szStatusNote, 6000, FALSE);

			if (m_iStatus == ID_STATUS_INVISIBLE) {
				if (m_bSsiEnabled)
					updateServVisibilityCode(3);
				icq_setstatus(MirandaStatusToIcq(m_iStatus), szStatusNote);
			}
			else {
				icq_setstatus(MirandaStatusToIcq(m_iStatus), szStatusNote);
				if (m_bSsiEnabled)
					updateServVisibilityCode(4);
			}
			SAFE_FREE(&szStatusNote);

			if (m_bAimEnabled) {
				mir_cslock l(m_modeMsgsMutex);

				char ** pszStatusNote = MirandaStatusToAwayMsg(m_iStatus);

				if (pszStatusNote)
					icq_sendSetAimAwayMsgServ(*pszStatusNote);
				else // clear the away message
					icq_sendSetAimAwayMsgServ(NULL);
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetAwayMsgThread - return a contact's status message

struct status_message_thread_data
{
	MCONTACT hContact;
	char *szMessage;
	HANDLE hProcess;
};

void __cdecl CIcqProto::GetAwayMsgThread(void *pStatusData)
{
	status_message_thread_data *pThreadData = (status_message_thread_data*)pStatusData;
	if (pThreadData) {
		// wait a little
		Sleep(100);

		setStatusMsgVar(pThreadData->hContact, pThreadData->szMessage, false);

		TCHAR *tszMsg = mir_utf8decodeT(pThreadData->szMessage);
		ProtoBroadcastAck(pThreadData->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, pThreadData->hProcess, (LPARAM)tszMsg);
		mir_free(tszMsg);

		SAFE_FREE(&pThreadData->szMessage);
		SAFE_FREE((void**)&pThreadData);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// PS_GetAwayMsg - returns a contact's away message

HANDLE __cdecl CIcqProto::GetAwayMsg(MCONTACT hContact)
{
	DWORD dwUin;
	uid_str szUID;
	if (getContactUid(hContact, &dwUin, &szUID))
		return 0; // Invalid contact

	if (!dwUin || !CheckContactCapabilities(hContact, CAPF_STATUS_MESSAGES)) { // No individual status messages, check if the contact has Status Note, if yes give it
		char *szStatusNote = getSettingStringUtf(hContact, DBSETTING_STATUS_NOTE, NULL);

		if (mir_strlen(szStatusNote) > 0) { // Give Status Note
			status_message_thread_data *pThreadData = (status_message_thread_data*)SAFE_MALLOC(sizeof(status_message_thread_data));

			pThreadData->hContact = hContact;
			pThreadData->szMessage = szStatusNote;
			pThreadData->hProcess = (HANDLE)GenerateCookie(0);
			ForkThread(&CIcqProto::GetAwayMsgThread, pThreadData);

			return pThreadData->hProcess;
		}
		SAFE_FREE(&szStatusNote);
	}

	if (!icqOnline())
		return 0;

	WORD wStatus = getContactStatus(hContact);

	if (dwUin) {
		int wMessageType = 0;

		switch (wStatus) {
		case ID_STATUS_ONLINE:
			if (CheckContactCapabilities(hContact, CAPF_STATUS_MESSAGES))
				wMessageType = MTYPE_AUTOONLINE;
			break;

		case ID_STATUS_AWAY:
			wMessageType = MTYPE_AUTOAWAY;
			break;

		case ID_STATUS_NA:
			wMessageType = MTYPE_AUTONA;
			break;

		case ID_STATUS_OCCUPIED:
			wMessageType = MTYPE_AUTOBUSY;
			break;

		case ID_STATUS_DND:
			wMessageType = MTYPE_AUTODND;
			break;

		case ID_STATUS_FREECHAT:
			wMessageType = MTYPE_AUTOFFC;
			break;
		}

		if (wMessageType) {
			if (m_bDCMsgEnabled && IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 0)) {
				int iRes = icq_sendGetAwayMsgDirect(hContact, wMessageType);
				if (iRes) return (HANDLE)iRes; // we succeded, return
			}

			WORD wVer = getWord(hContact, "Version", 0) <= 8 ? 8 : ICQ_VERSION;
			if (CheckContactCapabilities(hContact, CAPF_STATUS_MESSAGES))
				return (HANDLE)icq_sendGetAwayMsgServExt(hContact, dwUin, szUID, wMessageType, wVer);
			
			return (HANDLE)icq_sendGetAwayMsgServ(hContact, dwUin, wMessageType, wVer);
		}
	}
	else { // AIM contact
		if (wStatus == ID_STATUS_AWAY)
			return (HANDLE)icq_sendGetAimAwayMsgServ(hContact, szUID, MTYPE_AUTOAWAY);
	}

	return 0; // Failure
}


////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG - processes received status mode message

int __cdecl CIcqProto::RecvAwayMsg(MCONTACT hContact, int, PROTORECVEVENT* evt)
{
	setStatusMsgVar(hContact, evt->szMessage, false);

	TCHAR* pszMsg = mir_utf8decodeT(evt->szMessage);
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)evt->lParam, (LPARAM)pszMsg);
	mir_free(pszMsg);
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
// PS_SetAwayMsg - sets the away status message

int __cdecl CIcqProto::SetAwayMsg(int status, const TCHAR* msg)
{
	mir_cslock l(m_modeMsgsMutex);

	char **ppszMsg = MirandaStatusToAwayMsg(MirandaStatusToSupported(status));
	if (!ppszMsg)
		return 1; // Failure

	// Prepare UTF-8 status message
	char *szNewUtf = tchar_to_utf8(msg);

	if (mir_strcmp(szNewUtf, *ppszMsg)) {
		// Free old message
		SAFE_FREE(ppszMsg);

		// Set new message
		*ppszMsg = szNewUtf;
		szNewUtf = NULL;

		if ((m_iStatus == status) && icqOnline()) {	// update current status note
			char *szNote = *ppszMsg ? *ppszMsg : "";

			BYTE bXStatus = getContactXStatus(NULL);
			if (!bXStatus)
				SetStatusNote(szNote, 1000, FALSE);

			if (m_bAimEnabled)
				icq_sendSetAimAwayMsgServ(*ppszMsg);
		}
	}
	SAFE_FREE(&szNewUtf);

	return 0; // Success
}


/////////////////////////////////////////////////////////////////////////////////////////
// GetMyAwayMsg - obtain the current away message

INT_PTR CIcqProto::GetMyAwayMsg(WPARAM wParam, LPARAM lParam)
{
	mir_cslock l(m_modeMsgsMutex);

	char **ppszMsg = MirandaStatusToAwayMsg(wParam ? wParam : m_iStatus);

	if (!ppszMsg || !*ppszMsg)
		return 0;

	size_t nMsgLen = mir_strlen(*ppszMsg) + 1;

	if (lParam & SGMA_UNICODE) {
		WCHAR *szMsg = (WCHAR*)_alloca(nMsgLen * sizeof(WCHAR));

		make_unicode_string_static(*ppszMsg, szMsg, nMsgLen);
		return (INT_PTR)mir_wstrdup(szMsg);
	}
	else { // convert to ansi
		char *szMsg = (char*)_alloca(nMsgLen);

		if (utf8_decode_static(*ppszMsg, szMsg, nMsgLen))
			return (INT_PTR)mir_strdup(szMsg);
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// PS_UserIsTyping - sends a UTN notification

int __cdecl CIcqProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && icqOnline()) {
		if (CheckContactCapabilities(hContact, CAPF_TYPING)) {
			switch (type) {
			case PROTOTYPE_SELFTYPING_ON:
				sendTypingNotification(hContact, MTN_BEGUN);
				return 0;

			case PROTOTYPE_SELFTYPING_OFF:
				sendTypingNotification(hContact, MTN_FINISHED);
				return 0;
			}
		}
	}

	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////
// OnEvent - maintain protocol events

int __cdecl CIcqProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) {
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(0, 0);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(0, 0);

	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam, lParam);

	case EV_PROTO_ONERASE:
		{
			char szDbSetting[MAX_PATH];
			mir_snprintf(szDbSetting, SIZEOF(szDbSetting), "%sP2P", m_szModuleName);
			CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szDbSetting);
			mir_snprintf(szDbSetting, SIZEOF(szDbSetting), "%sSrvGroups", m_szModuleName);
			CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szDbSetting);
			mir_snprintf(szDbSetting, SIZEOF(szDbSetting), "%sGroups", m_szModuleName);
			CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szDbSetting);
		}
		break;

	case EV_PROTO_ONCONTACTDELETED:
		return ServListDbContactDeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return ServListDbSettingChanged(wParam, lParam);
	}
	return 1;
}
