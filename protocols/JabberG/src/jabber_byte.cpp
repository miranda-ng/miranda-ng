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
#include "jabber_iq.h"
#include "jabber_byte.h"
#include "jabber_caps.h"

#define JABBER_NETWORK_BUFFER_SIZE 4096

///////////////// Bytestream sending /////////////////////////

JABBER_BYTE_TRANSFER::~JABBER_BYTE_TRANSFER()
{
	filetransfer* pft = ft;
	if (pft)
		pft->jbt = nullptr;

	mir_free(srcJID);
	mir_free(dstJID);
	mir_free(streamhostJID);
	mir_free(iqId);
	mir_free(sid);

	xmlDestroyNode(iqNode);

	// XEP-0065 proxy support
	mir_free(szProxyHost);
	mir_free(szProxyPort);
	mir_free(szProxyJid);
	mir_free(szStreamhostUsed);
}

void CJabberProto::IqResultProxyDiscovery(HXML iqNode, CJabberIqInfo *pInfo)
{
	JABBER_BYTE_TRANSFER *jbt = (JABBER_BYTE_TRANSFER *)pInfo->GetUserData();

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML queryNode = XmlGetChild(iqNode , "query");
		if (queryNode) {
			const wchar_t *queryXmlns = XmlGetAttrValue(queryNode, L"xmlns");
			if (queryXmlns && !mir_wstrcmp(queryXmlns, JABBER_FEAT_BYTESTREAMS)) {
				HXML streamHostNode = XmlGetChild(queryNode , "streamhost");
				if (streamHostNode) {
					const wchar_t *streamJid = XmlGetAttrValue(streamHostNode, L"jid");
					const wchar_t *streamHost = XmlGetAttrValue(streamHostNode, L"host");
					const wchar_t *streamPort = XmlGetAttrValue(streamHostNode, L"port");
					if (streamJid && streamHost && streamPort) {
						jbt->szProxyHost = mir_wstrdup(streamHost);
						jbt->szProxyJid = mir_wstrdup(streamJid);
						jbt->szProxyPort = mir_wstrdup(streamPort);
						jbt->bProxyDiscovered = TRUE;
	}	}	}	}	}
	else if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR)
		jbt->state = JBT_ERROR;

	if (jbt->hProxyEvent)
		SetEvent(jbt->hProxyEvent);
}

void JabberByteSendConnection(HNETLIBCONN hConn, DWORD /*dwRemoteIP*/, void* extra)
{
	CJabberProto *ppro = (CJabberProto*)extra;
	wchar_t szPort[8];
	JABBER_BYTE_TRANSFER *jbt;
	int recvResult, bytesParsed;
	HANDLE hListen;
	JABBER_LIST_ITEM *item;
	char* buffer;
	int datalen;

	NETLIBCONNINFO connInfo = {};
	Netlib_GetConnectionInfo(hConn, &connInfo);

	mir_snwprintf(szPort, L"%u", connInfo.wPort);
	ppro->debugLogA("bytestream_send_connection incoming connection accepted: %s", connInfo.szIpPort);

	if ((item = ppro->ListGetItemPtr(LIST_BYTE, szPort)) == nullptr) {
		ppro->debugLogA("No bytestream session is currently active, connection closed.");
		Netlib_CloseHandle(hConn);
		return;
	}

	jbt = item->jbt;

	if ((buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE)) == nullptr) {
		ppro->debugLogA("bytestream_send cannot allocate network buffer, connection closed.");
		jbt->state = JBT_ERROR;
		Netlib_CloseHandle(hConn);
		if (jbt->hEvent != nullptr) SetEvent(jbt->hEvent);
		return;
	}

	hListen = jbt->hConn;
	jbt->hConn = hConn;
	jbt->state = JBT_INIT;
	datalen = 0;
	while (jbt->state!=JBT_DONE && jbt->state!=JBT_ERROR) {
		recvResult = Netlib_Recv(hConn, buffer+datalen, JABBER_NETWORK_BUFFER_SIZE-datalen, 0);
		if (recvResult <= 0)
			break;

		datalen += recvResult;
		bytesParsed = ppro->ByteSendParse(hConn, jbt, buffer, datalen);
		if (bytesParsed < datalen)
			memmove(buffer, buffer+bytesParsed, datalen-bytesParsed);
		datalen -= bytesParsed;
	}

	if (jbt->hConn)
		Netlib_CloseHandle(jbt->hConn);

	ppro->debugLogA("bytestream_send_connection closing connection");
	jbt->hConn = hListen;
	mir_free(buffer);

	if (jbt->hEvent != nullptr)
		SetEvent(jbt->hEvent);
}

