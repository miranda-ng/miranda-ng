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
#include "jabber_ibb.h"
#include "jabber_caps.h"

void CJabberProto::FtCancel(filetransfer *ft)
{
	debugLogA("Invoking JabberFtCancel()");

	// For file sending session that is still in si negotiation phase
	if (m_iqManager.ExpireByUserData(ft))
		return;

	// For file receiving session that is still in si negotiation phase
	LISTFOREACH(i, this, LIST_FTRECV)
	{
		JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
		if (item->ft == ft) {
			debugLogA("Canceling file receiving session while in si negotiation");
			ListRemoveByIndex(i);
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft);
			delete ft;
			return;
		}
	}
	// For file transfer through bytestream
	JABBER_BYTE_TRANSFER *jbt = ft->jbt;
	if (jbt != nullptr) {
		debugLogA("Canceling bytestream session");
		jbt->state = JBT_ERROR;
		if (jbt->hConn) {
			debugLogA("Force closing bytestream session");
			Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = nullptr;
		}
		if (jbt->hSendEvent) SetEvent(jbt->hSendEvent);
		if (jbt->hEvent) SetEvent(jbt->hEvent);
		if (jbt->hProxyEvent) SetEvent(jbt->hProxyEvent);
	}

	// For file transfer through IBB
	JABBER_IBB_TRANSFER *jibb = ft->jibb;
	if (jibb != nullptr) {
		debugLogA("Canceling IBB session");
		jibb->state = JIBB_ERROR;
		m_iqManager.ExpireByUserData(jibb);
	}
}

///////////////// File sending using stream initiation /////////////////////////

static void __cdecl FakeAckThread(void *param)
{
	Sleep(100);

	auto *ft = (filetransfer *)param;
	ft->complete();
	delete ft;
}

static void __cdecl FailedAckThread(void *param)
{
	Sleep(100);

	auto *ft = (filetransfer *)param;
	ft->ppro->ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft);
	delete ft;
}

