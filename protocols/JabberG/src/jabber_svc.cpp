/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

#include "jabber_disco.h"

/////////////////////////////////////////////////////////////////////////////////////////
// GetMyAwayMsg - obtain the current away message

INT_PTR __cdecl CJabberProto::GetMyAwayMsg(WPARAM wParam, LPARAM lParam)
{
	char *szStatus = nullptr;

	mir_cslock lck(m_csModeMsgMutex);
	switch (wParam ? (int)wParam : m_iStatus) {
	case ID_STATUS_ONLINE:
		szStatus = m_modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
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
		return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_utf8decodeW(szStatus) : (INT_PTR)mir_utf8decode(szStatus, 0);
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

	if (!m_bEnableAvatars)
		return -2;

	GetAvatarFileName(0, buf, size);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarCaps - returns directives how to process avatars

INT_PTR __cdecl CJabberProto::JabberGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		{
			POINT* size = (POINT*)lParam;
			if (size)
				size->x = size->y = 96;
		}
		return 0;

	case AF_FORMATSUPPORTED: // Jabber supports avatars of virtually all formats
		return 1;

	case AF_ENABLED:
		return m_bEnableAvatars;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarInfo - retrieves the avatar info

INT_PTR __cdecl CJabberProto::JabberGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!m_bEnableAvatars)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*)lParam;

	ptrA szHashValue(getStringA(pai->hContact, "AvatarHash"));
	if (szHashValue == nullptr) {
		debugLogA("No avatar");
		return GAIR_NOAVATAR;
	}

	wchar_t tszFileName[MAX_PATH];
	GetAvatarFileName(pai->hContact, tszFileName, _countof(tszFileName));
	wcsncpy_s(pai->filename, tszFileName, _TRUNCATE);

	pai->format = (pai->hContact == 0) ? PA_FORMAT_PNG : getByte(pai->hContact, "AvatarType", 0);

	if (::_waccess(pai->filename, 0) == 0) {
		debugLogA("Avatar is Ok");
		return GAIR_SUCCESS;
	}

	if ((wParam & GAIF_FORCE) != 0 && pai->hContact != 0 && m_bJabberOnline) {
		ptrA tszJid(getUStringA(pai->hContact, "jid"));
		if (tszJid != nullptr) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, tszJid);
			if (item != nullptr) {
				CMStringA szJid;
				if (item->arResources.getCount() != 0)
					if (char *bestResName = ListGetBestClientResourceNamePtr(tszJid))
						szJid = MakeJid(tszJid, bestResName);

				if (szJid.IsEmpty())
					szJid = tszJid;

				debugLogA("Rereading %s for %s", JABBER_FEAT_VCARD_TEMP, szJid.c_str());
				m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetVCardAvatar, JABBER_IQ_TYPE_GET, szJid)) << XCHILDNS("vCard", JABBER_FEAT_VCARD_TEMP));
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
		wchar_t tFileName[MAX_PATH];
		GetAvatarFileName(0, tFileName, MAX_PATH);
		DeleteFile(tFileName);

		delSetting("AvatarHash");
	}
	else {
		int fileIn = _wopen(tszFileName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE);
		if (fileIn == -1) {
			mir_free(tszFileName);
			return 1;
		}

		long dwPngSize = _filelength(fileIn);
		char *pResult = new char[dwPngSize];
		if (pResult == nullptr) {
			_close(fileIn);
			mir_free(tszFileName);
			return 2;
		}

		_read(fileIn, pResult, dwPngSize);
		_close(fileIn);

		uint8_t digest[MIR_SHA1_HASH_SIZE];
		mir_sha1_ctx sha1ctx;
		mir_sha1_init(&sha1ctx);
		mir_sha1_append(&sha1ctx, (uint8_t*)pResult, dwPngSize);
		mir_sha1_finish(&sha1ctx, digest);

		wchar_t tFileName[MAX_PATH];
		GetAvatarFileName(0, tFileName, MAX_PATH);
		DeleteFile(tFileName);

		char buf[MIR_SHA1_HASH_SIZE * 2 + 1];
		bin2hex(digest, sizeof(digest), buf);

		GetAvatarFileName(0, tFileName, MAX_PATH);
		FILE *out = _wfopen(tFileName, L"wb");
		if (out != nullptr) {
			fwrite(pResult, dwPngSize, 1, out);
			fclose(out);
		}
		delete[] pResult;

		setString("AvatarHash", buf);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// JabberSetNickname - sets the user nickname without UI

INT_PTR __cdecl CJabberProto::JabberSetNickname(WPARAM wParam, LPARAM lParam)
{
	wchar_t *nickname = (wParam & SMNN_UNICODE) ? mir_wstrdup((wchar_t*)lParam) : mir_a2u((char*)lParam);

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

static const wchar_t
	*JabberEnum2AffilationStr[] = { LPGENW("None"), LPGENW("Outcast"), LPGENW("Member"), LPGENW("Admin"), LPGENW("Owner") },
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
	const wchar_t *pwszRoomId((wchar_t*)wParam), *pwszUserId((wchar_t*)lParam);
	if (!pwszRoomId || !pwszUserId)
		return 0; //room global tooltip not supported yet

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, T2Utf(pwszRoomId));
	if (item == nullptr)
		return 0;  //no room found

	pResourceStatus info(item->findResource(T2Utf(pwszUserId)));
	if (info == nullptr)
		return 0; //no info found

	// ok process info output will be:
	// JID:			real@jid/resource or
	// Nick:		Nickname
	// Status:		StatusText
	// Role:		Moderator
	// Affiliation:  Affiliation

	bool bIsTipper = ServiceExists(MS_TIPPER_HIDETIP);

	//JID:
	CMStringW outBuf;
	if (strchr(info->m_szResourceName, '@') != nullptr)
		appendString(bIsTipper, LPGENW("JID:"), pwszUserId, outBuf);
	else if (lParam) //or simple nick
		appendString(bIsTipper, LPGENW("Nick:"), pwszUserId, outBuf);

	// status
	if (info->m_iStatus >= ID_STATUS_OFFLINE && info->m_iStatus <= ID_STATUS_IDLE)
		appendString(bIsTipper, LPGENW("Status:"), Clist_GetStatusModeDescription(info->m_iStatus, 0), outBuf);

	// status text
	if (info->m_szStatusMessage)
		appendString(bIsTipper, LPGENW("Status message:"), Utf2T(info->m_szStatusMessage), outBuf);

	// Role
	appendString(bIsTipper, LPGENW("Role:"), TranslateW(JabberEnum2RoleStr[info->m_role]), outBuf);

	// Affiliation
	appendString(bIsTipper, LPGENW("Affiliation:"), TranslateW(JabberEnum2AffilationStr[info->m_affiliation]), outBuf);

	// real jid
	if (info->m_szRealJid)
		appendString(bIsTipper, LPGENW("Real JID:"), Utf2T(info->m_szRealJid), outBuf);

	return (outBuf.IsEmpty() ? 0 : (INT_PTR)mir_wstrdup(outBuf));
}

