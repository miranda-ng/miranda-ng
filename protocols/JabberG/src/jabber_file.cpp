/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
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
#include "jabber_caps.h"

#define JABBER_NETWORK_BUFFER_SIZE 2048

void __cdecl CJabberProto::FileReceiveThread(filetransfer *ft)
{
	ThreadData info(this, JABBER_SESSION_NORMAL);

	debugLogA("Thread started: type=file_receive server='%s' port='%d'", ft->httpHostName, ft->httpPort);

	ft->type = FT_OOB;

	NETLIBOPENCONNECTION nloc = { 0 };
	nloc.cbSize = sizeof(nloc);
	nloc.cbSize = sizeof(NETLIBOPENCONNECTION);
	nloc.szHost = ft->httpHostName;
	nloc.wPort = ft->httpPort;
	info.s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&nloc);
	if (info.s == NULL) {
		debugLogA("Connection failed (%d), thread ended", WSAGetLastError());
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
	}
	else {
		ft->s = info.s;

		char buffer[JABBER_NETWORK_BUFFER_SIZE];
		int datalen = mir_snprintf(buffer, SIZEOF(buffer), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", ft->httpPath, ft->httpHostName);
		info.send(buffer, datalen);
		ft->state = FT_CONNECTING;

		debugLogA("Entering file_receive recv loop");

		datalen = 0;
		while (ft->state != FT_DONE && ft->state != FT_ERROR) {
			int recvResult, bytesParsed;

			debugLogA("Waiting for data...");
			recvResult = info.recv(buffer+datalen, JABBER_NETWORK_BUFFER_SIZE-datalen);
			if (recvResult <= 0)
				break;
			datalen += recvResult;

			bytesParsed = FileReceiveParse(ft, buffer, datalen);
			if (bytesParsed < datalen)
				memmove(buffer, buffer+bytesParsed, datalen-bytesParsed);
			datalen -= bytesParsed;
		}

		ft->s = NULL;

		if (ft->state==FT_DONE || (ft->state==FT_RECEIVING && ft->std.currentFileSize < 0))
			ft->complete();

		debugLogA("Thread ended: type=file_receive server='%s'", ft->httpHostName);
	}
	delete ft;
}

int CJabberProto::FileReceiveParse(filetransfer *ft, char* buffer, int datalen)
{
	char* p, *q, *s, *eob;
	char* str;
	int num, code;

	eob = buffer + datalen;
	p = buffer;
	num = 0;
	while (true) {
		if (ft->state==FT_CONNECTING || ft->state==FT_INITIALIZING) {
			for (q=p; q+1<eob && (*q!='\r' || *(q+1)!='\n'); q++);
			if (q+1 < eob) {
				if ((str=(char*)mir_alloc(q-p+1)) != NULL) {
					strncpy_s(str, q - p, p, _TRUNCATE);
					str[q-p] = '\0';
					debugLogA("FT Got: %s", str);
					if (ft->state == FT_CONNECTING) {
						// looking for "HTTP/1.1 200 OK"
						if (sscanf(str, "HTTP/%*d.%*d %d %*s", &code)==1 && code==200) {
							ft->state = FT_INITIALIZING;
							ft->std.currentFileSize = -1;
							debugLogA("Change to FT_INITIALIZING");
							ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft, 0);
						}
					}
					else {	// FT_INITIALIZING
						if (str[0] == '\0') {
							TCHAR *s;
							if ((s = _tcsrchr(ft->httpPath, '/')) != NULL)
								s++;
							else
								s = ft->httpPath;
							ft->std.tszCurrentFile = mir_tstrdup(s);
							JabberHttpUrlDecode(ft->std.tszCurrentFile);
							if (ft->create() == -1) {
								ft->state = FT_ERROR;
								break;
							}
							ft->state = FT_RECEIVING;
							ft->std.currentFileProgress = 0;
							debugLogA("Change to FT_RECEIVING");
						}
						else if ((s=strchr(str, ':')) != NULL) {
							*s = '\0';
							if (!strcmp(str, "Content-Length"))
								ft->std.totalBytes = ft->std.currentFileSize = _atoi64(s+1);
					}	}

					mir_free(str);
					q += 2;
					num += (q-p);
					p = q;
				}
				else {
					ft->state = FT_ERROR;
					break;
				}
			}
			else {
				break;
			}
		}
		else if (ft->state == FT_RECEIVING) {
			int bufferSize, writeSize;
            __int64 remainingBytes;

			if (ft->std.currentFileSize < 0 || ft->std.currentFileProgress < ft->std.currentFileSize) {
				bufferSize = eob - p;
				remainingBytes = ft->std.currentFileSize - ft->std.currentFileProgress;
				if (remainingBytes < bufferSize)
					writeSize = remainingBytes;
				else
					writeSize = bufferSize;
				if (_write(ft->fileId, p, writeSize) != writeSize) {
					debugLogA("_write() error");
					ft->state = FT_ERROR;
				}
				else {
					ft->std.currentFileProgress += writeSize;
					ft->std.totalProgress += writeSize;
					ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
					if (ft->std.currentFileProgress == ft->std.currentFileSize)
						ft->state = FT_DONE;
				}
			}
			num = datalen;
			break;
		}
		else break;
	}

	return num;
}

