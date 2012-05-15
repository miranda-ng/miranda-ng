/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
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

#include "msn_global.h"
#include "msn_proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Starts a file sending thread

void MSN_ConnectionProc(HANDLE hNewConnection, DWORD /* dwRemoteIP */, void* extra)
{
	CMsnProto *proto = (CMsnProto*)extra;
	
	proto->MSN_DebugLog("File transfer connection accepted");

	WORD localPort = 0;
	SOCKET s = MSN_CallService(MS_NETLIB_GETSOCKET, (WPARAM)hNewConnection, 0);
	if (s != INVALID_SOCKET) 
	{
		SOCKADDR_IN saddr;
		int len = sizeof(saddr);
		if (getsockname(s, (SOCKADDR*)&saddr, &len) != SOCKET_ERROR)
			localPort = ntohs(saddr.sin_port);
	}

	if (localPort != 0) 
	{
		ThreadData* T = proto->MSN_GetThreadByPort(localPort);
		if (T != NULL && T->s == NULL) 
		{
			T->s = hNewConnection;
			ReleaseSemaphore(T->hWaitEvent, 1, NULL);
			return;
		}
		proto->MSN_DebugLog("There's no registered file transfers for incoming port #%d, connection closed", localPort);
	}
	else proto->MSN_DebugLog("Unable to determine the local port, file server connection closed.");

	Netlib_CloseHandle(hNewConnection);
}


