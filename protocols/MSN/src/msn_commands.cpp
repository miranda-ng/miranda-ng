/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2017 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

void CMsnProto::MSN_SetMirVer(MCONTACT hContact, MsnPlace *place)
{
	static const char* MirVerStr[] =
	{
		"Messenger (Windows)",
		"Web",
		"Windows Phone",
		"Xbox",
		"Zune",
		"Messenger (iPhone) ",
		"Messenger (Mac)",
		"Messenger (SMS)",
		"Messenger (Modern)",
		"Skype",
		"Skype (Windows)",
		"Skype (Windows 8)",
		"Skype (Mac)",
		"Skype (Linux)",
		"Skype (Windows Phone)",
		"Skype (iOS)",
		"Skype (Android)",
		"Skype"
	};

	char szVersion[64];

	if (!place || !place->client) return;
	mir_snprintf(szVersion, sizeof(szVersion), "%s (%s)", 
		MirVerStr[place->client>=sizeof(MirVerStr)/sizeof(MirVerStr[0])?9:place->client-1], place->szClientVer);
	setString(hContact, "MirVer", szVersion);
	setByte(hContact, "StdMirVer", 1);
}


/////////////////////////////////////////////////////////////////////////////////////////
//	MSN_ReceiveMessage - receives message or a file from the server
/////////////////////////////////////////////////////////////////////////////////////////
void CMsnProto::MSN_ReceiveMessage(ThreadData* info, char* cmdString, char* params)
{
	union {
		char* tWords[6];
		struct { char *fromEmail, *fromNick, *strMsgBytes; } data;
		struct { char *fromEmail, *fromNetId, *toEmail, *toNetId, *typeId, *strMsgBytes; } datau;
		struct { char *typeId, *strMsgBytes; } datas;
	};

	if (sttDivideWords(params, _countof(tWords), tWords) < 2) {
		debugLogA("Invalid %.3s command, ignoring", cmdString);
		return;
	}

	int msgBytes;
	char *nick = NULL, *email = NULL;
	wchar_t *mChatID = NULL;
	bool ubmMsg = strncmp(cmdString, "UBM", 3) == 0;
	bool sdgMsg = strncmp(cmdString, "SDG", 3) == 0;
	bool nfyMsg = strncmp(cmdString, "NFY", 3) == 0;
	bool sentMsg = false;

	if (sdgMsg) {
		msgBytes = atol(datas.strMsgBytes);
		if (mir_strcmpi(datas.typeId, "MSGR"))
			return;
	}
	else if (nfyMsg) {
		msgBytes = atol(datas.strMsgBytes);
		if (mir_strcmpi(datas.typeId, "MSGR\\HOTMAIL"))
			return;
	}
	else {
		if (ubmMsg) {
			msgBytes = atol(datau.strMsgBytes);
			nick = datau.fromEmail;
			email = datau.fromEmail;
		}
		else {
			msgBytes = atol(data.strMsgBytes);
			nick = data.fromNick;
			email = data.fromEmail;
			UrlDecode(nick);
		}
		stripBBCode(nick);
		stripColorCode(nick);
	}

	char* msg = (char*)alloca(msgBytes+1);

	HReadBuffer buf(info, 0);
	BYTE* msgb = buf.surelyRead(msgBytes);
	if (msgb == NULL) return;

	memcpy(msg, msgb, msgBytes);
	msg[msgBytes] = 0;

	debugLogA("Message:\n%s", msg);					                                                            

	MimeHeaders tHeader;
	char* msgBody = tHeader.readFromBuffer(msg);

	if (sdgMsg) {
		if (tHeader["Ack-Id"]) {
			CMStringA szBody;
			szBody.AppendFormat("Ack-Id: %s\r\n", tHeader["Ack-Id"]);
			if (msnRegistration) szBody.AppendFormat("Registration: %s\r\n", msnRegistration);
			szBody.AppendFormat("\r\n");
			msnNsThread->sendPacket("ACK", "MSGR %d\r\n%s", szBody.GetLength(), szBody.c_str());
		}
		msgBody = tHeader.readFromBuffer(msgBody);
		if (!(email = NEWSTR_ALLOCA(tHeader["From"]))) return;
		mChatID = mir_a2u(tHeader["To"]);
		if (wcsncmp(mChatID, L"19:", 3)) mChatID[0]=0; // NETID_THREAD
		msgBody = tHeader.readFromBuffer(msgBody);
		msgBody = tHeader.readFromBuffer(msgBody);
		nick = NEWSTR_ALLOCA(tHeader["IM-Display-Name"]);
		if (!mir_strcmp(tHeader["Message-Type"], "RichText")) {
			msgBody = NEWSTR_ALLOCA(msgBody);
			stripHTML(msgBody);
			HtmlDecode(msgBody);
		}
	}
	else if (nfyMsg) msgBody = tHeader.readFromBuffer(msgBody);
	else mChatID = info->mChatID;

	const char* tMsgId = tHeader["Message-ID"];
	if (tMsgId) lastMsgId=_atoi64(tMsgId);

	// Chunked message
	char* newbody = NULL;
	if (!sdgMsg && tMsgId) {
		int idx;
		const char* tChunks = tHeader["Chunks"];
		if (tChunks)
			idx = addCachedMsg(tMsgId, msg, 0, msgBytes, atol(tChunks), true);
		else
			idx = addCachedMsg(tMsgId, msgBody, 0, mir_strlen(msgBody), 0, true);

		size_t newsize;
		if (!getCachedMsg(idx, newbody, newsize)) return;
		msgBody = tHeader.readFromBuffer(newbody);
	}

	// message from the server (probably)
	if (!ubmMsg && !sdgMsg && !nfyMsg && strchr(email, '@') == NULL && _stricmp(email, "Hotmail"))
		return;

	const char* tContentType = tHeader["Content-Type"];
	if (tContentType == NULL)
		return;

	if (nfyMsg)
		msgBody = tHeader.readFromBuffer(msgBody);

	if (!_strnicmp(tContentType, "text/x-clientcaps", 17)) {
		MimeHeaders tFileInfo;
		tFileInfo.readFromBuffer(msgBody);
		info->firstMsgRecv = true;

		MCONTACT hContact = MSN_HContactFromEmail(email);
		const char* mirver = tFileInfo["Client-Name"];
		if (hContact != NULL && mirver != NULL) {
			setString(hContact, "MirVer", mirver);
			delSetting(hContact, "StdMirVer");
		}
	}
	else if (!ubmMsg && !sdgMsg && !nfyMsg && !info->firstMsgRecv) {
		info->firstMsgRecv = true;
		MsnContact *cont = Lists_Get(email);
		if (cont && cont->hContact != NULL && cont->places.getCount() > 0)
			MSN_SetMirVer(cont->hContact, &cont->places[0]);
	}

	if (!_strnicmp(tContentType, "text/plain", 10) ||
		(!_strnicmp(tContentType, "application/user+xml", 10) && tHeader["Message-Type"] && !strncmp(tHeader["Message-Type"], "RichText", 8))) {
		MCONTACT hContact = strncmp(email, "19:", 3) ? MSN_HContactFromEmail(email, nick, true, true) : NULL;

		if (!_stricmp(tHeader["Message-Type"], "RichText/UriObject") || !_stricmp(tHeader["Message-Type"], "RichText/Media_GenericFile")) {
			ezxml_t xmli = ezxml_parse_str(msgBody, strlen(msgBody));
			if (xmli) {
				MSN_ProcessURIObject(hContact, xmli);
				ezxml_free(xmli);
			}
		}
		else if (!_stricmp(tHeader["Message-Type"], "RichText/Contacts")) {
			ezxml_t xmli = ezxml_parse_str(msgBody, mir_strlen(msgBody));
			if (xmli) {
				if (!mir_strcmp(xmli->name, "contacts")) {
					ezxml_t c;
					int cnt;
					PROTOSEARCHRESULT **psr;

					for (c = ezxml_child(xmli, "c"), cnt=0; c; c = c->next) cnt++;
					if (psr = (PROTOSEARCHRESULT**)mir_calloc(sizeof(PROTOSEARCHRESULT*) * cnt)) {
						cnt=0;
						for (c = ezxml_child(xmli, "c"); c; c = c->next) {
							const char *t = ezxml_attr(c, "t"), *wlid;
							if (t && (wlid = ezxml_attr(c, t))) {
								switch (*t)
								{
								case 's':
								case 'p':
									psr[cnt] = (PROTOSEARCHRESULT*)mir_calloc(sizeof(PROTOSEARCHRESULT));
									psr[cnt]->cbSize = sizeof(psr);
									psr[cnt]->flags = PSR_UNICODE;
									psr[cnt]->id.w = psr[cnt]->nick.w = psr[cnt]->email.w = mir_a2u(wlid);
									cnt++;
								}
							}
						}
						if (cnt) {
							PROTORECVEVENT pre = { 0 };
							pre.timestamp = (DWORD)time(NULL);
							pre.szMessage = (char *)psr;
							pre.lParam = cnt;
							ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&pre);
							for (cnt=0; cnt<pre.lParam; cnt++) {
								mir_free(psr[cnt]->email.w);
								mir_free(psr[cnt]);
							}
						}
						mir_free(psr);
					}
				}
				ezxml_free(xmli);
			}
		}
		else if (!_stricmp(tHeader["Message-Type"], "Control/Typing")) 
			CallService(MS_PROTO_CONTACTISTYPING, hContact, 7);
		else if (!_stricmp(tHeader["Message-Type"], "Control/ClearTyping"))
			CallService(MS_PROTO_CONTACTISTYPING, hContact, 0);
		else {
			const char* p = tHeader["X-MMS-IM-Format"];
			bool isRtl = p != NULL && strstr(p, "RL=1") != NULL;

			/*if (info->mJoinedContactsWLID.getCount() > 1)
				MSN_ChatStart(info);
			else */{
				char *szNet, *szEmail;
				parseWLID(NEWSTR_ALLOCA(email), &szNet, &szEmail, NULL);
				sentMsg = _stricmp(szEmail, GetMyUsername(atoi(szNet))) == 0;
				if (sentMsg)
					hContact = ubmMsg ? MSN_HContactFromEmail(datau.toEmail, nick) : info->getContactHandle();
			}

			const char* tP4Context = tHeader["P4-Context"];
			if (tP4Context) {
				size_t newlen = mir_strlen(msgBody) + mir_strlen(tP4Context) + 4;
				char* newMsgBody = (char*)mir_alloc(newlen);
				mir_snprintf(newMsgBody, newlen, "[%s] %s", tP4Context, msgBody);
				mir_free(newbody);
				msgBody = newbody = newMsgBody;
			}

			if (mChatID[0]) {
				if (!_strnicmp(tHeader["Message-Type"], "ThreadActivity/", 15)) {
					ezxml_t xmli = ezxml_parse_str(msgBody, mir_strlen(msgBody));
					if (xmli) {
						MSN_GCProcessThreadActivity(xmli, mChatID);
						ezxml_free(xmli);
					}
				}
				else MSN_GCAddMessage(mChatID, hContact, email, time(NULL), sentMsg, msgBody);
			}
			else if (hContact) {
				if (!sentMsg) {
					CallService(MS_PROTO_CONTACTISTYPING, WPARAM(hContact), 0);

					PROTORECVEVENT pre = { 0 };
					pre.szMessage = (char*)msgBody;
					pre.flags = (isRtl ? PREF_RTL : 0);
					pre.timestamp = (DWORD)time(NULL);
					pre.lParam = 0;
					ProtoChainRecvMsg(hContact, &pre);
				}
				else {
					bool haveWnd = MSN_MsgWndExist(hContact);

					DBEVENTINFO dbei = {};
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.flags = DBEF_SENT | DBEF_UTF | (haveWnd ? 0 : DBEF_READ) | (isRtl ? DBEF_RTL : 0);
					dbei.szModule = m_szModuleName;
					dbei.timestamp = time(NULL);
					dbei.cbBlob = (unsigned)mir_strlen(msgBody) + 1;
					dbei.pBlob = (PBYTE)msgBody;
					db_event_add(hContact, &dbei);
				}
			}
		}
	}
	else if (!_strnicmp(tContentType, "text/x-msmsgsprofile", 20)) {
		replaceStr(msnExternalIP, tHeader["ClientIP"]);
		abchMigrated = atol(tHeader["ABCHMigrated"]);
		langpref = atol(tHeader["lang_preference"]);
		emailEnabled = atol(tHeader["EmailEnabled"]) != 0;

		if (!MSN_RefreshContactList()) {
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
			info->sendTerminate();
		}
		else {
			MSN_SetServerStatus(m_iDesiredStatus);
			MSN_EnableMenuItems(true);
		}
	}
	else if (!_strnicmp(tContentType, "text/x-msmsgscontrol", 20) ||
			 (sdgMsg && !_strnicmp(tContentType, "Application/Message", 19))) {
		const char* tTypingUser = sdgMsg?email:tHeader["TypingUser"];

		if (tTypingUser != NULL && info->mChatID[0] == 0 && _stricmp(email, MyOptions.szEmail)) {
			MCONTACT hContact = MSN_HContactFromEmail(tTypingUser, tTypingUser);
			CallService(MS_PROTO_CONTACTISTYPING, hContact, 
				sdgMsg && !_stricmp(tHeader["Message-Type"], "Control/ClearTyping")?0:7);
		}
	}
	else if (!_strnicmp(tContentType, "text/x-msnmsgr-datacast", 23)) {
		//if (info->mJoinedContactsWLID.getCount()) {
			MCONTACT tContact;

			if (mChatID[0]) {
				GC_INFO gci = { 0 };
				gci.Flags = GCF_HCONTACT;
				gci.pszModule = m_szModuleName;
				gci.pszID = mChatID;
				Chat_GetInfo(&gci);
				tContact = gci.hContact;
			}
			else tContact = MSN_HContactFromEmail(email, nick, true, true);
			if (!mir_strcmp(tHeader["Message-Type"], "Nudge"))
				NotifyEventHooks(hMSNNudge, (WPARAM)tContact, 0);
			/* Other msg types: 
			 * Wink
			 * Voice
			 * Data
			 */
	}
	else if (!_strnicmp(tContentType, "text/x-msmsgsemailnotification", 30))
		sttNotificationMessage(msgBody, false);
	else if (!_strnicmp(tContentType, "text/x-msmsgsinitialemailnotification", 37))
		sttNotificationMessage(msgBody, true);
	else if (!_strnicmp(tContentType, "text/x-msmsgsactivemailnotification", 35))
		sttNotificationMessage(msgBody, false);
	else if (!_strnicmp(tContentType, "text/x-msmsgsinitialmdatanotification", 37))
		sttNotificationMessage(msgBody, true);
	else if (!_strnicmp(tContentType, "text/x-msmsgsoimnotification", 28))
		sttNotificationMessage(msgBody, false);

	mir_free(mChatID);
	mir_free(newbody);
}

