/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "m_addcontact.h"
#include "jabber_disco.h"

/////////////////////////////////////////////////////////////////////////////////////////
// GetMyAwayMsg - obtain the current away message

INT_PTR __cdecl CJabberProto::GetMyAwayMsg(WPARAM wParam, LPARAM lParam)
{
	wchar_t *szStatus = nullptr;

	mir_cslock lck(m_csModeMsgMutex);
	switch (wParam ? (int)wParam : m_iStatus) {
	case ID_STATUS_ONLINE:
		szStatus = m_modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		szStatus = m_modeMsgs.szAway;
		break;
	case ID_STATUS_NA:
		szStatus = m_modeMsgs.szNa;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		szStatus = m_modeMsgs.szDnd;
		break;
	case ID_STATUS_FREECHAT:
		szStatus = m_modeMsgs.szFreechat;
		break;
	default: // Should not reach here
		break;
	}

	if (szStatus)
		return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_wstrdup(szStatus) : (INT_PTR)mir_u2a(szStatus);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatar - retrieves the file name of my own avatar

INT_PTR __cdecl CJabberProto::JabberGetAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t *buf = (wchar_t*)wParam;
	int size = (int)lParam;

	if (buf == nullptr || size <= 0)
		return -1;

	if (!m_options.EnableAvatars)
		return -2;

	GetAvatarFileName(0, buf, size);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarCaps - returns directives how to process avatars

INT_PTR __cdecl CJabberProto::JabberGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case AF_MAXSIZE:
		{
			POINT* size = (POINT*)lParam;
			if (size)
				size->x = size->y = 96;
		}
      return 0;

	case AF_PROPORTION:
		return PIP_NONE;

	case AF_FORMATSUPPORTED: // Jabber supports avatars of virtually all formats
		return 1;

	case AF_ENABLED:
		return m_options.EnableAvatars;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarInfo - retrieves the avatar info

INT_PTR __cdecl CJabberProto::JabberGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!m_options.EnableAvatars)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*)lParam;

	ptrA szHashValue( getStringA(pai->hContact, "AvatarHash"));
	if (szHashValue == nullptr) {
		debugLogA("No avatar");
		return GAIR_NOAVATAR;
	}

	wchar_t tszFileName[MAX_PATH];
	GetAvatarFileName(pai->hContact, tszFileName, _countof(tszFileName));
	wcsncpy_s(pai->filename, tszFileName, _TRUNCATE);

	pai->format = (pai->hContact == 0) ? PA_FORMAT_PNG : getByte(pai->hContact, "AvatarType", 0);

	if (::_waccess(pai->filename, 0) == 0) {
		ptrA szSavedHash( getStringA(pai->hContact, "AvatarSaved"));
		if (szSavedHash != nullptr && !mir_strcmp(szSavedHash, szHashValue)) {
			debugLogA("Avatar is Ok: %s == %s", szSavedHash, szHashValue);
			return GAIR_SUCCESS;
		}
	}

	if ((wParam & GAIF_FORCE) != 0 && pai->hContact != 0 && m_bJabberOnline) {
		ptrW tszJid( getWStringA(pai->hContact, "jid"));
		if (tszJid != nullptr) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, tszJid);
			if (item != nullptr) {
				BOOL isXVcard = getByte(pai->hContact, "AvatarXVcard", 0);

				wchar_t szJid[JABBER_MAX_JID_LEN]; szJid[0] = 0;
				if (item->arResources.getCount() != 0 && !isXVcard)
					if (wchar_t *bestResName = ListGetBestClientResourceNamePtr(tszJid))
						mir_snwprintf(szJid, L"%s/%s", tszJid, bestResName);

				if (szJid[0] == 0)
					wcsncpy_s(szJid, tszJid, _TRUNCATE);

				debugLogW(L"Rereading %s for %s", isXVcard ? JABBER_FEAT_VCARD_TEMP : JABBER_FEAT_AVATAR, szJid);

				m_ThreadInfo->send((isXVcard) ?
					XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetVCardAvatar, JABBER_IQ_TYPE_GET, szJid)) << XCHILDNS(L"vCard", JABBER_FEAT_VCARD_TEMP) :
					XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetClientAvatar, JABBER_IQ_TYPE_GET, szJid)) << XQUERY(JABBER_FEAT_AVATAR));
				return GAIR_WAITFOR;
			}
		}
	}

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberGetEventTextChatStates - retrieves a chat state description from an event