void CMsnProto::sttSetMirVer(HANDLE hContact, DWORD dwValue, bool always)
{
	static const char* MirVerStr[] =
	{
		"MSN 4.x-5.x",
		"MSN 6.0",
		"MSN 6.1",
		"MSN 6.2",
		"MSN 7.0",
		"MSN 7.5",
		"WLM 8.0",
		"WLM 8.1",
		"WLM 8.5",
		"WLM 9.0 Beta",
		"WLM 2009",
		"WLM 2011",
		"WLM 2012",
		"WLM Unknown",
	};

	if (dwValue & 0x1)
		setString(hContact, "MirVer", "MSN Mobile");
	else if (dwValue & 0x200)
		setString(hContact, "MirVer", "Webmessenger");
	else if (dwValue == 0x800800)
		setString(hContact, "MirVer", "Yahoo");
	else if (dwValue == 0x800)
		setString(hContact, "MirVer", "LCS");
	else if (dwValue == 0x50000000)
		setString(hContact, "MirVer", "Miranda IM 0.5.x (MSN v.0.5.x)");
	else if (dwValue == 0x30000024)
		setString(hContact, "MirVer", "Miranda IM 0.4.x (MSN v.0.4.x)");
	else if (always || getByte(hContact, "StdMirVer", 0)) 
	{
		unsigned wlmId = min(dwValue >> 28 & 0xff, SIZEOF(MirVerStr)-1);
		setString(hContact, "MirVer", MirVerStr[wlmId]);
	}
	else 
		return;

	setByte(hContact, "StdMirVer", 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Processes various invitations

void CMsnProto::sttInviteMessage(ThreadData* info, char* msgBody, char* email, char* nick)
{
	MimeHeaders tFileInfo;
	tFileInfo.readFromBuffer(msgBody);

	const char* Appname = tFileInfo["Application-Name"];
	const char* AppGUID = tFileInfo["Application-GUID"];
	const char* Invcommand = tFileInfo["Invitation-Command"];
	const char* Invcookie = tFileInfo["Invitation-Cookie"];
	const char* Appfile = tFileInfo["Application-File"];
	const char* Appfilesize = tFileInfo["Application-FileSize"];
	const char* IPAddress = tFileInfo["IP-Address"];
	const char* IPAddressInt = tFileInfo["IP-Address-Internal"];
	const char* Port = tFileInfo["Port"];
	const char* PortX = tFileInfo["PortX"];
	const char* PortXInt = tFileInfo["PortX-Internal"];
	const char* AuthCookie = tFileInfo["AuthCookie"];
	const char* SessionID = tFileInfo["Session-ID"];
	const char* SessionProtocol = tFileInfo["Session-Protocol"];
//	const char* Connectivity = tFileInfo["Connectivity"];
 
	if (AppGUID != NULL)
	{
		if (!strcmp(AppGUID, "{02D3C01F-BF30-4825-A83A-DE7AF41648AA}")) 
		{
			MSN_ShowPopup(info->getContactHandle(),
				TranslateT("Contact tried to open an audio conference (currently not supported)"),
				MSN_ALLOW_MSGBOX);
			return;
		}	
	}

	if (Invcommand && (strcmp(Invcommand, "CANCEL") == 0)) 
	{
		delete info->mMsnFtp;
		info->mMsnFtp = NULL;
	}

	if (Appname != NULL && Appfile != NULL && Appfilesize != NULL)  // receive first
	{
		filetransfer* ft = info->mMsnFtp = new filetransfer(this);

		ft->std.hContact = MSN_HContactFromEmail(email, nick, true, true);
		mir_free(ft->std.tszCurrentFile);
		ft->std.tszCurrentFile = mir_utf8decodeT(Appfile);
		ft->std.totalBytes = ft->std.currentFileSize = _atoi64(Appfilesize);
		ft->std.totalFiles = 1;
		ft->szInvcookie = mir_strdup(Invcookie);
		ft->p2p_dest = mir_strdup(email);

		TCHAR tComment[40];
		mir_sntprintf(tComment, SIZEOF(tComment), TranslateT("%I64u bytes"), ft->std.currentFileSize);

		PROTORECVFILET pre = {0};
		pre.flags = PREF_TCHAR;
		pre.fileCount = 1;
		pre.timestamp = time(NULL);
		pre.tszDescription = tComment;
		pre.ptszFiles = &ft->std.tszCurrentFile;
		pre.lParam = (LPARAM)ft;

		CCSDATA ccs;
		ccs.hContact = ft->std.hContact;
		ccs.szProtoService = PSR_FILE;
		ccs.wParam = 0;
		ccs.lParam = (LPARAM)&pre;
		MSN_CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
		return;
	}

	if (IPAddress != NULL && Port != NULL && AuthCookie != NULL) // receive Second
	{
		ThreadData* newThread = new ThreadData;

		if (inet_addr(IPAddress) != MyConnection.extIP || !IPAddressInt)
			mir_snprintf(newThread->mServer, sizeof(newThread->mServer), "%s:%s", IPAddress, Port);
		else
			mir_snprintf(newThread->mServer, sizeof(newThread->mServer), "%s:%u", IPAddressInt, atol(PortXInt) ^ 0x3141);

		newThread->mType = SERVER_FILETRANS;

		if (info->mMsnFtp == NULL)
		{
			ThreadData* otherThread = MSN_GetOtherContactThread(info);
			if (otherThread)
			{
				info->mMsnFtp = otherThread->mMsnFtp;
				otherThread->mMsnFtp = NULL;
			}
		}

		newThread->mMsnFtp = info->mMsnFtp; info->mMsnFtp = NULL;
		strcpy(newThread->mCookie, AuthCookie);

		newThread->startThread(&CMsnProto::MSNServerThread, this);
		return;
	}

	if (Invcommand != NULL && Invcookie != NULL && Port == NULL && AuthCookie == NULL && SessionID == NULL)  // send 1
	{
		msnftp_startFileSend(info, Invcommand, Invcookie);
		return;
	}

	if (Appname == NULL && SessionID != NULL && SessionProtocol != NULL)  // netmeeting send 1
	{
		if (!_stricmp(Invcommand,"ACCEPT")) 
		{
			ShellExecuteA(NULL, "open", "conf.exe", NULL, NULL, SW_SHOW);
			Sleep(3000);

			char command[1024];
			int  nBytes = mir_snprintf(command, sizeof(command),
				"MIME-Version: 1.0\r\n"
				"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
				"Invitation-Command: ACCEPT\r\n"
				"Invitation-Cookie: %s\r\n"
				"Session-ID: {1A879604-D1B8-11D7-9066-0003FF431510}\r\n"
				"Launch-Application: TRUE\r\n"
				"IP-Address: %s\r\n\r\n",
				Invcookie, MyConnection.GetMyExtIPStr());
			info->sendPacket("MSG", "N %d\r\n%s", nBytes, command);
		}
		return;
	}

	if (Appname != NULL && !_stricmp(Appname,"NetMeeting")) // netmeeting receive 1
	{
		char command[1024];
		int nBytes;

		TCHAR text[512], *tszEmail = mir_a2t(email);
		mir_sntprintf(text, SIZEOF(text), TranslateT("Accept NetMeeting request from %s?"), tszEmail);
		mir_free(tszEmail);

		if (MessageBox(NULL, text, TranslateT("MSN Protocol"), MB_YESNO | MB_ICONQUESTION) == IDYES) 
		{
			nBytes = mir_snprintf(command, sizeof(command),
				"MIME-Version: 1.0\r\n"
				"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
				"Invitation-Command: ACCEPT\r\n"
				"Invitation-Cookie: %s\r\n"
				"Session-ID: {A2ED5ACF-F784-4B47-A7D4-997CD8F643CC}\r\n"
				"Session-Protocol: SM1\r\n"
				"Launch-Application: TRUE\r\n"
				"Request-Data: IP-Address:\r\n"
				"IP-Address: %s\r\n\r\n",
				Invcookie, MyConnection.GetMyExtIPStr());
		}
		else 
		{
			nBytes = mir_snprintf(command, sizeof(command),
				"MIME-Version: 1.0\r\n"
				"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
				"Invitation-Command: CANCEL\r\n"
				"Invitation-Cookie: %s\r\n"
				"Cancel-Code: REJECT\r\n\r\n",
				Invcookie);
		}
		info->sendPacket("MSG", "N %d\r\n%s", nBytes, command);
		return;
	}

	if (IPAddress != NULL && Port == NULL && SessionID != NULL && SessionProtocol == NULL) { // netmeeting receive 2
		char	ipaddr[256];
		mir_snprintf(ipaddr, sizeof(ipaddr), "callto://%s", IPAddress);
		ShellExecuteA(NULL, "open", ipaddr, NULL, NULL, SW_SHOW);
}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Processes custom smiley messages

void CMsnProto::sttCustomSmiley(const char* msgBody, char* email, char* nick, int iSmileyType)
{
	HANDLE hContact = MSN_HContactFromEmail(email, nick, true, true);

	char smileyList[500] = "";

	const char *tok1 = msgBody, *tok2;
	char *smlp = smileyList;
	char lastsml[50];

	unsigned iCount = 0;
	bool parseSmiley = true;

	for(;;)
	{
		tok2 = strchr(tok1, '\t');
		if (tok2 == NULL) break;

		size_t sz = tok2 - tok1;
		if (parseSmiley)
		{
			sz = min(sz, sizeof(lastsml) - 1);
			memcpy(lastsml, tok1, sz);
			lastsml[sz] = 0;

			memcpy(smlp, tok1, sz); smlp += sz;
			*(smlp++) = '\n'; *smlp = 0;
			++iCount;
		}
		else
		{
			filetransfer* ft = new filetransfer(this);
			ft->std.hContact = hContact;

			ft->p2p_object = (char*)mir_alloc(sz + 1);
			memcpy(ft->p2p_object, tok1, sz);
			ft->p2p_object[sz] = 0;

			size_t slen = strlen(lastsml);
			size_t rlen = Netlib_GetBase64EncodedBufferSize(slen);
			char* buf = (char*)mir_alloc(rlen);

			NETLIBBASE64 nlb = { buf, (int)rlen, (PBYTE)lastsml, (int)slen };
			MSN_CallService(MS_NETLIB_BASE64ENCODE, 0, LPARAM(&nlb));

			char* smileyName = (char*)mir_alloc(rlen*3);
			UrlEncode(buf, smileyName, rlen*3);
			mir_free(buf);

			TCHAR path[MAX_PATH];
			MSN_GetCustomSmileyFileName(hContact, path, SIZEOF(path), smileyName, iSmileyType);
			ft->std.tszCurrentFile = mir_tstrdup(path);
			mir_free(smileyName);

			if (p2p_IsDlFileOk(ft))
				delete ft;
			else
			{
				MSN_DebugLog("Custom Smiley p2p invite for object : %s", ft->p2p_object);
				p2p_invite(iSmileyType, ft, email);
				Sleep(3000);
			}
		}
		parseSmiley = !parseSmiley;
		tok1 = tok2 + 1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//	MSN_ReceiveMessage - receives message or a file from the server
/////////////////////////////////////////////////////////////////////////////////////////


void CMsnProto::MSN_ReceiveMessage(ThreadData* info, char* cmdString, char* params)
{
	union 
	{
		char* tWords[6];
		struct { char *fromEmail, *fromNick, *strMsgBytes; } data;
		struct { char *fromEmail, *fromNetId, *toEmail, *toNetId, *typeId, *strMsgBytes; } datau;
	};

	if (sttDivideWords(params, SIZEOF(tWords), tWords) < 3)
	{
		MSN_DebugLog("Invalid %.3s command, ignoring", cmdString);
		return;
	}

	int msgBytes;
	char *nick, *email;
	bool ubmMsg = strncmp(cmdString, "UBM", 3) == 0;
	bool sentMsg = false;

	if (ubmMsg)
	{
		msgBytes = atol(datau.strMsgBytes);
		nick = datau.fromEmail;
		email = datau.fromEmail;
	}
	else
	{
		msgBytes = atol(data.strMsgBytes);
		nick = data.fromNick;
		email = data.fromEmail;
		UrlDecode(nick);
	}
	stripBBCode(nick);
	stripColorCode(nick);

	char* msg = (char*)alloca(msgBytes+1);

	HReadBuffer buf(info, 0);
	BYTE* msgb = buf.surelyRead(msgBytes);
	if (msgb == NULL) return;

	memcpy(msg, msgb, msgBytes);
	msg[msgBytes] = 0;

	MSN_DebugLog("Message:\n%s", msg);

	MimeHeaders tHeader;
	char* msgBody = tHeader.readFromBuffer(msg);

	const char* tMsgId = tHeader["Message-ID"];

	// Chunked message
	char* newbody = NULL;
	if (tMsgId)
	{
		int idx;
		const char* tChunks = tHeader["Chunks"];
		if (tChunks)
			idx = addCachedMsg(tMsgId, msg, 0, msgBytes, atol(tChunks), true);
		else
			idx = addCachedMsg(tMsgId, msgBody, 0, strlen(msgBody), 0, true);

		size_t newsize;
		if (!getCachedMsg(idx, newbody, newsize)) return;
		msgBody = tHeader.readFromBuffer(newbody);
	}

	// message from the server (probably)
	if (!ubmMsg && strchr(email, '@') == NULL && _stricmp(email, "Hotmail"))
		return;

	const char* tContentType = tHeader["Content-Type"];
	if (tContentType == NULL)
		return;

	if (!_strnicmp(tContentType, "text/x-clientcaps", 17)) 
	{
		MimeHeaders tFileInfo;
		tFileInfo.readFromBuffer(msgBody);
		info->firstMsgRecv = true;

		HANDLE hContact = MSN_HContactFromEmail(email);
		const char* mirver = tFileInfo["Client-Name"];
		if (hContact != NULL && mirver != NULL)
		{
			setString(hContact, "MirVer", mirver);
			deleteSetting(hContact, "StdMirVer");
		}
	}
	else if (!ubmMsg && !info->firstMsgRecv) 
	{
		info->firstMsgRecv = true;
		MsnContact *cont = Lists_Get(email);
		if (cont && cont->hContact != NULL)
			sttSetMirVer(cont->hContact, cont->cap1, true);
	}

	if (!_strnicmp(tContentType, "text/plain", 10)) 
	{
		CCSDATA ccs = {0};

		ccs.hContact = MSN_HContactFromEmail(email, nick, true, true);

		const char* p = tHeader["X-MMS-IM-Format"];
		bool isRtl =  p != NULL && strstr(p, "RL=1") != NULL;

		if (info->mJoinedContactsWLID.getCount() > 1) 
		{
			if (msnHaveChatDll)
				MSN_ChatStart(info);
			else
			{
				for (int j=0; j < info->mJoinedContactsWLID.getCount(); j++) 
				{
					if (_stricmp(info->mJoinedContactsWLID[j], email) == 0 && j != 0) 
					{
						ccs.hContact = info->getContactHandle();
						break;
					}	
				}
			}
		}
		else
		{
			char* szEmail;
			parseWLID(NEWSTR_ALLOCA(email), NULL, &szEmail, NULL);
			sentMsg = _stricmp(szEmail, MyOptions.szEmail) == 0;
			if (sentMsg)
				ccs.hContact = ubmMsg ? MSN_HContactFromEmail(datau.toEmail, nick) : 
					info->getContactHandle();
		}

		const char* tP4Context = tHeader["P4-Context"];
		if (tP4Context) 
		{
			size_t newlen  = strlen(msgBody) + strlen(tP4Context) + 4;
			char* newMsgBody = (char*)mir_alloc(newlen);
			mir_snprintf(newMsgBody, newlen, "[%s] %s", tP4Context, msgBody);
			mir_free(newbody);
			msgBody = newbody = newMsgBody;
		}

		if (info->mChatID[0]) 
		{
			GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_MESSAGE };
			gcd.ptszID = info->mChatID;

			GCEVENT gce = {0};
			gce.cbSize = sizeof(GCEVENT);
			gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
			gce.pDest = &gcd;
			gce.ptszUID = mir_a2t(email);
			gce.ptszNick = GetContactNameT(ccs.hContact);
			gce.time = time(NULL);
			gce.bIsMe = FALSE;

			TCHAR* p = mir_utf8decodeT(msgBody);
			gce.ptszText = EscapeChatTags(p);
			mir_free(p);

			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
			mir_free((void*)gce.pszText);
			mir_free((void*)gce.ptszUID);
		}
		else if (ccs.hContact)
		{
			if (!sentMsg)
			{
				MSN_CallService(MS_PROTO_CONTACTISTYPING, WPARAM(ccs.hContact), 0);
				
				PROTORECVEVENT pre;
				pre.szMessage = (char*)msgBody;
				pre.flags = PREF_UTF + (isRtl ? PREF_RTL : 0);
				pre.timestamp = (DWORD)time(NULL);
				pre.lParam = 0;

				ccs.szProtoService = PSR_MESSAGE;
				ccs.wParam = 0;
				ccs.lParam = (LPARAM)&pre;
				MSN_CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
			}
			else
			{
				DBEVENTINFO dbei = {0};

				bool haveWnd = MSN_MsgWndExist(ccs.hContact);
				
				dbei.cbSize = sizeof(dbei);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.flags = DBEF_SENT | DBEF_UTF | (haveWnd ? 0 : DBEF_READ) | (isRtl ? DBEF_RTL : 0);
				dbei.szModule = m_szModuleName;
				dbei.timestamp = time(NULL);
				dbei.cbBlob = (unsigned)strlen(msgBody) + 1;
				dbei.pBlob = (PBYTE)msgBody;
				MSN_CallService(MS_DB_EVENT_ADD, (WPARAM)ccs.hContact, (LPARAM)&dbei);
			}
		}
	}
	else if (!_strnicmp(tContentType, "text/x-msmsgsprofile", 20)) 
	{
		replaceStr(msnExternalIP, tHeader["ClientIP"]);
		abchMigrated = atol(tHeader["ABCHMigrated"]);
		langpref = atol(tHeader["lang_preference"]);
		emailEnabled = atol(tHeader["EmailEnabled"]);

		if (!MSN_RefreshContactList()) 
		{
			SendBroadcast(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NOSERVER);
			info->sendTerminate();
		}
		else
		{
			MSN_SetServerStatus(m_iDesiredStatus);
			MSN_EnableMenuItems(true);
		}
	}
	else if (!_strnicmp(tContentType, "text/x-msmsgscontrol", 20)) 
	{
		const char* tTypingUser = tHeader["TypingUser"];

		if (tTypingUser != NULL && info->mChatID[0] == 0 && _stricmp(email, MyOptions.szEmail)) 
		{
			HANDLE hContact = MSN_HContactFromEmail(tTypingUser, tTypingUser);
			MSN_CallService(MS_PROTO_CONTACTISTYPING, (WPARAM) hContact, 7);
		}
	}
	else if (!_strnicmp(tContentType, "text/x-msnmsgr-datacast", 23)) 
	{
		if (info->mJoinedContactsWLID.getCount())
		{
			HANDLE tContact;

			if (info->mChatID[0])
			{
				GC_INFO gci = {0};
				gci.Flags = HCONTACT;
				gci.pszModule = m_szModuleName;
				gci.pszID = info->mChatID;
				CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci);
				tContact = gci.hContact;
			}
			else
				tContact = info->getContactHandle();

			MimeHeaders tFileInfo;
			tFileInfo.readFromBuffer(msgBody);

			const char* id = tFileInfo["ID"];
			if (id != NULL)
			{
				switch (atol(id))
				{
					case 1:  // Nudge
						NotifyEventHooks(hMSNNudge, (WPARAM)tContact, 0);
						break;

					case 2: // Wink
						break;

					case 4: // Action Message
						break;
				}
			}
		}
	}
	else if (!_strnicmp(tContentType,"text/x-msmsgsemailnotification", 30))
		sttNotificationMessage(msgBody, false);
	else if (!_strnicmp(tContentType, "text/x-msmsgsinitialemailnotification", 37))
		sttNotificationMessage(msgBody, true);
	else if (!_strnicmp(tContentType, "text/x-msmsgsactivemailnotification", 35))
		sttNotificationMessage(msgBody, false);
	else if (!_strnicmp(tContentType, "text/x-msmsgsinitialmdatanotification", 37))
		sttNotificationMessage(msgBody, true);
	else if (!_strnicmp(tContentType, "text/x-msmsgsoimnotification", 28))
		sttNotificationMessage(msgBody, false);
	else if (!_strnicmp(tContentType, "text/x-msmsgsinvite", 19))
		sttInviteMessage(info, msgBody, email, nick);
	else if (!_strnicmp(tContentType, "application/x-msnmsgrp2p", 24))
	{
		const char* dest = tHeader["P2P-Dest"];
		if (dest)
		{
			char *szEmail, *szInst;
			parseWLID(NEWSTR_ALLOCA(dest), NULL, &szEmail, &szInst);

			if (stricmp(szEmail, MyOptions.szEmail) == 0)
			{
				const char* src = tHeader["P2P-Src"];
				if (src == NULL) src = email;

				if (szInst == NULL)
					p2p_processMsg(info, msgBody, src);
				else if (stricmp(szInst, MyOptions.szMachineGuidP2P) == 0)
					p2p_processMsgV2(info, msgBody, src);
			}
		}
	}
	else if (!_strnicmp(tContentType, "text/x-mms-emoticon", 19))
		sttCustomSmiley(msgBody, email, nick, MSN_APPID_CUSTOMSMILEY);
	else if (!_strnicmp(tContentType, "text/x-mms-animemoticon", 23))
		sttCustomSmiley(msgBody, email, nick, MSN_APPID_CUSTOMANIMATEDSMILEY);

	mir_free(newbody);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Process Yahoo Find

void CMsnProto::sttProcessYFind(char* buf, size_t len)
{
	if (buf == NULL) return;
	ezxml_t xmli = ezxml_parse_str(buf, len);
	
	ezxml_t dom  = ezxml_child(xmli, "d");
	const char* szDom = ezxml_attr(dom, "n");

	ezxml_t cont = ezxml_child(dom, "c");
	const char* szCont = ezxml_attr(cont, "n");
			
	char szEmail[128];
	mir_snprintf(szEmail, sizeof(szEmail), "%s@%s", szCont, szDom);

	const char *szNetId = ezxml_attr(cont, "t");
	if (msnSearchId != NULL)
	{
		if (szNetId != NULL)
		{
			TCHAR* szEmailT = mir_utf8decodeT(szEmail);
			PROTOSEARCHRESULT isr = {0};
			isr.cbSize = sizeof(isr);
			isr.flags = PSR_TCHAR;
			isr.id = szEmailT;
			isr.nick = szEmailT;
			isr.email = szEmailT;

			SendBroadcast(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, msnSearchId, (LPARAM)&isr);
			mir_free(szEmailT);
		}
		SendBroadcast(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, msnSearchId, 0);
	
		msnSearchId = NULL;
	}
	else
	{
		if (szNetId != NULL)
		{
			int netId = atol(szNetId);
			HANDLE hContact = MSN_HContactFromEmail(szEmail, szEmail, true, false);
			if (MSN_AddUser(hContact, szEmail, netId, LIST_FL))
			{
				MSN_AddUser(hContact, szEmail, netId, LIST_PL + LIST_REMOVE);
				MSN_AddUser(hContact, szEmail, netId, LIST_BL + LIST_REMOVE);
				MSN_AddUser(hContact, szEmail, netId, LIST_AL);
				DBDeleteContactSetting(hContact, "CList", "Hidden");
			}
			MSN_SetContactDb(hContact, szEmail);
		}
	}

	ezxml_free(xmli);
}			


/////////////////////////////////////////////////////////////////////////////////////////
// Process user addition

void CMsnProto::sttProcessAdd(char* buf, size_t len)
{
	if (buf == NULL) return;

	ezxml_t xmli = ezxml_parse_str(buf, len);
	ezxml_t dom  = ezxml_child(xmli, "d");
	while (dom != NULL)
	{
		const char* szDom = ezxml_attr(dom, "n");
		ezxml_t cont = ezxml_child(dom, "c");
		while (cont != NULL)
		{
			const char* szCont = ezxml_attr(cont, "n");
			const char* szNick = ezxml_attr(cont, "f");
			int listId =  atol(ezxml_attr(cont, "l"));
			int netId =  atol(ezxml_attr(cont, "t"));
			
			char szEmail[128];
			mir_snprintf(szEmail, sizeof(szEmail), "%s@%s", szCont, szDom);

			UrlDecode((char*)szNick);

			if (listId == LIST_FL)
			{
				HANDLE hContact = MSN_HContactFromEmail(szEmail, szNick, true, false);
				MSN_SetContactDb(hContact, szEmail);
			}

			if (listId == LIST_RL)
				MSN_SharingFindMembership(true);
			else
				MSN_AddUser(NULL, szEmail, netId, listId);

			MsnContact* msc = Lists_Get(szEmail);
			if (msc == NULL)
			{
				Lists_Add(listId, netId, szEmail);
				msc = Lists_Get(szEmail);
			}

			if (listId == LIST_RL)
			{
				if ((msc->list & (LIST_AL | LIST_BL)) == 0)
				{
					MSN_AddAuthRequest(szEmail, szNick, msc->invite);
					msc->netId = netId;
				}
				else
					MSN_AddUser(NULL, szEmail, netId, LIST_PL + LIST_REMOVE);
			}

			cont = ezxml_next(cont);
		}
		dom = ezxml_next(dom);
	}
	ezxml_free(xmli);
}


void CMsnProto::sttProcessRemove(char* buf, size_t len)
{
	ezxml_t xmli = ezxml_parse_str(buf, len);
	ezxml_t dom  = ezxml_child(xmli, "d");
	while (dom != NULL)
	{
		const char* szDom = ezxml_attr(dom, "n");
		ezxml_t cont = ezxml_child(dom, "c");
		while (cont != NULL)
		{
			const char* szCont = ezxml_attr(cont, "n");
			int listId =  atol(ezxml_attr(cont, "l"));
			
			char szEmail[128];
			mir_snprintf(szEmail, sizeof(szEmail), "%s@%s", szCont, szDom);
			Lists_Remove(listId, szEmail);

			MsnContact* msc = Lists_Get(szEmail);
			if (msc == NULL || (msc->list & (LIST_RL | LIST_FL | LIST_LL)) == 0) 
			{
				if (msc->hContact && _stricmp(szEmail, MyOptions.szEmail))
				{
					MSN_CallService(MS_DB_CONTACT_DELETE, (WPARAM)msc->hContact, 0);
					msc->hContact = NULL;
				}
			}

			cont = ezxml_next(cont);
		}
		dom = ezxml_next(dom);
	}
	ezxml_free(xmli);
}


/////////////////////////////////////////////////////////////////////////////////////////
//	MSN_HandleCommands - process commands from the server
/////////////////////////////////////////////////////////////////////////////////////////

void CMsnProto::sttProcessStatusMessage(char* buf, unsigned len, const char* wlid)
{
	HANDLE hContact = MSN_HContactFromEmail(wlid);
	if (hContact == NULL) return;

	ezxml_t xmli = ezxml_parse_str(buf, len);

	char* szEmail;
	parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);

	// Add endpoints
	for (ezxml_t endp = ezxml_child(xmli, "EndpointData"); endp; endp = ezxml_next(endp))
	{
		const char *id = ezxml_attr(endp, "id");
		const char *caps = ezxml_txt(ezxml_child(endp, "Capabilities"));
		char* end = NULL;
		unsigned cap1 = caps ? strtoul(caps, &end, 10) : 0;
		unsigned cap2 = end && *end == ':' ? strtoul(end + 1, NULL, 10) : 0;

		Lists_AddPlace(szEmail, id, cap1, cap2);
	}

	// Process status message info
	const char* szStatMsg = ezxml_txt(ezxml_child(xmli, "PSM"));
	if (*szStatMsg)
	{
		stripBBCode((char*)szStatMsg);
		stripColorCode((char*)szStatMsg);
		DBWriteContactSettingStringUtf(hContact, "CList", "StatusMsg", szStatMsg);
	}
	else
		DBDeleteContactSetting(hContact, "CList", "StatusMsg");

	mir_utf8decode((char*)szStatMsg, NULL);
	SendBroadcast(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, NULL, (LPARAM)szStatMsg);

	// Process current media info
	const char* szCrntMda = ezxml_txt(ezxml_child(xmli, "CurrentMedia"));
	if (!*szCrntMda)
	{
		deleteSetting(hContact, "ListeningTo");
		ezxml_free(xmli);
		return;
	}

	// Get parts separeted by "\\0"
	char *parts[16];
	unsigned pCount;

	char* p = (char*)szCrntMda;
	for (pCount = 0; pCount < SIZEOF(parts); ++pCount)
	{
		parts[pCount] = p;

		char* p1 = strstr(p, "\\0");
		if (p1 == NULL) break;

		*p1 = '\0';
		p = p1 + 2;
	}

	// Now let's mount the final string
	if (pCount <= 4)  
	{
		deleteSetting(hContact, "ListeningTo");
		ezxml_free(xmli);
		return;
	}

	// Check if there is any info in the string
	bool foundUsefullInfo = false;
	for (unsigned i = 4; i < pCount; i++) 
	{
		if (parts[i][0] != '\0')  
		{
			foundUsefullInfo = true;
			break;
		}
	}
	if (!foundUsefullInfo) 
	{
		deleteSetting(hContact, "ListeningTo");
		ezxml_free(xmli);
		return;
	}

	if (!ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT) ||
		!ServiceExists(MS_LISTENINGTO_OVERRIDECONTACTOPTION) ||
		!CallService(MS_LISTENINGTO_OVERRIDECONTACTOPTION, 0, (LPARAM) hContact))
	{
		// User contact options
		char *format = mir_strdup(parts[3]);
		char *unknown = NULL;
		if (ServiceExists(MS_LISTENINGTO_GETUNKNOWNTEXT))
			unknown = mir_utf8encodeT((TCHAR *) CallService(MS_LISTENINGTO_GETUNKNOWNTEXT, 0, 0));

		for (unsigned i = 4; i < pCount; i++) 
		{
			char part[16];
			size_t lenPart = mir_snprintf(part, sizeof(part), "{%d}", i - 4);
			if (parts[i][0] == '\0' && unknown != NULL)
				parts[i] = unknown;
			size_t lenPartsI = strlen(parts[i]);
			for (p = strstr(format, part); p; p = strstr(p + lenPartsI, part)) 
			{
				if (lenPart < lenPartsI) 
				{
					int loc = p - format;
					format = (char *)mir_realloc(format, strlen(format) + (lenPartsI - lenPart) + 1);
					p = format + loc;
				}
				memmove(p + lenPartsI, p + lenPart, strlen(p + lenPart) + 1);
				memmove(p, parts[i], lenPartsI);
			}	
		}

		setStringUtf(hContact, "ListeningTo", format);
		mir_free(unknown);
		mir_free(format);
	}
	else
	{
		// Use user options
		LISTENINGTOINFO lti = {0};
		lti.cbSize = sizeof(LISTENINGTOINFO);

		lti.ptszTitle = mir_utf8decodeT(parts[4]);
		if (pCount > 5)  lti.ptszArtist = mir_utf8decodeT(parts[5] );
		if (pCount > 6)  lti.ptszAlbum  = mir_utf8decodeT(parts[6] );
		if (pCount > 7)  lti.ptszTrack  = mir_utf8decodeT(parts[7] );
		if (pCount > 8)  lti.ptszYear   = mir_utf8decodeT(parts[8] );
		if (pCount > 9)  lti.ptszGenre  = mir_utf8decodeT(parts[9] );
		if (pCount > 10) lti.ptszLength = mir_utf8decodeT(parts[10]);
		if (pCount > 11) lti.ptszPlayer = mir_utf8decodeT(parts[11]);
		else lti.ptszPlayer = mir_utf8decodeT(parts[0]);
		if (pCount > 12) lti.ptszType = mir_utf8decodeT(parts[12]);
		else lti.ptszType = mir_utf8decodeT(parts[1]);

		TCHAR *cm = (TCHAR *) CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM) _T("%title% - %artist%"), (LPARAM) &lti);
		setTString(hContact, "ListeningTo", cm);

		mir_free(cm);

		mir_free(lti.ptszArtist);
		mir_free(lti.ptszAlbum);
		mir_free(lti.ptszTitle);
		mir_free(lti.ptszTrack);
		mir_free(lti.ptszYear);
		mir_free(lti.ptszGenre);
		mir_free(lti.ptszLength);
		mir_free(lti.ptszPlayer);
		mir_free(lti.ptszType);
	}
	ezxml_free(xmli);
}