void CJabberProto::ByteSendThread(JABBER_BYTE_TRANSFER *jbt)
{
	wchar_t szPort[8];
	HANDLE hEvent = nullptr;
	int nIqId = 0;

	debugLogA("Thread started: type=bytestream_send");
	Thread_SetName("Jabber: ByteSendThread");

	BOOL bDirect = m_options.BsDirect;

	if (m_options.BsProxyManual) {
		ptrW proxyJid( getWStringA("BsProxyServer"));
		if (proxyJid) {
			jbt->bProxyDiscovered = FALSE;
			jbt->szProxyHost = nullptr;
			jbt->szProxyPort = nullptr;
			jbt->szProxyJid = nullptr;
			jbt->hProxyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::IqResultProxyDiscovery, JABBER_IQ_TYPE_GET, proxyJid, 0, -1, jbt);
			nIqId = pInfo->GetIqId();
			XmlNodeIq iq(pInfo);
			iq << XQUERY(JABBER_FEAT_BYTESTREAMS);
			m_ThreadInfo->send(iq);

			WaitForSingleObject(jbt->hProxyEvent, INFINITE);
			m_iqManager.ExpireIq(nIqId);
			CloseHandle(jbt->hProxyEvent);
			jbt->hProxyEvent = nullptr;

			if (jbt->state == JBT_ERROR && !bDirect) {
				debugLogA("Bytestream proxy failure");
				MsgPopup(pInfo->GetHContact(), TranslateT("Bytestream Proxy not available"), proxyJid);
				jbt->ft->state = FT_DENIED;
				(this->*jbt->pfnFinal)(FALSE, jbt->ft);
				jbt->ft = nullptr;
				delete jbt;
				return;
	}	}	}

	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::ByteInitiateResult, JABBER_IQ_TYPE_SET, jbt->dstJID, 0, -1, jbt);
	nIqId = pInfo->GetIqId();
	{
		XmlNodeIq iq(pInfo);
		HXML query = iq << XQUERY(JABBER_FEAT_BYTESTREAMS) << XATTR(L"sid", jbt->sid);

		if (bDirect) {
			ptrA localAddr;
			if (m_options.BsDirectManual)
				localAddr = getStringA("BsDirectAddr");

			NETLIBBIND nlb = {};
			nlb.pfnNewConnectionV2 = JabberByteSendConnection;
			nlb.pExtra = this;
			nlb.wPort = 0;	// Use user-specified incoming port ranges, if available
			
			jbt->hConn = Netlib_BindPort(m_hNetlibUser, &nlb);
			if (jbt->hConn == nullptr) {
				debugLogA("Cannot allocate port for bytestream_send thread, thread ended.");
				delete jbt;
				return;
			}

			if (localAddr == nullptr) {
				sockaddr_in sin = {};
				sin.sin_family = AF_INET;
				sin.sin_addr.S_un.S_addr = nlb.dwExternalIP;
				localAddr = Netlib_AddressToString(&sin);
			}

			mir_snwprintf(szPort, L"%d", nlb.wPort);
			JABBER_LIST_ITEM *item = ListAdd(LIST_BYTE, szPort);
			item->jbt = jbt;
			hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			jbt->hEvent = hEvent;
			jbt->hSendEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			query << XCHILD(L"streamhost") << XATTR(L"jid", m_ThreadInfo->fullJID) << XATTR(L"host", _A2T(localAddr)) << XATTRI(L"port", nlb.wPort);

			NETLIBIPLIST* ihaddr = Netlib_GetMyIp(true);
			for (unsigned i=0; i < ihaddr->cbNum; i++)
				if (mir_strcmp(localAddr, ihaddr->szIp[i]))
					query << XCHILD(L"streamhost") << XATTR(L"jid", m_ThreadInfo->fullJID) << XATTR(L"host", _A2T(ihaddr->szIp[i])) << XATTRI(L"port", nlb.wPort);

			mir_free(ihaddr);
		}

		if (jbt->bProxyDiscovered)
			query << XCHILD(L"streamhost") << XATTR(L"jid", jbt->szProxyJid) << XATTR(L"host", jbt->szProxyHost) << XATTR(L"port", jbt->szProxyPort);

		jbt->hProxyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		jbt->szStreamhostUsed = nullptr;

		m_ThreadInfo->send(iq);
	}

	WaitForSingleObject(jbt->hProxyEvent, INFINITE);
	m_iqManager.ExpireIq(nIqId);
	CloseHandle(jbt->hProxyEvent);
	jbt->hProxyEvent = nullptr;

	if (!jbt->szStreamhostUsed) {
		if (bDirect) {
			SetEvent(jbt->hSendEvent);
			CloseHandle(jbt->hSendEvent);
			CloseHandle(hEvent);
			jbt->hEvent = nullptr;
			if (jbt->hConn != nullptr)
				Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = nullptr;
			ListRemove(LIST_BYTE, szPort);
		}
		(this->*jbt->pfnFinal)((jbt->state==JBT_DONE)?TRUE:FALSE, jbt->ft);
		jbt->ft = nullptr;
		// stupid fix: wait for listening thread exit
		Sleep(100);
		delete jbt;
		return;
	}

	if (jbt->bProxyDiscovered && !mir_wstrcmp(jbt->szProxyJid, jbt->szStreamhostUsed)) {
		// jabber proxy used
		if (bDirect) {
			SetEvent(jbt->hSendEvent);
			CloseHandle(jbt->hSendEvent);
			CloseHandle(hEvent);
			jbt->hEvent = nullptr;
			if (jbt->hConn != nullptr)
				Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = nullptr;
			ListRemove(LIST_BYTE, szPort);
		}
		ByteSendViaProxy(jbt);
	}
	else {
		SetEvent(jbt->hSendEvent);
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
		CloseHandle(jbt->hSendEvent);
		jbt->hEvent = nullptr;
		(this->*jbt->pfnFinal)((jbt->state == JBT_DONE) ? TRUE : FALSE, jbt->ft);
		jbt->ft = nullptr;
		if (jbt->hConn != nullptr)
			Netlib_CloseHandle(jbt->hConn);
		jbt->hConn = nullptr;
		ListRemove(LIST_BYTE, szPort);
	}

	// stupid fix: wait for listening connection thread exit
	Sleep(100);
	delete jbt;
	debugLogA("Thread ended: type=bytestream_send");
}

