/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-14  Miranda NG project

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

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_byte.h"
#include "jabber_caps.h"

#define JABBER_NETWORK_BUFFER_SIZE 4096

///////////////// Bytestream sending /////////////////////////

JABBER_BYTE_TRANSFER::~JABBER_BYTE_TRANSFER()
{
	filetransfer* pft = ft;
	if (pft)
		pft->jbt = NULL;

	mir_free(srcJID);
	mir_free(dstJID);
	mir_free(streamhostJID);
	mir_free(iqId);
	mir_free(sid);

	xi.destroyNode(iqNode);

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
		HXML queryNode = xmlGetChild(iqNode , "query");
		if (queryNode) {
			const TCHAR *queryXmlns = xmlGetAttrValue(queryNode, _T("xmlns"));
			if (queryXmlns && !_tcscmp(queryXmlns, JABBER_FEAT_BYTESTREAMS)) {
				HXML streamHostNode = xmlGetChild(queryNode , "streamhost");
				if (streamHostNode) {
					const TCHAR *streamJid = xmlGetAttrValue(streamHostNode, _T("jid"));
					const TCHAR *streamHost = xmlGetAttrValue(streamHostNode, _T("host"));
					const TCHAR *streamPort = xmlGetAttrValue(streamHostNode, _T("port"));
					if (streamJid && streamHost && streamPort) {
						jbt->szProxyHost = mir_tstrdup(streamHost);
						jbt->szProxyJid = mir_tstrdup(streamJid);
						jbt->szProxyPort = mir_tstrdup(streamPort);
						jbt->bProxyDiscovered = TRUE;
	}	}	}	}	}
	else if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR)
		jbt->state = JBT_ERROR;

	if (jbt->hProxyEvent)
		SetEvent(jbt->hProxyEvent);
}

void JabberByteSendConnection(HANDLE hConn, DWORD /*dwRemoteIP*/, void* extra)
{
	CJabberProto *ppro = (CJabberProto*)extra;
	TCHAR szPort[8];
	JABBER_BYTE_TRANSFER *jbt;
	int recvResult, bytesParsed;
	HANDLE hListen;
	JABBER_LIST_ITEM *item;
	char* buffer;
	int datalen;

	NETLIBCONNINFO connInfo = { sizeof(connInfo) };
	CallService(MS_NETLIB_GETCONNECTIONINFO, (WPARAM)hConn, (LPARAM)&connInfo);

	mir_sntprintf(szPort, SIZEOF(szPort), _T("%u"), connInfo.wPort);
	ppro->debugLogA("bytestream_send_connection incoming connection accepted: %s", connInfo.szIpPort);

	if ((item = ppro->ListGetItemPtr(LIST_BYTE, szPort)) == NULL) {
		ppro->debugLogA("No bytestream session is currently active, connection closed.");
		Netlib_CloseHandle(hConn);
		return;
	}

	jbt = item->jbt;

	if ((buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE)) == NULL) {
		ppro->debugLogA("bytestream_send cannot allocate network buffer, connection closed.");
		jbt->state = JBT_ERROR;
		Netlib_CloseHandle(hConn);
		if (jbt->hEvent != NULL) SetEvent(jbt->hEvent);
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

	if (jbt->hEvent != NULL)
		SetEvent(jbt->hEvent);
}