INT_PTR __cdecl CJabberProto::OnGetEventTextChatStates(WPARAM pEvent, LPARAM datatype)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)pEvent;
	if (dbei->cbBlob > 0) {
		if (dbei->pBlob[0] == JABBER_DB_EVENT_CHATSTATES_GONE) {
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("closed chat session"));
			
			return (INT_PTR)mir_strdup(Translate("closed chat session"));
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// OnGetEventTextPresence - retrieves presence state description from an event

INT_PTR __cdecl CJabberProto::OnGetEventTextPresence(WPARAM pEvent, LPARAM datatype)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)pEvent;
	if (dbei->cbBlob > 0) {
		switch (dbei->pBlob[0]) {
		case JABBER_DB_EVENT_PRESENCE_SUBSCRIBE:
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("sent subscription request"));
			return (INT_PTR)mir_strdup(Translate("sent subscription request"));

		case JABBER_DB_EVENT_PRESENCE_SUBSCRIBED:
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("approved subscription request"));
			return (INT_PTR)mir_strdup(Translate("approved subscription request"));

		case JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBE:
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("declined subscription"));
			return (INT_PTR)mir_strdup(Translate("declined subscription"));

		case JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBED:
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("declined subscription"));
			return (INT_PTR)mir_strdup(Translate("declined subscription"));

		case JABBER_DB_EVENT_PRESENCE_ERROR:
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("sent error presence"));
			return (INT_PTR)mir_strdup(Translate("sent error presence"));

		default:
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("sent unknown presence type"));
			return (INT_PTR)mir_strdup(Translate("sent unknown presence type"));
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetAvatar - sets an avatar without UI

