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

void CMsnProto::msnftp_sendAcceptReject(filetransfer *ft, bool acc)
{
	ThreadData* thread = MSN_GetThreadByContact(ft->p2p_dest);
	if (thread == NULL) return;

	if (acc) {
		thread->sendPacket("MSG",
			"U %d\r\nMIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
			"Invitation-Command: ACCEPT\r\n"
			"Invitation-Cookie: %s\r\n"
			"Launch-Application: FALSE\r\n"
			"Request-Data: IP-Address:\r\n\r\n",
			172 + 4 + strlen(ft->szInvcookie), ft->szInvcookie);
	}
	else {
		thread->sendPacket("MSG",
			"U %d\r\nMIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
			"Invitation-Command: CANCEL\r\n"
			"Invitation-Cookie: %s\r\n"
			"Cancel-Code: REJECT\r\n\r\n",
			172 - 33 + 4 + strlen(ft->szInvcookie), ft->szInvcookie);
	}
}

void CMsnProto::msnftp_invite(filetransfer *ft)
{
	bool isOffline;
	ThreadData* thread = MSN_StartSB(ft->p2p_dest, isOffline);
	if (isOffline) return;
	if (thread != NULL) thread->mMsnFtp = ft;

	TCHAR* pszFiles = _tcsrchr(ft->std.ptszFiles[0], '\\');
	if (pszFiles)
		pszFiles++;
	else
		pszFiles = *ft->std.ptszFiles;

	char msg[1024];
	mir_snprintf(msg, SIZEOF(msg),
		"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
		"Application-Name: File Transfer\r\n"
		"Application-GUID: {5D3E02AB-6190-11d3-BBBB-00C04F795683}\r\n"
		"Invitation-Command: INVITE\r\n"
		"Invitation-Cookie: %i\r\n"
		"Application-File: %s\r\n"
		"Application-FileSize: %I64u\r\n\r\n",
		MSN_GenRandom(), UTF8(pszFiles), ft->std.currentFileSize);

	if (thread == NULL)
		MsgQueue_Add(ft->p2p_dest, 'S', msg, -1, ft);
	else
		thread->sendMessage('S', NULL, NETID_MSN, msg, MSG_DISABLE_HDR);
}


/////////////////////////////////////////////////////////////////////////////////////////
//	MSN File Transfer Protocol commands processing