void CJabberProto::ByteSendThread(JABBER_BYTE_TRANSFER *jbt)
{
	TCHAR szPort[8];
	HANDLE hEvent = NULL;
	CJabberIqInfo *pInfo = NULL;
	int nIqId = 0;

	debugLogA("Thread started: type=bytestream_send");

	BOOL bDirect = m_options.BsDirect;

	if (m_options.BsProxyManual) {
		ptrT proxyJid( getTStringA("BsProxyServer"));
		if (proxyJid) {
			jbt->bProxyDiscovered = FALSE;
			jbt->szProxyHost = NULL;
			jbt->szProxyPort = NULL;
			jbt->szProxyJid = NULL;
			jbt->hProxyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

			pInfo = AddIQ(&CJabberProto::IqResultProxyDiscovery, JABBER_IQ_TYPE_GET, proxyJid, 0, -1, jbt);
			nIqId = pInfo->GetIqId();
			XmlNodeIq iq(pInfo);
			iq << XQUERY(JABBER_FEAT_BYTESTREAMS);
			m_ThreadInfo->send(iq);

			WaitForSingleObject(jbt->hProxyEvent, INFINITE);
			m_iqManager.ExpireIq (nIqId);
			CloseHandle(jbt->hProxyEvent);
			jbt->hProxyEvent = NULL;

			if (jbt->state == JBT_ERROR && !bDirect) {
				debugLogA("Bytestream proxy failure");
				MsgPopup( pInfo->GetHContact(), TranslateT("Bytestream Proxy not available"), pInfo->GetReceiver());
				jbt->ft->state = FT_DENIED;
				(this->*jbt->pfnFinal)(FALSE, jbt->ft);
				jbt->ft = NULL;
				delete jbt;
				return;
			}
	}	}

	pInfo = AddIQ(&CJabberProto::ByteInitiateResult, JABBER_IQ_TYPE_SET, jbt->dstJID, 0, -1, jbt);
	nIqId = pInfo->GetIqId();
	{
		XmlNodeIq iq(pInfo);
		HXML query = iq << XQUERY(JABBER_FEAT_BYTESTREAMS) << XATTR(_T("sid"), jbt->sid);

		if (bDirect) {
			ptrA localAddr;
			if (m_options.BsDirectManual)
				localAddr = getStringA("BsDirectAddr");

			NETLIBBIND nlb = {0};
			nlb.cbSize = sizeof(NETLIBBIND);
			nlb.pfnNewConnectionV2 = JabberByteSendConnection;
			nlb.pExtra = this;
			nlb.wPort = 0;	// Use user-specified incoming port ranges, if available
			jbt->hConn = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hNetlibUser, (LPARAM)&nlb);
			if (jbt->hConn == NULL) {
				debugLogA("Cannot allocate port for bytestream_send thread, thread ended.");
				delete jbt;
				return;
			}

			if (localAddr == NULL)
				localAddr = (char*)CallService(MS_NETLIB_ADDRESSTOSTRING, 1, nlb.dwExternalIP);

			mir_sntprintf(szPort, SIZEOF(szPort), _T("%d"), nlb.wPort);
			JABBER_LIST_ITEM *item = ListAdd(LIST_BYTE, szPort);
			item->jbt = jbt;
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			jbt->hEvent = hEvent;
			jbt->hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			query << XCHILD(_T("streamhost")) << XATTR(_T("jid"), m_ThreadInfo->fullJID) << XATTR(_T("host"), _A2T(localAddr)) << XATTRI(_T("port"), nlb.wPort);

			NETLIBIPLIST* ihaddr = (NETLIBIPLIST*)CallService(MS_NETLIB_GETMYIP, 1, 0);
			for (unsigned i=0; i < ihaddr->cbNum; i++)
				if (strcmp(localAddr, ihaddr->szIp[i]))
					query << XCHILD(_T("streamhost")) << XATTR(_T("jid"), m_ThreadInfo->fullJID) << XATTR(_T("host"), _A2T(ihaddr->szIp[i])) << XATTRI(_T("port"), nlb.wPort);

			mir_free(ihaddr);
		}

		if (jbt->bProxyDiscovered)
			query << XCHILD(_T("streamhost")) << XATTR(_T("jid"), jbt->szProxyJid) << XATTR(_T("host"), jbt->szProxyHost) << XATTR(_T("port"), jbt->szProxyPort);

		jbt->hProxyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		jbt->szStreamhostUsed = NULL;

		m_ThreadInfo->send(iq);
	}

	WaitForSingleObject(jbt->hProxyEvent, INFINITE);
	m_iqManager.ExpireIq(nIqId);
	CloseHandle(jbt->hProxyEvent);
	jbt->hProxyEvent = NULL;

	if (!jbt->szStreamhostUsed) {
		if (bDirect) {
			SetEvent(jbt->hSendEvent);
			CloseHandle(jbt->hSendEvent);
			CloseHandle(hEvent);
			jbt->hEvent = NULL;
			if (jbt->hConn != NULL)
				Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = NULL;
			ListRemove(LIST_BYTE, szPort);
		}
		(this->*jbt->pfnFinal)((jbt->state==JBT_DONE)?TRUE:FALSE, jbt->ft);
		jbt->ft = NULL;
		// stupid fix: wait for listening thread exit
		Sleep(100);
		delete jbt;
		return;
	}

	if (jbt->bProxyDiscovered && !_tcscmp(jbt->szProxyJid, jbt->szStreamhostUsed)) {
		// jabber proxy used
		if (bDirect) {
			SetEvent(jbt->hSendEvent);
			CloseHandle(jbt->hSendEvent);
			CloseHandle(hEvent);
			jbt->hEvent = NULL;
			if (jbt->hConn != NULL)
				Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = NULL;
			ListRemove(LIST_BYTE, szPort);
		}
		ByteSendViaProxy(jbt);
	}
	else {
		SetEvent(jbt->hSendEvent);
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
		CloseHandle(jbt->hSendEvent);
		jbt->hEvent = NULL;
		(this->*jbt->pfnFinal)((jbt->state == JBT_DONE) ? TRUE : FALSE, jbt->ft);
		jbt->ft = NULL;
		if (jbt->hConn != NULL)
			Netlib_CloseHandle(jbt->hConn);
		jbt->hConn = NULL;
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
		HXML queryNode = xmlGetChild(iqNode , "query");
		if (queryNode) {
			const TCHAR *queryXmlns = xmlGetAttrValue(queryNode, _T("xmlns"));
			if (queryXmlns && !_tcscmp(queryXmlns, JABBER_FEAT_BYTESTREAMS)) {
				HXML streamHostNode = xmlGetChild(queryNode ,  "streamhost-used");
				if (streamHostNode) {
					const TCHAR *streamJid = xmlGetAttrValue(streamHostNode, _T("jid"));
					if (streamJid)
						jbt->szStreamhostUsed = mir_tstrdup(streamJid);
	}	}	}	}

	if (jbt->hProxyEvent)
		SetEvent(jbt->hProxyEvent);
}

