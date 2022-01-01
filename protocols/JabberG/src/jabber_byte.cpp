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

	// XEP-0065 proxy support
	mir_free(szProxyHost);
	mir_free(szProxyPort);
	mir_free(szProxyJid);
	mir_free(szStreamhostUsed);
}

void CJabberProto::IqResultProxyDiscovery(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	JABBER_BYTE_TRANSFER *jbt = (JABBER_BYTE_TRANSFER *)pInfo->GetUserData();

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		auto *queryNode = XmlFirstChild(iqNode, "query");
		if (queryNode) {
			const char *queryXmlns = XmlGetAttr(queryNode, "xmlns");
			if (queryXmlns && !mir_strcmp(queryXmlns, JABBER_FEAT_BYTESTREAMS)) {
				auto *streamHostNode = XmlFirstChild(queryNode, "streamhost");
				if (streamHostNode) {
					const char *streamJid = XmlGetAttr(streamHostNode, "jid");
					const char *streamHost = XmlGetAttr(streamHostNode, "host");
					const char *streamPort = XmlGetAttr(streamHostNode, "port");
					if (streamJid && streamHost && streamPort) {
						jbt->szProxyHost = mir_strdup(streamHost);
						jbt->szProxyJid = mir_strdup(streamJid);
						jbt->szProxyPort = mir_strdup(streamPort);
						jbt->bProxyDiscovered = true;
					}
				}
			}
		}
	}
	else if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR)
		jbt->state = JBT_ERROR;

	if (jbt->hProxyEvent)
		SetEvent(jbt->hProxyEvent);
}

void JabberByteSendConnection(HNETLIBCONN hConn, uint32_t /*dwRemoteIP*/, void* extra)
{
	CJabberProto *ppro = (CJabberProto*)extra;
	JABBER_BYTE_TRANSFER *jbt;
	int recvResult, bytesParsed;
	HANDLE hListen;
	JABBER_LIST_ITEM *item;
	char* buffer;
	int datalen;

	NETLIBCONNINFO connInfo = {};
	Netlib_GetConnectionInfo(hConn, &connInfo);

	char szPort[8];
	itoa(connInfo.wPort, szPort, 10);
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
	while (jbt->state != JBT_DONE && jbt->state != JBT_ERROR) {
		recvResult = Netlib_Recv(hConn, buffer + datalen, JABBER_NETWORK_BUFFER_SIZE - datalen, 0);
		if (recvResult <= 0)
			break;

		datalen += recvResult;
		bytesParsed = ppro->ByteSendParse(hConn, jbt, buffer, datalen);
		if (bytesParsed < datalen)
			memmove(buffer, buffer + bytesParsed, datalen - bytesParsed);
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
	HANDLE hEvent = nullptr;
	int nIqId = 0;

	debugLogA("Thread started: type=bytestream_send");
	Thread_SetName("Jabber: ByteSendThread");

	BOOL bDirect = m_bBsDirect;

	if (m_bBsProxyManual) {
		ptrA proxyJid(getUStringA("BsProxyServer"));
		if (proxyJid) {
			jbt->bProxyDiscovered = false;
			jbt->szProxyHost = nullptr;
			jbt->szProxyPort = nullptr;
			jbt->szProxyJid = nullptr;
			jbt->hProxyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::IqResultProxyDiscovery, JABBER_IQ_TYPE_GET, proxyJid, jbt);
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
				MsgPopup(pInfo->GetHContact(), TranslateT("Bytestream Proxy not available"), _A2T(proxyJid));
				jbt->ft->state = FT_DENIED;
				(this->*jbt->pfnFinal)(false, jbt->ft);
				jbt->ft = nullptr;
				delete jbt;
				return;
			}
		}
	}

	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::ByteInitiateResult, JABBER_IQ_TYPE_SET, jbt->dstJID, jbt);
	nIqId = pInfo->GetIqId();
	char szPort[8];
	{
		XmlNodeIq iq(pInfo);
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_BYTESTREAMS) << XATTR("sid", jbt->sid);

		if (bDirect) {
			ptrA localAddr;
			if (m_bBsDirectManual)
				localAddr = getStringA("BsDirectAddr");

			NETLIBBIND nlb = {};
			nlb.pfnNewConnection = JabberByteSendConnection;
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
				sin.sin_addr.S_un.S_addr = htonl(nlb.dwExternalIP);
				localAddr = Netlib_AddressToString(&sin);
			}

			itoa(nlb.wPort, szPort, 10);
			JABBER_LIST_ITEM *item = ListAdd(LIST_BYTE, szPort);
			item->jbt = jbt;
			hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			jbt->hEvent = hEvent;
			jbt->hSendEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			query << XCHILD("streamhost") << XATTR("jid", m_ThreadInfo->fullJID) << XATTR("host", localAddr) << XATTRI("port", nlb.wPort);

			NETLIBIPLIST *ihaddr = Netlib_GetMyIp(true);
			for (unsigned i = 0; i < ihaddr->cbNum; i++)
				if (mir_strcmp(localAddr, ihaddr->szIp[i]))
					query << XCHILD("streamhost") << XATTR("jid", m_ThreadInfo->fullJID) << XATTR("host", ihaddr->szIp[i]) << XATTRI("port", nlb.wPort);

			mir_free(ihaddr);
		}

		if (jbt->bProxyDiscovered)
			query << XCHILD("streamhost") << XATTR("jid", jbt->szProxyJid) << XATTR("host", jbt->szProxyHost) << XATTR("port", jbt->szProxyPort);

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
		(this->*jbt->pfnFinal)(jbt->state == JBT_DONE, jbt->ft);
		jbt->ft = nullptr;
		// stupid fix: wait for listening thread exit
		Sleep(100);
		delete jbt;
		return;
	}

	if (jbt->bProxyDiscovered && !mir_strcmp(jbt->szProxyJid, jbt->szStreamhostUsed)) {
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
		(this->*jbt->pfnFinal)(jbt->state == JBT_DONE, jbt->ft);
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

void CJabberProto::ByteInitiateResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	JABBER_BYTE_TRANSFER *jbt = (JABBER_BYTE_TRANSFER *)pInfo->GetUserData();

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		auto *queryNode = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_BYTESTREAMS);
		if (queryNode) {
			auto *streamHostNode = XmlFirstChild(queryNode, "streamhost-used");
			if (streamHostNode) {
				const char *streamJid = XmlGetAttr(streamHostNode, "jid");
				if (streamJid)
					jbt->szStreamhostUsed = mir_strdup(streamJid);
			}
		}
	}

	if (jbt->hProxyEvent)
		SetEvent(jbt->hProxyEvent);
}

