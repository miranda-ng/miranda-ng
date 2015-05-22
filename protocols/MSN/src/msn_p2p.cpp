/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
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
#include "m_smileyadd.h"

static const char sttP2Pheader[] =
	"Content-Type: application/x-msnmsgrp2p\r\n"
	"P2P-Dest: %s\r\n\r\n";

static const char sttP2PheaderV2[] =
	"Content-Type: application/x-msnmsgrp2p\r\n"
	"P2P-Dest: %s\r\n"
	"P2P-Src: %s;%s\r\n\r\n";

const char sttVoidUid[] = "{00000000-0000-0000-0000-000000000000}";
#ifdef OBSOLETE
static const char szUbnCall[] = "{F13B5C79-0126-458F-A29D-747C79C56530}";

static const char p2pV2Caps[] = { 0x01, 0x0C, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x01, 0x00, 0x00 };

void P2P_Header::logHeader(CMsnProto *ppro)
{
	ppro->debugLogA("--- Printing message header");
	ppro->debugLogA("    SessionID = %08X", mSessionID);
	ppro->debugLogA("    MessageID = %08X", mID);
#ifndef __GNUC__
	ppro->debugLogA("    Offset of data = %I64u", mOffset);
	ppro->debugLogA("    Total amount of data = %I64u", mTotalSize);
#else
	ppro->debugLogA("    Offset of data = %llu", mOffset);
	ppro->debugLogA("    Total amount of data = %llu", hdrdata->mTotalSize);
#endif
	ppro->debugLogA("    Data in packet = %lu bytes", mPacketLen);
	ppro->debugLogA("    Flags = %08X", mFlags);
	ppro->debugLogA("    Acknowledged session ID: %08X", mAckSessionID);
	ppro->debugLogA("    Acknowledged message ID: %08X", mAckUniqueID);
#ifndef __GNUC__
	ppro->debugLogA("    Acknowledged data size: %I64u", mAckDataSize);
#else
	ppro->debugLogA("    Acknowledged data size: %llu", mAckDataSize);
#endif
	ppro->debugLogA("------------------------");
}

void P2PV2_Header::logHeader(CMsnProto *ppro)
{
	ppro->debugLogA("--- Printing message header");
	ppro->debugLogA("    SessionID = %08X", mSessionID);
	ppro->debugLogA("    MessageID = %08X", mID);
#ifndef __GNUC__
	ppro->debugLogA("    Remaining amount of data = %I64u", mRemSize);
#else
	ppro->debugLogA("    Remaining amount of data = %llu", mTotalSize);
#endif
	ppro->debugLogA("    Data in packet = %lu bytes", mPacketLen);
	ppro->debugLogA("    Packet Number = %lu", mPacketNum);
	ppro->debugLogA("    Operation Code = %08X", mOpCode);
	ppro->debugLogA("    TF Code = %08X", mTFCode);
	ppro->debugLogA("    Acknowledged message ID: %08X", mAckUniqueID);
	ppro->debugLogA("------------------------");
}

bool CMsnProto::p2p_createListener(filetransfer* ft, directconnection *dc, MimeHeaders& chdrs)
{
	if (MyConnection.extIP == 0) return false;

	NETLIBBIND nlb = { 0 };
	nlb.cbSize = sizeof(nlb);
	nlb.pfnNewConnectionV2 = MSN_ConnectionProc;
	nlb.pExtra = this;
	HANDLE sb = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hNetlibUser, (LPARAM)&nlb);
	if (sb == NULL) {
		debugLogA("Unable to bind the port for incoming transfers");
		return false;
	}

	ThreadData* newThread = new ThreadData;
	newThread->mType = SERVER_P2P_DIRECT;
	newThread->mCaller = 3;
	newThread->mIncomingBoundPort = sb;
	newThread->mIncomingPort = nlb.wPort;
	strncpy(newThread->mCookie, dc->callId, sizeof(newThread->mCookie));
	newThread->mInitialContactWLID = mir_strdup(ft->p2p_dest);

	newThread->startThread(&CMsnProto::p2p_filePassiveThread, this);

	char szIpv4[256] = "";
	char szIpv6[256] = "";
	const char *szExtIp = MyConnection.GetMyExtIPStr();

	bool ipInt = false;
	int i4 = 0, i6 = 0;

	NETLIBIPLIST* ihaddr = (NETLIBIPLIST*)CallService(MS_NETLIB_GETMYIP, 1, 0);
	for (unsigned i = 0; i < ihaddr->cbNum; ++i) {
		if (strchr(ihaddr->szIp[i], ':')) {
			if (i6++ != 0) strcat(szIpv6, " ");
			strcat(szIpv6, ihaddr->szIp[i]);
		}
		else {
			if (i4++ != 0) strcat(szIpv4, " ");
			ipInt |= (mir_strcmp(ihaddr->szIp[i], szExtIp) == 0);
			strcat(szIpv4, ihaddr->szIp[i]);
		}
	}
	mir_free(ihaddr);

	chdrs.addString("Bridge", "TCPv1");
	chdrs.addBool("Listening", true);

	if (dc->useHashedNonce)
		chdrs.addString("Hashed-Nonce", dc->mNonceToHash(), 2);
	else
		chdrs.addString("Nonce", dc->mNonceToText(), 2);

	bool bUbnCall = !ft->p2p_sessionid;

	if (!ipInt) {
		chdrs.addString("IPv4External-Addrs", mir_strdup(MyConnection.GetMyExtIPStr()), bUbnCall ? 6 : 2);
		chdrs.addLong("IPv4External-Port", nlb.wExPort, bUbnCall ? 4 : 0);
	}
	chdrs.addString("IPv4Internal-Addrs", mir_strdup(szIpv4), bUbnCall ? 6 : 2);
	chdrs.addLong("IPv4Internal-Port", nlb.wPort, bUbnCall ? 4 : 0);
	if (szIpv6[0]) {
		chdrs.addString("IPv6-Addrs", mir_strdup(szIpv6), 2);
		chdrs.addLong("IPv6-Port", nlb.wPort);
	}
	chdrs.addULong("SessionID", ft->p2p_sessionid);
	chdrs.addString("SChannelState", "0");
	chdrs.addString("Capabilities-Flags", "1");

	return true;
}

bool p2p_IsDlFileOk(filetransfer* ft)
{
	mir_sha1_ctx sha1ctx;
	BYTE sha[MIR_SHA1_HASH_SIZE];
	mir_sha1_init(&sha1ctx);

	bool res = false;

	int fileId = _topen(ft->std.tszCurrentFile, O_RDONLY | _O_BINARY, _S_IREAD);
	if (fileId != -1) {
		BYTE buf[4096];
		int bytes;

		while ((bytes = _read(fileId, buf, sizeof(buf))) > 0)
			mir_sha1_append(&sha1ctx, buf, bytes);

		_close(fileId);
		mir_sha1_finish(&sha1ctx, sha);

		char *szSha = arrayToHex(sha, MIR_SHA1_HASH_SIZE);
		char *szAvatarHash = MSN_GetAvatarHash(ft->p2p_object);

		res = szAvatarHash != NULL && _stricmp(szAvatarHash, szSha) == 0;

		mir_free(szSha);
		mir_free(szAvatarHash);
	}
	return res;
}



/////////////////////////////////////////////////////////////////////////////////////////
// sttSavePicture2disk - final handler for avatars downloading

void CMsnProto::p2p_pictureTransferFailed(filetransfer* ft)
{
	switch (ft->p2p_type) {
	case MSN_APPID_AVATAR:
	case MSN_APPID_AVATAR2:
	{
		PROTO_AVATAR_INFORMATIONT AI = { 0 };
		AI.cbSize = sizeof(AI);
		AI.hContact = ft->std.hContact;
		delSetting(ft->std.hContact, "AvatarHash");
		ProtoBroadcastAck(AI.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &AI, 0);
	}
	break;
	}
	_tremove(ft->std.tszCurrentFile);
}