int CMsnProto::MSN_HandleMSNFTP(ThreadData *info, char *cmdString)
{
	char* params = "";
	filetransfer* ft = info->mMsnFtp;

	if (cmdString[3])
		params = cmdString + 4;

	switch ((*(PDWORD)cmdString & 0x00FFFFFF) | 0x20000000) {
	case ' EYB':    //********* BYE
		ft->complete();
		return 1;

	case ' LIF':    //********* FIL
		char filesize[30];
		if (sscanf(params, "%s", filesize) < 1)
			goto LBL_InvalidCommand;

		info->mCaller = 1;
		info->send("TFR\r\n", 5);
		break;

	case ' RFT':    //********* TFR
		{
			char *sendpacket = (char*)alloca(2048);
			filetransfer* ft = info->mMsnFtp;

			info->mCaller = 3;

			while (ft->std.currentFileProgress < ft->std.currentFileSize) {
				if (ft->bCanceled) {
					sendpacket[0] = 0x01;
					sendpacket[1] = 0x00;
					sendpacket[2] = 0x00;
					info->send(sendpacket, 3);
					return 0;
				}

				int wPlace = 0;
				sendpacket[wPlace++] = 0x00;
				unsigned __int64 packetLen = ft->std.currentFileSize - ft->std.currentFileProgress;
				if (packetLen > 2045) packetLen = 2045;

				sendpacket[wPlace++] = (char)(packetLen & 0x00ff);
				sendpacket[wPlace++] = (char)((packetLen & 0xff00) >> 8);
				_read(ft->fileId, &sendpacket[wPlace], packetLen);

				info->send(&sendpacket[0], packetLen + 3);

				ft->std.totalProgress += packetLen;
				ft->std.currentFileProgress += packetLen;

				ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
			}

			ft->complete();
		}
		break;

	case ' RSU':    //********* USR
		char email[130], authcookie[14];
		if (sscanf(params, "%129s %13s", email, authcookie) < 2) {
			debugLogA("Invalid USR OK command, ignoring");
			break;
		}

		char tCommand[30];
		mir_snprintf(tCommand, sizeof(tCommand), "FIL %i\r\n", info->mMsnFtp->std.totalBytes);
		info->send(tCommand, strlen(tCommand));
		break;

	case ' REV':    //********* VER
		char protocol1[7];
		if (sscanf(params, "%6s", protocol1) < 1) {
LBL_InvalidCommand:
			debugLogA("Invalid %.3s command, ignoring", cmdString);
			break;
		}

		if (strcmp(protocol1, "MSNFTP") != 0) {
			int tempInt;
			int tFieldCount = sscanf(params, "%d %6s", &tempInt, protocol1);
			if (tFieldCount != 2 || strcmp(protocol1, "MSNFTP") != 0) {
				debugLogA("Another side requested the unknown protocol (%s), closing thread", params);
				return 1;
			}
		}

		if (info->mCaller == 0) { //receive
			char tCommand[MSN_MAX_EMAIL_LEN + 50];
			mir_snprintf(tCommand, sizeof(tCommand), "USR %s %s\r\n", MyOptions.szEmail, info->mCookie);
			info->send(tCommand, strlen(tCommand));
		}
		else if (info->mCaller == 2) { //send
			static const char sttCommand[] = "VER MSNFTP\r\n";
			info->send(sttCommand, strlen(sttCommand));
		}
		break;

	default:		// receiving file
		HReadBuffer tBuf(info, int(cmdString - info->mData));

		for (;;) {
			if (ft->bCanceled) {
				info->send("CCL\r\n", 5);
				ft->close();
				return 1;
			}

			BYTE* p = tBuf.surelyRead(3);
			if (p == NULL) {
LBL_Error:
				ft->close();
				MSN_ShowError("file transfer is canceled by remote host");
				return 1;
			}

			BYTE tIsTransitionFinished = *p++;
			WORD dataLen = *p++;
			dataLen |= (*p++ << 8);

			if (tIsTransitionFinished) {
LBL_Success:
				static const char sttCommand[] = "BYE 16777989\r\n";
				info->send(sttCommand, strlen(sttCommand));
				return 1;
			}

			p = tBuf.surelyRead(dataLen);
			if (p == NULL)
				goto LBL_Error;

			_write(ft->fileId, p, dataLen);
			ft->std.totalProgress += dataLen;
			ft->std.currentFileProgress += dataLen;

			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);

			if (ft->std.currentFileProgress == ft->std.totalBytes) {
				ft->complete();
				goto LBL_Success;
			}
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	ft_startFileSend - sends a file using the old f/t protocol

void __cdecl CMsnProto::msnftp_sendFileThread(void* arg)
{
	ThreadData* info = (ThreadData*)arg;

	debugLogA("Waiting for an incoming connection to '%s'...", info->mServer);

	switch (WaitForSingleObject(info->hWaitEvent, 60000)) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		debugLogA("Incoming connection timed out, closing file transfer");
		return;
	}

	info->mBytesInData = 0;

	for (;;) {
		int recvResult = info->recv(info->mData + info->mBytesInData, 1000 - info->mBytesInData);
		if (recvResult == SOCKET_ERROR || !recvResult)
			break;

		info->mBytesInData += recvResult;

		//pull off each line for parsing
		if (info->mCaller == 3 && info->mType == SERVER_FILETRANS) {
			if (MSN_HandleMSNFTP(info, info->mData))
				break;
		}
		else   // info->mType!=SERVER_FILETRANS
		{
			for (;;) {
				char* peol = strchr(info->mData, '\r');
				if (peol == NULL)
					break;

				if (info->mBytesInData < peol - info->mData + 2)
					break;  //wait for full line end

				char msg[sizeof(info->mData)];
				memcpy(msg, info->mData, peol - info->mData); msg[peol - info->mData] = 0;
				if (*++peol != '\n')
					debugLogA("Dodgy line ending to command: ignoring");
				else
					peol++;

				info->mBytesInData -= peol - info->mData;
				memmove(info->mData, peol, info->mBytesInData);

				debugLogA("RECV:%s", msg);

				if (!isalnum(msg[0]) || !isalnum(msg[1]) || !isalnum(msg[2]) || (msg[3] && msg[3] != ' ')) {
					debugLogA("Invalid command name");
					continue;
				}

				if (MSN_HandleMSNFTP(info, msg))
					break;
			}
		}

		if (info->mBytesInData == sizeof(info->mData)) {
			debugLogA("sizeof(data) is too small: the longest line won't fit");
			break;
		}
	}

	debugLogA("Closing file transfer thread");
}

void CMsnProto::msnftp_startFileSend(ThreadData* info, const char* Invcommand, const char* Invcookie)
{
	if (_stricmp(Invcommand, "ACCEPT"))
		return;

	NETLIBBIND nlb = { 0 };
	HANDLE sb = NULL;

	filetransfer* ft = info->mMsnFtp; info->mMsnFtp = NULL;
	if (ft != NULL && MyConnection.extIP) {
		nlb.cbSize = sizeof(nlb);
		nlb.pfnNewConnectionV2 = MSN_ConnectionProc;
		nlb.pExtra = this;

		sb = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hNetlibUser, (LPARAM)&nlb);
		if (sb == NULL)
			debugLogA("Unable to bind the port for incoming transfers");
	}

	char hostname[256] = "";
	gethostname(hostname, sizeof(hostname));
	PHOSTENT he = gethostbyname(hostname);

	const PIN_ADDR addr = (PIN_ADDR)he->h_addr_list[0];
	if (addr)
		strcpy(hostname, inet_ntoa(*addr));
	else
		hostname[0] = 0;

	char command[1024];
	int  nBytes = mir_snprintf(command, sizeof(command),
		"MIME-Version: 1.0\r\n"
		"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
		"Invitation-Command: %s\r\n"
		"Invitation-Cookie: %s\r\n"
		"IP-Address: %s\r\n"
		"IP-Address-Internal: %s\r\n"
		"Port: %i\r\n"
		"PortX: %i\r\n"
		"PortX-Internal: %i\r\n"
		"AuthCookie: %i\r\n"
		"Launch-Application: FALSE\r\n"
		"Request-Data: IP-Address:\r\n\r\n",
		sb && MyConnection.extIP ? "ACCEPT" : "CANCEL",
		Invcookie, MyConnection.GetMyExtIPStr(), hostname,
		nlb.wExPort, nlb.wExPort ^ 0x3141, nlb.wPort ^ 0x3141,
		MSN_GenRandom());

	info->sendPacket("MSG", "N %d\r\n%s", nBytes, command);

	if (sb) {
		ThreadData* newThread = new ThreadData;
		newThread->mType = SERVER_FILETRANS;
		newThread->mCaller = 2;
		newThread->mMsnFtp = ft;
		newThread->mIncomingBoundPort = sb;
		newThread->mIncomingPort = nlb.wPort;
		newThread->startThread(&CMsnProto::msnftp_sendFileThread, this);
	}
	else delete ft;
}