int CJabberProto::ByteSendParse(HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
{
	int nMethods;
	BYTE data[10];
	int i;
	char* str;

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
			TCHAR text[256];

			TCHAR *szInitiatorJid = JabberPrepareJid(jbt->srcJID);
			TCHAR *szTargetJid = JabberPrepareJid(jbt->dstJID);
			mir_sntprintf(text, SIZEOF(text), _T("%s%s%s"), jbt->sid, szInitiatorJid, szTargetJid);
			mir_free(szInitiatorJid);
			mir_free(szTargetJid);

			char* szAuthString = mir_utf8encodeT(text);
			debugLogA("Auth: '%s'", szAuthString);
			if ((str = JabberSha1(szAuthString)) != NULL) {
				for (i=0; i<40 && buffer[i+5]==str[i]; i++);
				mir_free(str);

				memset(data, 0, sizeof(data));
				data[1] = (i>=20)?0:2;
				data[0] = 5;
				data[3] = 1;
				Netlib_Send(hConn, (char*)data, 10, 0);

				// wait stream activation
				WaitForSingleObject(jbt->hSendEvent, INFINITE);

				if (jbt->state == JBT_ERROR)
					break;

				if (i>=20 && (this->*jbt->pfnSend)(hConn, jbt->ft)==TRUE)
					jbt->state = JBT_DONE;
				else
					jbt->state = JBT_ERROR;
			}
			mir_free(szAuthString);
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

	TCHAR listJid[JABBER_MAX_JID_LEN];
	mir_sntprintf(listJid, SIZEOF(listJid), _T("ftproxy_%d"), id);

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTIQID, listJid);
	if (item == NULL)
		return;

	if (!mir_tstrcmp(xmlGetAttrValue(iqNode, _T("type")), _T("result")))
		item->jbt->bStreamActivated = TRUE;

	if (item->jbt->hProxyEvent)
		SetEvent(item->jbt->hProxyEvent);
}