void CMsnProto::MSN_ProcessURIObject(MCONTACT hContact, ezxml_t xmli)
{
	const char *pszSkypeToken;

	if ((pszSkypeToken=authSkypeToken.Token()) && xmli) {
		/* FIXME: As soon as core has functions to POST images in a conversation AND gives the possibility to supply a
			* callback for fetching that image, this may be possible, but currently due to required Auth-Header, this
			* is not possible and we just send an incoming file transfer 
		const char *thumb = ezxml_attr(xmli, "url_thumbnail");
		if (thumb && ServiceExists("IEVIEW/NewWindow")) {
			// Supply callback to detch thumb with auth-header and embed [img] BB-code?
		}
		*/
		char *uri = (char*)ezxml_attr(xmli, "uri");
		if (uri) {
			// First get HTTP header of file to get content length
			unsigned __int64  fileSize = 0;
			NETLIBHTTPHEADER nlbhHeaders[2] = { 0 };
			nlbhHeaders[0].szName = "User-Agent";		nlbhHeaders[0].szValue = (LPSTR)MSN_USER_AGENT;
			nlbhHeaders[1].szName = "Authorization";	nlbhHeaders[1].szValue = (char*)pszSkypeToken;

			NETLIBHTTPREQUEST nlhr = { 0 }, *nlhrReply;
			nlhr.cbSize = sizeof(nlhr);
			nlhr.requestType = REQUEST_GET;
			nlhr.flags = NLHRF_PERSISTENT;
			nlhr.szUrl = uri;
			nlhr.headers = (NETLIBHTTPHEADER*)&nlbhHeaders;
			nlhr.headersCount = _countof(nlbhHeaders);
			nlhr.nlc = hHttpsConnection;

			mHttpsTS = clock();
			nlhrReply = Netlib_HttpTransaction(hNetlibUserHttps, &nlhr);
			mHttpsTS = clock();
			if (nlhrReply) {
				hHttpsConnection = nlhrReply->nlc;
				if (nlhrReply->resultCode == 200) {
					char *pLength, *pEnd;
					
					if ((pLength = strstr(nlhrReply->pData, "\"contents\":")) && 
						(pLength = strstr(pLength, "\"length\":")) && (pEnd = strchr(pLength+9, ','))) {
							pLength+=9;
							*pEnd = 0;
							fileSize=_atoi64(pLength);
					}
				}

				Netlib_FreeHttpRequest(nlhrReply);
			}  else hHttpsConnection = NULL;

			if (fileSize) {
				filetransfer* ft = new filetransfer(this);
				char *pszFile = "", *pszType;
				ezxml_t originalName, desc;

				ft->std.hContact = hContact;
				ft->tType = SERVER_HTTP;
				ft->p2p_appID = MSN_APPID_FILE;
				mir_free(ft->std.tszCurrentFile);
				if (!((originalName = ezxml_child(xmli, "OriginalName")) && (pszFile = (char*)ezxml_attr(originalName, "v")))) {
					if ((originalName = ezxml_child(xmli, "meta")))
						pszFile = (char*)ezxml_attr(originalName, "originalName");
				}
				if (!pszFile || !*pszFile) {
					if ((originalName = ezxml_child(xmli, "meta")) && (pszFile = (char*)ezxml_attr(originalName, "type"))) {
						if (!mir_strcmp(pszFile, "photo")) pszFile="IMG00001.JPG";
					}
					if (!pszFile || !*pszFile) pszFile="file";
				}
				ft->std.tszCurrentFile = mir_utf8decodeW(pszFile);
				ft->std.totalBytes = ft->std.currentFileSize = fileSize;
				ft->std.totalFiles = 1;
				ft->szInvcookie = (char*)mir_calloc(strlen(uri)+16);
				if ((pszType = (char*)ezxml_attr(xmli, "type")) && !mir_strcmp(pszType, "File.1"))
					sprintf(ft->szInvcookie, "%s/content/original", uri);
				else
					sprintf(ft->szInvcookie, "%s/content/imgpsh", uri);

				wchar_t tComment[40];
				mir_snwprintf(tComment, TranslateT("%I64u bytes"), ft->std.currentFileSize);

				PROTORECVFILET pre = { 0 };
				pre.dwFlags = PRFF_UNICODE;
				pre.fileCount = 1;
				pre.timestamp = time(NULL);
				pre.descr.w = (desc = ezxml_child(xmli, "Description"))?mir_utf8decodeW(desc->txt):tComment;
				pre.files.w = &ft->std.tszCurrentFile;
				pre.lParam = (LPARAM)ft;
				ProtoChainRecvFile(ft->std.hContact, &pre);
				if (desc) mir_free(pre.descr.w);
			} else uri=NULL;
		}

		if (uri == NULL) {
			// Fallback: Just filter out the link and post it as a message
			CallService(MS_PROTO_CONTACTISTYPING, WPARAM(hContact), 0);

			PROTORECVEVENT pre = { 0 };
			CMStringA msgtxt((char*)ezxml_txt(xmli));
			ezxml_t urllnk;
			if (urllnk=ezxml_child(xmli, "a")) msgtxt.AppendFormat(" %s", ezxml_txt(urllnk));
			pre.szMessage = (char*)(const char*)msgtxt;
			pre.timestamp = (DWORD)time(NULL);
			ProtoChainRecvMsg(hContact, &pre);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Process Yahoo Find

void CMsnProto::MSN_ProcessYFind(char* buf, size_t len)
{
	if (buf == NULL) return;
	ezxml_t xmli = ezxml_parse_str(buf, len);

	ezxml_t dom = ezxml_child(xmli, "d");
	const char* szDom = ezxml_attr(dom, "n");

	ezxml_t cont = ezxml_child(dom, "c");
	const char* szCont = ezxml_attr(cont, "n");

	char szEmail[128];
	mir_snprintf(szEmail, "%s@%s", szCont, szDom);

	const char *szNetId = ezxml_attr(cont, "t");
	if (msnSearchId != NULL) {
		if (szNetId != NULL) {
			ptrW szEmailT(mir_utf8decodeW(szEmail));

			PROTOSEARCHRESULT psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.id.w = szEmailT;
			psr.nick.w = szEmailT;
			psr.email.w = szEmailT;
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, msnSearchId, (LPARAM)&psr);
		}
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, msnSearchId, 0);

		msnSearchId = NULL;
	}
	else {
		if (szNetId != NULL) {
			int netId = atol(szNetId);
			MCONTACT hContact = MSN_HContactFromEmail(szEmail, szEmail, true, false);
			if (MSN_AddUser(hContact, szEmail, netId, LIST_FL)) {
				MSN_AddUser(hContact, szEmail, netId, LIST_PL + LIST_REMOVE);
				MSN_AddUser(hContact, szEmail, netId, LIST_BL + LIST_REMOVE);
				MSN_AddUser(hContact, szEmail, netId, LIST_AL);
				db_unset(hContact, "CList", "Hidden");
			}
			MSN_SetContactDb(hContact, szEmail);
		}
	}

	ezxml_free(xmli);
}

/////////////////////////////////////////////////////////////////////////////////////////
//	MSN_HandleCommands - process commands from the server
/////////////////////////////////////////////////////////////////////////////////////////

void CMsnProto::MSN_ProcessNLN(const char *userStatus, const char *wlid, char *userNick, const char *objid, char *cmdstring)
{
	if (userNick) {
		UrlDecode(userNick);
		stripBBCode(userNick);
		stripColorCode(userNick);
	}

	bool isMe = false;
	char* szEmail, *szNet, *szInst;
	parseWLID(NEWSTR_ALLOCA(wlid), &szNet, &szEmail, &szInst);
	if (!mir_strcmpi(szEmail, GetMyUsername(atoi(szNet)))) {
		if (!*userStatus) return;
		isMe = true;
		int newStatus = MSNStatusToMiranda(userStatus);
		if (newStatus != m_iStatus && newStatus != ID_STATUS_IDLE) {
			int oldMode = m_iStatus;
			m_iDesiredStatus = m_iStatus = newStatus;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldMode, m_iStatus);
		}
	}

	WORD lastStatus = ID_STATUS_OFFLINE;

	MsnContact *cont = Lists_Get(szEmail);

	MCONTACT hContact = NULL;
	if (!cont && !isMe) {
		hContact = MSN_HContactFromEmail(wlid, userNick, true, true);
		cont = Lists_Get(szEmail);
	}
	if (cont) hContact = cont->hContact;

	if (hContact != NULL) {
		if (userNick) setStringUtf(hContact, "Nick", userNick);
		lastStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
		if (lastStatus == ID_STATUS_OFFLINE || lastStatus == ID_STATUS_INVISIBLE)
			db_unset(hContact, "CList", "StatusMsg");

		int newStatus = MSNStatusToMiranda(userStatus);
		setWord(hContact, "Status", newStatus != ID_STATUS_IDLE ? newStatus : ID_STATUS_AWAY);
		setDword(hContact, "IdleTS", newStatus != ID_STATUS_IDLE ? 0 : time(NULL));
	}

	if (cont) {
		if (objid) {
			char* end = NULL;
			cont->cap1 = strtoul(objid, &end, 10);
			cont->cap2 = end && *end == ':' ? strtoul(end + 1, NULL, 10) : 0;
		}

		if (szInst) MSN_SetMirVer(hContact, cont->places.find((MsnPlace*)&szInst));

		char *pszUrl, *pszAvatarHash;
		if (cmdstring && *cmdstring && mir_strcmp(cmdstring, "0") &&
			(pszAvatarHash = MSN_GetAvatarHash(cmdstring, &pszUrl))) 
		{
			setString(hContact, "PictContext", cmdstring);
			setString(hContact, "AvatarHash", pszAvatarHash);
			if (pszUrl)
				setString(hContact, "AvatarUrl", pszUrl);
			else
				delSetting(hContact, "AvatarUrl");

			if (hContact != NULL) {
				char szSavedHash[64] = "";
				if (!db_get_static(hContact, m_szModuleName, "AvatarSavedHash", szSavedHash, sizeof(szSavedHash))) {
					if (mir_strcmpi(szSavedHash, pszAvatarHash))
						pushAvatarRequest(hContact, pszUrl);
					else {
						char szSavedContext[64];
						int result = db_get_static(hContact, m_szModuleName, "PictSavedContext", szSavedContext, sizeof(szSavedContext));
						if (result || mir_strcmp(szSavedContext, cmdstring))
							pushAvatarRequest(hContact, pszUrl);
					}
				}
			}
			mir_free(pszAvatarHash);
			mir_free(pszUrl);
		}
		else {
			delSetting(hContact, "AvatarHash");
			delSetting(hContact, "AvatarSavedHash");
			delSetting(hContact, "AvatarUrl");
			delSetting(hContact, "PictContext");
			delSetting(hContact, "PictSavedContext");

			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
		}
	}
	else if (lastStatus == ID_STATUS_OFFLINE)
		delSetting(hContact, "MirVer");
}