void CJabberProto::ByteInitiateResult(HXML iqNode, CJabberIqInfo *pInfo)
{
	JABBER_BYTE_TRANSFER *jbt = (JABBER_BYTE_TRANSFER *)pInfo->GetUserData();

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML queryNode = XmlGetChild(iqNode , "query");
		if (queryNode) {
			const wchar_t *queryXmlns = XmlGetAttrValue(queryNode, L"xmlns");
			if (queryXmlns && !mir_wstrcmp(queryXmlns, JABBER_FEAT_BYTESTREAMS)) {
				HXML streamHostNode = XmlGetChild(queryNode ,  "streamhost-used");
				if (streamHostNode) {
					const wchar_t *streamJid = XmlGetAttrValue(streamHostNode, L"jid");
					if (streamJid)
						jbt->szStreamhostUsed = mir_wstrdup(streamJid);
	}	}	}	}

	if (jbt->hProxyEvent)
		SetEvent(jbt->hProxyEvent);
}

int CJabberProto::ByteSendParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
{
	int nMethods;
	BYTE data[10];
	int i;

	switch (jbt->state) {
	case JBT_INIT:
		// received:
		// 00-00 ver (0x05)
		// 01-01 nmethods
		// 02-xx list of methods (nmethods bytes)
		// send:
		// 00-00 ver (0x05)
		// 01-01 select method (0=no auth required)
		if (datalen>=2 && buffer[0]==5 && buffer[1]+2==datalen) {
			nMethods = buffer[1];
			for (i=0; i<nMethods && buffer[2+i]!=0; i++);
			if (i < nMethods) {
				data[1] = 0;
				jbt->state = JBT_CONNECT;
			}
			else {
				data[1] = 0xff;
				jbt->state = JBT_ERROR;
			}
			data[0] = 5;
			Netlib_Send(hConn, (char*)data, 2, 0);
		}
		else jbt->state = JBT_ERROR;
		break;

	case JBT_CONNECT:
		// received:
		// 00-00 ver (0x05)
		// 01-01 cmd (1=connect)
		// 02-02 reserved (0)
		// 03-03 address type (3)
		// 04-44 dst.addr (41 bytes: 1-byte length, 40-byte SHA1 hash of [sid,srcJID,dstJID])
		// 45-46 dst.port (0)
		// send:
		// 00-00 ver (0x05)
		// 01-01 reply (0=success,2=not allowed)
		// 02-02 reserved (0)
		// 03-03 address type (1=IPv4 address)
		// 04-07 bnd.addr server bound address
		// 08-09 bnd.port server bound port
		if (datalen == 47 && *((DWORD*)buffer)==0x03000105 && buffer[4]==40 && *((WORD*)(buffer+45))==0) {
			wchar_t text[256];

			wchar_t *szInitiatorJid = JabberPrepareJid(jbt->srcJID);
			wchar_t *szTargetJid = JabberPrepareJid(jbt->dstJID);
			mir_snwprintf(text, L"%s%s%s", jbt->sid, szInitiatorJid, szTargetJid);
			mir_free(szInitiatorJid);
			mir_free(szTargetJid);

			T2Utf szAuthString(text);
			debugLogA("Auth: '%s'", szAuthString);

			JabberShaStrBuf buf;
			JabberSha1(szAuthString, buf);
			for (i = 0; i < 40 && buffer[i + 5] == buf[i]; i++);

			memset(data, 0, sizeof(data));
			data[1] = (i >= 40) ? 0 : 2;
			data[0] = 5;
			data[3] = 1;
			Netlib_Send(hConn, (char*)data, 10, 0);

			// wait stream activation
			WaitForSingleObject(jbt->hSendEvent, INFINITE);

			if (jbt->state == JBT_ERROR)
				break;

			if (i >= 40 && (this->*jbt->pfnSend)(hConn, jbt->ft) == TRUE)
				jbt->state = JBT_DONE;
			else
				jbt->state = JBT_ERROR;
		}
		else
			jbt->state = JBT_ERROR;
		break;
	}

	return datalen;
}