void CJabberProto::FtInitiate(filetransfer *ft)
{
	if (ft == nullptr)
		return;

	if (!m_bJabberOnline) {
		debugLogA("Protocol is offline, file transfer failed");

	LBL_Error:
		mir_forkthread(FailedAckThread, ft);
		return;
	}

	wchar_t *filename = ft->std.pszFiles.w[ft->std.currentFileNumber];
	if (wchar_t *p = wcsrchr(filename, '\\'))
		filename = p + 1;

	// if we enabled XEP-0231, try to inline a picture (but only for private chats)
	if (m_bInlinePictures && ProtoGetAvatarFileFormat(ft->std.szCurrentFile.w) && !isChatRoom(ft->std.hContact)) {
		if (FtTryInlineFile(ft)) {
			mir_forkthread(FakeAckThread, ft);
			return;
		}
	}

	// if we use XEP-0363, send a slot allocation request
	if (m_bUseHttpUpload) {
		ptrA szUploadService(getStringA("HttpUpload"));
		if (szUploadService != nullptr) {
			ft->type = FT_HTTP;

			struct _stat64 st;
			_wstat64(ft->std.szCurrentFile.w, &st);

			uint32_t maxsize = getDword("HttpUploadMaxSize");
			if (maxsize && st.st_size > maxsize) {
				MsgPopup(ft->std.hContact, CMStringW(FORMAT, TranslateT("%s is too large. Maximum size supported by the service is %d KB"),
					ft->std.szCurrentFile.w, maxsize / 1024), L"HTTP Upload");
				goto LBL_Error;
			}

			auto *pwszContentType = ProtoGetAvatarMimeType(ProtoGetAvatarFileFormat(ft->std.szCurrentFile.w));
			if (pwszContentType == nullptr)
				pwszContentType = "application/octet-stream";

			char szSize[100];
			_i64toa(st.st_size, szSize, 10);

			XmlNodeIq iq(AddIQ(&CJabberProto::OnHttpSlotAllocated, JABBER_IQ_TYPE_GET, szUploadService, ft));
			if (getByte("HttpUploadVer")) {
				auto *p = iq << XCHILDNS("request", JABBER_FEAT_UPLOAD);
				p << XCHILD("filename", T2Utf(filename));
				p << XCHILD("size", szSize);
				p << XCHILD("content-type", pwszContentType);
			}
			else {
				iq << XCHILDNS("request", JABBER_FEAT_UPLOAD0)
					<< XATTR("filename", T2Utf(filename)) << XATTR("size", szSize) << XATTR("content-type", pwszContentType);
			}
			m_ThreadInfo->send(iq);
			return;
		}
	}

	// offline methods are over, the following code assumes that a contact is online
	if (getWord(ft->std.hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
		debugLogA("%S is offline, file transfer failed", Clist_GetContactDisplayName(ft->std.hContact));
		goto LBL_Error;
	}

	auto r = ft->pItem->getBestResource();
	if (r == nullptr) {
		debugLogA("%S has no current resource available, file transfer failed", Clist_GetContactDisplayName(ft->std.hContact));
		goto LBL_Error;
	}

	JabberCapsBits jcb = GetResourceCapabilities(ft->pItem->jid, r);
	if (jcb == JABBER_RESOURCE_CAPS_IN_PROGRESS) {
		Sleep(600);
		jcb = GetResourceCapabilities(ft->pItem->jid);
	}

	if (!m_bBsDirect) {
		jcb &= ~JABBER_CAPS_OOB;
		if(!m_bBsProxyManual) // disable bytestreams
			jcb &= ~JABBER_CAPS_BYTESTREAMS;
	}

	// if only JABBER_CAPS_SI_FT feature set (without BS or IBB), disable JABBER_CAPS_SI_FT
	if ((jcb & (JABBER_CAPS_SI_FT | JABBER_CAPS_IBB | JABBER_CAPS_BYTESTREAMS)) == JABBER_CAPS_SI_FT)
		jcb &= ~JABBER_CAPS_SI_FT;

	if ((jcb & JABBER_RESOURCE_CAPS_ERROR) // can't get caps
		|| (jcb == JABBER_RESOURCE_CAPS_NONE) // caps not already received
		|| !(jcb & (JABBER_CAPS_SI_FT | JABBER_CAPS_OOB))) // XEP-0096 and OOB not supported?
	{
		MsgPopup(ft->std.hContact, TranslateT("No compatible file transfer mechanism exists"), Utf2T(ft->pItem->jid));
		goto LBL_Error;
	}

	// no cloud services enabled, try to initiate a p2p file transfer
	if (jcb & JABBER_CAPS_SI_FT) {
		ft->type = FT_SI;
		char sid[9];
		for (int i = 0; i < 8; i++)
			sid[i] = (rand() % 10) + '0';
		sid[8] = '\0';
		replaceStr(ft->sid, sid);

		auto *pIq = AddIQ(&CJabberProto::OnFtSiResult, JABBER_IQ_TYPE_SET, MakeJid(ft->pItem->jid, r->m_szResourceName), ft);
		pIq->SetParamsToParse(JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO);
		XmlNodeIq iq(pIq);
		TiXmlElement *si = iq << XCHILDNS("si", JABBER_FEAT_SI) << XATTR("id", sid)
			<< XATTR("mime-type", "binary/octet-stream") << XATTR("profile", JABBER_FEAT_SI_FT);
		si << XCHILDNS("file", JABBER_FEAT_SI_FT) << XATTR("name", T2Utf(filename))
			<< XATTRI64("size", ft->fileSize[ft->std.currentFileNumber]) << XCHILD("desc", T2Utf(ft->szDescription));

		TiXmlElement *field = si << XCHILDNS("feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form")
			<< XCHILD("field") << XATTR("var", "stream-method") << XATTR("type", "list-single");

		// bytestreams support?
		if (m_bBsDirect || m_bBsProxyManual)
			field << XCHILD("option") << XCHILD("value", JABBER_FEAT_BYTESTREAMS);

		field << XCHILD("option") << XCHILD("value", JABBER_FEAT_IBB);
		m_ThreadInfo->send(iq);
	}
	else // OOB then
		ForkThread((MyThreadFunc)&CJabberProto::FileServerThread, ft);
}

void CJabberProto::OnFtSiResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	filetransfer *ft = (filetransfer *)pInfo->GetUserData();
	if (!ft)
		return;

	if ((pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) && pInfo->GetFrom() && pInfo->GetTo()) {
		if (auto *siNode = XmlFirstChild(iqNode, "si")) {
			// fix for very smart clients, like gajim
			BOOL bDirect = m_bBsDirect;
			BOOL bProxy = m_bBsProxyManual;

			if (auto *featureNode = XmlFirstChild(siNode, "feature")) {
				if (auto *xNode = XmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) {
					if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "stream-method")) {
						if (auto *valueNode = XmlFirstChild(fieldNode, "value")) {
							if ((bDirect || bProxy) && !mir_strcmp(valueNode->GetText(), JABBER_FEAT_BYTESTREAMS)) {
								// Start Bytestream session
								JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
								memset(jbt, 0, sizeof(JABBER_BYTE_TRANSFER));
								jbt->srcJID = mir_strdup(pInfo->GetTo());
								jbt->dstJID = mir_strdup(pInfo->GetFrom());
								jbt->sid = mir_strdup(ft->sid);
								jbt->pfnSend = &CJabberProto::FtSend;
								jbt->pfnFinal = &CJabberProto::FtSendFinal;
								jbt->ft = ft;
								ft->type = FT_BYTESTREAM;
								ft->jbt = jbt;
								ForkThread((MyThreadFunc)&CJabberProto::ByteSendThread, jbt);
							}
							else if (!mir_strcmp(valueNode->GetText(), JABBER_FEAT_IBB)) {
								JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)mir_alloc(sizeof(JABBER_IBB_TRANSFER));
								memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
								jibb->srcJID = mir_strdup(pInfo->GetTo());
								jibb->dstJID = mir_strdup(pInfo->GetFrom());
								jibb->sid = mir_strdup(ft->sid);
								jibb->pfnSend = &CJabberProto::FtIbbSend;
								jibb->pfnFinal = &CJabberProto::FtSendFinal;
								jibb->ft = ft;
								ft->type = FT_IBB;
								ft->jibb = jibb;
								ForkThread((MyThreadFunc)&CJabberProto::IbbSendThread, jibb);
							}
						}
					}
				}
			}
		}
	}
	else {
		debugLogA("File transfer stream initiation request denied or failed");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft);
		delete ft;
	}
}