void JabberFileServerConnection(JABBER_SOCKET hConnection, DWORD /*dwRemoteIP*/, void* extra)
{
	CJabberProto *ppro = (CJabberProto*)extra;

	NETLIBCONNINFO connInfo = { sizeof(connInfo) };
	CallService(MS_NETLIB_GETCONNECTIONINFO, (WPARAM)hConnection, (LPARAM)&connInfo);

	TCHAR szPort[10];
	mir_sntprintf(szPort, SIZEOF(szPort), _T("%d"), connInfo.wPort);
	ppro->debugLogA("File server incoming connection accepted: %s", connInfo.szIpPort);

	JABBER_LIST_ITEM *item = ppro->ListGetItemPtr(LIST_FILE, szPort);
	if (item == NULL) {
		ppro->debugLogA("No file is currently served, file server connection closed.");
		Netlib_CloseHandle(hConnection);
		return;
	}

	filetransfer *ft = item->ft;
	JABBER_SOCKET slisten = ft->s;
	ft->s = hConnection;
	ppro->debugLogA("Set ft->s to %d (saving %d)", hConnection, slisten);

	char* buffer = (char*)mir_alloc(JABBER_NETWORK_BUFFER_SIZE+1);
	if (buffer == NULL) {
		ppro->debugLogA("Cannot allocate network buffer, file server connection closed.");
		Netlib_CloseHandle(hConnection);
		ft->state = FT_ERROR;
		if (ft->hFileEvent != NULL)
			SetEvent(ft->hFileEvent);
		return;
	}

	ppro->debugLogA("Entering recv loop for this file connection... (ft->s is hConnection)");
	int datalen = 0;
	while (ft->state!=FT_DONE && ft->state!=FT_ERROR) {
		int recvResult, bytesParsed;

		recvResult = Netlib_Recv(hConnection, buffer+datalen, JABBER_NETWORK_BUFFER_SIZE-datalen, 0);
		if (recvResult <= 0)
			break;
		datalen += recvResult;

		buffer[datalen] = '\0';
		ppro->debugLogA("RECV:%s", buffer);

		bytesParsed = ppro->FileSendParse(hConnection, ft, buffer, datalen);
		if (bytesParsed < datalen)
			memmove(buffer, buffer+bytesParsed, datalen-bytesParsed);
		datalen -= bytesParsed;
	}

	ppro->debugLogA("Closing connection for this file transfer... (ft->s is now hBind)");
	Netlib_CloseHandle(hConnection);
	ft->s = slisten;
	ppro->debugLogA("ft->s is restored to %d", ft->s);
	if (ft->hFileEvent != NULL)
		SetEvent(ft->hFileEvent);
	mir_free(buffer);
}