void CMsnProto::MSN_ProcessStatusMessage(ezxml_t xmli, const char* wlid)
{
	MCONTACT hContact = MSN_HContactFromEmail(wlid);
	if (hContact == NULL) return;

	char* szEmail, *szNetId, *szInst;
	parseWLID(NEWSTR_ALLOCA(wlid), &szNetId, &szEmail, &szInst);

	bool bHasPSM=false;
	char* szStatMsg = NULL;

	for (ezxml_t s = ezxml_child(xmli, "s"); s; s = s->next) {
		const char *n = ezxml_attr(s, "n");
		if (!mir_strcmp(n, "SKP")) {
			szStatMsg = ezxml_txt(ezxml_child(s, "Mood"));
			if (*szStatMsg) db_set_utf(hContact, "CList", "StatusMsg", szStatMsg);
			else if (!bHasPSM) db_unset(hContact, "CList", "StatusMsg");
		} else if (!mir_strcmp(n, "PE")) {
			szStatMsg = ezxml_txt(ezxml_child(s, "PSM"));
			if (*szStatMsg) {
				stripBBCode((char*)szStatMsg);
				stripColorCode((char*)szStatMsg);
				db_set_utf(hContact, "CList", "StatusMsg", szStatMsg);
				bHasPSM = true;
			}
			else db_unset(hContact, "CList", "StatusMsg");
		}
	}

	// Add endpoints
	for (ezxml_t endp = ezxml_child(xmli, "sep"); endp; endp = ezxml_next(endp)) {
		const char *n = ezxml_attr(endp, "n");
		if (!mir_strcmp(n, "IM")) {
			const char *id = ezxml_attr(endp, "epid");
			const char *caps = ezxml_txt(ezxml_child(endp, "Capabilities"));
			char* end = NULL;
			unsigned cap1 = caps ? strtoul(caps, &end, 10) : 0;
			unsigned cap2 = end && *end == ':' ? strtoul(end + 1, NULL, 10) : 0;

			Lists_AddPlace(szEmail, id, cap1, cap2);
		}
		else if (!mir_strcmp(n, "PE")) {
			MsnPlace *place = Lists_GetPlace(szEmail, ezxml_attr(endp, "epid"));
			if (place) {
				place->client = atoi(ezxml_txt(ezxml_child(endp, "TYP")));
				mir_strncpy(place->szClientVer, ezxml_txt(ezxml_child(endp, "VER")), sizeof(place->szClientVer));
			}
		}
	}

	if (szInst)
		MSN_SetMirVer(hContact, Lists_GetPlace(szEmail, szInst));
	else {
		MsnContact *cont = Lists_Get(hContact);
		if (cont->places.getCount() > 0)
			MSN_SetMirVer(hContact, &cont->places[0]);
	}
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, NULL, ptrW(mir_utf8decodeW(szStatMsg)));
}