INT_PTR __cdecl CJabberProto::JabberSetAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *tszFileName = (wchar_t*)lParam;

	if (m_bJabberOnline) {
		SetServerVcard(TRUE, tszFileName);
		SendPresence(m_iDesiredStatus, false);
	}
	else if (tszFileName == nullptr || tszFileName[0] == 0) {
		// Remove avatar
		wchar_t tFileName[ MAX_PATH ];
		GetAvatarFileName(0, tFileName, MAX_PATH);
		DeleteFile(tFileName);

		delSetting("AvatarSaved");
		delSetting("AvatarHash");
	}
	else {
		int fileIn = _wopen(tszFileName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE);
		if (fileIn == -1) {
			mir_free(tszFileName);
			return 1;
		}

		long dwPngSize = _filelength(fileIn);
		char *pResult = new char[ dwPngSize ];
		if (pResult == nullptr) {
			_close(fileIn);
			mir_free(tszFileName);
			return 2;
		}

		_read(fileIn, pResult, dwPngSize);
		_close(fileIn);

		BYTE digest[MIR_SHA1_HASH_SIZE];
		mir_sha1_ctx sha1ctx;
		mir_sha1_init(&sha1ctx);
		mir_sha1_append(&sha1ctx, (BYTE*)pResult, dwPngSize);
		mir_sha1_finish(&sha1ctx, digest);

		wchar_t tFileName[MAX_PATH];
		GetAvatarFileName(0, tFileName, MAX_PATH);
		DeleteFile(tFileName);

		char buf[MIR_SHA1_HASH_SIZE*2+1];
		bin2hex(digest, sizeof(digest), buf);

		m_options.AvatarType = ProtoGetBufferFormat(pResult);

		GetAvatarFileName(0, tFileName, MAX_PATH);
		FILE *out = _wfopen(tFileName, L"wb");
		if (out != nullptr) {
			fwrite(pResult, dwPngSize, 1, out);
			fclose(out);
		}
		delete[] pResult;

		setString("AvatarSaved", buf);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetNickname - sets the user nickname without UI

INT_PTR __cdecl CJabberProto::JabberSetNickname(WPARAM wParam, LPARAM lParam)
{
	wchar_t *nickname = (wParam & SMNN_UNICODE) ? mir_wstrdup((WCHAR*)lParam) : mir_a2u((char*)lParam);

	setWString("Nick", nickname);
	SetServerVcard(FALSE, L"");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// "/SendXML" - Allows external plugins to send XML to the server

INT_PTR __cdecl CJabberProto::ServiceSendXML(WPARAM, LPARAM lParam)
{
	return m_ThreadInfo->send((char*)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// "/GCGetToolTipText" - gets tooltip text

static const wchar_t *JabberEnum2AffilationStr[] = { LPGENW("None"), LPGENW("Outcast"), LPGENW("Member"), LPGENW("Admin"), LPGENW("Owner") },
	*JabberEnum2RoleStr[] = { LPGENW("None"), LPGENW("Visitor"), LPGENW("Participant"), LPGENW("Moderator") };

static void appendString(bool bIsTipper, const wchar_t *tszTitle, const wchar_t *tszValue, CMStringW &out)
{
	if (!out.IsEmpty())
		out.Append(bIsTipper ? L"\n" : L"\r\n");

	if (bIsTipper)
		out.AppendFormat(L"<b>%s</b>\t%s", TranslateW(tszTitle), tszValue);
	else {
		wchar_t *p = TranslateW(tszTitle);
		out.AppendFormat(L"%s%s\t%s", p, mir_wstrlen(p) <= 7 ? L"\t" : L"", tszValue);
	}
}

INT_PTR __cdecl CJabberProto::JabberGCGetToolTipText(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return 0; //room global tooltip not supported yet

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, (wchar_t*)wParam);
	if (item == nullptr)
		return 0;  //no room found

	pResourceStatus info( item->findResource((wchar_t*)lParam));
	if (info == nullptr)
		return 0; //no info found

	// ok process info output will be:
	// JID:			real@jid/resource or
	// Nick:		Nickname
	// Status:		StatusText
	// Role:		Moderator
	// Affiliation:  Affiliation

	bool bIsTipper = ServiceExists("mToolTip/HideTip");

	//JID:
	CMStringW outBuf;
	if (wcschr(info->m_tszResourceName, '@') != nullptr)
		appendString(bIsTipper, LPGENW("JID:"), info->m_tszResourceName, outBuf);
	else if (lParam) //or simple nick
		appendString(bIsTipper, LPGENW("Nick:"), (wchar_t*)lParam, outBuf);

	// status
	if (info->m_iStatus >= ID_STATUS_OFFLINE && info->m_iStatus <= ID_STATUS_IDLE )
		appendString(bIsTipper, LPGENW("Status:"), pcli->pfnGetStatusModeDescription(info->m_iStatus, 0), outBuf);

	// status text
	if (info->m_tszStatusMessage)
		appendString(bIsTipper, LPGENW("Status message:"), info->m_tszStatusMessage, outBuf);

	// Role
	appendString(bIsTipper, LPGENW("Role:"), TranslateW(JabberEnum2RoleStr[info->m_role]), outBuf);

	// Affiliation
	appendString(bIsTipper, LPGENW("Affiliation:"), TranslateW(JabberEnum2AffilationStr[info->m_affiliation]), outBuf);

	// real jid
	if (info->m_tszRealJid)
		appendString(bIsTipper, LPGENW("Real JID:"), info->m_tszRealJid, outBuf);

	return (outBuf.IsEmpty() ? 0 : (INT_PTR)mir_wstrdup(outBuf));
}

// File Association Manager plugin support
INT_PTR __cdecl CJabberProto::JabberServiceParseXmppURI(WPARAM, LPARAM lParam)
{
	wchar_t *arg = (wchar_t *)lParam;
	if (arg == nullptr)
		return 1;

	// skip leading prefix
	wchar_t szUri[ 1024 ];
	wcsncpy_s(szUri, arg, _TRUNCATE);
	wchar_t *szJid = wcschr(szUri, ':');
	if (szJid == nullptr)
		return 1;

	// skip //
	for (++szJid; *szJid == '/'; ++szJid);

	// empty jid?
	if (!*szJid)
		return 1;

	// command code
	wchar_t *szCommand = szJid;
	szCommand = wcschr(szCommand, '?');
	if (szCommand)
		*(szCommand++) = 0;

	// parameters
	wchar_t *szSecondParam = szCommand ? wcschr(szCommand, ';') : nullptr;
	if (szSecondParam)
		*(szSecondParam++) = 0;

	// no command or message command
	if (!szCommand || (szCommand && !mir_wstrcmpi(szCommand, L"message"))) {
		// message
		if (!ServiceExists(MS_MSG_SENDMESSAGEW))
			return 1;

		wchar_t *szMsgBody = nullptr;
		MCONTACT hContact = HContactFromJID(szJid, false);
		if (hContact == 0)
			hContact = DBCreateContact(szJid, szJid, true, true);
		if (hContact == 0)
			return 1;

		if (szSecondParam) { //there are parameters to message
			szMsgBody = wcsstr(szSecondParam, L"body=");
			if (szMsgBody) {
				szMsgBody += 5;
				wchar_t *szDelim = wcschr(szMsgBody, ';');
				if (szDelim)
					szDelim = 0;
				JabberHttpUrlDecode(szMsgBody);
			}
		}

		CallService(MS_MSG_SENDMESSAGEW, hContact, (LPARAM)szMsgBody);
		return 0;
	}
	
	if (!mir_wstrcmpi(szCommand, L"roster")) {
		if (!HContactFromJID(szJid)) {
			PROTOSEARCHRESULT psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.nick.w = szJid;
			psr.id.w = szJid;

			ADDCONTACTSTRUCT acs;
			acs.handleType = HANDLE_SEARCHRESULT;
			acs.szProto = m_szModuleName;
			acs.psr = &psr;
			CallService(MS_ADDCONTACT_SHOW, 0, (LPARAM)&acs);
		}
		return 0;
	}
	
	// chat join invitation
	if (!mir_wstrcmpi(szCommand, L"join")) {
		GroupchatJoinRoomByJid(nullptr, szJid);
		return 0;
	}
	
	// service discovery request
	if (!mir_wstrcmpi(szCommand, L"disco")) {
		OnMenuHandleServiceDiscovery(0, (LPARAM)szJid);
		return 0;
	}
	
	// ad-hoc commands
	if (!mir_wstrcmpi(szCommand, L"command")) {
		if (szSecondParam) {
			if (!wcsnicmp(szSecondParam, L"node=", 5)) {
				szSecondParam += 5;
				if (!*szSecondParam)
					szSecondParam = nullptr;
			}
			else szSecondParam = nullptr;
		}
		CJabberAdhocStartupParams* pStartupParams = new CJabberAdhocStartupParams(this, szJid, szSecondParam);
		ContactMenuRunCommands(0, (LPARAM)pStartupParams);
		return 0;
	}
	
	// send file
	if (!mir_wstrcmpi(szCommand, L"sendfile")) {
		MCONTACT hContact = HContactFromJID(szJid, false);
		if (hContact == 0)
			hContact = DBCreateContact(szJid, szJid, true, true);
		if (hContact == 0)
			return 1;
		CallService(MS_FILE_SENDFILE, hContact, 0);
		return 0;
	}

	return 1; /* parse failed */
}

// XEP-0224 support (Attention/Nudge)
INT_PTR __cdecl CJabberProto::JabberSendNudge(WPARAM hContact, LPARAM)
{
	if (!m_bJabberOnline)
		return 0;

	ptrW jid( getWStringA(hContact, "jid"));
	if (jid == nullptr)
		return 0;

	wchar_t tszJid[JABBER_MAX_JID_LEN];
	wchar_t *szResource = ListGetBestClientResourceNamePtr(jid);
	if (szResource)
		mir_snwprintf(tszJid, L"%s/%s", jid, szResource);
	else
		wcsncpy_s(tszJid, jid, _TRUNCATE);

	m_ThreadInfo->send(
		XmlNode(L"message") << XATTR(L"type", L"headline") << XATTR(L"to", tszJid)
			<< XCHILDNS(L"attention", JABBER_FEAT_ATTENTION));
	return 0;
}

BOOL CJabberProto::SendHttpAuthReply(CJabberHttpAuthParams *pParams, BOOL bAuthorized)
{
	if (!m_bJabberOnline || !pParams || !m_ThreadInfo)
		return FALSE;

	if (pParams->m_nType == CJabberHttpAuthParams::IQ) {
		XmlNodeIq iq(bAuthorized ? L"result" : L"error", pParams->m_szIqId, pParams->m_szFrom);
		if (!bAuthorized) {
			iq << XCHILDNS(L"confirm", JABBER_FEAT_HTTP_AUTH) << XATTR(L"id", pParams->m_szId)
					<< XATTR(L"method", pParams->m_szMethod) << XATTR(L"url", pParams->m_szUrl);
			iq << XCHILD(L"error") << XATTRI(L"code", 401) << XATTR(L"type", L"auth")
					<< XCHILDNS(L"not-authorized", L"urn:ietf:params:xml:xmpp-stanzas");
		}
		m_ThreadInfo->send(iq);
	}
	else if (pParams->m_nType == CJabberHttpAuthParams::MSG) {
		XmlNode msg(L"message");
		msg << XATTR(L"to", pParams->m_szFrom);
		if (!bAuthorized)
			msg << XATTR(L"type", L"error");
		if (pParams->m_szThreadId)
			msg << XCHILD(L"thread", pParams->m_szThreadId);

		msg << XCHILDNS(L"confirm", JABBER_FEAT_HTTP_AUTH) << XATTR(L"id", pParams->m_szId)
					<< XATTR(L"method", pParams->m_szMethod) << XATTR(L"url", pParams->m_szUrl);

		if (!bAuthorized)
			msg << XCHILD(L"error") << XATTRI(L"code", 401) << XATTR(L"type", L"auth")
					<< XCHILDNS(L"not-authorized", L"urn:ietf:params:xml:xmpp-stanzas");

		m_ThreadInfo->send(msg);
	}
	else return FALSE;

	return TRUE;
}

class CJabberDlgHttpAuth: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

public:
	CJabberDlgHttpAuth(CJabberProto *proto, HWND hwndParent, CJabberHttpAuthParams *pParams):
		CSuper(proto, IDD_HTTP_AUTH, true),
		m_txtInfo(this, IDC_EDIT_HTTP_AUTH_INFO),
		m_btnAuth(this, IDOK),
		m_btnDeny(this, IDCANCEL),
		m_pParams(pParams)
	{
		SetParent(hwndParent);

		m_btnAuth.OnClick = Callback(this, &CJabberDlgHttpAuth::btnAuth_OnClick);
		m_btnDeny.OnClick = Callback(this, &CJabberDlgHttpAuth::btnDeny_OnClick);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_GetIconHandle(IDI_OPEN));

		SetDlgItemText(m_hwnd, IDC_TXT_URL, m_pParams->m_szUrl);
		SetDlgItemText(m_hwnd, IDC_TXT_FROM, m_pParams->m_szFrom);
		SetDlgItemText(m_hwnd, IDC_TXT_ID, m_pParams->m_szId);
		SetDlgItemText(m_hwnd, IDC_TXT_METHOD, m_pParams->m_szMethod);
	}

	BOOL SendReply(BOOL bAuthorized)
	{
		BOOL bRetVal = m_proto->SendHttpAuthReply(m_pParams, bAuthorized);
		m_pParams->Free();
		mir_free(m_pParams);
		m_pParams = nullptr;
		return bRetVal;
	}

	void btnAuth_OnClick(CCtrlButton*)
	{
		SendReply(TRUE);
		Close();
	}
	void btnDeny_OnClick(CCtrlButton*)
	{
		SendReply(FALSE);
		Close();
	}

	UI_MESSAGE_MAP(CJabberDlgHttpAuth, CSuper);
		UI_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic);
	UI_MESSAGE_MAP_END();

	INT_PTR OnCtlColorStatic(UINT, WPARAM, LPARAM)
	{
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	}

private:
	CCtrlEdit	m_txtInfo;
	CCtrlButton	m_btnAuth;
	CCtrlButton	m_btnDeny;

	CJabberHttpAuthParams *m_pParams;
};

// XEP-0070 support (http auth)
INT_PTR __cdecl CJabberProto::OnHttpAuthRequest(WPARAM wParam, LPARAM lParam)
{
	CLISTEVENT *pCle = (CLISTEVENT *)lParam;
	CJabberHttpAuthParams *pParams = (CJabberHttpAuthParams *)pCle->lParam;
	if (!pParams)
		return 0;

	CJabberDlgHttpAuth *pDlg = new CJabberDlgHttpAuth(this, (HWND)wParam, pParams);
	if (!pDlg) {
		pParams->Free();
		mir_free(pParams);
		return 0;
	}

	pDlg->Show();

	return 0;
}