int CJabberProto::ByteSendParse(HNETLIBCONN hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
{
	int nMethods;
	uint8_t data[10];
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
		if (datalen >= 2 && buffer[0] == 5 && buffer[1] + 2 == datalen) {
			nMethods = buffer[1];
			for (i = 0; i < nMethods && buffer[2 + i] != 0; i++);
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
		if (datalen == 47 && *((uint32_t*)buffer) == 0x03000105 && buffer[4] == 40 && *((uint16_t*)(buffer + 45)) == 0) {
			ptrA szInitiatorJid(JabberPrepareJid(jbt->srcJID));
			ptrA szTargetJid(JabberPrepareJid(jbt->dstJID));
			CMStringA szAuthString(FORMAT, "%s%s%s", jbt->sid, szInitiatorJid.get(), szTargetJid.get());
			debugLogA("Auth: '%s'", szAuthString.c_str());

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

void CJabberProto::IqResultStreamActivate(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	int id = JabberGetPacketID(iqNode);

	char listJid[JABBER_MAX_JID_LEN];
	mir_snprintf(listJid, "ftproxy_%d", id);

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTIQID, listJid);
	if (item == nullptr)
		return;

	if (!mir_strcmp(XmlGetAttr(iqNode, "type"), "result"))
		item->jbt->bStreamActivated = true;

	if (item->jbt->hProxyEvent)
		SetEvent(item->jbt->hProxyEvent);
}


void CJabberProto::ByteSendViaProxy(JABBER_BYTE_TRANSFER *jbt)
{
	if (jbt == nullptr) return;

	char* buffer;
	if ((buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE)) == nullptr) {
		m_ThreadInfo->send(XmlNodeIq("error", jbt->iqId, jbt->srcJID)
			<< XCHILD("error") << XATTRI("code", 406) << XATTR("type", "auth")
			<< XCHILDNS("not-acceptable", "urn:ietf:params:xml:ns:xmpp-stanzas"));
		return;
	}

	jbt->state = JBT_INIT;
	bool validStreamhost = false;
	char *szPort = jbt->szProxyPort;
	char *szHost = jbt->szProxyHost;

	uint16_t port = (uint16_t)atoi(szPort);
	replaceStr(jbt->streamhostJID, jbt->szProxyJid);

	NETLIBOPENCONNECTION nloc = {};
	nloc.szHost = szHost;
	nloc.wPort = port;

	HNETLIBCONN hConn = Netlib_OpenConnection(m_hNetlibUser, &nloc);
	mir_free((void*)nloc.szHost);

	if (hConn != nullptr) {
		jbt->hConn = hConn;

		char data[3] = { 5, 1, 0 };
		Netlib_Send(hConn, data, 3, 0);

		jbt->state = JBT_INIT;
		int datalen = 0;
		while (jbt->state != JBT_DONE && jbt->state != JBT_ERROR && jbt->state != JBT_SOCKSERR) {
			int recvResult = Netlib_Recv(hConn, buffer + datalen, JABBER_NETWORK_BUFFER_SIZE - datalen, 0);
			if (recvResult <= 0)
				break;

			datalen += recvResult;
			int bytesParsed = ByteSendProxyParse(hConn, jbt, buffer, datalen);
			if (bytesParsed < datalen)
				memmove(buffer, buffer + bytesParsed, datalen - bytesParsed);
			datalen -= bytesParsed;
			if (jbt->state == JBT_DONE)
				validStreamhost = true;
		}
		Netlib_CloseHandle(hConn);
	}
	mir_free(buffer);
	(this->*jbt->pfnFinal)(jbt->state == JBT_DONE, jbt->ft);
	jbt->ft = nullptr;
	if (!validStreamhost)
		m_ThreadInfo->send(XmlNodeIq("error", jbt->iqId, jbt->srcJID)
			<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
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
			uint8_t data[47];
			memset(data, 0, sizeof(data));
			*((uint32_t*)data) = 0x03000105;
			data[4] = 40;

			char szAuthString[256];
			mir_snprintf(szAuthString, "%s%s%s", jbt->sid, ptrA(JabberPrepareJid(jbt->srcJID)).get(), ptrA(JabberPrepareJid(jbt->dstJID)).get());
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
			jbt->bStreamActivated = false;

			auto *pIq = AddIQ(&CJabberProto::IqResultStreamActivate, JABBER_IQ_TYPE_SET, jbt->streamhostJID);

			char listJid[256];
			mir_snprintf(listJid, "ftproxy_%d", pIq->GetIqId());

			JABBER_LIST_ITEM *item = ListAdd(LIST_FTIQID, listJid);
			item->jbt = jbt;

			m_ThreadInfo->send(XmlNodeIq(pIq) << XQUERY(JABBER_FEAT_BYTESTREAMS) << XATTR("sid", jbt->sid) << XCHILD("activate", jbt->dstJID));
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
	Thread_SetName("Jabber: ByteReceiveThread");

	if (jbt == nullptr)
		return;

	const TiXmlElement *iqNode, *queryNode = nullptr;
	uint16_t port;
	char data[3];
	char* buffer;
	int datalen, bytesParsed, recvResult;
	bool validStreamhost = false;

	jbt->state = JBT_INIT;

	const char *sid = nullptr, *from = nullptr, *to = nullptr, *szId = nullptr;
	if (iqNode = jbt->iqNode) {
		from = XmlGetAttr(iqNode, "from");
		to = XmlGetAttr(iqNode, "to");
		szId = XmlGetAttr(iqNode, "id");

		queryNode = XmlFirstChild(iqNode, "query");
		if (queryNode)
			sid = XmlGetAttr(queryNode, "sid");
	}

	if (szId && from && to && sid) {
		jbt->iqId = mir_strdup(szId);
		jbt->srcJID = mir_strdup(from);
		jbt->dstJID = mir_strdup(to);
		jbt->sid = mir_strdup(sid);

		if ((buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE))) {
			for (auto *n : TiXmlFilter(queryNode, "streamhost")) {
				const char *str = XmlGetAttr(n, "jid");
				const char *szHost = XmlGetAttr(n, "host");
				const char *szPort = XmlGetAttr(n, "port");
				if (str != nullptr && szHost != nullptr && szPort != nullptr) {
					port = (uint16_t)atoi(szPort);
					replaceStr(jbt->streamhostJID, str);

					debugLogA("bytestream_recv connecting to %s:%d", szHost, port);
					NETLIBOPENCONNECTION nloc = { 0 };
					nloc.szHost = mir_strdup(szHost);
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
						if (jbt->state == JBT_RECVING)
							validStreamhost = true;
					}
					Netlib_CloseHandle(hConn);
					debugLogA("bytestream_recv_connection closing connection");
				}
				if (jbt->state == JBT_ERROR || validStreamhost)
					break;
				debugLogA("bytestream_recv_connection stream cannot be established, try next streamhost");
			}
			mir_free(buffer);
		}
	}

	(this->*jbt->pfnFinal)(jbt->state == JBT_DONE, jbt->ft);
	jbt->ft = nullptr;
	if (!validStreamhost && szId && from) {
		debugLogA("bytestream_recv_connection session not completed");

		m_ThreadInfo->send(XmlNodeIq("error", szId, from)
			<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
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
			uint8_t data[47];
			memset(data, 0, sizeof(data));
			*((uint32_t*)data) = 0x03000105;
			data[4] = 40;

			CMStringA szAuthString(FORMAT, "%s%s%s", jbt->sid, ptrA(JabberPrepareJid(jbt->srcJID)).get(), ptrA(JabberPrepareJid(jbt->dstJID)).get());
			debugLogA("Auth: '%s'", szAuthString.c_str());

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
				XmlNodeIq("result", jbt->iqId, jbt->srcJID) << XQUERY(JABBER_FEAT_BYTESTREAMS)
				<< XCHILD("streamhost-used") << XATTR("jid", jbt->streamhostJID));
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