void CMsnProto::p2p_savePicture2disk(filetransfer* ft)
{
	ft->close();

	if (p2p_IsDlFileOk(ft)) {
		int fileId = _topen(ft->std.tszCurrentFile, O_RDONLY | _O_BINARY, _S_IREAD);
		if (fileId == -1) {
			p2p_pictureTransferFailed(ft);
			return;
		}

		const TCHAR* ext;
		int format;
		BYTE buf[6];

		int bytes = _read(fileId, buf, sizeof(buf));
		_close(fileId);
		if (bytes > 4)
			format = ProtoGetBufferFormat(buf, &ext);
		else {
			p2p_pictureTransferFailed(ft);
			return;
		}

		switch (ft->p2p_type) {
		case MSN_APPID_AVATAR:
		case MSN_APPID_AVATAR2:
		{
			PROTO_AVATAR_INFORMATIONT AI = { 0 };
			AI.cbSize = sizeof(AI);
			AI.format = format;
			AI.hContact = ft->std.hContact;
			MSN_GetAvatarFileName(AI.hContact, AI.filename, SIZEOF(AI.filename), ext);

			_trename(ft->std.tszCurrentFile, AI.filename);

			// Store also avatar hash
			char *szAvatarHash = MSN_GetAvatarHash(ft->p2p_object);
			setString(ft->std.hContact, "AvatarSavedHash", szAvatarHash);
			mir_free(szAvatarHash);

			setString(ft->std.hContact, "PictSavedContext", ft->p2p_object);
			ProtoBroadcastAck(AI.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &AI, 0);

			debugLogA("Avatar for contact %08x saved to file '%s'", AI.hContact, T2Utf(AI.filename));
		}
		break;

		case MSN_APPID_CUSTOMSMILEY:
		case MSN_APPID_CUSTOMANIMATEDSMILEY:
		{
			SMADD_CONT cont;
			cont.cbSize = sizeof(SMADD_CONT);
			cont.hContact = ft->std.hContact;
			cont.type = 1;

			TCHAR* pathcpy = mir_tstrdup(ft->std.tszCurrentFile);
			_tcscpy(_tcsrchr(pathcpy, '.') + 1, ext);
			_trename(ft->std.tszCurrentFile, pathcpy);

			cont.path = pathcpy;

			CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, (LPARAM)&cont);
			mir_free(pathcpy);
		}
		break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendAck - sends MSN P2P acknowledgement to the received message

static const char sttVoidSession[] = "ACHTUNG!!! an attempt made to send a message via the empty session";

void CMsnProto::p2p_sendMsg(const char *wlid, unsigned appId, P2PB_Header& hdrdata, char* msgbody, size_t msgsz)
{
	ThreadData* info = MSN_GetP2PThreadByContact(wlid);
	if (info == NULL) {
		bool isOffline;
		info = MSN_StartSB(wlid, isOffline);
	}
	p2p_sendMsg(info, wlid, appId, hdrdata, msgbody, msgsz);
}

void CMsnProto::p2p_sendMsg(ThreadData* info, const char *wlid, unsigned appId, P2PB_Header& hdrdata, char* msgbody, size_t msgsz)
{
	unsigned msgType;

	if (info == NULL) msgType = 0;
	else if (info->mType == SERVER_P2P_DIRECT) msgType = 1;
	else msgType = 2;

	unsigned fportion = msgType == 1 ? 1352 : 1202;
	if (hdrdata.isV2Hdr()) fportion += 4;

	char* buf = (char*)alloca(sizeof(sttP2PheaderV2) + MSN_MAX_EMAIL_LEN +
		120 + fportion);

	size_t offset = 0;
	do {
		size_t portion = msgsz - offset;
		if (portion > fportion) portion = fportion;

		char* p = buf;

		// add message header
		p += msgType == 1 ? sizeof(unsigned) :
			sprintf(p, hdrdata.isV2Hdr() ? sttP2PheaderV2 : sttP2Pheader, wlid, MyOptions.szEmail, MyOptions.szMachineGuidP2P); //!!!!!!!!!!!

		if (hdrdata.isV2Hdr()) {
			P2PV2_Header *ph = (P2PV2_Header*)&hdrdata;
			if (offset == 0) {
				if (!info || !info->mBridgeInit) {
					if (info && ph->mSessionID) {
						P2PV2_Header tHdr;
						tHdr.mID = ph->mID;
						p2p_sendMsg(info, wlid, 0, tHdr, NULL, 0);
					}
					else {
						ph->mOpCode |= ph->mAckUniqueID && msgType != 1 ? 1 : 3;
						ph->mCap = p2pV2Caps;
						if (info) info->mBridgeInit = true;
					}
				}
			}
			else {
				ph->mOpCode = 0;
				ph->mCap = NULL;
			}
		}

		if (msgsz) {
			if (hdrdata.isV2Hdr()) {
				P2PV2_Header *ph = (P2PV2_Header*)&hdrdata;
				ph->mPacketLen = (unsigned)portion;
				ph->mRemSize = msgsz - offset - portion;
				ph->mTFCode = offset ? ph->mTFCode & 0xfe : ph->mTFCode | 0x01;

				if (offset == 0)
					ph->mPacketNum = p2p_getPktNum(wlid);
			}
			else {
				P2P_Header *ph = (P2P_Header*)&hdrdata;
				ph->mPacketLen = (unsigned)portion;
				ph->mOffset = offset;
				ph->mTotalSize = msgsz;
			}
		}

		// add message body
		p = hdrdata.createMsg(p, wlid, this);
		hdrdata.logHeader(this);

		if (msgsz)
			memcpy(p, msgbody + offset, portion); p += portion;

		// add message footer
		if (msgType != 1) {
			*(unsigned*)p = _htonl(appId);
			p += 4;
		}

		char* szEmail;
		switch (msgType) {
		case 0:
			parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);
			MsgQueue_Add(szEmail, 'D', buf, p - buf);
			break;

		case 1:
			*(unsigned*)buf = (unsigned)(p - buf - sizeof(unsigned));
			info->send(buf, p - buf);
			break;

		case 2:
			info->sendRawMessage('D', buf, p - buf);
			break;
		}
		offset += portion;
	} while (offset < msgsz);
}


void CMsnProto::p2p_sendAck(const char *wlid, P2PB_Header* hdr)
{
	if (hdr == NULL) return;

	if (!hdr->isV2Hdr()) {
		P2P_Header *hdrdata = (P2P_Header*)hdr;
		P2P_Header tHdr;

		tHdr.mSessionID = hdrdata->mSessionID;
		tHdr.mAckDataSize = hdrdata->mTotalSize;
		tHdr.mFlags = 2;
		tHdr.mAckSessionID = hdrdata->mID;
		tHdr.mAckUniqueID = hdrdata->mAckSessionID;

		p2p_sendMsg(wlid, 0, tHdr, NULL, 0);
	}
	else {
		P2PV2_Header *hdrdata = (P2PV2_Header*)hdr;
		P2PV2_Header tHdr;

		tHdr.mAckUniqueID = hdrdata->mID;

		p2p_sendMsg(wlid, 0, tHdr, NULL, 0);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendEndSession - sends MSN P2P file transfer end packet

void CMsnProto::p2p_sendAbortSession(filetransfer* ft)
{
	if (ft == NULL) {
		debugLogA(sttVoidSession);
		return;
	}

	if (ft->p2p_isV2) return;

	P2P_Header tHdr;

	tHdr.mSessionID = ft->p2p_sessionid;
	tHdr.mAckSessionID = ft->p2p_sendmsgid;
	tHdr.mID = p2p_getMsgId(ft->p2p_dest, 1);

	if (ft->std.flags & PFTS_SENDING) {
		tHdr.mFlags = 0x40;
		tHdr.mAckSessionID = tHdr.mID - 2;
	}
	else {
		tHdr.mAckUniqueID = 0x8200000f;
		tHdr.mFlags = 0x80;
		tHdr.mAckDataSize = ft->std.currentFileSize;
	}

	p2p_sendMsg(ft->p2p_dest, 0, tHdr, NULL, 0);
	ft->ts = time(NULL);
}

void CMsnProto::p2p_sendRedirect(filetransfer* ft)
{
	if (ft == NULL) {
		debugLogA(sttVoidSession);
		return;
	}

	if (ft->p2p_isV2) return;

	P2P_Header tHdr;

	tHdr.mSessionID = ft->p2p_sessionid;
	tHdr.mFlags = 0x01;
	tHdr.mAckSessionID = ft->p2p_sendmsgid;
	tHdr.mAckDataSize = ft->std.currentFileProgress;

	p2p_sendMsg(ft->p2p_dest, 0, tHdr, NULL, 0);

	ft->tTypeReq = MSN_GetP2PThreadByContact(ft->p2p_dest) ? SERVER_P2P_DIRECT : SERVER_SWITCHBOARD;
	ft->ts = time(NULL);
	ft->p2p_waitack = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendSlp - send MSN P2P SLP packet

void CMsnProto::p2p_sendSlp(int iKind, filetransfer *ft, MimeHeaders &pHeaders,
	MimeHeaders &pContent, const char *wlid)
{
	if (ft == NULL) {
		debugLogA(sttVoidSession);
		return;
	}

	if (wlid == NULL) wlid = ft->p2p_dest;

	size_t cbContLen = pContent.getLength();
	pHeaders.addULong("Content-Length", (unsigned)cbContLen + 1);

	char* buf = (char*)alloca(pHeaders.getLength() + cbContLen + 512);
	char* p = buf;

	switch (iKind) {
	case -3:   p += sprintf(p, "ACK MSNMSGR:%s MSNSLP/1.0", wlid); break; //!!!!!!!!!!!!!!!!!!
	case -2:   p += sprintf(p, "INVITE MSNMSGR:%s MSNSLP/1.0", wlid); break; //!!!!!!!!!!!!!!!!!!
	case -1:   p += sprintf(p, "BYE MSNMSGR:%s MSNSLP/1.0", wlid); break; //!!!!!!!!!!!!!!!!!!
	case 200:  p += sprintf(p, "MSNSLP/1.0 200 OK");	break; //!!!!!!!!!!!!!!!!!!
	case 481:  p += sprintf(p, "MSNSLP/1.0 481 No Such Call"); break; //!!!!!!!!!!!!!!!!!!
	case 500:  p += sprintf(p, "MSNSLP/1.0 500 Internal Error"); break; //!!!!!!!!!!!!!!!!!!
	case 603:  p += sprintf(p, "MSNSLP/1.0 603 DECLINE"); break; //!!!!!!!!!!!!!!!!!!
	case 1603: p += sprintf(p, "MSNSLP/1.0 603 Decline"); break; //!!!!!!!!!!!!!!!!!!
	default: return;
	}

	if (iKind < 0) {
		mir_free(ft->p2p_branch);
		ft->p2p_branch = getNewUuid();
	}

	if (ft->p2p_isV2) {
		p += sprintf(p,
			"\r\nTo: <msnmsgr:%s>\r\n"
			"From: <msnmsgr:%s;%s>\r\n"
			"Via: MSNSLP/1.0/TLP ;branch=%s\r\n",
			wlid, MyOptions.szEmail, MyOptions.szMachineGuidP2P, ft->p2p_branch); //!!!!!!!!!!!!!!!!!!
	}
	else {
		p += sprintf(p,
			"\r\nTo: <msnmsgr:%s>\r\n"
			"From: <msnmsgr:%s>\r\n"
			"Via: MSNSLP/1.0/TLP ;branch=%s\r\n",
			wlid, MyOptions.szEmail, ft->p2p_branch); //!!!!!!!!!!!!!!!!!!
	}

	p = pHeaders.writeToBuffer(p);
	p = pContent.writeToBuffer(p);

	unsigned short status = getWord(ft->std.hContact, "Status", ID_STATUS_OFFLINE);
	if (!(myFlags & cap_SupportsP2PBootstrap) || ft->p2p_sessionid ||
		MSN_GetThreadByContact(wlid, SERVER_P2P_DIRECT) ||
		status == ID_STATUS_OFFLINE || status == ID_STATUS_INVISIBLE ||
		m_iStatus == ID_STATUS_INVISIBLE) {
		if (!ft->p2p_isV2) {
			P2P_Header tHdr;
			tHdr.mAckSessionID = ft->p2p_acksessid;

			p2p_sendMsg(wlid, 0, tHdr, buf, p - buf + 1);
			ft->p2p_waitack = true;

			switch (iKind) {
			case -1: case 500: case 603:
				ft->p2p_byemsgid = tHdr.mID;
				break;
			}

		}
		else {
			P2PV2_Header tHdr;
			tHdr.mTFCode = 0x01;

			p2p_sendMsg(wlid, 0, tHdr, buf, p - buf + 1);
		}
	}
	else
		msnNsThread->sendPacket("UUN", "%s 3 %d\r\n%s", wlid, p - buf, buf);

	ft->ts = time(NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendBye - closes P2P session

void CMsnProto::p2p_sendBye(filetransfer* ft)
{
	if (ft == NULL) {
		debugLogA(sttVoidSession);
		return;
	}

	MimeHeaders tHeaders(8);
	tHeaders.addString("CSeq", "0 ");
	tHeaders.addString("Call-ID", ft->p2p_callID);
	tHeaders.addLong("Max-Forwards", 0);
	tHeaders.addString("Content-Type", "application/x-msnmsgr-sessionclosebody");

	MimeHeaders chdrs(2);
	chdrs.addULong("SessionID", ft->p2p_sessionid);
	chdrs.addString("SChannelState", "0");

	p2p_sendSlp(-1, ft, tHeaders, chdrs);
}

void CMsnProto::p2p_sendCancel(filetransfer* ft)
{
	p2p_sendBye(ft);
	p2p_sendAbortSession(ft);
}

void CMsnProto::p2p_sendNoCall(filetransfer* ft)
{
	if (ft == NULL) {
		debugLogA(sttVoidSession);
		return;
	}

	MimeHeaders tHeaders(8);
	tHeaders.addString("CSeq", "0 ");
	tHeaders.addString("Call-ID", ft->p2p_callID);
	tHeaders.addLong("Max-Forwards", 0);
	tHeaders.addString("Content-Type", "application/x-msnmsgr-session-failure-respbody");

	MimeHeaders chdrs(2);
	chdrs.addULong("SessionID", ft->p2p_sessionid);
	chdrs.addString("SChannelState", "0");

	p2p_sendSlp(481, ft, tHeaders, chdrs);
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendStatus - send MSN P2P status and its description

void CMsnProto::p2p_sendStatus(filetransfer* ft, long lStatus)
{
	if (ft == NULL) {
		debugLogA(sttVoidSession);
		return;
	}

	MimeHeaders tHeaders(8);
	tHeaders.addString("CSeq", "1 ");
	tHeaders.addString("Call-ID", ft->p2p_callID);
	tHeaders.addLong("Max-Forwards", 0);

	MimeHeaders chdrs(2);
	chdrs.addULong("SessionID", ft->p2p_sessionid);

	if (lStatus != 1603) {
		tHeaders.addString("Content-Type", "application/x-msnmsgr-sessionreqbody");

		chdrs.addString("SChannelState", "0");
	}
	else
		tHeaders.addString("Content-Type", "application/x-msnmsgr-transrespbody");

	p2p_sendSlp(lStatus, ft, tHeaders, chdrs);
}

void CMsnProto::p2p_sendAvatarInit(filetransfer* ft)
{
	unsigned body = 0;

	if (ft->p2p_isV2) {
		P2PV2_Header tHdr;
		tHdr.mSessionID = ft->p2p_sessionid;
		tHdr.mTFCode = 0x01;
		p2p_sendMsg(ft->p2p_dest, ft->p2p_appID, tHdr, (char*)&body, sizeof(body));
	}
	else {
		P2P_Header tHdr;
		tHdr.mSessionID = ft->p2p_sessionid;
		tHdr.mAckSessionID = ft->p2p_acksessid;
		p2p_sendMsg(ft->p2p_dest, ft->p2p_appID, tHdr, (char*)&body, sizeof(body));

		ft->ts = time(NULL);
		ft->p2p_waitack = true;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// p2p_connectTo - connects to a remote P2P server

static const char p2p_greeting[8] = { 4, 0, 0, 0, 'f', 'o', 'o', 0 };

static void sttSendPacket(ThreadData* T, void* hdr, unsigned len)
{
	T->send((char*)&len, sizeof(unsigned));
	T->send((char*)hdr, len);
}

bool CMsnProto::p2p_connectTo(ThreadData* info, directconnection *dc)
{
	NETLIBOPENCONNECTION tConn = { 0 };
	tConn.cbSize = sizeof(tConn);
	tConn.szHost = info->mServer;
	tConn.flags = NLOCF_V2;
	tConn.timeout = 5;

	char* tPortDelim = strrchr(info->mServer, ':');
	if (tPortDelim != NULL) {
		*tPortDelim = '\0';
		tConn.wPort = (WORD)atol(tPortDelim + 1);
	}

	debugLogA("Connecting to %s:%d", tConn.szHost, tConn.wPort);

	info->s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&tConn);
	if (info->s == NULL) {
		TWinErrorCode err;
		debugLogA("Connection Failed (%d): %s", err.mErrorCode, err.getText());
		return false;
	}
	info->send(p2p_greeting, sizeof(p2p_greeting));

	bool isV2 = strchr(info->mInitialContactWLID, ';') != NULL;

	P2P_Header reply;
	if (!isV2) {
		reply.mFlags = 0x100;

		if (dc->useHashedNonce)
			memcpy(&reply.mAckSessionID, dc->mNonce, sizeof(UUID));
		else
			dc->xNonceToBin((UUID*)&reply.mAckSessionID);

		char buf[48];
		reply.createMsg(buf, info->mInitialContactWLID, this);
		sttSendPacket(info, buf, sizeof(buf));
	}
	else
		sttSendPacket(info, dc->mNonce, sizeof(UUID));

	long cbPacketLen;
	HReadBuffer buf(info, 0);
	BYTE* p;
	if ((p = buf.surelyRead(4)) == NULL) {
		debugLogA("Error reading data, closing filetransfer");
		return false;
	}

	cbPacketLen = *(long*)p;
	if ((p = buf.surelyRead(cbPacketLen)) == NULL)
		return false;

	bool cookieMatch;

	if (!isV2) {
		P2P_Header cookie((char*)p);

		if (dc->useHashedNonce) {
			char* hnonce = dc->calcHashedNonce((UUID*)&cookie.mAckSessionID);
			cookieMatch = mir_strcmp(hnonce, dc->xNonce) == 0;
			mir_free(hnonce);
		}
		else
			cookieMatch = memcmp(&cookie.mAckSessionID, &reply.mAckSessionID, sizeof(UUID)) == 0;
	}
	else {
		char* hnonce = dc->calcHashedNonce((UUID*)p);
		cookieMatch = mir_strcmp(hnonce, dc->xNonce) == 0;
		mir_free(hnonce);
	}

	if (!cookieMatch) {
		debugLogA("Invalid cookie received, exiting");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_listen - acts like a local P2P server

bool CMsnProto::p2p_listen(ThreadData* info, directconnection *dc)
{
	switch (WaitForSingleObject(info->hWaitEvent, 10000)) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		debugLogA("Incoming connection timed out, closing file transfer");
		MSN_StartP2PTransferByContact(info->mInitialContactWLID);
LBL_Error:
		debugLogA("File listen failed");
		return false;
	}

	HReadBuffer buf(info, 0);
	BYTE* p;

	if ((p = buf.surelyRead(8)) == NULL)
		goto LBL_Error;

	if (memcmp(p, p2p_greeting, 8) != 0) {
		debugLogA("Invalid input data, exiting");
		return false;
	}

	if ((p = buf.surelyRead(4)) == NULL) {
		debugLogA("Error reading data, closing filetransfer");
		return false;
	}

	long cbPacketLen = *(long*)p;
	if ((p = buf.surelyRead(cbPacketLen)) == NULL)
		goto LBL_Error;

	bool cookieMatch;
	bool isV2 = strchr(info->mInitialContactWLID, ';') != NULL;

	if (!isV2) {
		P2P_Header cookie((char*)p);

		if (dc->useHashedNonce) {
			char* hnonce = dc->calcHashedNonce((UUID*)&cookie.mAckSessionID);
			cookieMatch = mir_strcmp(hnonce, dc->xNonce) == 0;
			mir_free(hnonce);
			memcpy(&cookie.mAckSessionID, dc->mNonce, sizeof(UUID));
		}
		else
			cookieMatch = memcmp(&cookie.mAckSessionID, dc->mNonce, sizeof(UUID)) == 0;

		if (!cookieMatch) {
			debugLogA("Invalid cookie received, exiting");
			return false;
		}

		char buf[48];
		cookie.createMsg(buf, info->mInitialContactWLID, this);
		sttSendPacket(info, buf, sizeof(buf));
	}
	else {
		char* hnonce = dc->calcHashedNonce((UUID*)p);
		cookieMatch = mir_strcmp(hnonce, dc->xNonce) == 0;
		mir_free(hnonce);

		if (!cookieMatch) {
			debugLogA("Invalid cookie received, exiting");
			goto LBL_Error;
		}

		sttSendPacket(info, dc->mNonce, sizeof(UUID));
	}

	return true;
}

LONG CMsnProto::p2p_sendPortion(filetransfer* ft, ThreadData* T, bool isV2)
{
	LONG trid;
	char databuf[1500], *p = databuf;

	// Compute the amount of data to send
	unsigned fportion = T->mType == SERVER_P2P_DIRECT ? 1352 : 1202;
	if (isV2) fportion += 4;

	const unsigned __int64 dt = ft->std.currentFileSize - ft->std.currentFileProgress;
	const unsigned portion = dt > fportion ? fportion : dt;

	// Fill data size for direct transfer

	if (T->mType != SERVER_P2P_DIRECT)
		p += sprintf(p, isV2 ? sttP2PheaderV2 : sttP2Pheader, ft->p2p_dest, MyOptions.szEmail, MyOptions.szMachineGuidP2P); //!!!!!!!!!!!!!!!!!!
	else
		p += sizeof(unsigned);

	if (!isV2) {
		// Fill P2P header
		P2P_Header H;

		H.mSessionID = ft->p2p_sessionid;
		H.mID = ft->p2p_sendmsgid;
		H.mFlags = ft->p2p_appID == MSN_APPID_FILE ? 0x01000030 : 0x20;
		H.mTotalSize = ft->std.currentFileSize;
		H.mOffset = ft->std.currentFileProgress;
		H.mPacketLen = portion;
		H.mAckSessionID = ft->p2p_acksessid;

		p = H.createMsg(p, ft->p2p_dest, this);
	}
	else {
		P2PV2_Header H;

		H.mSessionID = ft->p2p_sessionid;
		H.mTFCode = (ft->p2p_appID == MSN_APPID_FILE ? 6 : 4) | (ft->std.currentFileProgress ? 0 : 1);
		H.mRemSize = ft->std.currentFileSize - ft->std.currentFileProgress - portion;
		H.mPacketLen = portion;
		H.mPacketNum = ft->p2p_sendmsgid;

		p = H.createMsg(p, ft->p2p_dest, this);
		H.logHeader(this);
	}

	if (T->mType == SERVER_P2P_DIRECT)
		*(unsigned*)databuf = portion + (p - databuf) - (unsigned)sizeof(unsigned);

	// Fill data (payload) for transfer
	if (ft->fileId == -1) return 0;
	_read(ft->fileId, p, portion);
	p += portion;

	if (T->mType == SERVER_P2P_DIRECT)
		trid = T->send(databuf, p - databuf);
	else {
		// Define packet footer for server transfer
		*(unsigned*)p = _htonl(ft->p2p_appID);
		p += sizeof(unsigned);

		trid = T->sendRawMessage('D', (char *)databuf, p - databuf);
	}

	if (trid != 0) {
		ft->std.totalProgress += portion;
		ft->std.currentFileProgress += portion;
		if (ft->p2p_appID == MSN_APPID_FILE && clock() >= ft->nNotify) {
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
			ft->nNotify = clock() + 500;
		}
	}
	else
		debugLogA(" Error sending");
	ft->ts = time(NULL);
	ft->p2p_waitack = true;

	return trid;
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendFeedThread - sends a file via server

void __cdecl CMsnProto::p2p_sendFeedThread(void* arg)
{
	ThreadData* info = (ThreadData*)arg;

	bool isV2 = strchr(info->mInitialContactWLID, ';') != NULL;

	info->contactJoined(info->mInitialContactWLID);
	mir_free(info->mInitialContactWLID); info->mInitialContactWLID = NULL;

	debugLogA("File send thread started");

	switch (WaitForSingleObject(info->hWaitEvent, 6000)) {
	case WAIT_FAILED:
		debugLogA("File send wait failed");
		return;
	}

	HANDLE hLockHandle = NULL;
	ThreadData* T = NULL;
	TInfoType lastType = SERVER_NOTIFICATION;

	filetransfer *ft = p2p_getSessionByCallID(info->mCookie,
		info->mJoinedIdentContactsWLID.getCount() ? info->mJoinedIdentContactsWLID[0] : info->mJoinedContactsWLID[0]);

	if (ft != NULL && WaitForSingleObject(ft->hLockHandle, 2000) == WAIT_OBJECT_0) {
		hLockHandle = ft->hLockHandle;

		if (isV2)
			ft->p2p_sendmsgid = p2p_getPktNum(ft->p2p_dest);
		else {
			if (ft->p2p_sendmsgid == 0)
				ft->p2p_sendmsgid = p2p_getMsgId(ft->p2p_dest, 1);
		}

		T = MSN_GetP2PThreadByContact(ft->p2p_dest);
		if (T != NULL)
			ft->tType = lastType = T->mType;

		ReleaseMutex(hLockHandle);
	}
	else
		return;

	bool fault = false;
	while (WaitForSingleObject(hLockHandle, 2000) == WAIT_OBJECT_0 &&
		ft->std.currentFileProgress < ft->std.currentFileSize && !ft->bCanceled) {
		if (ft->tType != lastType)
			T = MSN_GetThreadByContact(ft->p2p_dest, ft->tType);

		if (ft->bCanceled) break;
		bool cfault = (T == NULL || p2p_sendPortion(ft, T, isV2) == 0);

		if (cfault) {
			if (fault) {
				debugLogA("File send failed");
				break;
			}
			else
				SleepEx(3000, TRUE);  // Allow 3 sec for redirect request
		}
		fault = cfault;

		ReleaseMutex(hLockHandle);

		if (T != NULL && T->mType != SERVER_P2P_DIRECT)
			WaitForSingleObject(T->hWaitEvent, 5000);
	}
	ReleaseMutex(hLockHandle);

	if (ft->p2p_appID == MSN_APPID_FILE)
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);

	if (isV2) {
		if (!ft->bCanceled) {
			ft->bCompleted = true;
			p2p_sendBye(ft);
		}
		p2p_sessionComplete(ft);
	}

	debugLogA("File send thread completed");
}


void CMsnProto::p2p_sendFeedStart(filetransfer* ft)
{
	if (ft->std.flags & PFTS_SENDING) {
		ThreadData* newThread = new ThreadData;
		newThread->mType = SERVER_FILETRANS;
		strcpy(newThread->mCookie, ft->p2p_callID);
		newThread->mInitialContactWLID = mir_strdup(ft->p2p_dest);
		newThread->startThread(&CMsnProto::p2p_sendFeedThread, this);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_sendFileDirectly - sends a file via MSN P2P protocol

void CMsnProto::p2p_sendRecvFileDirectly(ThreadData* info)
{
	long cbPacketLen = 0;
	long state = 0;

	HReadBuffer buf(info, 0);
	char *wlid = info->mInitialContactWLID;

	info->contactJoined(wlid);
	info->mInitialContactWLID = NULL;

	MSN_StartP2PTransferByContact(wlid);
	p2p_redirectSessions(wlid);
	p2p_startSessions(wlid);

	bool isV2 = strchr(wlid, ';') != NULL;

	for (;;) {
		long len = state ? cbPacketLen : 4;

		BYTE* p = buf.surelyRead(len);

		if (p == NULL)
			break;

		if (state == 0)
			cbPacketLen = *(long*)p;
		else if (!isV2)
			p2p_processMsg(info, (char*)p, wlid);
		else
			p2p_processMsgV2(info, (char*)p, wlid);

		state = (state + 1) % 2;
	}

	info->contactLeft(wlid);
	p2p_redirectSessions(wlid);
	mir_free(wlid);
}

/////////////////////////////////////////////////////////////////////////////////////////
// bunch of thread functions to cover all variants of P2P file transfers

void __cdecl CMsnProto::p2p_fileActiveThread(void* arg)
{
	ThreadData* info = (ThreadData*)arg;

	debugLogA("p2p_fileActiveThread() started: connecting to '%s'", info->mServer);

	directconnection *dc = p2p_getDCByCallID(info->mCookie, info->mInitialContactWLID);
	if (dc) {
		if (p2p_connectTo(info, dc))
			p2p_sendRecvFileDirectly(info);
		else {
			mir_free(info->mInitialContactWLID);
			info->mInitialContactWLID = NULL;
		}

		if (!MSN_GetThreadByContact(dc->wlid, SERVER_P2P_DIRECT) && !MSN_GetUnconnectedThread(dc->wlid, SERVER_P2P_DIRECT))
			p2p_unregisterDC(dc);
	}

	debugLogA("p2p_fileActiveThread() completed: connecting to '%s'", info->mServer);
}

void __cdecl CMsnProto::p2p_filePassiveThread(void* arg)
{
	ThreadData* info = (ThreadData*)arg;

	debugLogA("p2p_filePassiveThread() started: listening");

	directconnection *dc = p2p_getDCByCallID(info->mCookie, info->mInitialContactWLID);
	if (dc) {
		if (p2p_listen(info, dc))
			p2p_sendRecvFileDirectly(info);
		else {
			mir_free(info->mInitialContactWLID); info->mInitialContactWLID = NULL;
		}

		if (!MSN_GetThreadByContact(dc->wlid, SERVER_P2P_DIRECT) && !MSN_GetUnconnectedThread(dc->wlid, SERVER_P2P_DIRECT))
			p2p_unregisterDC(dc);
	}

	debugLogA("p2p_filePassiveThread() completed");
}


void CMsnProto::p2p_InitFileTransfer(
	ThreadData*		info,
	MimeHeaders&	tFileInfo,
	MimeHeaders&	tFileInfo2,
	const char* wlid)
{
	if (info->mJoinedContactsWLID.getCount() == 0 && info->mJoinedIdentContactsWLID.getCount() == 0)
		return;

	const char	*szCallID = tFileInfo["Call-ID"],
		*szBranch = tFileInfo["Via"];

	if (szBranch != NULL) {
		szBranch = strstr(szBranch, "branch=");
		if (szBranch != NULL)
			szBranch += 7;
	}
	if (szCallID == NULL || szBranch == NULL) {
		debugLogA("Ignoring invalid invitation: CallID='%s', szBranch='%s'", szCallID, szBranch);
		return;
	}

	const char	*szSessionID = tFileInfo2["SessionID"],
		*szEufGuid = tFileInfo2["EUF-GUID"],
		*szContext = tFileInfo2["Context"],
		*szAppId = tFileInfo2["AppID"];

	if (szSessionID == NULL || szAppId == NULL || szEufGuid == NULL) {
		debugLogA("Ignoring invalid invitation: SessionID='%s', AppID=%s, Branch='%s',Context='%s'",
			szSessionID, szAppId, szEufGuid, szContext);
		return;
	}

	unsigned dwAppID = strtoul(szAppId, NULL, 10);
	unsigned dwSessionId = strtoul(szSessionID, NULL, 10);

	if (p2p_getSessionByID(dwSessionId))
		return;

	szContext = (char*)mir_base64_decode(szContext, 0);

	filetransfer* ft = new filetransfer(this);
	ft->p2p_acksessid = MSN_GenRandom();
	ft->p2p_sessionid = dwSessionId;
	ft->p2p_appID = dwAppID == MSN_APPID_AVATAR ? MSN_APPID_AVATAR2 : dwAppID;
	ft->p2p_type = dwAppID;
	ft->p2p_ackID = dwAppID == MSN_APPID_FILE ? 2000 : 1000;
	replaceStr(ft->p2p_callID, szCallID);
	replaceStr(ft->p2p_branch, szBranch);
	ft->p2p_dest = mir_strdup(wlid);
	ft->p2p_isV2 = strchr(wlid, ';') != NULL;
	ft->std.hContact = MSN_HContactFromEmail(wlid);

	p2p_registerSession(ft);

	switch (dwAppID) {
	case MSN_APPID_AVATAR:
	case MSN_APPID_AVATAR2:
		if (!_stricmp(szEufGuid, "{A4268EEC-FEC5-49E5-95C3-F126696BDBF6}")) {
			DBVARIANT dbv;
			bool pictmatch = !getString("PictObject", &dbv);
			if (pictmatch) {
				UrlDecode(dbv.pszVal);

				ezxml_t xmlcon = ezxml_parse_str((char*)szContext, mir_strlen(szContext));
				ezxml_t xmldb = ezxml_parse_str(dbv.pszVal, mir_strlen(dbv.pszVal));

				const char *szCtBuf = ezxml_attr(xmlcon, "SHA1C");
				if (szCtBuf) {
					const char *szPtBuf = ezxml_attr(xmldb, "SHA1C");
					pictmatch = szPtBuf && mir_strcmp(szCtBuf, szPtBuf) == 0;
				}
				else {
					const char *szCtBuf = ezxml_attr(xmlcon, "SHA1D");
					const char *szPtBuf = ezxml_attr(xmldb, "SHA1D");
					pictmatch = szCtBuf && szPtBuf && mir_strcmp(szCtBuf, szPtBuf) == 0;
				}

				ezxml_free(xmlcon);
				ezxml_free(xmldb);
				db_free(&dbv);
			}
			if (pictmatch) {
				TCHAR szFileName[MAX_PATH];
				MSN_GetAvatarFileName(NULL, szFileName, SIZEOF(szFileName), NULL);
				ft->fileId = _topen(szFileName, O_RDONLY | _O_BINARY, _S_IREAD);
				if (ft->fileId == -1) {
					p2p_sendStatus(ft, 603);
					MSN_ShowError("Your avatar not set correctly. Avatar should be set in View/Change My Details | Avatar");
					debugLogA("Unable to open avatar file '%s', error %d", szFileName, errno);
					p2p_unregisterSession(ft);
				}
				else {
					mir_free(ft->std.tszCurrentFile);
					ft->std.tszCurrentFile = mir_tstrdup(szFileName);
					//						debugLogA("My avatar file opened for %s as %08p::%d", szEmail, ft, ft->fileId);
					ft->std.totalBytes = ft->std.currentFileSize = _filelengthi64(ft->fileId);
					ft->std.flags |= PFTS_SENDING;

					//---- send 200 OK Message
					p2p_sendStatus(ft, 200);
					p2p_sendFeedStart(ft);

					if (ft->p2p_isV2) {
						p2p_sendAvatarInit(ft);
						MSN_StartP2PTransferByContact(ft->p2p_dest);
					}
				}
			}
			else {
				p2p_sendStatus(ft, 603);
				debugLogA("Requested avatar does not match current avatar");
				p2p_unregisterSession(ft);
			}
		}
		break;

	case MSN_APPID_FILE:
		if (!_stricmp(szEufGuid, "{5D3E02AB-6190-11D3-BBBB-00C04F795683}")) {
			wchar_t* wszFileName = ((HFileContext*)szContext)->wszFileName;
			for (wchar_t* p = wszFileName; *p != 0; p++) {
				switch (*p) {
				case ':': case '?': case '/': case '\\': case '*':
					*p = '_';
				}
			}

			mir_free(ft->std.tszCurrentFile);
			ft->std.tszCurrentFile = mir_u2t(wszFileName);

			ft->std.totalBytes = ft->std.currentFileSize = ((HFileContext*)szContext)->dwSize;
			ft->std.totalFiles = 1;

			TCHAR tComment[40];
			mir_sntprintf(tComment, SIZEOF(tComment), TranslateT("%I64u bytes"), ft->std.currentFileSize);

			PROTORECVFILET pre = { 0 };
			pre.dwFlags = PRFF_TCHAR;
			pre.fileCount = 1;
			pre.timestamp = time(NULL);
			pre.tszDescription = tComment;
			pre.ptszFiles = &ft->std.tszCurrentFile;
			pre.lParam = (LPARAM)ft;
			ProtoChainRecvFile(ft->std.hContact, &pre);
		}
		break;

	case MSN_APPID_WEBCAM:
		if (!_stricmp(szEufGuid, "{4BD96FC0-AB17-4425-A14A-439185962DC8}")) {
			MSN_ShowPopup(ft->std.hContact,
				TranslateT("Contact tried to send its webcam data (not currently supported)"),
				MSN_ALLOW_MSGBOX | MSN_SHOW_ERROR);
		}
		if (!_stricmp(szEufGuid, "{1C9AA97E-9C05-4583-A3BD-908A196F1E92}")) {
			MSN_ShowPopup(ft->std.hContact,
				TranslateT("Contact tried to view your webcam data (not currently supported)"),
				MSN_ALLOW_MSGBOX | MSN_SHOW_ERROR);
		}
		p2p_sendStatus(ft, 603);
		p2p_unregisterSession(ft);
		break;

	case MSN_APPID_MEDIA_SHARING:
		//		MSN_ShowPopup(ft->std.hContact,
		//			TranslateT("Contact tried to share media with us (not currently supported)"),
		//			MSN_ALLOW_MSGBOX | MSN_SHOW_ERROR);
		p2p_sendStatus(ft, 603);
		p2p_unregisterSession(ft);
		break;

	default:
		p2p_sendStatus(ft, 603);
		p2p_unregisterSession(ft);
		debugLogA("Invalid or unknown data transfer request (AppID/EUF-GUID: %ld/%s)", dwAppID, szEufGuid);
		break;
	}

	mir_free((void*)szContext);
}

void CMsnProto::p2p_InitDirectTransfer(MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid)
{
	const char	*szCallID = tFileInfo["Call-ID"],
		*szBranch = tFileInfo["Via"],
		*szConnType = tFileInfo2["Conn-Type"],
		*szUPnPNat = tFileInfo2["UPnPNat"],
		*szNetID = tFileInfo2["NetID"],
		*szICF = tFileInfo2["ICF"],
		*szHashedNonce = tFileInfo2["Hashed-Nonce"];

	if (szBranch != NULL) {
		szBranch = strstr(szBranch, "branch=");
		if (szBranch != NULL)
			szBranch += 7;
	}
	if (szCallID == NULL || szBranch == NULL) {
		debugLogA("Ignoring invalid invitation: CallID='%s', Branch='%s'", szCallID, szBranch);
		return;
	}

	if (szConnType == NULL || szUPnPNat == NULL || szICF == NULL || szNetID == NULL) {
		debugLogA("Ignoring invalid invitation: ConnType='%s', UPnPNat='%s', ICF='%s', NetID='%s'",
			szConnType, szUPnPNat, szICF, szNetID);
		return;
	}

	filetransfer ftl(this), *ft = p2p_getSessionByCallID(szCallID, wlid);
	if (!ft || !ft->p2p_sessionid) {
		ft = &ftl;
		replaceStr(ft->p2p_dest, wlid);
		replaceStr(ft->p2p_callID, szCallID);
		replaceStr(ft->p2p_branch, szBranch);
		ft->p2p_isV2 = strchr(wlid, ';') != NULL;
		ft->std.hContact = MSN_HContactFromEmail(wlid);
	}
	else {
		replaceStr(ft->p2p_callID, szCallID);
		replaceStr(ft->p2p_branch, szBranch);
		ft->p2p_acksessid = MSN_GenRandom();
		/*
				if (p2p_isAvatarOnly(ft->std.hContact))
				{
				p2p_sendStatus(ft, 1603);
				return;
				}
				else
				ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft, 0);
				*/
	}

	directconnection *dc = p2p_getDCByCallID(szCallID, wlid);
	if (dc) {
		if (MSN_GetThreadByContact(wlid, SERVER_P2P_DIRECT)) {
			p2p_sendStatus(ft, 1603);
			p2p_unregisterDC(dc);
			return;
		}
		p2p_unregisterDC(dc);
	}

	dc = new directconnection(szCallID, wlid);
	dc->useHashedNonce = szHashedNonce != NULL;
	if (dc->useHashedNonce)
		dc->xNonce = mir_strdup(szHashedNonce);
	p2p_registerDC(dc);

	MimeHeaders tResult(8);
	tResult.addString("CSeq", "1 ");
	tResult.addString("Call-ID", szCallID);
	tResult.addLong("Max-Forwards", 0);

	MyConnectionType conType = { 0 };

	conType.extIP = atol(szNetID);
	conType.SetUdpCon(szConnType);
	conType.upnpNAT = mir_strcmp(szUPnPNat, "true") == 0;
	conType.icf = mir_strcmp(szICF, "true") == 0;
	conType.CalculateWeight();

	MimeHeaders chdrs(12);
	bool listen = false;

	debugLogA("Connection weight, his: %d mine: %d", conType.weight, MyConnection.weight);
	if (conType.weight <= MyConnection.weight)
		listen = p2p_createListener(ft, dc, chdrs);

	if (!listen) {
		chdrs.addString("Bridge", "TCPv1");
		chdrs.addBool("Listening", false);

		if (dc->useHashedNonce)
			chdrs.addString("Hashed-Nonce", dc->mNonceToHash(), 2);
		else
			chdrs.addString("Nonce", sttVoidUid);

		chdrs.addULong("SessionID", ft->p2p_sessionid);
		chdrs.addString("SChannelState", "0");
		chdrs.addString("Capabilities-Flags", "1");
	}

	tResult.addString("Content-Type", "application/x-msnmsgr-transrespbody");

	if (!ft->p2p_isV2) p2p_getMsgId(ft->p2p_dest, -1);
	p2p_sendSlp(200, ft, tResult, chdrs);
}


void CMsnProto::p2p_startConnect(const char* wlid, const char* szCallID, const char* addr, const char* port, bool ipv6)
{
	if (port == NULL) return;

	char *pPortTokBeg = (char*)port;
	for (;;) {
		char *pPortTokEnd = strchr(pPortTokBeg, ' ');
		if (pPortTokEnd != NULL) *pPortTokEnd = 0;

		char *pAddrTokBeg = (char*)addr;
		for (;;) {
			char *pAddrTokEnd = strchr(pAddrTokBeg, ' ');
			if (pAddrTokEnd != NULL) *pAddrTokEnd = 0;

			ThreadData* newThread = new ThreadData;

			newThread->mType = SERVER_P2P_DIRECT;
			newThread->mInitialContactWLID = mir_strdup(wlid);
			strncpy_s(newThread->mCookie, szCallID, _TRUNCATE);
			mir_snprintf(newThread->mServer, SIZEOF(newThread->mServer),
				ipv6 ? "[%s]:%s" : "%s:%s", pAddrTokBeg, pPortTokBeg);

			newThread->startThread(&CMsnProto::p2p_fileActiveThread, this);

			if (pAddrTokEnd == NULL) break;

			*pAddrTokEnd = ' ';
			pAddrTokBeg = pAddrTokEnd + 1;
		}

		if (pPortTokEnd == NULL) break;

		*pPortTokEnd = ' ';
		pPortTokBeg = pPortTokEnd + 1;
	}
}

void CMsnProto::p2p_InitDirectTransfer2(MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid)
{
	const char  *szCallID = tFileInfo["Call-ID"],
		*szInternalAddress = tFileInfo2["IPv4Internal-Addrs"],
		*szInternalPort = tFileInfo2["IPv4Internal-Port"],
		*szExternalAddress = tFileInfo2["IPv4External-Addrs"],
		*szExternalPort = tFileInfo2["IPv4External-Port"],
		*szNonce = tFileInfo2["Nonce"],
		*szHashedNonce = tFileInfo2["Hashed-Nonce"],
		*szListening = tFileInfo2["Listening"],
		*szV6Address = tFileInfo2["IPv6-Addrs"],
		*szV6Port = tFileInfo2["IPv6-Port"];

	if ((szNonce == NULL && szHashedNonce == NULL) || szListening == NULL) {
		debugLogA("Ignoring invalid invitation: Listening='%s', Nonce=%s", szListening, szNonce);
		return;
	}

	directconnection* dc = p2p_getDCByCallID(szCallID, wlid);
	if (dc == NULL) {
		dc = new directconnection(szCallID, wlid);
		p2p_registerDC(dc);
	}

	dc->useHashedNonce = szHashedNonce != NULL;
	replaceStr(dc->xNonce, szHashedNonce ? szHashedNonce : szNonce);

	if (!mir_strcmp(szListening, "true") && mir_strcmp(dc->xNonce, sttVoidUid)) {
		p2p_startConnect(wlid, szCallID, szV6Address, szV6Port, true);
		p2p_startConnect(wlid, szCallID, szInternalAddress, szInternalPort, false);
		p2p_startConnect(wlid, szCallID, szExternalAddress, szExternalPort, false);
	}
}

void CMsnProto::p2p_AcceptTransfer(MimeHeaders& tFileInfo, MimeHeaders& tFileInfo2, const char* wlid)
{
	const char *szCallID = tFileInfo["Call-ID"];
	const char* szOldContentType = tFileInfo["Content-Type"];
	const char *szBranch = tFileInfo["Via"];

	if (szBranch != NULL) {
		szBranch = strstr(szBranch, "branch=");
		if (szBranch != NULL)
			szBranch += 7;
	}

	filetransfer ftl(this), *ft = p2p_getSessionByCallID(szCallID, wlid);

	if (!ft || !ft->p2p_sessionid) {
		ft = &ftl;
		replaceStr(ft->p2p_branch, szBranch);
		replaceStr(ft->p2p_callID, szCallID);
		replaceStr(ft->p2p_dest, wlid);
		ft->p2p_isV2 = strchr(wlid, ';') != NULL;
		ft->std.hContact = MSN_HContactFromEmail(wlid);
	}
	else {
		if (!(ft->std.flags & PFTS_SENDING)) {
			replaceStr(ft->p2p_branch, szBranch);
			replaceStr(ft->p2p_callID, szCallID);
		}
	}

	if (szCallID == NULL || szBranch == NULL || szOldContentType == NULL) {
		debugLogA("Ignoring invalid invitation: CallID='%s', szBranch='%s'", szCallID, szBranch);
LBL_Close:
		p2p_sendStatus(ft, 500);
		return;
	}

	MimeHeaders tResult(8);
	tResult.addString("CSeq", "0 ");
	tResult.addString("Call-ID", ft->p2p_callID);
	tResult.addLong("Max-Forwards", 0);

	MimeHeaders chdrs(12);

	if (!mir_strcmp(szOldContentType, "application/x-msnmsgr-sessionreqbody")) {
		if (ft == &ftl) {
			p2p_sendCancel(ft);
			return;
		}

		if (!ft->bAccepted) {
			replaceStr(ft->p2p_dest, wlid);
			ft->bAccepted = true;
		}
		else
			return;

		if (ft->p2p_type != MSN_APPID_FILE) {
			if (ft->fileId == -1) ft->create();
			return;
		}

		p2p_sendFeedStart(ft);

		ThreadData* T = MSN_GetP2PThreadByContact(ft->p2p_dest);
		if (T != NULL && T->mType == SERVER_P2P_DIRECT) {
			MSN_StartP2PTransferByContact(ft->p2p_dest);
			return;
		}

		if (usingGateway)
			MSN_StartP2PTransferByContact(ft->p2p_dest);

		directconnection* dc = new directconnection(szCallID, wlid);
		p2p_registerDC(dc);

		tResult.addString("Content-Type", "application/x-msnmsgr-transreqbody");

		chdrs.addString("Bridges", "TCPv1");
		chdrs.addLong("NetID", MyConnection.extIP);
		chdrs.addString("Conn-Type", MyConnection.GetMyUdpConStr());
		chdrs.addBool("UPnPNat", MyConnection.upnpNAT);
		chdrs.addBool("ICF", MyConnection.icf);
		chdrs.addString("IPv6-global", GetGlobalIp(), 2);
		chdrs.addString("Hashed-Nonce", dc->mNonceToHash(), 2);
	}
	else if (!mir_strcmp(szOldContentType, "application/x-msnmsgr-transrespbody")) {
		const char	*szListening = tFileInfo2["Listening"],
			*szNonce = tFileInfo2["Nonce"],
			*szHashedNonce = tFileInfo2["Hashed-Nonce"],
			*szExternalAddress = tFileInfo2["IPv4External-Addrs"],
			*szExternalPort = tFileInfo2["IPv4External-Port"],
			*szInternalAddress = tFileInfo2["IPv4Internal-Addrs"],
			*szInternalPort = tFileInfo2["IPv4Internal-Port"],
			*szV6Address = tFileInfo2["IPv6-Addrs"],
			*szV6Port = tFileInfo2["IPv6-Port"];

		if ((szNonce == NULL && szHashedNonce == NULL) || szListening == NULL) {
			debugLogA("Invalid data packet, exiting...");
			goto LBL_Close;
		}

		directconnection* dc = p2p_getDCByCallID(szCallID, wlid);
		if (dc == NULL) return;

		if (!dc->bAccepted)
			dc->bAccepted = true;
		else
			return;

		dc->useHashedNonce = szHashedNonce != NULL;
		replaceStr(dc->xNonce, szHashedNonce ? szHashedNonce : szNonce);

		// another side reported that it will be a server.
		if (!mir_strcmp(szListening, "true") && (szNonce == NULL || mir_strcmp(szNonce, sttVoidUid))) {
			p2p_startConnect(ft->p2p_dest, szCallID, szV6Address, szV6Port, true);
			p2p_startConnect(ft->p2p_dest, szCallID, szInternalAddress, szInternalPort, false);
			p2p_startConnect(ft->p2p_dest, szCallID, szExternalAddress, szExternalPort, false);
			return;
		}

		// no, send a file via server
		if (!p2p_createListener(ft, dc, chdrs)) {
			p2p_unregisterDC(dc);
			if (ft != &ftl)
				MSN_StartP2PTransferByContact(ft->p2p_dest);
			else
				p2p_startSessions(ft->p2p_dest);
			return;
		}

		tResult.addString("Content-Type", "application/x-msnmsgr-transrespbody");
	}
	else if (!mir_strcmp(szOldContentType, "application/x-msnmsgr-transreqbody")) {
		const char *szHashedNonce = tFileInfo2["Hashed-Nonce"];
		const char *szNonce = tFileInfo2["Nonce"];

		directconnection* dc = p2p_getDCByCallID(szCallID, wlid);
		if (dc == NULL) {
			dc = new directconnection(szCallID, wlid);
			p2p_registerDC(dc);
		}

		dc->useHashedNonce = szHashedNonce != NULL;
		replaceStr(dc->xNonce, szHashedNonce ? szHashedNonce : szNonce);

		// no, send a file via server
		if (!p2p_createListener(ft, dc, chdrs)) {
			p2p_unregisterDC(dc);
			MSN_StartP2PTransferByContact(ft->p2p_dest);
			return;
		}

		tResult.addString("Content-Type", "application/x-msnmsgr-transrespbody");
	}
	else
		return;

	if (!ft->p2p_isV2) p2p_getMsgId(ft->p2p_dest, -1);
	p2p_sendSlp(-2, ft, tResult, chdrs);
}


/////////////////////////////////////////////////////////////////////////////////////////
// p2p_processSIP - processes all MSN SIP commands

void CMsnProto::p2p_processSIP(ThreadData* info, char* msgbody, P2PB_Header* hdrdata, const char* wlid)
{
	int iMsgType = 0;
	if (!memcmp(msgbody, "INVITE MSNMSGR:", 15))
		iMsgType = 1;
	else if (!memcmp(msgbody, "MSNSLP/1.0 200 ", 15))
		iMsgType = 2;
	else if (!memcmp(msgbody, "BYE MSNMSGR:", 12))
		iMsgType = 3;
	else if (!memcmp(msgbody, "MSNSLP/1.0 603 ", 15))
		iMsgType = 4;
	else if (!memcmp(msgbody, "MSNSLP/1.0 481 ", 15))
		iMsgType = 4;
	else if (!memcmp(msgbody, "MSNSLP/1.0 500 ", 15))
		iMsgType = 4;
	else if (!memcmp(msgbody, "ACK MSNMSGR:", 12))
		iMsgType = 5;

	char* peol = strstr(msgbody, "\r\n");
	if (peol != NULL)
		msgbody = peol + 2;

	MimeHeaders tFileInfo, tFileInfo2;
	msgbody = tFileInfo.readFromBuffer(msgbody);
	msgbody = tFileInfo2.readFromBuffer(msgbody);

	const char* szContentType = tFileInfo["Content-Type"];
	if (szContentType == NULL) {
		debugLogA("Invalid or missing Content-Type field, exiting");
		return;
	}

	if (hdrdata && !hdrdata->isV2Hdr()) {
		if (iMsgType == 2 || (iMsgType == 1 && !mir_strcmp(szContentType, "application/x-msnmsgr-transreqbody")))
			p2p_getMsgId(wlid, 1);
	}

	switch (iMsgType) {
	case 1:
		if (!mir_strcmp(szContentType, "application/x-msnmsgr-sessionreqbody"))
			p2p_InitFileTransfer(info, tFileInfo, tFileInfo2, wlid);
		else if (!mir_strcmp(szContentType, "application/x-msnmsgr-transreqbody"))
			p2p_InitDirectTransfer(tFileInfo, tFileInfo2, wlid);
		else if (!mir_strcmp(szContentType, "application/x-msnmsgr-transrespbody"))
			p2p_InitDirectTransfer2(tFileInfo, tFileInfo2, wlid);
		break;

	case 2:
		p2p_AcceptTransfer(tFileInfo, tFileInfo2, wlid);
		break;

	case 3:
		if (!mir_strcmp(szContentType, "application/x-msnmsgr-sessionclosebody")) {
			filetransfer* ft = p2p_getSessionByCallID(tFileInfo["Call-ID"], wlid);
			if (ft != NULL) {
				if (ft->std.currentFileProgress < ft->std.currentFileSize) {
					ft->bCanceled = true;
					p2p_sendAbortSession(ft);
				}
				else {
					if (!(ft->std.flags & PFTS_SENDING))
						ft->bCompleted = true;
				}

				p2p_sessionComplete(ft);
			}
		}
		break;

	case 4:
	{
		const char* szCallID = tFileInfo["Call-ID"];

		//			application/x-msnmsgr-session-failure-respbody

		directconnection *dc = p2p_getDCByCallID(szCallID, wlid);
		if (dc != NULL) {
			p2p_unregisterDC(dc);
			break;
		}

		filetransfer* ft = p2p_getSessionByCallID(szCallID, wlid);
		if (ft == NULL)
			break;

		ft->close();
		if (!(ft->std.flags & PFTS_SENDING)) _tremove(ft->std.tszCurrentFile);

		p2p_unregisterSession(ft);
	}
	break;

	case 5:
		if (!mir_strcmp(szContentType, "application/x-msnmsgr-turnsetup")) {
			//			tFileInfo2["ServerAddress"];
			//			tFileInfo2["SessionUsername"];
			//			tFileInfo2["SessionPassword"];
		}
		else if (!mir_strcmp(szContentType, "application/x-msnmsgr-transudpswitch")) {
			//			tFileInfo2["IPv6AddrsAndPorts"];
			//			tFileInfo2["IPv4ExternalAddrsAndPorts"];
			//			tFileInfo2["IPv4InternalAddrsAndPorts"];
		}
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// p2p_processMsg - processes all MSN P2P incoming messages
void CMsnProto::p2p_processMsgV2(ThreadData* info, char* msgbody, const char* wlid)
{
	P2PV2_Header hdrdata;

	char *msg = hdrdata.parseMsg(msgbody);
	hdrdata.logHeader(this);

	if (hdrdata.mSessionID == 0) {
		if (hdrdata.mPacketLen == 0) {
			if (hdrdata.mOpCode & 0x02)
				p2p_sendAck(wlid, &hdrdata);
			return;
		}

		if (hdrdata.mRemSize || hdrdata.mTFCode == 0) {
			char msgid[128];
			mir_snprintf(msgid, SIZEOF(msgid), "%s_%08x", wlid, hdrdata.mPacketNum);

			int idx;
			if (hdrdata.mTFCode == 0x01) {
				const size_t portion = hdrdata.mPacketLen + (msg - msgbody);
				const size_t len = portion + hdrdata.mRemSize;
				idx = addCachedMsg(msgid, msgbody, 0, portion, len, false);
			}
			else {
				size_t len = hdrdata.mPacketLen + hdrdata.mRemSize;
				size_t offset = getCachedMsgSize(msgid); if (offset >= len) offset -= len;
				idx = addCachedMsg(msgid, msg, offset, hdrdata.mPacketLen, len, false);
			}

			if (hdrdata.mRemSize == 0) {
				size_t newsize;
				if (getCachedMsg(idx, msgbody, newsize)) {
					unsigned id = hdrdata.mID;
					msg = hdrdata.parseMsg(msgbody);
					hdrdata.mID = id;

					if (hdrdata.mOpCode & 0x02)
						p2p_sendAck(wlid, &hdrdata);

					if (hdrdata.mTFCode)
						p2p_processSIP(info, msg, &hdrdata, wlid);
					mir_free(msgbody);
				}
				else
					clearCachedMsg(idx);
			}
		}
		else {
			if (hdrdata.mOpCode & 0x02)
				p2p_sendAck(wlid, &hdrdata);

			p2p_processSIP(info, msg, &hdrdata, wlid);
		}

		return;
	}

	if (hdrdata.mOpCode & 0x02)
		p2p_sendAck(wlid, &hdrdata);

	filetransfer* ft = p2p_getSessionByID(hdrdata.mSessionID);
	if (ft == NULL) return;

	ft->ts = time(NULL);

	if (hdrdata.mTFCode >= 4 && hdrdata.mTFCode <= 7) {
		_write(ft->fileId, msg, hdrdata.mPacketLen);

		ft->std.totalProgress += hdrdata.mPacketLen;
		ft->std.currentFileProgress += hdrdata.mPacketLen;

		if (ft->p2p_appID == MSN_APPID_FILE && clock() >= ft->nNotify) {
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
			ft->nNotify = clock() + 500;

			//---- send an ack: body was transferred correctly
			debugLogA("Transferred %I64u bytes remaining %I64u", ft->std.currentFileProgress, hdrdata.mRemSize);
		}

		if (hdrdata.mRemSize == 0) {
			if (ft->p2p_appID == MSN_APPID_FILE) {
				ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
				ft->complete();
			}
			else {
				p2p_savePicture2disk(ft);
				if (!ft->p2p_isV2) p2p_sendBye(ft);
			}
		}
	}
}

void CMsnProto::p2p_processMsg(ThreadData* info, char* msgbody, const char* wlid)
{
	P2P_Header hdrdata;
	msgbody = hdrdata.parseMsg(msgbody);
	hdrdata.logHeader(this);

	//---- if we got a message
	if (LOWORD(hdrdata.mFlags) == 0 && hdrdata.mSessionID == 0) {
		//		MsnContact *cont = Lists_Get(wlid);
		//		if (cont && cont->places.getCount())
		//			return;

		if (hdrdata.mPacketLen < hdrdata.mTotalSize) {
			char msgid[128];
			mir_snprintf(msgid, SIZEOF(msgid), "%s_%08x", wlid, hdrdata.mID);
			int idx = addCachedMsg(msgid, msgbody, (size_t)hdrdata.mOffset, hdrdata.mPacketLen,
				(size_t)hdrdata.mTotalSize, false);

			char* newbody;
			size_t newsize;
			if (getCachedMsg(idx, newbody, newsize)) {
				p2p_sendAck(wlid, &hdrdata);
				p2p_processSIP(info, newbody, &hdrdata, wlid);
				mir_free(newbody);
			}
			else {
				if (hdrdata.mOffset + hdrdata.mPacketLen >= hdrdata.mTotalSize)
					clearCachedMsg(idx);
			}
		}
		else {
			p2p_sendAck(wlid, &hdrdata);
			p2p_processSIP(info, msgbody, &hdrdata, wlid);
		}

		return;
	}

	filetransfer* ft = p2p_getSessionByID(hdrdata.mSessionID);
	if (ft == NULL)
		ft = p2p_getSessionByUniqueID(hdrdata.mAckUniqueID);

	if (ft == NULL) return;

	ft->ts = time(NULL);

	//---- receiving redirect -----------
	if (hdrdata.mFlags == 0x01) {
		if (WaitForSingleObject(ft->hLockHandle, INFINITE) == WAIT_OBJECT_0) {
			__int64 dp = (__int64)(ft->std.currentFileProgress - hdrdata.mAckDataSize);
			ft->std.totalProgress -= dp;
			ft->std.currentFileProgress -= dp;
			_lseeki64(ft->fileId, ft->std.currentFileProgress, SEEK_SET);
			ft->tType = info->mType;
			ReleaseMutex(ft->hLockHandle);
		}
	}

	//---- receiving ack -----------
	if (hdrdata.mFlags == 0x02) {
		ft->p2p_waitack = false;

		if (hdrdata.mAckSessionID == ft->p2p_sendmsgid) {
			if (ft->p2p_appID == MSN_APPID_FILE) {
				ft->bCompleted = true;
				p2p_sendBye(ft);
			}
			return;
		}

		if (hdrdata.mAckSessionID == ft->p2p_byemsgid) {
			p2p_sessionComplete(ft);
			return;
		}

		switch (ft->p2p_ackID) {
		case 1000:
			//---- send Data Preparation Message
			p2p_sendAvatarInit(ft);
			break;

		case 1001:
			//---- send Data Messages
			MSN_StartP2PTransferByContact(ft->p2p_dest);
			break;
		}

		ft->p2p_ackID++;
		return;
	}

	if (LOWORD(hdrdata.mFlags) == 0) {
		//---- accept the data preparation message ------
		//		const unsigned* pLongs = (unsigned*)msgbody;
		if (hdrdata.mPacketLen == 4 && hdrdata.mTotalSize == 4) {
			p2p_sendAck(ft->p2p_dest, &hdrdata);
			return;
		}
		else
			hdrdata.mFlags = 0x20;
	}

	//---- receiving data -----------
	if (LOWORD(hdrdata.mFlags) == 0x20 || LOWORD(hdrdata.mFlags) == 0x30) {
		if (hdrdata.mOffset + hdrdata.mPacketLen > hdrdata.mTotalSize)
			hdrdata.mPacketLen = DWORD(hdrdata.mTotalSize - hdrdata.mOffset);

		if (ft->tTypeReq == 0 || ft->tTypeReq == info->mType) {
			ft->tType = info->mType;
			ft->p2p_sendmsgid = hdrdata.mID;
		}

		__int64 dsz = ft->std.currentFileSize - hdrdata.mOffset;
		if (dsz > hdrdata.mPacketLen) dsz = hdrdata.mPacketLen;

		if (ft->tType == info->mType) {
			if (dsz > 0 && ft->fileId >= 0) {
				if (ft->lstFilePtr != hdrdata.mOffset)
					_lseeki64(ft->fileId, hdrdata.mOffset, SEEK_SET);
				_write(ft->fileId, msgbody, (unsigned int)dsz);

				ft->lstFilePtr = hdrdata.mOffset + dsz;

				__int64 dp = ft->lstFilePtr - ft->std.currentFileProgress;
				if (dp > 0) {
					ft->std.totalProgress += dp;
					ft->std.currentFileProgress += dp;

					if (ft->p2p_appID == MSN_APPID_FILE && clock() >= ft->nNotify) {
						ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
						ft->nNotify = clock() + 500;
					}
				}

				//---- send an ack: body was transferred correctly
				debugLogA("Transferred %I64u bytes out of %I64u", ft->std.currentFileProgress, hdrdata.mTotalSize);
			}

			if (ft->std.currentFileProgress >= hdrdata.mTotalSize) {
				ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
				p2p_sendAck(ft->p2p_dest, &hdrdata);
				if (ft->p2p_appID == MSN_APPID_FILE) {
					ft->ts = time(NULL);
					ft->p2p_waitack = true;
					ft->complete();
				}
				else {
					p2p_savePicture2disk(ft);
					p2p_sendBye(ft);
				}
			}
		}
	}

	if (hdrdata.mFlags == 0x40 || hdrdata.mFlags == 0x80) {
		p2p_sendAbortSession(ft);
		p2p_unregisterSession(ft);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// p2p_invite - invite another side to transfer an avatar

void CMsnProto::p2p_invite(unsigned iAppID, filetransfer* ft, const char *wlid)
{
	const char* szAppID;
	switch (iAppID) {
	case MSN_APPID_FILE:			        szAppID = "{5D3E02AB-6190-11D3-BBBB-00C04F795683}";	break;
	case MSN_APPID_AVATAR:			        szAppID = "{A4268EEC-FEC5-49E5-95C3-F126696BDBF6}";	break;
	case MSN_APPID_CUSTOMSMILEY:	        szAppID = "{A4268EEC-FEC5-49E5-95C3-F126696BDBF6}";	break;
	case MSN_APPID_CUSTOMANIMATEDSMILEY:	szAppID = "{A4268EEC-FEC5-49E5-95C3-F126696BDBF6}";	break;
	default: return;
	}

	ft->p2p_type = iAppID;
	ft->p2p_acksessid = MSN_GenRandom();
	mir_free(ft->p2p_callID);
	ft->p2p_callID = getNewUuid();

	MsnContact* cont = Lists_Get(ft->std.hContact);
	if (cont == NULL) return;

	if (ft->p2p_dest == NULL) {
		ft->p2p_isV2 = (cont->cap2 & capex_SupportsPeerToPeerV2) != 0 || (cont->cap1 >> 28) >= 10;
		ft->p2p_dest = mir_strdup(wlid ? wlid : cont->email);
	}

	char*  pContext = NULL;
	size_t cbContext = 0;

	switch (iAppID) {
	case MSN_APPID_FILE:
		cbContext = sizeof(HFileContext);
		pContext = (char*)malloc(cbContext);
		{
			HFileContext* ctx = (HFileContext*)pContext;
			memset(pContext, 0, cbContext);
			if (ft->p2p_isV2) {
				cbContext -= 64;
				ctx->ver = 2;
			}
			else {
				ctx->ver = 3;
				ctx->id = 0xffffffff;
			}
			ctx->len = (unsigned)cbContext;
			ctx->type = MSN_TYPEID_FTNOPREVIEW;
			ctx->dwSize = ft->std.currentFileSize;

			TCHAR* pszFiles = _tcsrchr(ft->std.tszCurrentFile, '\\');
			if (pszFiles)
				pszFiles++;
			else
				pszFiles = ft->std.tszCurrentFile;

			wchar_t *fname = mir_t2u(pszFiles);
			wcsncpy(ctx->wszFileName, fname, MAX_PATH);
			mir_free(fname);

			ft->p2p_appID = MSN_APPID_FILE;
		}
		break;

	default:
		ft->p2p_appID = MSN_APPID_AVATAR2;

		if (ft->p2p_object == NULL) {
			delete ft;
			return;
		}

		ezxml_t xmlo = ezxml_parse_str(NEWSTR_ALLOCA(ft->p2p_object), mir_strlen(ft->p2p_object));
		ezxml_t xmlr = ezxml_new("msnobj");

		const char* p;
		p = ezxml_attr(xmlo, "Creator");
		if (p != NULL)
			ezxml_set_attr(xmlr, "Creator", p);
		p = ezxml_attr(xmlo, "Size");
		if (p != NULL) {
			ezxml_set_attr(xmlr, "Size", p);
			ft->std.totalBytes = ft->std.currentFileSize = _atoi64(p);
		}
		p = ezxml_attr(xmlo, "Type");
		if (p != NULL)
			ezxml_set_attr(xmlr, "Type", p);
		p = ezxml_attr(xmlo, "Location");
		if (p != NULL)
			ezxml_set_attr(xmlr, "Location", p);
		p = ezxml_attr(xmlo, "Friendly");
		if (p != NULL)
			ezxml_set_attr(xmlr, "Friendly", p);
		p = ezxml_attr(xmlo, "SHA1D");
		if (p != NULL)
			ezxml_set_attr(xmlr, "SHA1D", p);
		p = ezxml_attr(xmlo, "SHA1C");
		if (p != NULL)
			ezxml_set_attr(xmlr, "SHA1C", p);

		pContext = ezxml_toxml(xmlr, false);
		cbContext = mir_strlen(pContext) + 1;

		ezxml_free(xmlr);
		ezxml_free(xmlo);

		break;
	}

	bool sessionExist = p2p_sessionRegistered(ft);
	if (!sessionExist) {
		p2p_registerSession(ft);

		unsigned short status = getWord(ft->std.hContact, "Status", ID_STATUS_OFFLINE);
		if ((myFlags & 0x4000000) && cont->places.getCount() <= 1 &&
			status != ID_STATUS_OFFLINE && status != ID_STATUS_INVISIBLE && m_iStatus != ID_STATUS_INVISIBLE) {
			if (ft->p2p_isV2) {
				if (cont->places.getCount() && cont->places[0].cap1 & cap_SupportsP2PBootstrap) {
					char wlid[128];
					mir_snprintf(wlid, SIZEOF(wlid),
						mir_strcmp(cont->places[0].id, sttVoidUid) ? "%s;%s" : "%s",
						cont->email, cont->places[0].id);

					if (!MSN_GetThreadByContact(wlid, SERVER_P2P_DIRECT))
						p2p_inviteDc(ft, wlid);
					else
						p2p_invite(ft->p2p_type, ft, wlid);

					free(pContext);
					return;
				}
			}
			else {
				const char *wlid = cont->email;
				if (cont->cap1 & cap_SupportsP2PBootstrap) {
					if (!MSN_GetThreadByContact(wlid, SERVER_P2P_DIRECT))
						p2p_inviteDc(ft, wlid);
					else
						p2p_invite(ft->p2p_type, ft, wlid);

					free(pContext);
					return;
				}
			}
		}
	}

	if (!ft->bAccepted)
		ft->p2p_sessionid = MSN_GenRandom();

	ptrA szContextEnc(mir_base64_encode((PBYTE)pContext, (unsigned)cbContext));

	MimeHeaders chdrs(10);
	chdrs.addString("EUF-GUID", szAppID);
	chdrs.addULong("SessionID", ft->p2p_sessionid);
	chdrs.addULong("AppID", ft->p2p_appID);
	chdrs.addString("Context", szContextEnc);

	MimeHeaders tResult(8);
	tResult.addString("CSeq", "0 ");
	tResult.addString("Call-ID", ft->p2p_callID);
	tResult.addLong("Max-Forwards", 0);
	tResult.addString("Content-Type", "application/x-msnmsgr-sessionreqbody");

	if (iAppID != MSN_APPID_FILE)
		ft->p2p_waitack = true;

	if (ft->p2p_isV2 && ft->std.currentFileNumber == 0) {
		for (int i = 0; i < cont->places.getCount(); ++i) {
			char wlid[128];
			mir_snprintf(wlid, SIZEOF(wlid),
				mir_strcmp(cont->places[i].id, sttVoidUid) ? "%s;%s" : "%s",
				cont->email, cont->places[i].id);

			p2p_sendSlp(-2, ft, tResult, chdrs, wlid);
		}
	}
	else p2p_sendSlp(-2, ft, tResult, chdrs, wlid);

	free(pContext);
}


void CMsnProto::p2p_inviteDc(filetransfer* ft, const char *wlid)
{
	directconnection* dc = new directconnection(szUbnCall, wlid);
	p2p_registerDC(dc);

	MimeHeaders tResult(8);
	tResult.addString("CSeq", "0 ");
	tResult.addString("Call-ID", dc->callId);
	tResult.addLong("Max-Forwards", 0);
	tResult.addString("Content-Type", "application/x-msnmsgr-transreqbody");

	MimeHeaders chdrs(12);

	chdrs.addString("Bridges", "TCPv1 SBBridge");
	chdrs.addLong("NetID", MyConnection.extIP);
	chdrs.addString("Conn-Type", MyConnection.GetMyUdpConStr());
	chdrs.addBool("UPnPNat", MyConnection.upnpNAT);
	chdrs.addBool("ICF", MyConnection.icf);
	chdrs.addString("IPv6-global", GetGlobalIp(), 2);
	chdrs.addString("Hashed-Nonce", dc->mNonceToHash(), 2);
	chdrs.addString("SessionID", "0");
	chdrs.addString("SChannelState", "0");
	chdrs.addString("Capabilities-Flags", "1");

	p2p_sendSlp(-2, ft, tResult, chdrs, wlid);
}

/*
void CMsnProto::p2p_sendSessionAck(filetransfer* ft)
{
	MimeHeaders tResult(8);
	tResult.addString("CSeq", "0 ");
	tResult.addString("Call-ID", sttVoidUid);
	tResult.addLong("Max-Forwards", 0);
	tResult.addString("Content-Type", "application/x-msnmsgr-transdestaddrupdate");

	MimeHeaders chdrs(8);

	chdrs.addString("IPv4ExternalAddrsAndPorts", mir_strdup(MyConnection.GetMyExtIPStr()), 6);
	chdrs.addString("IPv4InternalAddrsAndPorts", mir_strdup(MyConnection.GetMyExtIPStr()), 6);
	chdrs.addString("SessionID", "0");
	chdrs.addString("SChannelState", "0");
	chdrs.addString("Capabilities-Flags", "1");

	p2p_sendSlp(-3, ft, tResult, chdrs);
}
*/

void CMsnProto::p2p_sessionComplete(filetransfer* ft)
{
	if (ft->p2p_appID != MSN_APPID_FILE)
		p2p_unregisterSession(ft);
	else if (ft->std.flags & PFTS_SENDING) {
		if (ft->openNext() == -1) {
			bool success = ft->std.currentFileNumber >= ft->std.totalFiles && ft->bCompleted;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, success ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, ft, 0);
		}
		else {
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
			p2p_invite(ft->p2p_appID, ft, NULL);
		}
	}
	else {
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ft->bCompleted ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, ft, 0);
		p2p_unregisterSession(ft);
	}
}

char* P2PV2_Header::parseMsg(char *buf)
{
	unsigned char hdrLen1 = *(unsigned char*)buf;
	mOpCode = *(unsigned char*)(buf + 1);
	mPacketLen = _htons(*(unsigned short*)(buf + 2));
	mID = _htonl(*(unsigned*)(buf + 4));

	char* buf1 = buf + hdrLen1;

	for (char *tlvp = buf + 8; tlvp < buf1 && *tlvp; tlvp += 2 + tlvp[1]) {
		switch (*tlvp) {
		case 1:
			mCap = tlvp;
			break;
		case 2:
			mAckUniqueID = _htonl(*(unsigned*)(tlvp + 4));
			break;
		case 3:
			break;
		}
	}

	if (mPacketLen == 0) return buf + hdrLen1;

	unsigned char hdrLen2 = *(unsigned char*)buf1;
	mTFCode = *(unsigned char*)(buf1 + 1);
	mPacketNum = _htons(*(unsigned short*)(buf1 + 2));
	mSessionID = _htonl(*(unsigned*)(buf1 + 4));

	char* buf2 = buf1 + hdrLen2;

	for (char *tlvp1 = buf1 + 8; tlvp1 < buf2 && *tlvp1; tlvp1 += 2 + tlvp1[1]) {
		switch (*tlvp1) {
		case 1:
			mRemSize = _htonl64(*(unsigned __int64*)(tlvp1 + 2));
			break;
		}
	}

	mPacketLen -= hdrLen2;
	return buf1 + hdrLen2;
}

char* P2PV2_Header::createMsg(char *buf, const char* wlid, CMsnProto *ppro)
{
	unsigned char hdrLen1 = 8 + (mAckUniqueID ? 6 : 0) + (mCap ? 2 + mCap[1] : 0);
	unsigned char comp = hdrLen1 & 3;
	hdrLen1 += comp ? 4 - comp : 0;

	unsigned char hdrLen2 = mPacketLen ? (8 + (mRemSize ? 10 : 0)) : 0;
	comp = hdrLen2 & 3;
	hdrLen2 += comp ? 4 - comp : 0;

	mID = ppro->p2p_getMsgId(wlid, mPacketLen + hdrLen2);

	memset(buf, 0, hdrLen1 + hdrLen2);

	*(unsigned char*)(buf + 0) = hdrLen1;
	*(unsigned char*)(buf + 1) = mOpCode;
	*(unsigned short*)(buf + 2) = _htons(mPacketLen + hdrLen2);
	*(unsigned*)(buf + 4) = _htonl(mID);

	char *buf1 = buf + 8;

	if (mAckUniqueID) {
		*(unsigned char*)buf1 = 2;
		*(unsigned char*)(buf1 + 1) = 4;
		*(unsigned*)(buf1 + 2) = _htonl(mAckUniqueID);
		buf1 += 6;
	}
	if (mCap) {
		unsigned len = 2 + mCap[1];
		memcpy(buf1, mCap, len);
		buf1 += len;
	}

	buf1 = buf + hdrLen1;

	if (hdrLen2 == 0) return buf1;

	*(unsigned char*)(buf1 + 0) = hdrLen2;
	*(unsigned char*)(buf1 + 1) = mTFCode;
	*(unsigned short*)(buf1 + 2) = _htons(mPacketNum);
	*(unsigned*)(buf1 + 4) = _htonl(mSessionID);

	if (mRemSize) {
		*(unsigned char*)(buf1 + 8) = 1;
		*(unsigned char*)(buf1 + 9) = 8;
		*(unsigned __int64*)(buf1 + 10) = _htonl64(mRemSize);
	}

	return buf1 + hdrLen2;
}

char* P2P_Header::createMsg(char *buf, const char* wlid, CMsnProto *ppro)
{
	if (!mID) mID = ppro->p2p_getMsgId(wlid, 1);
	memcpy(buf, &mSessionID, 48);
	return buf + 48;
}
#endif