bool CJabberProto::FtSend(HNETLIBCONN hConn, filetransfer *ft)
{
	debugLogW(L"Sending [%s]", ft->std.pszFiles.w[ft->std.currentFileNumber]);

	struct _stat64 statbuf;
	_wstat64(ft->std.pszFiles.w[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size
	int fd = _wopen(ft->std.pszFiles.w[ft->std.currentFileNumber], _O_BINARY | _O_RDONLY);
	if (fd < 0) {
		debugLogA("File cannot be opened");
		return false;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	if (char *buffer = (char *)mir_alloc(2048)) {
		int numRead;
		while ((numRead = _read(fd, buffer, 2048)) > 0) {
			if (Netlib_Send(hConn, buffer, numRead, 0) != numRead) {
				mir_free(buffer);
				_close(fd);
				return false;
			}
			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		}
		mir_free(buffer);
	}
	_close(fd);
	return true;
}

bool CJabberProto::FtIbbSend(int blocksize, filetransfer *ft)
{
	debugLogW(L"Sending [%s]", ft->std.pszFiles.w[ft->std.currentFileNumber]);

	struct _stat64 statbuf;
	_wstat64(ft->std.pszFiles.w[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size

	int fd = _wopen(ft->std.pszFiles.w[ft->std.currentFileNumber], _O_BINARY | _O_RDONLY);
	if (fd < 0) {
		debugLogA("File cannot be opened");
		return false;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	ptrA buffer((char*)mir_alloc(blocksize));
	if (buffer != nullptr) {
		int numRead;
		while ((numRead = _read(fd, buffer, blocksize)) > 0) {
			int iqId = SerialNext();
			XmlNode msg("message");
			XmlAddAttr(msg, "to", ft->jibb->dstJID);
			msg << XATTRID(iqId);

			// let others send data too
			Sleep(2);

			msg << XCHILD("data", ptrA(mir_base64_encode(buffer, numRead))) << XATTR("xmlns", JABBER_FEAT_IBB)
				<< XATTR("sid", ft->jibb->sid) << XATTRI("seq", ft->jibb->wPacketId);

			TiXmlElement *ampNode = msg << XCHILDNS("amp", JABBER_FEAT_AMP);
			ampNode << XCHILD("rule") << XATTR("condition", "deliver-at")
				<< XATTR("value", "stored") << XATTR("action", "error");
			ampNode << XCHILD("rule") << XATTR("condition", "match-resource")
				<< XATTR("value", "exact") << XATTR("action", "error");
			ft->jibb->wPacketId++;

			if (ft->jibb->state == JIBB_ERROR || ft->jibb->bStreamClosed || m_ThreadInfo->send(msg) == SOCKET_ERROR) {
				debugLogA("JabberFtIbbSend unsuccessful exit");
				_close(fd);
				return false;
			}

			ft->jibb->dwTransferredSize += (uint32_t)numRead;
			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		}
	}
	_close(fd);
	return true;
}

void CJabberProto::FtSendFinal(bool success, filetransfer *ft)
{
	if (!success) {
		debugLogA("File transfer complete with error");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ft->state == FT_DENIED ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft);
	}
	else {
		if (ft->std.currentFileNumber < ft->std.totalFiles - 1) {
			ft->std.currentFileNumber++;
			replaceStrW(ft->std.szCurrentFile.w, ft->std.pszFiles.w[ft->std.currentFileNumber]);
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft);
			FtInitiate(ft);
			return;
		}

		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft);
	}

	delete ft;
}

///////////////// File receiving through stream initiation /////////////////////////

void CJabberProto::FtHandleSiRequest(const TiXmlElement *iqNode)
{
	const char *from, *sid, *str, *filename;
	const TiXmlElement *siNode, *fileNode, *featureNode, *xNode, *fieldNode;

	if (!iqNode ||
		(from = XmlGetAttr(iqNode, "from")) == nullptr ||
		(str = XmlGetAttr(iqNode, "type")) == nullptr || mir_strcmp(str, "set") ||
		(siNode = XmlGetChildByTag(iqNode, "si", "xmlns", JABBER_FEAT_SI)) == nullptr)
		return;

	const char *szId = XmlGetAttr(iqNode, "id");
	if ((sid = XmlGetAttr(siNode, "id")) != nullptr &&
		(fileNode = XmlGetChildByTag(siNode, "file", "xmlns", JABBER_FEAT_SI_FT)) != nullptr &&
		(filename = XmlGetAttr(fileNode, "name")) != nullptr &&
		(str = XmlGetAttr(fileNode, "size")) != nullptr) {

		unsigned __int64 filesize = _atoi64(str);
		if ((featureNode = XmlGetChildByTag(siNode, "feature", "xmlns", JABBER_FEAT_FEATURE_NEG)) != nullptr &&
			(xNode = XmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) != nullptr &&
			(fieldNode = XmlGetChildByTag(xNode, "field", "var", "stream-method")) != nullptr) {

			BOOL bIbbOnly = !m_bBsDirect;
			const TiXmlElement *optionNode = nullptr;
			JABBER_FT_TYPE ftType = FT_OOB;

			if (!bIbbOnly) {
				for (auto *it : TiXmlFilter(fieldNode, "option")) {
					if (!mir_strcmp(XmlGetChildText(it, "value"), JABBER_FEAT_BYTESTREAMS)) {
						optionNode = it;
						ftType = FT_BYTESTREAM;
						break;
					}
				}
			}

			// try IBB only if bytestreams support not found or BsOnlyIBB flag exists
			if (bIbbOnly || !optionNode) {
				for (auto *it : TiXmlFilter(fieldNode, "option")) {
					if (!mir_strcmp(XmlGetChildText(it, "value"), JABBER_FEAT_IBB)) {
						optionNode = it;
						ftType = FT_IBB;
						break;
					}
				}
			}

			if (optionNode != nullptr) {
				// Found known stream mechanism
				filetransfer *ft = new filetransfer(this, 0);
				ft->dwExpectedRecvFileSize = filesize;
				ft->jid = mir_strdup(from);
				ft->std.hContact = HContactFromJID(from);
				ft->sid = mir_strdup(sid);
				ft->szId = mir_strdup(szId);
				ft->type = ftType;
				ft->std.totalFiles = 1;
				ft->std.szCurrentFile.w = mir_utf8decodeW(filename);
				ft->std.totalBytes = ft->std.currentFileSize = filesize;

				PROTORECVFILE pre = { 0 };
				pre.fileCount = 1;
				pre.timestamp = time(0);
				pre.files.a = &filename;
				pre.lParam = (LPARAM)ft;
				pre.descr.a = XmlGetChildText(fileNode, "desc");
				ProtoChainRecvFile(ft->std.hContact, &pre);
				return;
			}
		}
	}

	// Bad stream initiation, reply with bad-profile
	XmlNodeIq iq("error", szId, from);
	TiXmlElement *e = iq << XCHILD("error") << XATTRI("code", 400) << XATTR("type", "cancel");
	e << XCHILDNS("bad-request", "urn:ietf:params:xml:ns:xmpp-stanzas");
	e << XCHILDNS("bad-profile", JABBER_FEAT_SI);
	m_ThreadInfo->send(iq);
}

void CJabberProto::FtAcceptSiRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == nullptr || ft->jid == nullptr || ft->sid == nullptr) return;

	JABBER_LIST_ITEM *item;
	if ((item = ListAdd(LIST_FTRECV, ft->sid)) != nullptr) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq("result", ft->szId, ft->jid)
			<< XCHILDNS("si", JABBER_FEAT_SI)
			<< XCHILDNS("feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit")
			<< XCHILD("field") << XATTR("var", "stream-method")
			<< XCHILD("value", JABBER_FEAT_BYTESTREAMS));
	}
}