void CMsnProto::sttProcessPage(char* buf, unsigned len)
{
	if (buf == NULL) return;
	ezxml_t xmlnot = ezxml_parse_str(buf, len);

	ezxml_t xmlbdy = ezxml_get(xmlnot, "MSG", 0, "BODY", -1);
	const char* szMsg = ezxml_txt(ezxml_child(xmlbdy, "TEXT"));
	const char* szTel = ezxml_attr(ezxml_child(xmlnot, "FROM"), "name");

	if (szTel && *szMsg)
	{
		PROTORECVEVENT pre = {0};
		pre.szMessage = (char*)szMsg;
		pre.flags = PREF_UTF /*+ ((isRtl) ? PREF_RTL : 0)*/;
		pre.timestamp = time(NULL);

		CCSDATA ccs = {0};
		ccs.hContact = MSN_HContactFromEmail(szTel, szTel, true, true);
		ccs.szProtoService = PSR_MESSAGE;
		ccs.lParam = (LPARAM)&pre;
		MSN_CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);

	}
	ezxml_free(xmlnot);
}


void CMsnProto::sttProcessNotificationMessage(char* buf, unsigned len)
{
	if (buf == NULL) return;
	ezxml_t xmlnot = ezxml_parse_str(buf, len);

	if (strcmp(ezxml_attr(xmlnot, "siteid"), "0") == 0)
	{
		ezxml_free(xmlnot);
		return;
	}

	ezxml_t xmlmsg = ezxml_child(xmlnot, "MSG");
	ezxml_t xmlact = ezxml_child(xmlmsg, "ACTION");
	ezxml_t xmlbdy = ezxml_child(xmlmsg, "BODY");
	ezxml_t xmltxt = ezxml_child(xmlbdy, "TEXT");

	if (xmltxt != NULL)
	{
		char fullurl[1024];
		size_t sz = 0;

		const char* acturl = ezxml_attr(xmlact, "url");
		if (acturl == NULL || strstr(acturl, "://") == NULL) 
			sz += mir_snprintf(fullurl+sz, sizeof(fullurl)-sz, "%s", ezxml_attr(xmlnot, "siteurl"));
		
		sz += mir_snprintf(fullurl+sz, sizeof(fullurl)-sz, "%s", acturl);
		if (sz != 0 && fullurl[sz-1] != '?')
			sz += mir_snprintf(fullurl+sz, sizeof(fullurl)-sz, "?");

		mir_snprintf(fullurl+sz, sizeof(fullurl)-sz, "notification_id=%s&message_id=%s",
			ezxml_attr(xmlnot, "id"), ezxml_attr(xmlmsg, "id"));

		SkinPlaySound(alertsoundname);

		TCHAR* alrt = mir_utf8decodeT(ezxml_txt(xmltxt));
		MSN_ShowPopup(TranslateT("MSN Alert"), alrt, MSN_ALERT_POPUP | MSN_ALLOW_MSGBOX, fullurl);
		mir_free(alrt);
	}
	else if (xmlbdy)
	{
		const char *txt = ezxml_txt(xmlbdy);
		if (strstr(txt, "ABCHInternal"))
		{
			MSN_SharingFindMembership(true);
			MSN_ABFind("ABFindContactsPaged", NULL, true);
			MSN_StoreGetProfile();
		}
	}
	ezxml_free(xmlnot);
}