void CMsnProto::MSN_ProcessNotificationMessage(char* buf, size_t len)
{
	if (buf == NULL) return;
	ezxml_t xmlnot = ezxml_parse_str(buf, len);

	if (mir_strcmp(ezxml_attr(xmlnot, "siteid"), "0") == 0) {
		ezxml_free(xmlnot);
		return;
	}

	ezxml_t xmlmsg = ezxml_child(xmlnot, "MSG");
	ezxml_t xmlact = ezxml_child(xmlmsg, "ACTION");
	ezxml_t xmlbdy = ezxml_child(xmlmsg, "BODY");
	ezxml_t xmltxt = ezxml_child(xmlbdy, "TEXT");

	if (xmltxt != NULL) {
		char fullurl[1024];
		size_t sz = 0;

		const char* acturl = ezxml_attr(xmlact, "url");
		if (acturl == NULL || strstr(acturl, "://") == NULL)
			sz += mir_snprintf((fullurl + sz), (_countof(fullurl) - sz), "%s", ezxml_attr(xmlnot, "siteurl"));

		sz += mir_snprintf((fullurl + sz), (_countof(fullurl) - sz), "%s", acturl);
		if (sz != 0 && fullurl[sz - 1] != '?')
			sz += mir_snprintf((fullurl + sz), (_countof(fullurl) - sz), "?");

		mir_snprintf((fullurl + sz), (_countof(fullurl) - sz), "notification_id=%s&message_id=%s",
			ezxml_attr(xmlnot, "id"), ezxml_attr(xmlmsg, "id"));

		Skin_PlaySound(alertsoundname);

		wchar_t* alrt = mir_utf8decodeW(ezxml_txt(xmltxt));
		MSN_ShowPopup(TranslateT("MSN Alert"), alrt, MSN_ALERT_POPUP | MSN_ALLOW_MSGBOX, fullurl);
		mir_free(alrt);
	}
	else if (xmlbdy) {
		const char *txt = ezxml_txt(xmlbdy);
		if (strstr(txt, "ABCHInternal")) {
			MSN_SharingFindMembership(true);
			MSN_ABFind("ABFindContactsPaged", NULL, true);
			MSN_StoreGetProfile();
		}
	}
	ezxml_free(xmlnot);
}