void CJabberProto::FtAcceptIbbRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == nullptr || ft->jid == nullptr || ft->sid == nullptr) return;

	JABBER_LIST_ITEM *item;
	if ((item = ListAdd(LIST_FTRECV, ft->sid)) != nullptr) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq("result", ft->szId, ft->jid)
			<< XCHILDNS("si", JABBER_FEAT_SI)
			<< XCHILDNS("feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit")
			<< XCHILD("field") << XATTR("var", "stream-method")
			<< XCHILD("value", JABBER_FEAT_IBB));
	}
}

bool CJabberProto::FtHandleBytestreamRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	auto *queryNode = pInfo->GetChildNode();

	const char *sid = XmlGetAttr(queryNode, "sid");
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (sid && item && m_bBsDirect) {
		// Start Bytestream session
		JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
		jbt->iqNode = iqNode->DeepClone(&jbt->doc)->ToElement();
		jbt->pfnRecv = &CJabberProto::FtReceive;
		jbt->pfnFinal = &CJabberProto::FtReceiveFinal;
		jbt->ft = item->ft;
		item->ft->jbt = jbt;
		ForkThread((MyThreadFunc)&CJabberProto::ByteReceiveThread, jbt);
		ListRemove(LIST_FTRECV, sid);
		return true;
	}

	debugLogA("File transfer invalid bytestream initiation request received");
	return true;
}