// File Association Manager plugin support
INT_PTR __cdecl CJabberProto::JabberServiceParseXmppURI(WPARAM, LPARAM lParam)
{
	wchar_t *arg = (wchar_t *)lParam;
	if (arg == nullptr)
		return 1;

	// skip leading prefix
	wchar_t szUri[1024];
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

	T2Utf jid(szJid);
	// no command or message command
	if (!szCommand || (szCommand && !mir_wstrcmpi(szCommand, L"message"))) {
		// message
		if (!ServiceExists(MS_MSG_SENDMESSAGEW))
			return 1;

		wchar_t *szMsgBody = nullptr;
		MCONTACT hContact = HContactFromJID(jid, false);
		if (hContact == 0)
			hContact = DBCreateContact(jid, jid, true, true);
		if (hContact == 0)
			return 1;

		if (szSecondParam) { //there are parameters to message
			szMsgBody = wcsstr(szSecondParam, L"body=");
			if (szMsgBody) {
				szMsgBody += 5;
				wchar_t *szDelim = wcschr(szMsgBody, ';');
				if (szDelim)
					szDelim = nullptr;
				JabberHttpUrlDecode(szMsgBody);
			}
		}

		CallService(MS_MSG_SENDMESSAGEW, hContact, (LPARAM)szMsgBody);
		return 0;
	}

	if (!mir_wstrcmpi(szCommand, L"roster")) {
		if (!HContactFromJID(jid)) {
			PROTOSEARCHRESULT psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.nick.w = szJid;
			psr.id.w = szJid;
			Contact::AddBySearch(m_szModuleName, &psr);
		}
		return 0;
	}

	// chat join invitation
	if (!mir_wstrcmpi(szCommand, L"join")) {
		GroupchatJoinRoomByJid(nullptr, jid);
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
		CJabberAdhocStartupParams* pStartupParams = new CJabberAdhocStartupParams(this, jid, T2Utf(szSecondParam));
		ContactMenuRunCommands(0, (LPARAM)pStartupParams);
		return 0;
	}

	// send file
	if (!mir_wstrcmpi(szCommand, L"sendfile")) {
		MCONTACT hContact = HContactFromJID(jid, false);
		if (hContact == 0)
			hContact = DBCreateContact(jid, jid, true, true);
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

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid == nullptr)
		return 0;

	CMStringA szJid;
	char *szResource = ListGetBestClientResourceNamePtr(jid);
	if (szResource)
		szJid = MakeJid(jid, szResource);
	else
		szJid = jid;

	m_ThreadInfo->send(
		XmlNode("message") << XATTR("type", "headline") << XATTR("to", szJid)
		<< XCHILDNS("attention", JABBER_FEAT_ATTENTION));
	return 0;
}