void __cdecl CJabberProto::FileServerThread(filetransfer *ft)
{
	debugLogA("Thread started: type=file_send");

	ThreadData info(this, JABBER_SESSION_NORMAL);
	ft->type = FT_OOB;

	NETLIBBIND nlb = {0};
	nlb.cbSize = sizeof(NETLIBBIND);
	nlb.pfnNewConnectionV2 = JabberFileServerConnection;
	nlb.pExtra = this;
	nlb.wPort = 0;	// Use user-specified incoming port ranges, if available
	info.s = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hNetlibUser, (LPARAM)&nlb);
	if (info.s == NULL) {
		debugLogA("Cannot allocate port to bind for file server thread, thread ended.");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		delete ft;
		return;
	}

	ft->s = info.s;
	debugLogA("ft->s = %d", info.s);

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ft->hFileEvent = hEvent;

	TCHAR szPort[20];
	mir_sntprintf(szPort, SIZEOF(szPort), _T("%d"), nlb.wPort);
	JABBER_LIST_ITEM *item = ListAdd(LIST_FILE, szPort);
	item->ft = ft;

	TCHAR *ptszResource = ListGetBestClientResourceNamePtr(ft->jid);
	if (ptszResource != NULL) {
		ft->state = FT_CONNECTING;
		for (int i=0; i < ft->std.totalFiles && ft->state != FT_ERROR && ft->state != FT_DENIED; i++) {
			ft->std.currentFileNumber = i;
			ft->state = FT_CONNECTING;
			if (ft->httpPath) mir_free(ft->httpPath);
			ft->httpPath = NULL;

			TCHAR *p;
			if ((p = _tcschr(ft->std.ptszFiles[i], '\\')) != NULL)
				p++;
			else
				p = ft->std.ptszFiles[i];

			char *pFileName = mir_urlEncode( ptrA( mir_utf8encodeT(p)));
			if (pFileName != NULL) {
				ft->szId = JabberId2string(SerialNext());

				ptrA myAddr;
				if (m_options.BsDirect && m_options.BsDirectManual)
					myAddr = getStringA("BsDirectAddr");
				if (myAddr == NULL)
					myAddr = (char*)CallService(MS_NETLIB_ADDRESSTOSTRING, 1, nlb.dwExternalIP);

				char szAddr[256];
				mir_snprintf(szAddr, sizeof(szAddr), "http://%s:%d/%s", myAddr, nlb.wPort, pFileName);

				mir_free(pFileName);

				int len = lstrlen(ptszResource) + lstrlen(ft->jid) + 2;
				TCHAR *fulljid = (TCHAR *)alloca(sizeof(TCHAR) * len);
				mir_sntprintf(fulljid, len, _T("%s/%s"), ft->jid, ptszResource);

				XmlNodeIq iq(_T("set"), ft->szId, fulljid);
				HXML query = iq << XQUERY(JABBER_FEAT_OOB);
				query << XCHILD(_T("url"), _A2T(szAddr));
				query << XCHILD(_T("desc"), ft->szDescription);
				m_ThreadInfo->send(iq);

				debugLogA("Waiting for the file to be sent...");
				WaitForSingleObject(hEvent, INFINITE);
			}
			debugLogA("File sent, advancing to the next file...");
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
		}
		CloseHandle(hEvent);
		ft->hFileEvent = NULL;
		debugLogA("Finish all files");
	}

	ft->s = NULL;
	debugLogA("ft->s is NULL");

	ListRemove(LIST_FILE, szPort);

	switch (ft->state) {
	case FT_DONE:
		debugLogA("Finish successfully");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		break;
	case FT_DENIED:
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ft, 0);
		break;
	default: // FT_ERROR:
		debugLogA("Finish with errors");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		break;
	}

	debugLogA("Thread ended: type=file_send");
	delete ft;
}