bool CJabberProto::FtHandleIbbRequest(const TiXmlElement *iqNode, bool bOpen)
{
	if (iqNode == nullptr)
		return false;

	const char *id = XmlGetAttr(iqNode, "id");
	const char *from = XmlGetAttr(iqNode, "from");
	const char *to = XmlGetAttr(iqNode, "to");
	if (!id || !from || !to)
		return false;

	auto *ibbNode = XmlGetChildByTag(iqNode, bOpen ? "open" : "close", "xmlns", JABBER_FEAT_IBB);
	if (!ibbNode)
		return false;

	const char *sid = XmlGetAttr(ibbNode, "sid");
	if (!sid)
		return false;

	// already closed?
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == nullptr) {
		m_ThreadInfo->send(
			XmlNodeIq("error", id, from)
			<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
		return false;
	}

	// open event
	if (bOpen) {
		if (!item->jibb) {
			JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)mir_alloc(sizeof(JABBER_IBB_TRANSFER));
			memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
			jibb->srcJID = mir_strdup(from);
			jibb->dstJID = mir_strdup(to);
			jibb->sid = mir_strdup(sid);
			jibb->pfnRecv = &CJabberProto::FtReceive;
			jibb->pfnFinal = &CJabberProto::FtReceiveFinal;
			jibb->ft = item->ft;
			item->ft->jibb = jibb;
			item->jibb = jibb;
			ForkThread((MyThreadFunc)&CJabberProto::IbbReceiveThread, jibb);

			m_ThreadInfo->send(XmlNodeIq("result", id, from));
			return true;
		}
		// stream already open
		m_ThreadInfo->send(
			XmlNodeIq("error", id, from)
			<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
		return false;
	}

	// close event && stream already open
	if (item->jibb && item->jibb->hEvent) {
		item->jibb->bStreamClosed = true;
		SetEvent(item->jibb->hEvent);

		m_ThreadInfo->send(XmlNodeIq("result", id, from));
		return true;
	}

	ListRemove(LIST_FTRECV, sid);
	return false;
}