///////////////// Bytestream receiving /////////////////////////

void CJabberProto::IqResultStreamActivate(HXML iqNode, CJabberIqInfo*)
{
	int id = JabberGetPacketID(iqNode);

	wchar_t listJid[JABBER_MAX_JID_LEN];
	mir_snwprintf(listJid, L"ftproxy_%d", id);

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTIQID, listJid);
	if (item == nullptr)
		return;

	if (!mir_wstrcmp(XmlGetAttrValue(iqNode, L"type"), L"result"))
		item->jbt->bStreamActivated = TRUE;

	if (item->jbt->hProxyEvent)
		SetEvent(item->jbt->hProxyEvent);
}


void CJabberProto::ByteSendViaProxy(JABBER_BYTE_TRANSFER *jbt)
{
	wchar_t *szHost, *szPort;
	WORD port;
	char data[3];
	char* buffer;
	int datalen, bytesParsed, recvResult;
	BOOL validStreamhost;

	if (jbt == nullptr) return;
	if ((buffer=(char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE)) == nullptr) {
		m_ThreadInfo->send( XmlNodeIq(L"error", jbt->iqId, jbt->srcJID)
			<< XCHILD(L"error") << XATTRI(L"code", 406) << XATTR(L"type", L"auth")
			<< XCHILDNS(L"not-acceptable", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
		return;
	}

	jbt->state = JBT_INIT;
	validStreamhost = FALSE;
	szPort = jbt->szProxyPort;
	szHost = jbt->szProxyHost;

	port = (WORD)_wtoi(szPort);
	replaceStrW(jbt->streamhostJID, jbt->szProxyJid);

	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof(nloc);
	nloc.szHost = mir_u2a(szHost);
	nloc.wPort = port;
	HNETLIBCONN hConn = Netlib_OpenConnection(m_hNetlibUser, &nloc);
	mir_free((void*)nloc.szHost);

	if (hConn != nullptr) {
		jbt->hConn = hConn;

		data[0] = 5;
		data[1] = 1;
		data[2] = 0;
		Netlib_Send(hConn, data, 3, 0);

		jbt->state = JBT_INIT;
		datalen = 0;
		while (jbt->state!=JBT_DONE && jbt->state!=JBT_ERROR && jbt->state!=JBT_SOCKSERR) {
			recvResult = Netlib_Recv(hConn, buffer+datalen, JABBER_NETWORK_BUFFER_SIZE-datalen, 0);
			if (recvResult <= 0)
				break;

			datalen += recvResult;
			bytesParsed = ByteSendProxyParse(hConn, jbt, buffer, datalen);
			if (bytesParsed < datalen)
				memmove(buffer, buffer+bytesParsed, datalen-bytesParsed);
			datalen -= bytesParsed;
			if (jbt->state == JBT_DONE) validStreamhost = TRUE;
		}
		Netlib_CloseHandle(hConn);
	}
	mir_free(buffer);
	(this->*jbt->pfnFinal)((jbt->state == JBT_DONE) ? TRUE : FALSE, jbt->ft);
	jbt->ft = nullptr;
	if (!validStreamhost)
		m_ThreadInfo->send( XmlNodeIq(L"error", jbt->iqId, jbt->srcJID)
			<< XCHILD(L"error") << XATTRI(L"code", 404) << XATTR(L"type", L"cancel")
			<< XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
}

int CJabberProto::ByteSendProxyParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
{
	int num = datalen;

	switch (jbt->state) {
	case JBT_INIT:
		// received:
		// 00-00 ver (0x05)
		// 01-01 selected method (0=no auth, 0xff=error)
		// send:
		// 00-00 ver (0x05)
		// 01-01 cmd (1=connect)
		// 02-02 reserved (0)
		// 03-03 address type (3)
		// 04-44 dst.addr (41 bytes: 1-byte length, 40-byte SHA1 hash of [sid,srcJID,dstJID])
		// 45-46 dst.port (0)
		if (datalen == 2 && buffer[0] == 5 && buffer[1] == 0) {
			BYTE data[47];
			memset(data, 0, sizeof(data));
			*((DWORD*)data) = 0x03000105;
			data[4] = 40;

			wchar_t text[256];

			wchar_t *szInitiatorJid = JabberPrepareJid(jbt->srcJID);
			wchar_t *szTargetJid = JabberPrepareJid(jbt->dstJID);
			mir_snwprintf(text, L"%s%s%s", jbt->sid, szInitiatorJid, szTargetJid);
			mir_free(szInitiatorJid);
			mir_free(szTargetJid);

			T2Utf szAuthString(text);
			debugLogA("Auth: '%s'", szAuthString);

			JabberShaStrBuf buf;
			strncpy_s((char*)(data + 5), 40 + 1, JabberSha1(szAuthString, buf), _TRUNCATE);

			Netlib_Send(hConn, (char*)data, 47, 0);
			jbt->state = JBT_CONNECT;
		}
		else jbt->state = JBT_SOCKSERR;
		break;

	case JBT_CONNECT:
		// received:
		// 00-00 ver (0x05)
		// 01-01 reply (0=success,2=not allowed)
		// 02-02 reserved (0)
		// 03-03 address type (1=IPv4 address,3=host address)
		// 04-mm bnd.addr server bound address (4-byte IP if IPv4, 1-byte length + n-byte host address string if host address)
		// nn-nn+1 bnd.port server bound port
		if (datalen >= 5 && buffer[0] == 5 && buffer[1] == 0 && (buffer[3] == 1 || buffer[3] == 3 || buffer[3] == 0)) {
			if (buffer[3] == 1 && datalen >= 10)
				num = 10;
			else if (buffer[3] == 3 && datalen >= buffer[4] + 7)
				num = buffer[4] + 7;
			else if (buffer[3] == 0 && datalen >= 6)
				num = 6;
			else {
				jbt->state = JBT_SOCKSERR;
				break;
			}
			jbt->state = JBT_SENDING;

			jbt->hProxyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			jbt->bStreamActivated = FALSE;

			int iqId = SerialNext();

			wchar_t listJid[256];
			mir_snwprintf(listJid, L"ftproxy_%d", iqId);

			JABBER_LIST_ITEM *item = ListAdd(LIST_FTIQID, listJid);
			item->jbt = jbt;

			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::IqResultStreamActivate, JABBER_IQ_TYPE_SET, jbt->streamhostJID, 0, iqId))
				<< XQUERY(JABBER_FEAT_BYTESTREAMS) << XATTR(L"sid", jbt->sid) << XCHILD(L"activate", jbt->dstJID));

			WaitForSingleObject(jbt->hProxyEvent, INFINITE);

			CloseHandle(jbt->hProxyEvent);
			jbt->hProxyEvent = nullptr;

			ListRemove(LIST_FTIQID, listJid);

			if (jbt->bStreamActivated)
				jbt->state = (this->*jbt->pfnSend)(hConn, jbt->ft) ? JBT_DONE : JBT_ERROR;
			else
				jbt->state = JBT_ERROR;
		}
		else jbt->state = JBT_SOCKSERR;
		break;
	}

	return num;
}