void CMsnProto::MSN_InitSB(ThreadData* info, const char* szEmail)
{
	MsnContact *cont = Lists_Get(szEmail);

	if (cont->netId == NETID_MSN)
		info->sendCaps();

	bool typing = false;

	for (int i=3; --i;)
	{
		MsgQueueEntry E;
		while (MsgQueue_GetNext(szEmail, E))
		{
			if (E.msgType == 'X') ;
			else if (E.msgType == 2571)
				typing = E.flags != 0;
			else if (E.msgSize == 0) 
			{
				info->sendMessage(E.msgType, NULL, 1, E.message, E.flags);
				SendBroadcast(cont->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)E.seq, 0);
			}
			else
			{
				if (E.msgType == 'D' && !info->mBridgeInit /*&& strchr(data.flags, ':')*/)
				{
					info->mBridgeInit = true;

//					P2PV2_Header hdrdata(E.message);
//					P2PV2_Header tHdr;
//					tHdr.mID = hdrdata.mID;
//					p2p_sendMsg(info, E.wlid, 0, tHdr, NULL, 0);
				}
				info->sendRawMessage(E.msgType, E.message, E.msgSize);
			}

			mir_free(E.message);
			mir_free(E.wlid);

			if (E.ft != NULL)
				info->mMsnFtp = E.ft;
		}
		mir_free(info->mInitialContactWLID); info->mInitialContactWLID = NULL;
	}

	if (typing)
		MSN_StartStopTyping(info, true);

	if (getByte("EnableDeliveryPopup", 0))
	{
		MSN_ShowPopup(cont->hContact, info->mCaller ?
			TranslateT("Chat session established by my request") : 
			TranslateT("Chat session established by contact request"),
			0);
	}

	PROTO_AVATAR_INFORMATIONT ai = { sizeof(ai), cont->hContact };
	GetAvatarInfo(GAIF_FORCE, (LPARAM)&ai);
}