int CJabberProto::FileSendParse(JABBER_SOCKET s, filetransfer *ft, char* buffer, int datalen)
{
	char* p, *q, *t, *eob;
	char* str;
	int num;
	int currentFile;
	int fileId;
	int numRead;

	eob = buffer + datalen;
	p = buffer;
	num = 0;
	while (ft->state==FT_CONNECTING || ft->state==FT_INITIALIZING) {
		for (q=p; q+1<eob && (*q!='\r' || *(q+1)!='\n'); q++);
		if (q+1 >= eob)
			break;
		if ((str=(char*)mir_alloc(q-p+1)) == NULL) {
			ft->state = FT_ERROR;
			break;
		}
		strncpy_s(str, q - p, p, _TRUNCATE);
		str[q-p] = '\0';
		debugLogA("FT Got: %s", str);
		if (ft->state == FT_CONNECTING) {
			// looking for "GET filename.ext HTTP/1.1"
			if (!strncmp(str, "GET ", 4)) {
				for (t=str+4; *t!='\0' && *t!=' '; t++);
				*t = '\0';
				for (t=str+4; *t!='\0' && *t=='/'; t++);
				ft->httpPath = mir_a2t(t);
				JabberHttpUrlDecode(ft->httpPath);
				ft->state = FT_INITIALIZING;
				debugLogA("Change to FT_INITIALIZING");
			}
		}
		else {	// FT_INITIALIZING
			if (str[0] == '\0') {
				struct _stati64 statbuf;

				mir_free(str);
				num += 2;

				currentFile = ft->std.currentFileNumber;
				TCHAR *t = _tcsrchr(ft->std.ptszFiles[ currentFile ], '\\');
				if (t != NULL)
					t++;
				else
					t = ft->std.ptszFiles[currentFile];

				if (ft->httpPath == NULL || lstrcmp(ft->httpPath, t)) {
					if (ft->httpPath == NULL)
						debugLogA("Requested file name does not matched (httpPath == NULL)");
					else
						debugLog(_T("Requested file name does not matched ('%s' vs. '%s')"), ft->httpPath, t);
					ft->state = FT_ERROR;
					break;
				}
				debugLog(_T("Sending [%s]"), ft->std.ptszFiles[ currentFile ]);
				_tstati64(ft->std.ptszFiles[ currentFile ], &statbuf);	// file size in statbuf.st_size
				if ((fileId = _topen(ft->std.ptszFiles[currentFile], _O_BINARY|_O_RDONLY)) < 0) {
					debugLogA("File cannot be opened");
					ft->state = FT_ERROR;
					mir_free(ft->httpPath);
					ft->httpPath = NULL;
					break;
				}

				char fileBuffer[ 2048 ];
				int bytes = mir_snprintf(fileBuffer, sizeof(fileBuffer), "HTTP/1.1 200 OK\r\nContent-Length: %I64u\r\n\r\n", statbuf.st_size);
				WsSend(s, fileBuffer, bytes, MSG_DUMPASTEXT);

				ft->std.flags |= PFTS_SENDING;
				ft->std.currentFileProgress = 0;
				debugLogA("Sending file data...");

				while ((numRead = _read(fileId, fileBuffer, 2048)) > 0) {
					if (Netlib_Send(s, fileBuffer, numRead, 0) != numRead) {
						ft->state = FT_ERROR;
						break;
					}
					ft->std.currentFileProgress += numRead;
					ft->std.totalProgress += numRead;
					ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
				}
				_close(fileId);
				if (ft->state != FT_ERROR)
					ft->state = FT_DONE;
				debugLogA("Finishing this file...");
				mir_free(ft->httpPath);
				ft->httpPath = NULL;
				break;
		}	}

		mir_free(str);
		q += 2;
		num += (q-p);
		p = q;
	}

	return num;
}

/////////////////////////////////////////////////////////////////////////////////////////
// filetransfer class members

filetransfer::filetransfer(CJabberProto* proto)
{
	memset(this, 0, sizeof(filetransfer));
	ppro = proto;
	fileId = -1;
	std.cbSize = sizeof(std);
	std.flags = PFTS_TCHAR;
}

filetransfer::~filetransfer()
{
	ppro->debugLogA("Destroying file transfer session %08p", this);

	if (!bCompleted)
		ppro->ProtoBroadcastAck(std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, this, 0);

	close();

	if (hWaitEvent != INVALID_HANDLE_VALUE)
		CloseHandle(hWaitEvent);

	mir_free(szId);
	mir_free(jid);
	mir_free(sid);
	mir_free(fileSize);
	mir_free(httpHostName);
	mir_free(httpPath);
	mir_free(szDescription);

	mir_free(std.tszWorkingDir);
	mir_free(std.tszCurrentFile);

	if (std.ptszFiles) {
		for (int i=0; i < std.totalFiles; i++)
			mir_free(std.ptszFiles[i]);
		mir_free(std.ptszFiles);
}	}

void filetransfer::close()
{
	if (fileId != -1) {
		_close(fileId);
		fileId = -1;
}	}

void filetransfer::complete()
{
	close();

	bCompleted = true;
	ppro->ProtoBroadcastAck(std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, this, 0);
}

int filetransfer::create()
{
	if (fileId != -1)
		return fileId;

	TCHAR filefull[ MAX_PATH ];
	mir_sntprintf(filefull, SIZEOF(filefull), _T("%s\\%s"), std.tszWorkingDir, std.tszCurrentFile);
	replaceStrT(std.tszCurrentFile, filefull);

	if (hWaitEvent != INVALID_HANDLE_VALUE)
		CloseHandle(hWaitEvent);
	hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (ppro->ProtoBroadcastAck(std.hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, this, (LPARAM)&std))
		WaitForSingleObject(hWaitEvent, INFINITE);

	if (fileId == -1) {
		ppro->debugLog(_T("Saving to [%s]"), std.tszCurrentFile);
		fileId = _topen(std.tszCurrentFile, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE);
	}

	if (fileId == -1)
		ppro->debugLog(_T("Cannot create file '%s' during a file transfer"), filefull);
	else if (std.currentFileSize != 0)
		_chsize(fileId, std.currentFileSize);

	return fileId;
}