int CMsnProto::MSN_HandleCommands(ThreadData* info, char* cmdString)
{
	char* params = "";
	int trid = -1;

	if (cmdString[3]) {
		if (isdigit((BYTE)cmdString[4])) {
			trid = strtol(cmdString + 4, &params, 10);
			switch (*params) {
			case ' ':	case '\0':	case '\t':	case '\n':
				while (*params == ' ' || *params == '\t')
					params++;
				break;

			default:
				params = cmdString + 4;
			}
		}
		else params = cmdString + 4;
	}

	switch((*(PDWORD)cmdString & 0x00FFFFFF) | 0x20000000) {

	case ' SBS':
		break;

	case ' HTA':	//********* ATH: MSNP21+ Authentication
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2) {
LBL_InvalidCommand:
				debugLogA("Invalid %.3s command, ignoring", cmdString);
				break;
			}

			HReadBuffer buf(info, 0);
			buf.surelyRead(atol(data.strMsgBytes));

			if (!bIgnoreATH) {
				if (!bSentBND)
				{
					info->sendPacketPayload("BND", "CON\\MSGR",
						"<msgr><ver>%d</ver>%s<client><name>%s</name><ver>%s</ver></client>"
								"<epid>%.*s</epid></msgr>\r\n", 
								msnP24Ver, (msnP24Ver>1?"<altVersions><ver>1</ver></altVersions>":""),
								msnStoreAppId, msnProductVer, 
								mir_strlen(MyOptions.szMachineGuid)-2, MyOptions.szMachineGuid+1);
					bSentBND = true;
				}
				else
				{
					msnLoggedIn = true;
					isConnectSuccess = true;
					emailEnabled = MyOptions.netId==NETID_MSN; // Let's assume it?
					MSN_SetServerStatus(m_iDesiredStatus);
					MSN_EnableMenuItems(true);
					// Fork refreshing and populating contact list to the background
					ForkThread(&CMsnProto::msn_loginThread, NULL);
				}
			} else bIgnoreATH = false;
		}
		break;

	case ' DNB':	//********* BND: MSNP21+ bind request answer?
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2)
				goto LBL_InvalidCommand;

			MimeHeaders tHeader;
			HReadBuffer buf(info, 0);
			BYTE *msgb = buf.surelyRead(atol(data.strMsgBytes));
			if (!msgb) break;
			char* msgBody = tHeader.readFromBuffer((char*)msgb);

			replaceStr(msnRegistration,tHeader["Set-Registration"]);
			if (!mir_strcmp(data.typeId, "CON")) {
				ezxml_t xmlbnd = ezxml_parse_str(msgBody, mir_strlen(msgBody));
				ezxml_t xmlbdy = ezxml_child(xmlbnd, "nonce");
				if (xmlbdy)
				{
					char dgst[64];
					MSN_MakeDigest(xmlbdy->txt, dgst);
					info->sendPacketPayload("PUT", "MSGR\\CHALLENGE",
						"<challenge><appId>%s</appId><response>%s</response></challenge>\r\n",
						msnProductID, dgst);
				}
				ezxml_free(xmlbnd);
			}
		}
		break;

	case ' TNC':	//********* CNT: Connect, MSNP21+ Authentication
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2)
				goto LBL_InvalidCommand;

			HReadBuffer buf(info, 0);
			char* msgBody = (char*)buf.surelyRead(atol(data.strMsgBytes));
			if (mir_strcmp(data.typeId, "CON")) break;

			if (MyOptions.netId!=NETID_SKYPE) {
				/* MSN account login */

				if (MSN_AuthOAuth()<1) {
					m_iDesiredStatus = ID_STATUS_OFFLINE;
					return 1;
				}

			} else {
				/* Skype username/pass login */
				if (!msgBody) break;
				ezxml_t xmlcnt = ezxml_parse_str(msgBody, mir_strlen(msgBody));
				ezxml_t xmlnonce = ezxml_child(xmlcnt, "nonce");
				if (xmlnonce) {
					char szUIC[1024]={0};

					MSN_SkypeAuth(xmlnonce->txt, szUIC);
					replaceStr(authUIC, szUIC);
				}
				ezxml_free(xmlcnt);
			}
			MSN_SendATH(info);

			bSentBND = false;
			if (!hKeepAliveThreadEvt)
				ForkThread(&CMsnProto::msn_keepAliveThread, NULL);
		}
		break;

	case ' TEG':    //********* GET: MSNP21+ GET reply
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2)
				goto LBL_InvalidCommand;

			MimeHeaders tHeader;
			HReadBuffer buf(info, 0);
			BYTE *msgb = buf.surelyRead(atol(data.strMsgBytes));
			if (!msgb) break;
			char* msgBody = tHeader.readFromBuffer((char*)msgb);
			ezxml_t xmli;

			if (tHeader["Set-Registration"]) replaceStr(msnRegistration,tHeader["Set-Registration"]);
			if (xmli = ezxml_parse_str(msgBody, mir_strlen(msgBody)))
			{
				if (!mir_strcmp(xmli->name, "recentconversations-response"))
				{
					for (ezxml_t conv = ezxml_get(xmli, "conversations", 0, "conversation", -1); conv != NULL; conv = ezxml_next(conv)) {
						ezxml_t id;
						MCONTACT hContact;
						MEVENT hDbEvent;
						DWORD ts = 1;

						if (ezxml_get(conv, "messages", 0, "message", -1) && (id=ezxml_child(conv, "id"))) 
						{
							if (strncmp(id->txt, "19:", 3) == 0) {
								/* This is a thread (Groupchat)
								 * Find out about its current state on the server */
								hContact = MSN_HContactFromChatID(id->txt);
								msnNsThread->sendPacketPayload("GET", "MSGR\\THREADS", 
									"<threads><thread><id>%s</id></thread></threads>", id->txt);
							} else hContact = MSN_HContactFromEmail(id->txt, NULL, false, false);

							if (hContact) {
								// There are messages to be fetched
								if (hDbEvent = db_event_last(hContact)) {
									DBEVENTINFO dbei = {};
									db_event_get(hDbEvent, &dbei);
									ts = dbei.timestamp;
								}
								db_set_dw(hContact, m_szModuleName, "syncTS", ts);
							}
							msnNsThread->sendPacketPayload("GET", "MSGR\\MESSAGESBYCONVERSATION", 
								"<messagesbyconversation><id>%s</id><start>%llu</start><pagesize>100</pagesize></messagesbyconversation>",
								id->txt, ((unsigned __int64)ts)*1000);
						}
					}
				}
				else if (!mir_strcmp(xmli->name, "messagesbyconversation-response")) {
					ezxml_t id;
					MCONTACT hContact;

					if ((id=ezxml_child(xmli, "id"))) 
					{
						bool bIsChat = strncmp(id->txt, "19:", 3)==0;
						bool bHasMore = mir_strcmpi(ezxml_txt(ezxml_child(xmli, "hasmore")), "true") == 0;
						ezxml_t syncstate;
						hContact = MSN_HContactFromEmail(id->txt, NULL, false, false);
						if (!bHasMore && hContact) db_unset(hContact, m_szModuleName, "syncTS");

						/* We have to traverse the list in reverse order as newest events are on top (which is the opposite direction of Groupchat) */
						LIST<ezxml> msgs(10,PtrKeySortT);
						for (ezxml_t msg = ezxml_get(xmli, "messages", 0, "message", -1); msg != NULL; msg = ezxml_next(msg)) msgs.insert(msg, 0);
						for (int i=0; i<msgs.getCount(); i++) {
							ezxml_t msg = msgs[i];
							ezxml_t arrtime = ezxml_child(msg, "originalarrivaltime"), from = ezxml_child(msg, "from"),
									msgtype = ezxml_child(msg, "messagetype"), content = ezxml_child(msg, "content");
							time_t ts;
							char *netId, *email, *message;
							bool sentMsg;

							if (!arrtime || !from || !content) continue;
							ts=IsoToUnixTime(arrtime->txt);
							parseWLID(NEWSTR_ALLOCA(from->txt), &netId, &email, NULL);
							message=content->txt;
							sentMsg = mir_strcmpi(email, GetMyUsername(atoi(netId)))==0;
							if (msgtype) {
								if (!mir_strcmp(msgtype->txt, "RichText")) {
									message = NEWSTR_ALLOCA(message);
									stripHTML(message);
									HtmlDecode(message);
								} else if (!strncmp(msgtype->txt, "ThreadActivity/", 15)) {
									if (ezxml_t xmlact = ezxml_parse_str(content->txt, mir_strlen(content->txt))) {
										MSN_GCProcessThreadActivity(xmlact, _A2T(id->txt));
										ezxml_free(xmlact);
									}
									continue;
								} else if (!mir_strcmp(msgtype->txt, "RichText/UriObject") || !mir_strcmp(msgtype->txt, "RichText/Media_GenericFile")) {
									if (ezxml_t xmlact = ezxml_parse_str(content->txt, mir_strlen(content->txt))) {
										MSN_ProcessURIObject(hContact, xmlact);
										ezxml_free(xmlact);
									}
									continue;
								} else if (mir_strcmp(msgtype->txt, "Text")) continue;								/* TODO: Implement i.e. RichText/Files for announcement of file transfers */
							}

							if (bIsChat) {
								hContact = MSN_HContactFromEmail(from->txt, NULL, false, false);
								if (hContact)
									db_unset(hContact, m_szModuleName, "syncTS");
								MSN_GCAddMessage(_A2T(id->txt), hContact, email, ts, sentMsg, message);
							}
							else if (hContact) {
								/* Protect against double sync (Miranda MSGs only have granularity in seconds) */
								MEVENT hDbEvent;
								bool bDuplicate = false;
								DBEVENTINFO dbei = {};
								DWORD cbBlob = (DWORD)mir_strlen(message);
								dbei.cbBlob = cbBlob;
								BYTE *pszMsgBuf = (BYTE*)mir_calloc(cbBlob);
								if (pszMsgBuf) {
									dbei.pBlob = pszMsgBuf;
									for((hDbEvent = db_event_last(hContact)); 
										!bDuplicate && hDbEvent; 
										hDbEvent=db_event_prev(hContact, hDbEvent)) 
									{
										if (db_event_get(hDbEvent, &dbei) || dbei.timestamp > ts+1 || dbei.timestamp < ts) break;
										if (!memcmp((char*)dbei.pBlob, message, cbBlob)) bDuplicate = true;
										dbei.cbBlob = cbBlob;
									}
									mir_free(pszMsgBuf);
									if (bDuplicate) continue;
								}

								if (!sentMsg) {
									PROTORECVEVENT pre = { 0 };
									pre.szMessage = (char*)message;
									pre.timestamp = (DWORD)ts;
									ProtoChainRecvMsg(hContact, &pre);
								}
								else {
									memset(&dbei, 0, sizeof(dbei));
									dbei.eventType = EVENTTYPE_MESSAGE;
									dbei.flags = DBEF_SENT | DBEF_UTF;
									dbei.szModule = m_szModuleName;
									dbei.timestamp = ts;
									dbei.cbBlob = (unsigned)mir_strlen(message) + 1;
									dbei.pBlob = (PBYTE)message;
									db_event_add(hContact, &dbei);
								}
							}
						}
						/* In groupchat it wouldn't make much sense to sync more as older messages are coming now and that would jumble our log */
						if (!bIsChat && bHasMore && (syncstate = ezxml_child(xmli, "messagessyncstate"))) {
							msnNsThread->sendPacketPayload("GET", "MSGR\\MESSAGESBYCONVERSATION", 
								"<messagesbyconversation><id>%s</id><start>%llu</start><messagessyncstate>%s</messagessyncstate><pagesize>100</pagesize></messagesbyconversation>",
								id->txt, ((unsigned __int64)db_get_dw(hContact, m_szModuleName, "syncTS", 1000))*1000, syncstate->txt);
						}
						msgs.destroy();
					}
				}
				else if (!mir_strcmp(xmli->name, "threads-response")) {
					for (ezxml_t thread = ezxml_get(xmli, "threads", 0, "thread", -1); thread != NULL; thread = ezxml_next(thread))
						MSN_ChatStart(thread);
				}
				ezxml_free(xmli);
			}				

		}
		break;

	case ' YFN':   //********* NFY: MSNP21+ Notifications
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2)
				goto LBL_InvalidCommand;

			HReadBuffer buf(info, 0);
			char* msgBody = (char*)buf.surelyRead(atol(data.strMsgBytes));
			if (msgBody == NULL) break;
			if (!mir_strcmp(data.typeId, "MSGR\\HOTMAIL")) {
				char szParam[128];
				mir_snprintf(szParam, sizeof(szParam), "%s %s", data.typeId, data.strMsgBytes);
				MSN_ReceiveMessage(info, cmdString, szParam);
				break;
			} else if (!mir_strcmp(data.typeId, "MSGR\\ABCH")) {
				MimeHeaders tHeader;
				msgBody = tHeader.readFromBuffer(msgBody);
				MSN_ProcessNotificationMessage(msgBody, mir_strlen(msgBody));
				break;
			}

			if (!mir_strcmp(data.typeId, "MSGR\\PUT") || !mir_strcmp(data.typeId, "MSGR\\DEL")) {
				MimeHeaders tHeader;

				int i;
				for (i=0; i<2; i++) msgBody = tHeader.readFromBuffer(msgBody);
				char *pszTo = NULL, *pszToNet;
				if (tHeader["To"]) parseWLID(NEWSTR_ALLOCA(tHeader["To"]), &pszToNet, &pszTo, NULL);
				const char *pszFrom =  tHeader["From"];
				for (i=0; i<2; i++) msgBody = tHeader.readFromBuffer(msgBody);

				if (pszFrom)
				{
					ezxml_t xmli;
					if (xmli = ezxml_parse_str(msgBody, mir_strlen(msgBody)))
					{
						if (!mir_strcmp(xmli->name, "user"))
						{
							ezxml_t xmlstatus = ezxml_get(xmli, "s", 0, "Status", -1);
							/* FIXME: MSGR\DEL: Instance of user with given EPID disconnected, not 
							 * sure if this implies that contact is offline now... */
							if (xmlstatus || !mir_strcmp(data.typeId, "MSGR\\DEL"))
							{
								// These capabilities seem to be something different than in previous MSNP versions?
								//ezxml_t xmlcaps = ezxml_get(xmli, "sep", 0, "Capabilities", -1);
								ezxml_t usertile = ezxml_get(xmli, "s", 1, "UserTileLocation", -1);
								MSN_ProcessNLN(ezxml_txt(xmlstatus), pszFrom, NULL, NULL, usertile?usertile->txt:NULL);
							}
							MSN_ProcessStatusMessage(xmli, pszFrom);
						}
						ezxml_free(xmli);
					}				
				}
			}
			else if (!mir_strcmp(data.typeId, "MSGR\\THREAD")) {
				MimeHeaders tHeader;
				char *szBody = tHeader.readFromBuffer(info->mData);
				ezxml_t xmli = ezxml_parse_str(szBody, mir_strlen(szBody));
				if (xmli) {
					MSN_ChatStart(xmli);
					ezxml_free(xmli);
				}
			}
		}
		break;

	case ' TUP':	//******** MSNP21+: PUT notifications
	case ' GNP':	//******** MSNP21+: PNG reply
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2)
				goto LBL_InvalidCommand;

			MimeHeaders tHeader;
			HReadBuffer buf(info, 0);
			BYTE *msgb = buf.surelyRead(atol(data.strMsgBytes));
			if (!msgb) break;
			char* msgBody = tHeader.readFromBuffer((char*)msgb);

			if (tHeader["Set-Registration"]) replaceStr(msnRegistration,tHeader["Set-Registration"]);
			if (cmdString[1]=='N') { // PNG
				if (ezxml_t xmli = ezxml_parse_str(msgBody, mir_strlen(msgBody))) {
					if (ezxml_t wait = ezxml_child(xmli, "wait")) {
						msnPingTimeout = atoi(ezxml_txt(wait));
						if (msnPingTimeout && hKeepAliveThreadEvt != NULL)
							SetEvent(hKeepAliveThreadEvt);
					}
					ezxml_free(xmli);
				}				
			} else { // PUT
				ezxml_t xmli;
				if (*msgBody && (xmli = ezxml_parse_str(msgBody, mir_strlen(msgBody)))) {
					if (!mir_strcmp(xmli->name, "presence-response")) {
						ezxml_t user, from;
						if ((user = ezxml_child(xmli, "user")) && (from = ezxml_child(xmli, "from"))) {
							if (ezxml_t xmlstatus = ezxml_get(user, "s", 0, "Status", -1)) {
								ezxml_t usertile = ezxml_get(user, "s", 1, "UserTileLocation", -1);
								MSN_ProcessNLN(ezxml_txt(xmlstatus), from->txt, NULL, NULL, usertile?usertile->txt:NULL);
							} else {
								int oldMode = m_iStatus;
								m_iStatus = m_iDesiredStatus;
								ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldMode, m_iStatus);
							}
							MSN_ProcessStatusMessage(user, from->txt);
						}
					}
					ezxml_free(xmli);
				}				
			}
		}
		break;

	case ' GDS':   // SDG:  MSNP21+ Messaging
		MSN_ReceiveMessage(info, cmdString, params);
		break;

	case ' RFX':    //******** XFR: sections 7.4 Referral, 8.1 Referral to Switchboard
		{
			union {
				char* tWords[2];
				struct { char *typeId, *strMsgBytes; } data;
			};

			if (sttDivideWords(params, _countof(tWords), tWords) < 2)
				goto LBL_InvalidCommand;

			MimeHeaders tHeader;
			HReadBuffer buf(info, 0);
			BYTE *msgb = buf.surelyRead(atol(data.strMsgBytes));
			if (!msgb) break;
			char* msgBody = tHeader.readFromBuffer((char*)msgb);
			if (!mir_strcmp(data.typeId, "CON")) {
				ezxml_t xmlxfr = ezxml_parse_str(msgBody, mir_strlen(msgBody));
				ezxml_t xmltgt = ezxml_child(xmlxfr, "target");
				if (xmltgt)
				{
					ThreadData* newThread = new ThreadData;
					mir_strcpy(newThread->mServer, xmltgt->txt);
					mir_strcpy(newThread->mState, ezxml_txt(ezxml_child(xmlxfr, "state")));
					newThread->mType = SERVER_NOTIFICATION;
					newThread->mTrid = info->mTrid;
					newThread->mIsMainThread = true;
					info->mIsMainThread = false;

					debugLogA("Switching to notification server '%s'...", xmltgt->txt);
					newThread->startThread(&CMsnProto::MSNServerThread, this);
					ezxml_free(xmlxfr);
					return 1;  //kill the old thread
				}
				ezxml_free(xmlxfr);
			}
		}
		break;

	default:
		debugLogA("Unrecognised message: %s", cmdString);
		break;
	}

	return 0;
}