int CMsnProto::MSN_HandleCommands(ThreadData* info, char* cmdString)
{
	char* params = "";
	int trid = -1;

	if (cmdString[3]) 
	{
		if (isdigit((BYTE)cmdString[4])) 
		{
			trid = strtol(cmdString+4, &params, 10);
			switch (*params) 
			{
				case ' ':	case '\0':	case '\t':	case '\n':
					while (*params == ' ' || *params == '\t')
						params++;
					break;

				default: params = cmdString+4;
			}	
		}
		else params = cmdString+4;
	}
//	MSN_DebugLog("%s", cmdString);

	switch((*(PDWORD)cmdString & 0x00FFFFFF) | 0x20000000)
	{
		case ' KCA':    //********* ACK: section 8.7 Instant Messages
			ReleaseSemaphore(info->hWaitEvent, 1, NULL);

			if (info->mJoinedContactsWLID.getCount() > 0 && MyOptions.SlowSend)
				SendBroadcast(info->getContactHandle(), ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)trid, 0);
			break;

		case ' YQF':	//********* FQY: Find Yahoo User
			char* tWords[1];
			if (sttDivideWords(params, 1, tWords) != 1)
			{
				MSN_DebugLog("Invalid %.3s command, ignoring", cmdString);
			}
			else
			{
				size_t len = atol(tWords[0]);
				sttProcessYFind((char*)HReadBuffer(info, 0).surelyRead(len), len); 
			}
			break;

		case ' LDA':	//********* ADL: Add to the list
		{
			char* tWords[1];
			if (sttDivideWords(params, 1, tWords) != 1)
			{
LBL_InvalidCommand:
				MSN_DebugLog("Invalid %.3s command, ignoring", cmdString);
				break;
			}

			if (strcmp(tWords[0], "OK") != 0)
			{
				size_t len = atol(tWords[0]);
				sttProcessAdd((char*)HReadBuffer(info, 0).surelyRead(len), len); 
			}
			break;
		}

		case ' SBS':
			break;

		case ' SNA':    //********* ANS: section 8.4 Getting Invited to a Switchboard Session
			break;

		case ' PRP':
			break;

		case ' PLB':    //********* BLP: section 7.6 List Retrieval And Property Management
			break;

		case ' EYB':   //********* BYE: section 8.5 Session Participant Changes
		{
			union 
			{
				char* tWords[2];
				// modified for chat, orginally param2 = junk
				// param 2: quit due to idle = "1", normal quit = nothing
				struct { char *userEmail, *isIdle; } data;
			};

			sttDivideWords(params, 2, tWords);
			UrlDecode(data.userEmail);

			if (strchr(data.userEmail, ';'))
			{
				info->contactLeft(data.userEmail);
				break;
			}

			HANDLE hContact = MSN_HContactFromEmail(data.userEmail);

			if (getByte("EnableSessionPopup", 0))
				MSN_ShowPopup(hContact, TranslateT("Contact left channel"), 0);

			// modified for chat
			if (msnHaveChatDll) 
			{
				GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_QUIT };
				gcd.ptszID = info->mChatID;

				GCEVENT gce = {0};
				gce.cbSize = sizeof(GCEVENT);
				gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
				gce.pDest = &gcd;
				gce.ptszNick = GetContactNameT(hContact);
				gce.ptszUID = mir_a2t(data.userEmail);
				gce.time = time(NULL);
				gce.bIsMe = FALSE;
				CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
				mir_free((void*)gce.pszUID);
			}

			int personleft = info->contactLeft(data.userEmail);

			int temp_status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
			if (temp_status == ID_STATUS_INVISIBLE && MSN_GetThreadByContact(data.userEmail) == NULL)
				setWord(hContact, "Status", ID_STATUS_OFFLINE);

			// see if the session is quit due to idleness
			if (info->mChatID[0] && personleft == 1)
			{
				if (!strcmp(data.isIdle, "1")) 
				{
					GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_INFORMATION };
					gcd.ptszID = info->mChatID;

					GCEVENT gce = {0};
					gce.cbSize = sizeof(GCEVENT);
					gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
					gce.pDest = &gcd;
					gce.bIsMe = FALSE;
					gce.time = time(NULL);
					gce.ptszText = TranslateT("This conversation has been inactive, participants will be removed.");
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
					gce.ptszText = TranslateT("To resume the conversation, please quit this session and start a new chat session.");
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
				}
				else
				{
					if (!Miranda_Terminated() && MessageBox(NULL, 
								TranslateT("There is only 1 person left in the chat, do you want to switch back to standard message window?"),
								TranslateT("MSN Chat"), MB_YESNO|MB_ICONQUESTION) == IDYES)
					{
						// kill chat dlg and open srmm dialog
						MSN_KillChatSession(info->mChatID);
						
						// open up srmm dialog when quit while 1 person left
						HANDLE hContact = info->getContactHandle();
						if (hContact) CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
					}
				}
			}
			// this is not in chat session, quit the session when everyone left
			else if (info->mJoinedContactsWLID.getCount() < 1)
				return 1;

			break;
		}
		case ' LAC':    //********* CAL: section 8.3 Inviting Users to a Switchboard Session
			break;

		case ' GHC':    //********* CHG: section 7.7 Client States
		{
			int oldStatus = m_iStatus;
			int newStatus = MSNStatusToMiranda(params);
			if (oldStatus <= ID_STATUS_OFFLINE)
			{
				isConnectSuccess = true;
				int count = -1;
				for (;;)
				{
					MsnContact *msc = Lists_GetNext(count);
					if (msc == NULL) break;

					if (msc->netId == NETID_MOB)
						setWord(msc->hContact, "Status", ID_STATUS_ONTHEPHONE);
				}	
			}
			if (newStatus != ID_STATUS_IDLE)
			{
				m_iStatus = newStatus;
				SendBroadcast(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, newStatus);
				MSN_DebugLog("Status change acknowledged: %s", params);
				MSN_RemoveEmptyGroups();
			}
			if (newStatus == ID_STATUS_OFFLINE) return 1;
			break;
		}
		case ' LHC':    //********* CHL: Query from Server on MSNP7
		{
			char* authChallengeInfo;
			if (sttDivideWords(params, 1, &authChallengeInfo) != 1)
				goto LBL_InvalidCommand;

			char dgst[64];
			MSN_MakeDigest(authChallengeInfo, dgst);
			info->sendPacket("QRY", "%s 32\r\n%s", msnProductID, dgst);
			break;
		}
		case ' RVC':    //********* CVR: MSNP8
			break;

		case ' NLF':    //********* FLN: section 7.9 Notification Messages
		{	
			union 
			{
				char* tWords[2];
				struct { char *userEmail, *netId; } data;
			};

			int tArgs = sttDivideWords(params, 2, tWords);
			if (tArgs < 2)
				goto LBL_InvalidCommand;

			HANDLE hContact = MSN_HContactFromEmail(data.userEmail);
			if (hContact != NULL)
			{
				setWord(hContact, "Status", MSN_GetThreadByContact(data.userEmail) ? 
					ID_STATUS_INVISIBLE : ID_STATUS_OFFLINE);
				setDword(hContact, "IdleTS", 0);
				ForkThread(&CMsnProto::MsgQueue_AllClearThread, mir_strdup(data.userEmail));
			}
			break;
		}
		case ' NLI':
		case ' NLN':    //********* ILN/NLN: section 7.9 Notification Messages
		{
			union 
			{
				char* tWords[5];
				struct { char *userStatus, *wlid, *userNick, *objid, *cmdstring; } data;
			};

			int tArgs = sttDivideWords(params, 5, tWords);
			if (tArgs < 3)
				goto LBL_InvalidCommand;

			UrlDecode(data.userNick);
			stripBBCode(data.userNick);
			stripColorCode(data.userNick);

			bool isMe = false;
			char* szEmail, *szNet;
			parseWLID(NEWSTR_ALLOCA(data.wlid), &szNet, &szEmail, NULL);
			if (!stricmp(szEmail, MyOptions.szEmail) && !strcmp(szNet, "1"))
			{
				isMe = true;
				int newStatus = MSNStatusToMiranda(params);
				if (newStatus != m_iStatus && newStatus != ID_STATUS_IDLE)
				{
					int oldMode = m_iStatus;
					m_iDesiredStatus = m_iStatus = newStatus;
					SendBroadcast(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldMode, m_iStatus);
				}
			}

			WORD lastStatus = ID_STATUS_OFFLINE;

			MsnContact *cont = Lists_Get(szEmail);

			HANDLE hContact = NULL;
			if (!cont && !isMe)
			{
				hContact = MSN_HContactFromEmail(data.wlid, data.userNick, true, true);
				cont = Lists_Get(szEmail);
			}
			if (cont) hContact = cont->hContact;

			if (hContact != NULL)
			{
				setStringUtf(hContact, "Nick", data.userNick);
				lastStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
				if (lastStatus == ID_STATUS_OFFLINE || lastStatus == ID_STATUS_INVISIBLE)
					DBDeleteContactSetting(hContact, "CList", "StatusMsg");

				int newStatus = MSNStatusToMiranda(params);
				setWord(hContact, "Status",  newStatus != ID_STATUS_IDLE ? newStatus : ID_STATUS_AWAY);
				setDword(hContact, "IdleTS", newStatus != ID_STATUS_IDLE ? 0 : time(NULL));
			}

			if (tArgs > 3 && tArgs <= 5 && cont) 
			{
				UrlDecode(data.cmdstring);

				char* end = NULL;
				cont->cap1 = strtoul(data.objid, &end, 10);
				cont->cap2 = end && *end == ':' ? strtoul(end + 1, NULL, 10) : 0;

				if (lastStatus == ID_STATUS_OFFLINE) 
				{
					DBVARIANT dbv;
					bool always = getString(hContact, "MirVer", &dbv) != 0;
					if (!always) MSN_FreeVariant(&dbv);
					sttSetMirVer(hContact, cont->cap1, always);
				}

				if ((cont->cap1 & 0xf0000000) && data.cmdstring[0] && strcmp(data.cmdstring, "0")) 
				{
					char* szAvatarHash = MSN_GetAvatarHash(data.cmdstring);
					if (szAvatarHash == NULL) goto remove;

					setString(hContact, "PictContext", data.cmdstring);
					setString(hContact, "AvatarHash", szAvatarHash);

					if (hContact != NULL)
					{
						char szSavedHash[64] = "";
						getStaticString(hContact, "AvatarSavedHash", szSavedHash, sizeof(szSavedHash));
						if (stricmp(szSavedHash, szAvatarHash))
						{
							SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
						}
						else
						{
							char szSavedContext[64];
							int result = getStaticString(hContact, "PictSavedContext", szSavedContext, sizeof(szSavedContext));
							if (result || strcmp(szSavedContext, data.cmdstring))
								SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
						}
						mir_free(szAvatarHash);
					}
				}
				else 
				{
remove:
					deleteSetting(hContact, "AvatarHash");
					deleteSetting(hContact, "AvatarSavedHash");
					deleteSetting(hContact, "PictContext");
					deleteSetting(hContact, "PictSavedContext");

//					char tFileName[MAX_PATH];
//					MSN_GetAvatarFileName(hContact, tFileName, sizeof(tFileName));
//					remove(tFileName);

					SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
				}	
			}
			else 
			{
				if (lastStatus == ID_STATUS_OFFLINE)
					deleteSetting(hContact, "MirVer");
			}

			break;
		}
		case ' ORI':    //********* IRO: section 8.4 Getting Invited to a Switchboard Session
		{
			union 
			{
				char* tWords[5];
				struct { char *strThisContact, *totalContacts, *userEmail, *userNick, *flags; } data;
			};

			int tNumTokens = sttDivideWords(params, 5, tWords);
			if (tNumTokens < 4)
				goto LBL_InvalidCommand;

			info->contactJoined(data.userEmail);

			if (!strchr(data.userEmail, ';'))
			{
				UrlDecode(data.userNick);
				HANDLE hContact = MSN_HContactFromEmail(data.userEmail, data.userNick, true, true);
				if (tNumTokens == 5)
				{
					MsnContact *cont = Lists_Get(data.userEmail);
					if (cont)
					{
						char* end = NULL;
						cont->cap1 = strtoul(data.flags, &end, 10);
						cont->cap2 = end && *end == ':' ? strtoul(end + 1, NULL, 10) : 0;
					}
				}

				int temp_status = getWord(hContact, "Status", ID_STATUS_OFFLINE);
				if (temp_status == ID_STATUS_OFFLINE && Lists_IsInList(LIST_FL, data.userEmail))
					setWord(hContact, "Status", ID_STATUS_INVISIBLE);

				// only start the chat session after all the IRO messages has been recieved
				if (msnHaveChatDll && info->mJoinedContactsWLID.getCount() > 1 && !strcmp(data.strThisContact, data.totalContacts))
					MSN_ChatStart(info);
			}
			break;
		}
		case ' IOJ':    //********* JOI: section 8.5 Session Participant Changes
		{
			union {
				char* tWords[3];
				struct { char *userEmail, *userNick, *flags; } data;
			};

			int tNumTokens = sttDivideWords(params, 3, tWords);
			if (tNumTokens < 2)
				goto LBL_InvalidCommand;

			UrlDecode(data.userEmail); 
			
			if (strchr(data.userEmail, ';')) 
			{
				info->contactJoined(data.userEmail);
				break;
			}

			if (_stricmp(MyOptions.szEmail, data.userEmail) == 0)
			{
				if (!info->mCaller)
				{
					if (info->mJoinedContactsWLID.getCount() == 1) 
					{
						MSN_InitSB(info, info->mJoinedContactsWLID[0]);
					}
					else 
					{
						info->sendCaps();
						if (info->mInitialContactWLID && MsgQueue_CheckContact(info->mInitialContactWLID))
							msnNsThread->sendPacket("XFR", "SB");
						mir_free(info->mInitialContactWLID); info->mInitialContactWLID = NULL;
					}
					break;
				}

				const char* wlid;
				do {
					wlid = MsgQueue_GetNextRecipient();
				} while (wlid != NULL && MSN_GetUnconnectedThread(wlid) != NULL);

				if (wlid == NULL) //can happen if both parties send first message at the same time
				{
					MSN_DebugLog("USR (SB) internal: thread created for no reason");
					return 1;
				}

				if (strcmp(wlid, "chat") == 0)
				{
					MsgQueueEntry E;
					MsgQueue_GetNext(wlid, E);

					for (int i = 0; i < E.cont->getCount(); ++i)
						info->sendPacket("CAL", (*E.cont)[i]);

					MSN_ChatStart(info);
					
					delete E.cont;
					mir_free(E.wlid);
					break;
				}

				char* szEmail;
				parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);

				info->mInitialContactWLID = mir_strdup(szEmail);
				info->sendPacket("CAL", szEmail);
				break;
			}

			UrlDecode(data.userNick);
			stripBBCode(data.userNick);
			stripColorCode(data.userNick);

			HANDLE hContact = MSN_HContactFromEmail(data.userEmail, data.userNick, true, true);
			if (tNumTokens == 3)
			{
				MsnContact *cont = Lists_Get(data.userEmail);
				if (cont)
				{
					char* end = NULL;
					cont->cap1 = strtoul(data.flags, &end, 10);
					cont->cap2 = end && *end == ':' ? strtoul(end + 1, NULL, 10) : 0;
				}
			}

			mir_utf8decode(data.userNick, NULL);
			MSN_DebugLog("New contact in channel %s %s", data.userEmail, data.userNick);

			if (info->contactJoined(data.userEmail) <= 1) 
			{
				MSN_InitSB(info, data.userEmail);
			}
			else 
			{
				bool chatCreated = info->mChatID[0] != 0;

				info->sendCaps();

				if (msnHaveChatDll) 
				{
					if (chatCreated) 
					{
						GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_JOIN };
						gcd.ptszID = info->mChatID;

						GCEVENT gce = {0};
						gce.cbSize = sizeof(GCEVENT);
						gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
						gce.pDest = &gcd;
						gce.ptszNick = GetContactNameT(hContact);
						gce.ptszUID = mir_a2t(data.userEmail);
						gce.ptszStatus = TranslateT("Others");
						gce.time = time(NULL);
						gce.bIsMe = FALSE;
						CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
						mir_free((void*)gce.ptszUID);
					}
					else MSN_ChatStart(info);
				}	
			}
			break;
		}

		case ' GSM':   //********* MSG: sections 8.7 Instant Messages, 8.8 Receiving an Instant Message
			MSN_ReceiveMessage(info, cmdString, params);
			break;

		case ' MBU':
			MSN_ReceiveMessage(info, cmdString, params);
			break;

		case ' KAN':   //********* NAK: section 8.7 Instant Messages
			if (info->mJoinedContactsWLID.getCount() > 0 && MyOptions.SlowSend)
				SendBroadcast(info->getContactHandle(), 
					ACKTYPE_MESSAGE, ACKRESULT_FAILED, 
					(HANDLE)trid, (LPARAM)MSN_Translate("Message delivery failed"));
			MSN_DebugLog("Message send failed (trid=%d)", trid);
			break;

		case ' TON':   //********* NOT: notification message
			sttProcessNotificationMessage((char*)HReadBuffer(info, 0).surelyRead(trid), trid);
			break;

		case ' GPI':   //********* IPG: mobile page
			sttProcessPage((char*)HReadBuffer(info, 0).surelyRead(trid), trid);
			break;

		case ' FCG':   //********* GCF: 
			HReadBuffer(info, 0).surelyRead(atol(params));
			break;

		case ' TUO':   //********* OUT: sections 7.10 Connection Close, 8.6 Leaving a Switchboard Session
			if (!_stricmp(params, "OTH"))
			{
				SendBroadcast(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION);
				MSN_DebugLog("You have been disconnected from the MSN server because you logged on from another location using the same MSN passport.");
			}

			if (!_stricmp(params, "MIG")) // ignore it
				break;

			return 1;

		case ' YRQ':   //********* QRY:
			break;

		case ' GNQ':	//********* QNG: reply to PNG
			msnPingTimeout = trid;
			if (info->mType == SERVER_NOTIFICATION && hKeepAliveThreadEvt != NULL)
				SetEvent(hKeepAliveThreadEvt);
			break;

		case ' LMR':	//********* RML: Remove from the list
		{
			char* tWords[1];
			if (sttDivideWords(params, 1, tWords) != 1)
				goto LBL_InvalidCommand;

			if (strcmp(tWords[0], "OK") != 0)
			{
				size_t len = atol(tWords[0]);
				sttProcessRemove((char*)HReadBuffer(info, 0).surelyRead(len), len); 
			}
			break;
		}

		case ' GNR':    //********* RNG: section 8.4 Getting Invited to a Switchboard Session
			//note: unusual message encoding: trid==sessionid
		{
			union {
				char* tWords[8];
				struct { char *newServer, *security, *authChallengeInfo, *callerEmail, *callerNick,
							  *type, *srcUrl, *genGateway; } data;
			};

			if (sttDivideWords(params, 8, tWords) != 8)
				goto LBL_InvalidCommand;

			UrlDecode(data.newServer); UrlDecode(data.callerEmail);
			UrlDecode(data.callerNick);
			stripBBCode(data.callerNick);
			stripColorCode(data.callerNick);

			if (strcmp(data.security, "CKI")) {
				MSN_DebugLog("Unknown security package in RNG command: %s", data.security);
				break;
			}

			ThreadData* newThread = new ThreadData;
			strcpy(newThread->mServer, data.newServer);
			newThread->gatewayType = atol(data.genGateway) != 0;
			newThread->mType = SERVER_SWITCHBOARD;
			newThread->mInitialContactWLID = mir_strdup(data.callerEmail);
			MSN_HContactFromEmail(data.callerEmail, data.callerNick, true, true);
			mir_snprintf(newThread->mCookie, sizeof(newThread->mCookie), "%s %d", data.authChallengeInfo, trid);

			ReleaseSemaphore(newThread->hWaitEvent, MSN_PACKETS_COMBINE, NULL);

			MSN_DebugLog("Opening caller's switchboard server '%s'...", data.newServer);
			newThread->startThread(&CMsnProto::MSNServerThread, this);
			break;
		}

		case ' XBU':   // UBX : MSNP11+ User Status Message
		{
			union 
			{
				char* tWords[2];
				struct { char *wlid, *datalen; } data;
			};

			if (sttDivideWords(params, 2, tWords) != 2)
				goto LBL_InvalidCommand;

			int len = atol(data.datalen);
			if (len < 0 || len > 4000)
				goto LBL_InvalidCommand;

			sttProcessStatusMessage((char*)HReadBuffer(info, 0).surelyRead(len), len, data.wlid);
			break;
		}

		case ' NBU':	// UBN : MSNP13+ File sharing, P2P Bootstrap, TURN setup.
		{
			union 
			{
				char* tWords[3];
				struct { char *email, *typeId, *datalen; } data;
			};

			if (sttDivideWords(params, 3, tWords) != 3)
				goto LBL_InvalidCommand;

			int len = atol(data.datalen);
			if (len < 0 || len > 4000)
				goto LBL_InvalidCommand;

			HReadBuffer buf(info, 0);
			char* msgBody = (char*)buf.surelyRead(len);

			char *szEmail = data.email;
			if (strstr(data.email, sttVoidUid))
				parseWLID(NEWSTR_ALLOCA(data.email), NULL, &szEmail, NULL);

			switch (atol(data.typeId))
			{
				case 1:
					// File sharing stuff
					// sttProcessFileSharing(msgBody, len, hContact);
					break;

				case 3:
					// P2P Bootstrap
					p2p_processSIP(info, msgBody, NULL, szEmail);
					break;

				case 4:
				case 8:
					SendBroadcast( NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_OTHERLOCATION );
					MSN_DebugLog( "You have been disconnected from the MSN server because you logged on from another location using the same MSN passport." );
					break;

				case 6:
					MSN_SharingFindMembership(true);
					MSN_ABFind("ABFindContactsPaged", NULL, true);
					break;

				case 10:
					// TURN setup
					break;
			}
			break;
		}

		case ' NUU':	// UUN : MSNP13+ File sharing, P2P Bootstrap, TURN setup.
			break;

		case ' RSU':	//********* USR: sections 7.3 Authentication, 8.2 Switchboard Connections and Authentication
			if (info->mType == SERVER_SWITCHBOARD) //(section 8.2)
			{
				union {
					char* tWords[3];
					struct { char *status, *userHandle, *friendlyName; } data;
				};

				if (sttDivideWords(params, 3, tWords) != 3)
					goto LBL_InvalidCommand;

				UrlDecode(data.userHandle); UrlDecode(data.friendlyName);

				if (strcmp(data.status, "OK"))
				{
					MSN_DebugLog("Unknown status to USR command (SB): '%s'", data.status);
					break;
				}

				info->sendPacket("CAL", MyOptions.szEmail);
			}
			else 	   //dispatch or notification server (section 7.3)
			{
				union 
				{
					char* tWords[4];
					struct { char *security, *sequence, *authChallengeInfo, *nonce; } data;
				};

				if (sttDivideWords(params, 4, tWords) != 4)
					goto LBL_InvalidCommand;

				if (!strcmp(data.security, "SSO")) 
				{
					if (MSN_GetPassportAuth())
					{
						m_iDesiredStatus = ID_STATUS_OFFLINE;
						return 1;
					}

					char* sec = GenerateLoginBlob(data.nonce);
					info->sendPacket("USR", "SSO S %s %s %s", authStrToken ? authStrToken : "", sec, MyOptions.szMachineGuid);
					mir_free(sec);

					ForkThread(&CMsnProto::msn_keepAliveThread, NULL);
					ForkThread(&CMsnProto::MSNConnDetectThread, NULL);
				}
				else if (!strcmp(data.security, "OK")) 
				{
				}
				else
				{
					MSN_DebugLog("Unknown security package '%s'", data.security);

					if (info->mType == SERVER_NOTIFICATION) 
					{
						SendBroadcast(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPROTOCOL);
					}
					return 1;
				}	
			}
			break;

		case ' SFR':   // RFS: Refresh Contact List 
			if (!MSN_RefreshContactList()) 
			{
				MSN_ShowError("Cannot retrieve contact list");
				return 1;
			}
			break;

		case ' XUU':   // UUX: MSNP11 addition
		{
			char* tWords[1];

			if (sttDivideWords(params, SIZEOF(tWords), tWords) != SIZEOF(tWords))
				goto LBL_InvalidCommand;

			int len = atol(tWords[0]);
			if (len < 0 || len > 4000)
				goto LBL_InvalidCommand;

			HReadBuffer(info, 0).surelyRead(len);
			break;
		}
		case ' REV':	//******** VER: section 7.1 Protocol Versioning
		{
			char* protocol1;
			if (sttDivideWords(params, 1, &protocol1) != 1)
				goto LBL_InvalidCommand;

			if (MyOptions.szEmail[0] == 0) 
			{
				MSN_ShowError("You must specify your e-mail in Options/Network/MSN");
				return 1;
			}
/*
			if (strcmp(protocol1, msnProtID)) 
			{
				MSN_ShowError("Server has requested an unknown protocol set (%s)", params);

				if (info->mType == SERVER_NOTIFICATION) 
				{
					SendBroadcast(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPROTOCOL);
				}
				return 1;
			}
*/
			OSVERSIONINFOEX osvi = {0};
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			GetVersionEx((LPOSVERSIONINFO)&osvi);

			info->sendPacket("CVR","0x0409 %s %d.%d.%d i386 MSNMSGR %s msmsgs %s",
				osvi.dwPlatformId >= 2 ? "winnt" : "win", osvi.dwMajorVersion, 
				osvi.dwMinorVersion, osvi.wServicePackMajor,
				msnProductVer, MyOptions.szEmail);

			info->sendPacket("USR", "SSO I %s", MyOptions.szEmail);
			break;
		}
		case ' RFX':    //******** XFR: sections 7.4 Referral, 8.1 Referral to Switchboard
		{
			union 
			{
				char* tWords[7];
				struct { char *type, *newServer, *security, *authChallengeInfo,
							  *type2, *srcUrl, *genGateway; } data;
			};

			int numWords = sttDivideWords(params, 7, tWords);
			if (numWords < 3)
				goto LBL_InvalidCommand;

			if (!strcmp(data.type, "NS"))  //notification server
			{
				UrlDecode(data.newServer);
				ThreadData* newThread = new ThreadData;
				strcpy(newThread->mServer, data.newServer);
				newThread->mType = SERVER_NOTIFICATION;
				newThread->mTrid = info->mTrid;
				newThread->mIsMainThread = true;
				usingGateway |= (*data.security == 'G');
				info->mIsMainThread = false;

				MSN_DebugLog("Switching to notification server '%s'...", data.newServer);
				newThread->startThread(&CMsnProto::MSNServerThread, this);
				return 1;  //kill the old thread
			}

			if (!strcmp(data.type, "SB"))  //switchboard server
			{
				UrlDecode(data.newServer);

				if (numWords < 4)
					goto LBL_InvalidCommand;

				if (strcmp(data.security, "CKI")) 
				{
					MSN_DebugLog("Unknown XFR SB security package '%s'", data.security);
					break;
				}

				ThreadData* newThread = new ThreadData;
				strcpy(newThread->mServer, data.newServer);
				newThread->gatewayType = data.genGateway && atol(data.genGateway) != 0;
				newThread->mType = SERVER_SWITCHBOARD;
				newThread->mCaller = 1;
				strcpy(newThread->mCookie, data.authChallengeInfo);

				MSN_DebugLog("Opening switchboard server '%s'...", data.newServer);
				newThread->startThread(&CMsnProto::MSNServerThread, this);
			}
			else MSN_DebugLog("Unknown referral server: %s", data.type);
			break;
		}

		default:
			MSN_DebugLog("Unrecognised message: %s", cmdString);
			break;
	}

	return 0;
}