int CJabberProto::FtReceive(HNETLIBCONN, filetransfer *ft, char* buffer, int datalen)
{
	if (ft->create() == -1)
		return -1;

	__int64 remainingBytes = ft->std.currentFileSize - ft->std.currentFileProgress;
	if (remainingBytes > 0) {
		int writeSize = (remainingBytes < datalen) ? remainingBytes : datalen;
		if (_write(ft->fileId, buffer, writeSize) != writeSize) {
			debugLogA("_write() error");
			return -1;
		}

		ft->std.currentFileProgress += writeSize;
		ft->std.totalProgress += writeSize;
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		return (ft->std.currentFileSize == ft->std.currentFileProgress) ? 0 : writeSize;
	}

	return 0;
}

void CJabberProto::FtReceiveFinal(bool success, filetransfer *ft)
{
	if (success) {
		debugLogA("File transfer completed successfully");
		ft->complete();
	}
	else debugLogA("File transfer completed with error");

	delete ft;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::OnHttpSlotAllocated(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	filetransfer *ft = (filetransfer *)pInfo->GetUserData();
	if (!ft)
		return;

	if (pInfo->GetIqType() != JABBER_IQ_TYPE_RESULT) {
		debugLogA("HTTP upload aborted");
LBL_Fail:
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft);
		delete ft;
		return;
	}

	if (auto *slotNode = XmlFirstChild(iqNode, "slot")) {
		if (auto *putNode = XmlFirstChild(slotNode, "put")) {
			const char *szXmlns = slotNode->Attribute("xmlns"), *szUrl = nullptr;
			uint8_t version = 0;
			if (!mir_strcmp(szXmlns, JABBER_FEAT_UPLOAD)) {
				szUrl = putNode->GetText();
				debugLogA("%s: setting url to %s", szXmlns, szUrl);
				version = 1;
			}
			else if (!mir_strcmp(szXmlns, JABBER_FEAT_UPLOAD0)) {
				szUrl = putNode->Attribute("url");
				debugLogA("%s: setting url to %s", szXmlns, szUrl);
			}
			else debugLogA("missing url location");

			if (szUrl) {
				NETLIBHTTPHEADER hdr[10];

				NETLIBHTTPREQUEST nlhr = {};
				nlhr.cbSize = sizeof(nlhr);
				nlhr.requestType = REQUEST_PUT;
				nlhr.flags = NLHRF_NODUMPSEND | NLHRF_SSL | NLHRF_REDIRECT;
				nlhr.szUrl = (char *)szUrl;

				for (auto *it : TiXmlFilter(putNode, "header")) {
					auto *szName = it->Attribute("name");
					auto *szValue = it->GetText();
					if (szName && szValue && nlhr.headersCount < _countof(hdr)) {
						nlhr.headers = hdr;
						hdr[nlhr.headersCount].szName = (char *)szName;
						hdr[nlhr.headersCount].szValue = (char *)szValue;
						nlhr.headersCount++;
					}
				}

				const wchar_t *pwszFileName = ft->std.pszFiles.w[ft->std.currentFileNumber];

				int fileId = _wopen(pwszFileName, _O_BINARY | _O_RDONLY);
				if (fileId < 0) {
					debugLogA("error opening file %S", pwszFileName);
					goto LBL_Fail;
				}

				nlhr.dataLength = _filelength(fileId);
				nlhr.pData = new char[nlhr.dataLength];
				_read(fileId, nlhr.pData, nlhr.dataLength);
				_close(fileId);

				NETLIBHTTPREQUEST *res = Netlib_HttpTransaction(m_hNetlibUser, &nlhr);
				if (res == nullptr) {
					debugLogA("error uploading file %S", pwszFileName);
					goto LBL_Fail;
				}
				
				switch (res->resultCode) {
				case 200: // ok
				case 201: // created
					break;

				default:
					debugLogA("error uploading file %S: error %d", pwszFileName, res->resultCode);
					Netlib_FreeHttpRequest(res);
					goto LBL_Fail;
				}

				Netlib_FreeHttpRequest(res);

				// this parameter is optional, if not specified we simply use upload URL
				CMStringA szMessage;
				if (auto *szGetUrl = version ?  XmlGetChildText(slotNode, "get") : XmlGetAttr(XmlFirstChild(slotNode, "get"), "url"))
					szMessage = szGetUrl;
				else
					szMessage = szUrl;

				if (m_bEmbraceUrls && ProtoGetAvatarFormat(_A2T(szMessage)) != PA_FORMAT_UNKNOWN) {
					szMessage.Insert(0, "[img]");
					szMessage.Append("[/img]");
				}

				if (isChatRoom(ft->std.hContact))
					GroupchatSendMsg(ft->pItem, ptrA(mir_utf8encode(szMessage)));
				else if (ProtoChainSend(ft->std.hContact, PSS_MESSAGE, 0, (LPARAM)szMessage.c_str()) != -1) {
					PROTORECVEVENT recv = {};
					recv.flags = PREF_CREATEREAD | PREF_SENT;
					recv.szMessage = szMessage.GetBuffer();
					recv.timestamp = time(0);
					ProtoChainRecvMsg(ft->std.hContact, &recv);
				}

				FtSendFinal(true, ft);
				return;
			}
		}
		else debugLogA("missing put node");
	}
	else debugLogA("wrong or not recognizable http slot received");
	goto LBL_Fail;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CJabberProto::FtTryInlineFile(filetransfer *ft)
{
	int fileFormat = ProtoGetAvatarFormat(ft->std.szCurrentFile.w);
	if (fileFormat == PA_FORMAT_UNKNOWN) {
		debugLogA("Unknown picture format");
		return false;
	}

	char szClientJid[JABBER_MAX_JID_LEN];
	if (!m_bJabberOnline || !GetClientJID(ft->std.hContact, szClientJid, _countof(szClientJid))) {
		debugLogA("Protocol is offline or no contact %d", ft->std.hContact);
		return false;
	}

	int fileId = _wopen(ft->std.szCurrentFile.w, _O_BINARY | _O_RDONLY);
	if (fileId < 0) {
		debugLogW(L"File %s cannot be opened for inlining", ft->std.szCurrentFile.w);
		return false;
	}

	SHA_CTX ctx;
	SHA1_Init(&ctx);
	while (!_eof(fileId)) {
		char buf[1024];
		int nBytes = _read(fileId, buf, _countof(buf));
		SHA1_Update(&ctx, buf, nBytes);
	}
	_close(fileId);

	uint8_t hash[MIR_SHA1_HASH_SIZE];
	SHA1_Final(hash, &ctx);
	char szHash[MIR_SHA1_HASH_SIZE*2 + 1];
	bin2hex(hash, sizeof(hash), szHash);

	VARSW wszTempPath(L"%miranda_userdata%\\JabberTmp");
	CreateDirectoryTreeW(wszTempPath);

	CMStringW wszFileName(FORMAT, L"%s\\%S%s", wszTempPath.get(), szHash, ProtoGetAvatarExtension(fileFormat));
	if (_waccess(wszFileName, 0))
		if (!CopyFileW(ft->std.szCurrentFile.w, wszFileName, FALSE)) {
			uint32_t dwError = GetLastError();
			debugLogW(L"File <%s> cannot be copied to <%s>: error %d", ft->std.szCurrentFile.w, wszFileName.c_str(), dwError);
			return false;
		}

	XmlNode m("message");

	if (ListGetItemPtr(LIST_CHATROOM, szClientJid) && strchr(szClientJid, '/') == nullptr)
		XmlAddAttr(m, "type", "groupchat");
	else
		XmlAddAttr(m, "type", "chat");
	XmlAddAttr(m, "to", szClientJid);
	XmlAddAttrID(m, SerialNext());

	auto *nHtml = m << XCHILDNS("html", JABBER_FEAT_XHTML);
	auto *nBody = nHtml << XCHILDNS("body", "http://www.w3.org/1999/xhtml");
	auto *nPara = nBody << XCHILD("p");
	nPara << XCHILD("img") << XATTR("src", CMStringA(FORMAT, "cid:sha1+%s@bob.xmpp.org", szHash));

	if (IsSendAck(ft->std.hContact)) {
		m << XCHILDNS("request", JABBER_FEAT_MESSAGE_RECEIPTS);
		m << XCHILDNS("markable", JABBER_FEAT_CHAT_MARKERS);
	}
	
	m_ThreadInfo->send(m);

	// emulate a message for us
	CMStringA szMsg(FORMAT, "[img]%s[/img]", T2Utf(wszFileName).get());

	PROTORECVEVENT recv = {};
	recv.flags = PREF_CREATEREAD | PREF_SENT;
	recv.szMessage = szMsg.GetBuffer();
	recv.timestamp = time(0);
	ProtoChainRecvMsg(ft->std.hContact, &recv);
	return true;
}

bool CJabberProto::FtHandleCidRequest(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	auto *pChild = pInfo->GetChildNode();
	if (pChild == nullptr)
		return true;

	const char *cid = XmlGetAttr(pChild, "cid");
	if (cid == nullptr) {
	LBL_Error:
		XmlNodeIq iq("error", pInfo);
		TiXmlElement *error = iq << XCHILD("error") << XATTRI("code", 400) << XATTR("type", "cancel");
		error << XCHILDNS("bad-request", "urn:ietf:params:xml:ns:xmpp-stanzas");
		m_ThreadInfo->send(iq);
		return true;
	}

	if (memcmp(cid, "sha1+", 5))
		goto LBL_Error;

	CMStringA szCid(cid);
	szCid.Delete(0, 5); szCid.Truncate(40);
	VARSW wszTempPath(L"%miranda_userdata%\\JabberTmp");
	CMStringW wszFileMask(FORMAT, L"%s\\%S.*", wszTempPath.get(), szCid.c_str());
	WIN32_FIND_DATAW data;
	HANDLE hFind = FindFirstFileW(wszFileMask, &data);
	if (hFind == nullptr)
		goto LBL_Error;
	
	FindClose(hFind);

	int fileFormat = ProtoGetAvatarFormat(data.cFileName);
	wszFileMask.Format(L"%s\\%s", wszTempPath.get(), data.cFileName);
	int fileId = _wopen(wszFileMask, _O_BINARY | _O_RDONLY);
	if (fileId < 0)
		goto LBL_Error;

	mir_ptr<uint8_t> buf((uint8_t *)mir_alloc(data.nFileSizeLow));
	_read(fileId, buf, data.nFileSizeLow);
	_close(fileId);

	XmlNodeIq iq("result", pInfo);
	auto *pData = iq << XCHILDNS("data", JABBER_FEAT_BITS) << XATTR("max-age", "2565000") << XATTR("type", ProtoGetAvatarMimeType(fileFormat)) << XATTR("cid", cid);
	pData->SetText(ptrA(mir_base64_encode(buf, data.nFileSizeLow)).get());
	m_ThreadInfo->send(iq);
	return true;
}