void __cdecl CJabberProto::ByteReceiveThread(JABBER_BYTE_TRANSFER *jbt)
{
	HXML iqNode, queryNode = nullptr, n;
	const wchar_t *sid = nullptr, *from = nullptr, *to = nullptr, *szId = nullptr, *szHost, *szPort, *str;
	int i;
	WORD port;
	char data[3];
	char* buffer;
	int datalen, bytesParsed, recvResult;
	BOOL validStreamhost = FALSE;

	if (jbt == nullptr) return;
	Thread_SetName("Jabber: ByteReceiveThread");

	jbt->state = JBT_INIT;

	if (iqNode = jbt->iqNode) {
		from = XmlGetAttrValue(iqNode, L"from");
		to = XmlGetAttrValue(iqNode, L"to");
		szId = XmlGetAttrValue(iqNode, L"id");

		queryNode = XmlGetChild(iqNode, "query");
		if (queryNode)
			sid = XmlGetAttrValue(queryNode, L"sid");
	}

	if (szId && from && to && sid && (n = XmlGetChild(queryNode, "streamhost")) != nullptr) {
		jbt->iqId = mir_wstrdup(szId);
		jbt->srcJID = mir_wstrdup(from);
		jbt->dstJID = mir_wstrdup(to);
		jbt->sid = mir_wstrdup(sid);

		if ((buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE))) {
			for (i = 1; (n = XmlGetNthChild(queryNode, L"streamhost", i)) != nullptr; i++) {
				if ((szHost = XmlGetAttrValue(n, L"host")) != nullptr &&
					 (szPort = XmlGetAttrValue(n, L"port")) != nullptr &&
					 (str = XmlGetAttrValue(n, L"jid")) != nullptr) {

					port = (WORD)_wtoi(szPort);
					replaceStrW(jbt->streamhostJID, str);

					debugLogW(L"bytestream_recv connecting to %s:%d", szHost, port);
					NETLIBOPENCONNECTION nloc = { 0 };
					nloc.cbSize = sizeof(nloc);
					nloc.szHost = mir_u2a(szHost);
					nloc.wPort = port;
					HNETLIBCONN hConn = Netlib_OpenConnection(m_hNetlibUser, &nloc);
					mir_free((void*)nloc.szHost);

					if (hConn == nullptr) {
						debugLogA("bytestream_recv_connection connection failed (%d), try next streamhost", WSAGetLastError());
						continue;
					}

					jbt->hConn = hConn;

					data[0] = 5;
					data[1] = 1;
					data[2] = 0;
					Netlib_Send(hConn, data, 3, 0);

					jbt->state = JBT_INIT;
					datalen = 0;
					while (jbt->state != JBT_DONE && jbt->state != JBT_ERROR && jbt->state != JBT_SOCKSERR) {
						recvResult = Netlib_Recv(hConn, buffer + datalen, JABBER_NETWORK_BUFFER_SIZE - datalen, 0);
						if (recvResult <= 0) break;
						datalen += recvResult;
						bytesParsed = ByteReceiveParse(hConn, jbt, buffer, datalen);
						if (bytesParsed < datalen)
							memmove(buffer, buffer + bytesParsed, datalen - bytesParsed);
						datalen -= bytesParsed;
						if (jbt->state == JBT_RECVING) validStreamhost = TRUE;
					}
					Netlib_CloseHandle(hConn);
					debugLogA("bytestream_recv_connection closing connection");
				}
				if (jbt->state == JBT_ERROR || validStreamhost == TRUE)
					break;
				debugLogA("bytestream_recv_connection stream cannot be established, try next streamhost");
			}
			mir_free(buffer);
		}
	}

	(this->*jbt->pfnFinal)((jbt->state == JBT_DONE) ? TRUE : FALSE, jbt->ft);
	jbt->ft = nullptr;
	if (!validStreamhost && szId && from) {
		debugLogA("bytestream_recv_connection session not completed");

		m_ThreadInfo->send(XmlNodeIq(L"error", szId, from)
								 << XCHILD(L"error") << XATTRI(L"code", 404) << XATTR(L"type", L"cancel")
								 << XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
	}

	delete jbt;
	debugLogA("Thread ended: type=bytestream_recv");
}