bool CJabberProto::SendHttpAuthReply(CJabberHttpAuthParams *pParams, bool bAuthorized)
{
	if (!m_bJabberOnline || !pParams || !m_ThreadInfo)
		return false;

	if (pParams->m_nType == CJabberHttpAuthParams::IQ) {
		XmlNodeIq iq(bAuthorized ? "result" : "error", pParams->m_szIqId, pParams->m_szFrom);
		if (!bAuthorized) {
			iq << XCHILDNS("confirm", JABBER_FEAT_HTTP_AUTH) << XATTR("id", pParams->m_szId)
				<< XATTR("method", pParams->m_szMethod) << XATTR("url", pParams->m_szUrl);
			iq << XCHILD("error") << XATTRI("code", 401) << XATTR("type", "auth")
				<< XCHILDNS("not-authorized", "urn:ietf:params:xml:xmpp-stanzas");
		}
		m_ThreadInfo->send(iq);
		return true;
	}
	
	if (pParams->m_nType == CJabberHttpAuthParams::MSG) {
		XmlNode msg("message");
		msg << XATTR("to", pParams->m_szFrom);
		if (!bAuthorized)
			msg << XATTR("type", "error");
		if (pParams->m_szThreadId)
			msg << XCHILD("thread", pParams->m_szThreadId);

		msg << XCHILDNS("confirm", JABBER_FEAT_HTTP_AUTH) << XATTR("id", pParams->m_szId)
			<< XATTR("method", pParams->m_szMethod) << XATTR("url", pParams->m_szUrl);

		if (!bAuthorized)
			msg << XCHILD("error") << XATTRI("code", 401) << XATTR("type", "auth")
			<< XCHILDNS("not-authorized", "urn:ietf:params:xml:xmpp-stanzas");

		m_ThreadInfo->send(msg);
		return true;
	}
	return false;
}

class CJabberDlgHttpAuth : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlEdit	m_txtInfo;

	CJabberHttpAuthParams *m_pParams;

public:
	CJabberDlgHttpAuth(CJabberProto *proto, HWND hwndParent, CJabberHttpAuthParams *pParams) :
		CSuper(proto, IDD_HTTP_AUTH),
		m_txtInfo(this, IDC_EDIT_HTTP_AUTH_INFO),
		m_pParams(pParams)
	{
		SetParent(hwndParent);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_OPEN));

		SetDlgItemTextUtf(m_hwnd, IDC_TXT_URL, m_pParams->m_szUrl);
		SetDlgItemTextUtf(m_hwnd, IDC_TXT_FROM, m_pParams->m_szFrom);
		SetDlgItemTextUtf(m_hwnd, IDC_TXT_ID, m_pParams->m_szId);
		SetDlgItemTextUtf(m_hwnd, IDC_TXT_METHOD, m_pParams->m_szMethod);
		return true;
	}

	void OnDestroy() override
	{
		m_proto->SendHttpAuthReply(m_pParams, m_bSucceeded);
		m_pParams->Free();
		mir_free(m_pParams);
	}

	UI_MESSAGE_MAP(CJabberDlgHttpAuth, CSuper);
	UI_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic);
	UI_MESSAGE_MAP_END();

	INT_PTR OnCtlColorStatic(UINT, WPARAM, LPARAM)
	{
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	}
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
