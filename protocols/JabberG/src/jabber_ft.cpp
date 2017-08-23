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
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
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

void CJabberProto::FtInitiate(wchar_t* jid, filetransfer *ft)
{
	wchar_t *rs;
	wchar_t *filename, *p;
	int i;
	wchar_t sid[9];

	if (jid == nullptr || ft == nullptr || !m_bJabberOnline || (rs = ListGetBestClientResourceNamePtr(jid)) == nullptr) {
		if (ft) {
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			delete ft;
		}
		return;
	}
	ft->type = FT_SI;
	for (i = 0; i < 8; i++)
		sid[i] = (rand() % 10) + '0';
	sid[8] = '\0';
	if (ft->sid != nullptr) mir_free(ft->sid);
	ft->sid = mir_wstrdup(sid);
	filename = ft->std.ptszFiles[ft->std.currentFileNumber];
	if ((p = wcsrchr(filename, '\\')) != nullptr)
		filename = p + 1;

	wchar_t tszJid[512];
	mir_snwprintf(tszJid, L"%s/%s", jid, rs);

	XmlNodeIq iq(AddIQ(&CJabberProto::OnFtSiResult, JABBER_IQ_TYPE_SET, tszJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO, -1, ft));
	HXML si = iq << XCHILDNS(L"si", JABBER_FEAT_SI) << XATTR(L"id", sid)
		<< XATTR(L"mime-type", L"binary/octet-stream") << XATTR(L"profile", JABBER_FEAT_SI_FT);
	si << XCHILDNS(L"file", JABBER_FEAT_SI_FT) << XATTR(L"name", filename)
		<< XATTRI64(L"size", ft->fileSize[ft->std.currentFileNumber]) << XCHILD(L"desc", ft->szDescription);

	HXML field = si << XCHILDNS(L"feature", JABBER_FEAT_FEATURE_NEG)
		<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"form")
		<< XCHILD(L"field") << XATTR(L"var", L"stream-method") << XATTR(L"type", L"list-single");

	BOOL bDirect = m_options.BsDirect;
	BOOL bProxy = m_options.BsProxyManual;

	// bytestreams support?
	if (bDirect || bProxy)
		field << XCHILD(L"option") << XCHILD(L"value", JABBER_FEAT_BYTESTREAMS);

	field << XCHILD(L"option") << XCHILD(L"value", JABBER_FEAT_IBB);
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnFtSiResult(HXML iqNode, CJabberIqInfo *pInfo)
{
	HXML siNode, featureNode, xNode, fieldNode, valueNode;
	filetransfer *ft = (filetransfer *)pInfo->GetUserData();
	if (!ft) return;

	if ((pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) && pInfo->m_szFrom && pInfo->m_szTo) {
		if ((siNode = XmlGetChild(iqNode, "si")) != nullptr) {

			// fix for very smart clients, like gajim
			BOOL bDirect = m_options.BsDirect;
			BOOL bProxy = m_options.BsProxyManual;

			if ((featureNode = XmlGetChild(siNode, "feature")) != nullptr) {
				if ((xNode = XmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) != nullptr) {
					if ((fieldNode = XmlGetChildByTag(xNode, "field", "var", L"stream-method")) != nullptr) {
						if ((valueNode = XmlGetChild(fieldNode, "value")) != nullptr && XmlGetText(valueNode) != nullptr) {
							if ((bDirect || bProxy) && !mir_wstrcmp(XmlGetText(valueNode), JABBER_FEAT_BYTESTREAMS)) {
								// Start Bytestream session
								JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
								memset(jbt, 0, sizeof(JABBER_BYTE_TRANSFER));
								jbt->srcJID = mir_wstrdup(pInfo->m_szTo);
								jbt->dstJID = mir_wstrdup(pInfo->m_szFrom);
								jbt->sid = mir_wstrdup(ft->sid);
								jbt->pfnSend = &CJabberProto::FtSend;
								jbt->pfnFinal = &CJabberProto::FtSendFinal;
								jbt->ft = ft;
								ft->type = FT_BYTESTREAM;
								ft->jbt = jbt;
								ForkThread((MyThreadFunc)&CJabberProto::ByteSendThread, jbt);
							}
							else if (!mir_wstrcmp(XmlGetText(valueNode), JABBER_FEAT_IBB)) {
								JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)mir_alloc(sizeof(JABBER_IBB_TRANSFER));
								memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
								jibb->srcJID = mir_wstrdup(pInfo->m_szTo);
								jibb->dstJID = mir_wstrdup(pInfo->m_szFrom);
								jibb->sid = mir_wstrdup(ft->sid);
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
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft, 0);
		delete ft;
	}
}

BOOL CJabberProto::FtSend(HNETLIBCONN hConn, filetransfer *ft)
{
	struct _stati64 statbuf;
	int fd;
	char* buffer;
	int numRead;

	debugLogW(L"Sending [%s]", ft->std.ptszFiles[ft->std.currentFileNumber]);
	_wstat64(ft->std.ptszFiles[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size
	if ((fd = _wopen(ft->std.ptszFiles[ft->std.currentFileNumber], _O_BINARY | _O_RDONLY)) < 0) {
		debugLogW(L"File cannot be opened");
		return FALSE;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	if ((buffer = (char*)mir_alloc(2048)) != nullptr) {
		while ((numRead = _read(fd, buffer, 2048)) > 0) {
			if (Netlib_Send(hConn, buffer, numRead, 0) != numRead) {
				mir_free(buffer);
				_close(fd);
				return FALSE;
			}
			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		}
		mir_free(buffer);
	}
	_close(fd);
	return TRUE;
}

BOOL CJabberProto::FtIbbSend(int blocksize, filetransfer *ft)
{
	debugLogW(L"Sending [%s]", ft->std.ptszFiles[ft->std.currentFileNumber]);

	struct _stati64 statbuf;
	_wstat64(ft->std.ptszFiles[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size

	int fd = _wopen(ft->std.ptszFiles[ft->std.currentFileNumber], _O_BINARY | _O_RDONLY);
	if (fd < 0) {
		debugLogA("File cannot be opened");
		return FALSE;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	ptrA buffer((char*)mir_alloc(blocksize));
	if (buffer != nullptr) {
		int numRead;
		while ((numRead = _read(fd, buffer, blocksize)) > 0) {
			int iqId = SerialNext();
			XmlNode msg(L"message");
			XmlAddAttr(msg, L"to", ft->jibb->dstJID);
			msg << XATTRID(iqId);

			// let others send data too
			Sleep(2);

			char *encoded = mir_base64_encode((PBYTE)(char*)buffer, numRead);

			msg << XCHILD(L"data", _A2T(encoded)) << XATTR(L"xmlns", JABBER_FEAT_IBB)
				<< XATTR(L"sid", ft->jibb->sid) << XATTRI(L"seq", ft->jibb->wPacketId);

			HXML ampNode = msg << XCHILDNS(L"amp", JABBER_FEAT_AMP);
			ampNode << XCHILD(L"rule") << XATTR(L"condition", L"deliver-at")
				<< XATTR(L"value", L"stored") << XATTR(L"action", L"error");
			ampNode << XCHILD(L"rule") << XATTR(L"condition", L"match-resource")
				<< XATTR(L"value", L"exact") << XATTR(L"action", L"error");
			ft->jibb->wPacketId++;

			mir_free(encoded);

			if (ft->jibb->state == JIBB_ERROR || ft->jibb->bStreamClosed || m_ThreadInfo->send(msg) == SOCKET_ERROR) {
				debugLogA("JabberFtIbbSend unsuccessful exit");
				_close(fd);
				return FALSE;
			}

			ft->jibb->dwTransferredSize += (DWORD)numRead;

			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		}
	}
	_close(fd);
	return TRUE;
}

void CJabberProto::FtSendFinal(BOOL success, filetransfer *ft)
{
	if (!success) {
		debugLogA("File transfer complete with error");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ft->state == FT_DENIED ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft, 0);
	}
	else {
		if (ft->std.currentFileNumber < ft->std.totalFiles - 1) {
			ft->std.currentFileNumber++;
			replaceStrW(ft->std.tszCurrentFile, ft->std.ptszFiles[ft->std.currentFileNumber]);
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
			FtInitiate(ft->jid, ft);
			return;
		}

		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	}

	delete ft;
}

///////////////// File receiving through stream initiation /////////////////////////

void CJabberProto::FtHandleSiRequest(HXML iqNode)
{
	const wchar_t *from, *sid, *str, *szId, *filename;
	HXML siNode, fileNode, featureNode, xNode, fieldNode, n;
	int i;
	unsigned __int64 filesize;

	if (!iqNode ||
		(from = XmlGetAttrValue(iqNode, L"from")) == nullptr ||
		(str = XmlGetAttrValue(iqNode, L"type")) == nullptr || mir_wstrcmp(str, L"set") ||
		(siNode = XmlGetChildByTag(iqNode, "si", "xmlns", JABBER_FEAT_SI)) == nullptr)
		return;

	szId = XmlGetAttrValue(iqNode, L"id");
	if ((sid = XmlGetAttrValue(siNode, L"id")) != nullptr &&
		(fileNode = XmlGetChildByTag(siNode, "file", "xmlns", JABBER_FEAT_SI_FT)) != nullptr &&
		(filename = XmlGetAttrValue(fileNode, L"name")) != nullptr &&
		(str = XmlGetAttrValue(fileNode, L"size")) != nullptr) {

		filesize = _wtoi64(str);
		if ((featureNode = XmlGetChildByTag(siNode, "feature", "xmlns", JABBER_FEAT_FEATURE_NEG)) != nullptr &&
			(xNode = XmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) != nullptr &&
			(fieldNode = XmlGetChildByTag(xNode, "field", "var", L"stream-method")) != nullptr) {

			BOOL bIbbOnly = m_options.BsOnlyIBB;
			HXML optionNode = nullptr;
			JABBER_FT_TYPE ftType = FT_OOB;

			if (!bIbbOnly) {
				for (i = 0; ; i++) {
					optionNode = XmlGetChild(fieldNode, i);
					if (!optionNode)
						break;

					if (!mir_wstrcmp(XmlGetName(optionNode), L"option")) {
						if ((n = XmlGetChild(optionNode, "value")) != nullptr && XmlGetText(n)) {
							if (!mir_wstrcmp(XmlGetText(n), JABBER_FEAT_BYTESTREAMS)) {
								ftType = FT_BYTESTREAM;
								break;
							}
						}
					}
				}
			}

			// try IBB only if bytestreams support not found or BsOnlyIBB flag exists
			if (bIbbOnly || !optionNode) {
				for (i = 0; ; i++) {
					optionNode = XmlGetChild(fieldNode, i);
					if (!optionNode)
						break;

					if (!mir_wstrcmp(XmlGetName(optionNode), L"option")) {
						if ((n = XmlGetChild(optionNode, "value")) != nullptr && XmlGetText(n)) {
							if (!mir_wstrcmp(XmlGetText(n), JABBER_FEAT_IBB)) {
								ftType = FT_IBB;
								break;
							}
						}
					}
				}
			}

			if (optionNode != nullptr) {
				// Found known stream mechanism
				filetransfer *ft = new filetransfer(this);
				ft->dwExpectedRecvFileSize = filesize;
				ft->jid = mir_wstrdup(from);
				ft->std.hContact = HContactFromJID(from);
				ft->sid = mir_wstrdup(sid);
				ft->szId = mir_wstrdup(szId);
				ft->type = ftType;
				ft->std.totalFiles = 1;
				ft->std.tszCurrentFile = mir_wstrdup(filename);
				ft->std.totalBytes = ft->std.currentFileSize = filesize;

				PROTORECVFILET pre = { 0 };
				pre.dwFlags = PRFF_UNICODE;
				pre.fileCount = 1;
				pre.timestamp = time(nullptr);
				pre.files.w = (wchar_t**)&filename;
				pre.lParam = (LPARAM)ft;
				if ((n = XmlGetChild(fileNode, "desc")) != nullptr)
					pre.descr.w = (wchar_t*)XmlGetText(n);

				ProtoChainRecvFile(ft->std.hContact, &pre);
				return;
			}
			else {
				// Unknown stream mechanism
				XmlNodeIq iq(L"error", szId, from);
				HXML e = iq << XCHILD(L"error") << XATTRI(L"code", 400) << XATTR(L"type", L"cancel");
				e << XCHILDNS(L"bad-request", L"urn:ietf:params:xml:ns:xmpp-stanzas");
				e << XCHILDNS(L"no-valid-streams", JABBER_FEAT_SI);
				m_ThreadInfo->send(iq);
				return;
			}
		}
	}

	// Bad stream initiation, reply with bad-profile
	XmlNodeIq iq(L"error", szId, from);
	HXML e = iq << XCHILD(L"error") << XATTRI(L"code", 400) << XATTR(L"type", L"cancel");
	e << XCHILDNS(L"bad-request", L"urn:ietf:params:xml:ns:xmpp-stanzas");
	e << XCHILDNS(L"bad-profile", JABBER_FEAT_SI);
	m_ThreadInfo->send(iq);
}

void CJabberProto::FtAcceptSiRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == nullptr || ft->jid == nullptr || ft->sid == nullptr) return;

	JABBER_LIST_ITEM *item;
	if ((item = ListAdd(LIST_FTRECV, ft->sid)) != nullptr) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq(L"result", ft->szId, ft->jid)
			<< XCHILDNS(L"si", JABBER_FEAT_SI)
			<< XCHILDNS(L"feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"submit")
			<< XCHILD(L"field") << XATTR(L"var", L"stream-method")
			<< XCHILD(L"value", JABBER_FEAT_BYTESTREAMS));
	}
}

void CJabberProto::FtAcceptIbbRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == nullptr || ft->jid == nullptr || ft->sid == nullptr) return;

	JABBER_LIST_ITEM *item;
	if ((item = ListAdd(LIST_FTRECV, ft->sid)) != nullptr) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq(L"result", ft->szId, ft->jid)
			<< XCHILDNS(L"si", JABBER_FEAT_SI)
			<< XCHILDNS(L"feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"submit")
			<< XCHILD(L"field") << XATTR(L"var", L"stream-method")
			<< XCHILD(L"value", JABBER_FEAT_IBB));
	}
}

BOOL CJabberProto::FtHandleBytestreamRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	HXML queryNode = pInfo->GetChildNode();

	const wchar_t *sid;
	JABBER_LIST_ITEM *item;

	if ((sid = XmlGetAttrValue(queryNode, L"sid")) != nullptr && (item = ListGetItemPtr(LIST_FTRECV, sid)) != nullptr) {
		// Start Bytestream session
		JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
		memset(jbt, 0, sizeof(JABBER_BYTE_TRANSFER));
		jbt->iqNode = xmlCopyNode(iqNode);
		jbt->pfnRecv = &CJabberProto::FtReceive;
		jbt->pfnFinal = &CJabberProto::FtReceiveFinal;
		jbt->ft = item->ft;
		item->ft->jbt = jbt;
		ForkThread((MyThreadFunc)&CJabberProto::ByteReceiveThread, jbt);
		ListRemove(LIST_FTRECV, sid);
		return TRUE;
	}

	debugLogA("File transfer invalid bytestream initiation request received");
	return TRUE;
}

BOOL CJabberProto::FtHandleIbbRequest(HXML iqNode, BOOL bOpen)
{
	if (iqNode == nullptr) return FALSE;

	const wchar_t *id = XmlGetAttrValue(iqNode, L"id");
	const wchar_t *from = XmlGetAttrValue(iqNode, L"from");
	const wchar_t *to = XmlGetAttrValue(iqNode, L"to");
	if (!id || !from || !to) return FALSE;

	HXML ibbNode = XmlGetChildByTag(iqNode, bOpen ? "open" : "close", "xmlns", JABBER_FEAT_IBB);
	if (!ibbNode) return FALSE;

	const wchar_t *sid = XmlGetAttrValue(ibbNode, L"sid");
	if (!sid) return FALSE;

	// already closed?
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == nullptr) {
		m_ThreadInfo->send(
			XmlNodeIq(L"error", id, from)
			<< XCHILD(L"error") << XATTRI(L"code", 404) << XATTR(L"type", L"cancel")
			<< XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
		return FALSE;
	}

	// open event
	if (bOpen) {
		if (!item->jibb) {
			JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)mir_alloc(sizeof(JABBER_IBB_TRANSFER));
			memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
			jibb->srcJID = mir_wstrdup(from);
			jibb->dstJID = mir_wstrdup(to);
			jibb->sid = mir_wstrdup(sid);
			jibb->pfnRecv = &CJabberProto::FtReceive;
			jibb->pfnFinal = &CJabberProto::FtReceiveFinal;
			jibb->ft = item->ft;
			item->ft->jibb = jibb;
			item->jibb = jibb;
			ForkThread((MyThreadFunc)&CJabberProto::IbbReceiveThread, jibb);

			m_ThreadInfo->send(XmlNodeIq(L"result", id, from));
			return TRUE;
		}
		// stream already open
		m_ThreadInfo->send(
			XmlNodeIq(L"error", id, from)
			<< XCHILD(L"error") << XATTRI(L"code", 404) << XATTR(L"type", L"cancel")
			<< XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
		return FALSE;
	}

	// close event && stream already open
	if (item->jibb && item->jibb->hEvent) {
		item->jibb->bStreamClosed = TRUE;
		SetEvent(item->jibb->hEvent);

		m_ThreadInfo->send(XmlNodeIq(L"result", id, from));
		return TRUE;
	}

	ListRemove(LIST_FTRECV, sid);

	return FALSE;
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

void CJabberProto::FtReceiveFinal(BOOL success, filetransfer *ft)
{
	if (success) {
		debugLogA("File transfer complete successfully");
		ft->complete();
	}
	else debugLogA("File transfer complete with error");

	delete ft;
}