int CJabberProto::ByteReceiveParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
{
	int bytesReceived, num = datalen;

	switch (jbt->state) {
	case JBT_INIT:
		// received:
		// 00-00 ver (0x05)
		// 01-01 selected method (0=no auth, 0xff=error)
		// send:
		// 00-00 ver (0x05)
		// 01-01 cmd (1=connect)
		// 02-02 reserved (0)
		// 03-03 address type (3)
		// 04-44 dst.addr (41 bytes: 1-byte length, 40-byte SHA1 hash of [sid,srcJID,dstJID])
		// 45-46 dst.port (0)
		if (datalen == 2 && buffer[0] == 5 && buffer[1] == 0) {
			BYTE data[47];
			memset(data, 0, sizeof(data));
			*((DWORD*)data) = 0x03000105;
			data[4] = 40;

			wchar_t text[JABBER_MAX_JID_LEN * 2];
			{
				ptrW szInitiatorJid(JabberPrepareJid(jbt->srcJID));
				ptrW szTargetJid(JabberPrepareJid(jbt->dstJID));
				mir_snwprintf(text, L"%s%s%s", jbt->sid, szInitiatorJid, szTargetJid);
			}

			T2Utf szAuthString(text);
			debugLogA("Auth: '%s'", szAuthString);

			JabberShaStrBuf buf;
			strncpy_s((char*)(data + 5), 40 + 1, JabberSha1(szAuthString, buf), _TRUNCATE);

			Netlib_Send(hConn, (char*)data, 47, 0);
			jbt->state = JBT_CONNECT;
		}
		else jbt->state = JBT_SOCKSERR;
		break;

	case JBT_CONNECT:
		// received:
		// 00-00 ver (0x05)
		// 01-01 reply (0=success,2=not allowed)
		// 02-02 reserved (0)
		// 03-03 address type (1=IPv4 address,3=host address)
		// 04-mm bnd.addr server bound address (4-byte IP if IPv4, 1-byte length + n-byte host address string if host address)
		// nn-nn+1 bnd.port server bound port
		if (datalen >= 5 && buffer[0] == 5 && buffer[1] == 0 && (buffer[3] == 1 || buffer[3] == 3 || buffer[3] == 0)) {
			if (buffer[3] == 1 && datalen >= 10)
				num = 10;
			else if (buffer[3] == 3 && datalen >= buffer[4] + 7)
				num = buffer[4] + 7;
			else if (buffer[3] == 0 && datalen >= 6)
				num = 6;
			else {
				jbt->state = JBT_SOCKSERR;
				break;
			}
			jbt->state = JBT_RECVING;

			m_ThreadInfo->send(
				XmlNodeIq(L"result", jbt->iqId, jbt->srcJID) << XQUERY(JABBER_FEAT_BYTESTREAMS)
				<< XCHILD(L"streamhost-used") << XATTR(L"jid", jbt->streamhostJID));
		}
		else jbt->state = JBT_SOCKSERR;
		break;

	case JBT_RECVING:
		bytesReceived = (this->*jbt->pfnRecv)(hConn, jbt->ft, buffer, datalen);
		if (bytesReceived < 0)
			jbt->state = JBT_ERROR;
		else if (bytesReceived == 0)
			jbt->state = JBT_DONE;
		break;
	}

	return num;
}