void CJabberProto::ByteSendViaProxy(JABBER_BYTE_TRANSFER *jbt)
{
	TCHAR *szHost, *szPort;
	WORD port;
	HANDLE hConn;
	char data[3];
	char* buffer;
	int datalen, bytesParsed, recvResult;
	BOOL validStreamhost;

	if (jbt == NULL) return;
	if ((buffer=(char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE)) == NULL) {
		m_ThreadInfo->send( XmlNodeIq(_T("error"), jbt->iqId, jbt->srcJID)
			<< XCHILD(_T("error")) << XATTRI(_T("code"), 406) << XATTR(_T("type"), _T("auth"))
			<< XCHILDNS(_T("not-acceptable"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
		return;
	}

	jbt->state = JBT_INIT;
	validStreamhost = FALSE;
	szPort = jbt->szProxyPort;
	szHost = jbt->szProxyHost;

	port = (WORD)_ttoi(szPort);
	replaceStrT(jbt->streamhostJID, jbt->szProxyJid);

	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof(nloc);
	nloc.szHost = mir_t2a(szHost);
	nloc.wPort = port;
	hConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
	mir_free((void*)nloc.szHost);

	if (hConn != NULL) {
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
	jbt->ft = NULL;
	if (!validStreamhost)
		m_ThreadInfo->send( XmlNodeIq(_T("error"), jbt->iqId, jbt->srcJID)
			<< XCHILD(_T("error")) << XATTRI(_T("code"), 404) << XATTR(_T("type"), _T("cancel"))
			<< XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
}

int CJabberProto::ByteSendProxyParse(HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
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

			TCHAR text[256];

			TCHAR *szInitiatorJid = JabberPrepareJid(jbt->srcJID);
			TCHAR *szTargetJid = JabberPrepareJid(jbt->dstJID);
			mir_sntprintf(text, SIZEOF(text), _T("%s%s%s"), jbt->sid, szInitiatorJid, szTargetJid);
			mir_free(szInitiatorJid);
			mir_free(szTargetJid);

			char* szAuthString = mir_utf8encodeT(text);
			debugLogA("Auth: '%s'", szAuthString);
			char* szHash = JabberSha1(szAuthString);
			strncpy_s((char*)(data + 5), 40, szHash, _TRUNCATE);
			mir_free(szHash);
			Netlib_Send(hConn, (char*)data, 47, 0);
			jbt->state = JBT_CONNECT;
			mir_free(szAuthString);
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

			jbt->hProxyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			jbt->bStreamActivated = FALSE;

			int iqId = SerialNext();

			TCHAR listJid[256];
			mir_sntprintf(listJid, SIZEOF(listJid), _T("ftproxy_%d"), iqId);

			JABBER_LIST_ITEM *item = ListAdd(LIST_FTIQID, listJid);
			item->jbt = jbt;

			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::IqResultStreamActivate, JABBER_IQ_TYPE_SET, jbt->streamhostJID, 0, iqId))
				<< XQUERY(JABBER_FEAT_BYTESTREAMS) << XATTR(_T("sid"), jbt->sid) << XCHILD(_T("activate"), jbt->dstJID));

			WaitForSingleObject(jbt->hProxyEvent, INFINITE);

			CloseHandle(jbt->hProxyEvent);
			jbt->hProxyEvent = NULL;

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
	HXML iqNode, queryNode = NULL, n;
	const TCHAR *sid = NULL, *from = NULL, *to = NULL, *szId = NULL, *szHost, *szPort, *str;
	int i;
	WORD port;
	HANDLE hConn;
	char data[3];
	char* buffer;
	int datalen, bytesParsed, recvResult;
	BOOL validStreamhost = FALSE;

	if (jbt == NULL) return;

	jbt->state = JBT_INIT;

	if (iqNode = jbt->iqNode) {
		from = xmlGetAttrValue(iqNode, _T("from"));
		to = xmlGetAttrValue(iqNode, _T("to"));
		szId = xmlGetAttrValue(iqNode, _T("id"));

		queryNode = xmlGetChild(iqNode, "query");
		if (queryNode)
			sid = xmlGetAttrValue(queryNode, _T("sid"));
	}

	if (szId && from && to && sid && (n = xmlGetChild(queryNode, "streamhost")) != NULL) {
		jbt->iqId = mir_tstrdup(szId);
		jbt->srcJID = mir_tstrdup(from);
		jbt->dstJID = mir_tstrdup(to);
		jbt->sid = mir_tstrdup(sid);

		if ((buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE))) {
			for (i = 1; (n = xmlGetNthChild(queryNode, _T("streamhost"), i)) != NULL; i++) {
				if ((szHost = xmlGetAttrValue(n, _T("host"))) != NULL &&
					 (szPort = xmlGetAttrValue(n, _T("port"))) != NULL &&
					 (str = xmlGetAttrValue(n, _T("jid"))) != NULL) {

					port = (WORD)_ttoi(szPort);
					replaceStrT(jbt->streamhostJID, str);

					debugLog(_T("bytestream_recv connecting to %s:%d"), szHost, port);
					NETLIBOPENCONNECTION nloc = { 0 };
					nloc.cbSize = sizeof(nloc);
					nloc.szHost = mir_t2a(szHost);
					nloc.wPort = port;
					hConn = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
					mir_free((void*)nloc.szHost);

					if (hConn == NULL) {
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
	jbt->ft = NULL;
	if (!validStreamhost && szId && from) {
		debugLogA("bytestream_recv_connection session not completed");

		m_ThreadInfo->send(XmlNodeIq(_T("error"), szId, from)
								 << XCHILD(_T("error")) << XATTRI(_T("code"), 404) << XATTR(_T("type"), _T("cancel"))
								 << XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
	}

	delete jbt;
	debugLogA("Thread ended: type=bytestream_recv");
}

int CJabberProto::ByteReceiveParse(HANDLE hConn, JABBER_BYTE_TRANSFER *jbt, char* buffer, int datalen)
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

			TCHAR text[JABBER_MAX_JID_LEN * 2];
			TCHAR *szInitiatorJid = JabberPrepareJid(jbt->srcJID);
			TCHAR *szTargetJid = JabberPrepareJid(jbt->dstJID);
			mir_sntprintf(text, SIZEOF(text), _T("%s%s%s"), jbt->sid, szInitiatorJid, szTargetJid);
			mir_free(szInitiatorJid);
			mir_free(szTargetJid);
			char* szAuthString = mir_utf8encodeT(text);
			debugLogA("Auth: '%s'", szAuthString);
			char* szHash = JabberSha1(szAuthString);
			strncpy_s((char*)(data + 5), 40, szHash, _TRUNCATE);
			mir_free(szHash);
			Netlib_Send(hConn, (char*)data, 47, 0);
			jbt->state = JBT_CONNECT;
			mir_free(szAuthString);
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
				XmlNodeIq(_T("result"), jbt->iqId, jbt->srcJID) << XQUERY(JABBER_FEAT_BYTESTREAMS)
				<< XCHILD(_T("streamhost-used")) << XATTR(_T("jid"), jbt->streamhostJID));
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
