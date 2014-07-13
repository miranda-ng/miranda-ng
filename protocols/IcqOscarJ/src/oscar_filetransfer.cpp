// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  OSCAR File-Transfers implementation
// -----------------------------------------------------------------------------

#include "icqoscar.h"

struct oscarthreadstartinfo 
{
	int type;
	int incoming;
	MCONTACT hContact;
	HANDLE hConnection;
	DWORD dwRemoteIP;
	oscar_filetransfer *ft;
	oscar_listener *listener;
};


// small utility function
extern void NormalizeBackslash(char* path);

//
// Common functions
/////////////////////////////

char *FindFilePathContainer(const char **files, int iFile, char *szContainer)
{
	const char *szThisFile = files[iFile];
	char *szFileName = (char*)ExtractFileName(szThisFile);

	szContainer[0] = '\0';

	if (szThisFile != szFileName)
	{ // find an earlier subdirectory to be used as a container
		for (int i = iFile - 1; i >= 0; i--)
		{
			int len = strlennull(files[i]);

			if (!_strnicmp(files[i], szThisFile, len) && (szThisFile[len] == '\\' || szThisFile[len] == '/'))
			{
				const char *pszLastBackslash;

				if (((pszLastBackslash = strrchr(files[i], '\\')) == NULL) &&
					((pszLastBackslash = strrchr(files[i], '/')) == NULL))
				{
					strcpy(szContainer, files[i]);
				}
				else
				{
					len = pszLastBackslash - files[i] + 1;
					null_strcpy(szContainer, szThisFile + len,  szFileName - szThisFile - len);
				}
			}
		}
	}
	return szFileName;
}


//
// Utility functions
/////////////////////////////

oscar_filetransfer* CIcqProto::CreateOscarTransfer()
{
	oscar_filetransfer* ft = (oscar_filetransfer*)SAFE_MALLOC(sizeof(oscar_filetransfer));

	ft->ft_magic = FT_MAGIC_OSCAR; // Setup signature
	// Init members
	ft->fileId = -1;

	icq_lock l(oftMutex);

	fileTransferList = (basic_filetransfer**)SAFE_REALLOC(fileTransferList, sizeof(basic_filetransfer*)*(fileTransferCount + 1));
	fileTransferList[fileTransferCount++] = ft;

#ifdef _DEBUG
	NetLog_Direct("OFT: FT struct 0x%x created", ft);
#endif

	return ft;
}


filetransfer *CIcqProto::CreateIcqFileTransfer()
{
	filetransfer *ft = (filetransfer*)SAFE_MALLOC(sizeof(filetransfer));

	ft->ft_magic = FT_MAGIC_ICQ;

	icq_lock l(oftMutex);

	fileTransferList = (basic_filetransfer**)SAFE_REALLOC(fileTransferList, sizeof(basic_filetransfer*)*(fileTransferCount + 1));
	fileTransferList[fileTransferCount++] = (basic_filetransfer*)ft;

#ifdef _DEBUG
	NetLog_Direct("FT struct 0x%x created", ft);
#endif

	return ft;
}


int CIcqProto::getFileTransferIndex(void *ft)
{
	for (int i = 0; i < fileTransferCount; i++)
	{
		if (fileTransferList[i] == ft)
			return i;
	}
	return -1;
}


void CIcqProto::ReleaseFileTransfer(void *ft)
{
	int i = getFileTransferIndex(ft);

	if (i != -1)
	{
		fileTransferCount--;
		fileTransferList[i] = fileTransferList[fileTransferCount];
		fileTransferList = (basic_filetransfer**)SAFE_REALLOC(fileTransferList, sizeof(basic_filetransfer*)*fileTransferCount);
	}
}


int CIcqProto::IsValidFileTransfer(void *ft)
{
	icq_lock l(oftMutex);

	if (getFileTransferIndex(ft) != -1) return 1;

	return 0;
}


int CIcqProto::IsValidOscarTransfer(void *ft)
{
	icq_lock l(oftMutex);

	if (getFileTransferIndex(ft) != -1 && ((basic_filetransfer*)ft)->ft_magic == FT_MAGIC_OSCAR)
		return 1;

	return 0;
}


oscar_filetransfer* CIcqProto::FindOscarTransfer(MCONTACT hContact, DWORD dwID1, DWORD dwID2)
{
	icq_lock l(oftMutex);

	for (int i = 0; i < fileTransferCount; i++)
	{
		if (fileTransferList[i]->ft_magic == FT_MAGIC_OSCAR)
		{
			oscar_filetransfer *oft = (oscar_filetransfer*)fileTransferList[i];

			if (oft->hContact == hContact && oft->pMessage.dwMsgID1 == dwID1 && oft->pMessage.dwMsgID2 == dwID2)
				return oft;
		}
	}

	return NULL;
}


// Release file transfer structure
void CIcqProto::SafeReleaseFileTransfer(void **ft)
{
	basic_filetransfer **bft = (basic_filetransfer**)ft;

	icq_lock l(oftMutex);

	// Check for filetransfer validity
	if (getFileTransferIndex(*ft) == -1)
		return;

	if (*bft)
	{
		if ((*bft)->ft_magic == FT_MAGIC_ICQ)
		{ // release ICQ filetransfer structure and its contents
			filetransfer *ift = (filetransfer*)(*bft);

			SAFE_FREE(&ift->szFilename);
			SAFE_FREE(&ift->szDescription);
			SAFE_FREE(&ift->szSavePath);
			SAFE_FREE(&ift->szThisSubdir);
			if (ift->pszFiles)
			{
				for (int i = 0; i < (int)ift->dwFileCount; i++)
				{
					// szThisFile can be a duplicate of pszFiles[i]
					if (ift->szThisFile == ift->pszFiles[i])
						ift->szThisFile = NULL;
					SAFE_FREE(&ift->pszFiles[i]);
				}
				SAFE_FREE((void**)&ift->pszFiles);
			}
			SAFE_FREE(&ift->szThisFile);
			// Invalidate transfer
			ReleaseFileTransfer(ift);
#ifdef _DEBUG
			NetLog_Direct("FT struct 0x%x released", ft);
#endif
			// Release memory
			SAFE_FREE((void**)ft);
		}
		else if ((*bft)->ft_magic == FT_MAGIC_OSCAR)
		{ // release oscar filetransfer structure and its contents
			oscar_filetransfer *oft = (oscar_filetransfer*)(*bft);
			// If connected, close connection
			if (oft->connection)
				CloseOscarConnection(oft->connection);
			// Release oscar listener
			if (oft->listener)
				ReleaseOscarListener((oscar_listener**)&oft->listener);
			// Release cookie
			if (oft->dwCookie)
				FreeCookie(oft->dwCookie);
			// Release all dynamic members
			SAFE_FREE(&oft->rawFileName);
			SAFE_FREE(&oft->szSavePath);
			SAFE_FREE(&oft->szThisFile);
			SAFE_FREE(&oft->szThisPath);
			SAFE_FREE(&oft->szDescription);
			if (oft->files)
			{
				for (int i = 0; i < oft->wFilesCount; i++)
					SAFE_FREE(&oft->files[i].szFile);
				SAFE_FREE((void**)&oft->files);
			}
			if (oft->files_list)
			{
/*				for (int i = 0; i < oft->wFilesCount; i++)
					SAFE_FREE(&oft->files_list[i]);*/
				SAFE_FREE((void**)&oft->files_list);
			}
			if (oft->file_containers)
			{
				for (int i = 0; i < oft->containerCount; i++)
					SAFE_FREE(&oft->file_containers[i]);
				SAFE_FREE((void**)&oft->file_containers);
			}
			if (oft->fileId != -1)
			{
#ifdef _DEBUG
				NetLog_Direct("OFT: _close(%u)", oft->fileId);
#endif
				_close(oft->fileId);
			}
			// Invalidate transfer
			ReleaseFileTransfer(oft);
#ifdef _DEBUG
			NetLog_Direct("OFT: FT struct 0x%x released", ft);
#endif
			// Release memory
			SAFE_FREE((void**)ft);
		}
	}
}


// Calculate oft checksum of buffer
// --------------------------------
// Information was gathered from Gaim's sources, thanks
//
DWORD oft_calc_checksum(int offset, const BYTE *buffer, int len, DWORD dwChecksum)
{
	DWORD checksum = (dwChecksum >> 16) & 0xffff;

	for (int i = 0; i < len; i++)
	{
		WORD val = buffer[i];
		DWORD oldchecksum = checksum;

		if (((i + offset) & 1) == 0)
			val = val << 8;

		if (checksum < val)
			checksum -= val + 1;
		else // simulate carry
			checksum -= val;
	}
	checksum = ((checksum & 0x0000ffff) + (checksum >> 16));
	checksum = ((checksum & 0x0000ffff) + (checksum >> 16));
	return checksum << 16;
}


DWORD oft_calc_file_checksum(int hFile, __int64 maxSize)
{
	BYTE buf[OFT_BUFFER_SIZE];
	int bytesRead;
	__int64 offset = 0;
	DWORD dwCheck = 0xFFFF0000;

	_lseek(hFile, 0, SEEK_SET);
	bytesRead = _read(hFile, buf, (maxSize < sizeof(buf)) ? maxSize : (unsigned)sizeof(buf));
	if (bytesRead == -1)
		return dwCheck;

	while(bytesRead)
	{
		dwCheck = oft_calc_checksum((int)offset, buf, bytesRead, dwCheck);
		offset += bytesRead;
		bytesRead = _read(hFile, buf, sizeof(buf));
		if (bytesRead + offset > maxSize) bytesRead = (int)(maxSize - offset);
	}
	_lseek(hFile, 0, SEEK_SET); // back to beginning

	return dwCheck;
}


oscar_listener* CIcqProto::CreateOscarListener(oscar_filetransfer *ft, NETLIBNEWCONNECTIONPROC_V2 handler)
{
	oscar_listener *listener = (oscar_listener*)SAFE_MALLOC(sizeof(oscar_listener));

	if (listener)
	{
		listener->ppro = this;
		listener->ft = ft;
		if (listener->hBoundPort = NetLib_BindPort(handler, listener, &listener->wPort, NULL))
			return listener; // Success

		SAFE_FREE((void**)&listener);
	}

	return NULL; // Failure
}


void CIcqProto::ReleaseOscarListener(oscar_listener **pListener)
{
	oscar_listener *listener = *pListener;

	if (listener)
	{ // Close listening port
		if (listener->hBoundPort)
			NetLib_SafeCloseHandle(&listener->hBoundPort);

		NetLog_Direct("Oscar listener on port %d released.", listener->wPort);
	}
	SAFE_FREE((void**)pListener);
}


//
// Miranda FT interface handlers & services
/////////////////////////////

void CIcqProto::handleRecvServMsgOFT(BYTE *buf, WORD wLen, DWORD dwUin, char *szUID, DWORD dwID1, DWORD dwID2, WORD wCommand)
{
	MCONTACT hContact = HContactFromUID(dwUin, szUID, NULL);

	if (wCommand == 0)
	{ // this is OFT request
		oscar_tlv_chain* chain = readIntoTLVChain(&buf, wLen, 0);

		if (chain)
		{
			WORD wAckType = chain->getWord(0x0A, 1);

			if (wAckType == 1)
			{ // This is first request in this OFT
				oscar_filetransfer *ft = CreateOscarTransfer();
				char *pszFileName = NULL;
				char *pszDescription = NULL;
				WORD wFilenameLength;

				debugLogA("This is a file request");

				// This TLV chain may contain the following TLVs:
				// TLV(A): Acktype 0x0001 - file request / abort request
				//                 0x0002 - file ack
				// TLV(F): Unknown
				// TLV(E): Language ?
				// TLV(2): Proxy IP
				// TLV(16): Proxy IP Check
				// TLV(3): External IP
				// TLV(4): Internal IP
				// TLV(5): Port
				// TLV(17): Port Check
				// TLV(10): Proxy Flag
				// TLV(D): Charset of User Message
				// TLV(C): User Message (ICQ_COOL_FT)
				// TLV(2711): FT info
				// TLV(2712): Charset of file name

				// init filetransfer structure
				ft->pMessage.dwMsgID1 = dwID1;
				ft->pMessage.dwMsgID2 = dwID2;
				ft->bUseProxy = chain->getTLV(0x10, 1) ? 1 : 0;
				ft->dwProxyIP = chain->getDWord(0x02, 1);
				ft->dwRemoteInternalIP = chain->getDWord(0x03, 1);
				ft->dwRemoteExternalIP = chain->getDWord(0x04, 1);
				ft->wRemotePort = chain->getWord(0x05, 1);
				ft->wReqNum = wAckType;

				{ // User Message
					oscar_tlv* tlv = chain->getTLV(0x0C, 1);

					if (tlv)
					{ // parse User Message
						BYTE* tBuf = tlv->pData;

						pszDescription = (char*)_alloca(tlv->wLen + 2);
						unpackString(&tBuf, (char*)pszDescription, tlv->wLen);
						pszDescription[tlv->wLen] = '\0';
						pszDescription[tlv->wLen+1] = '\0';
						{ // apply User Message encoding
							oscar_tlv *charset = chain->getTLV(0x0D, 1);
							char *str = pszDescription;
							char *bTag,*eTag;

							if (charset)
							{ // decode charset
								char *szEnc = (char*)_alloca(charset->wLen + 1);

								null_strcpy(szEnc, (char*)charset->pData, charset->wLen);
								str = ApplyEncoding((char*)pszDescription, szEnc);
							}
							else 
								str = null_strdup(str);
							// eliminate HTML tags
							pszDescription = EliminateHtml(str, strlennull(str));

							bTag = strstrnull(pszDescription, "<DESC>");
							if (bTag)
							{ // take special Description - ICQJ's extension
								eTag = strstrnull(bTag, "</DESC>");
								if (eTag)
								{
									*eTag = '\0';
									str = null_strdup(bTag + 6);
									SAFE_FREE(&pszDescription);
									pszDescription = str;
								}
							}
							else
							{
								bTag = strstrnull(pszDescription, "<FS>");
								if (bTag)
								{ // take only <FS> - Description tag if present
									eTag = strstrnull(bTag, "</FS>");
									if (eTag)
									{
										*eTag = '\0';
										str = null_strdup(bTag + 4);
										SAFE_FREE(&pszDescription);
										pszDescription = str;
									}
								}
							}
						}
					}
					if (!strlennull(pszDescription))
					{
						SAFE_FREE(&pszDescription);
						pszDescription = ICQTranslateUtf(LPGEN("No description given"));
					}
				}
				{ // parse File Transfer Info block
					oscar_tlv* tlv = chain->getTLV(0x2711, 1);

					// sanity check
					if (!tlv || tlv->wLen < 8)
					{
						debugLogA("Error: Malformed file request");
						// release structures
						SafeReleaseFileTransfer((void**)&ft);
						SAFE_FREE(&pszDescription);
						return;
					}

					BYTE* tBuf = tlv->pData;
					WORD tLen = tlv->wLen;
					WORD wFlag;

					unpackWord(&tBuf, &wFlag); // FT flag
					unpackWord(&tBuf, &ft->wFilesCount);
					unpackDWord(&tBuf, (DWORD*)&ft->qwTotalSize);
					tLen -= 8;
					// Filename / Directory Name
					if (tLen)
					{ // some filename specified, unpack
						wFilenameLength = tLen - 1;
						pszFileName = (char*)_alloca(tLen);
						unpackString(&tBuf, (char*)pszFileName, wFilenameLength);
						pszFileName[wFilenameLength] = '\0';
					}
					else if (ft->wFilesCount == 1) // give some generic file name
						pszFileName = "unnamed_file";
					else // or empty directory name
						pszFileName = "";

					// apply Filename / Directory Name encoding
					oscar_tlv* charset = chain->getTLV(0x2712, 1);
					if (charset) {
						char* szEnc = (char*)_alloca(charset->wLen + 1);
						null_strcpy(szEnc, (char*)charset->pData, charset->wLen);
						pszFileName = ApplyEncoding(pszFileName, szEnc);
					}
					else pszFileName = ansi_to_utf8(pszFileName);

					if (ft->wFilesCount == 1)
					{  // Filename - use for DB event
						char *szFileName = (char*)_alloca(strlennull(pszFileName) + 1);

						strcpy(szFileName, pszFileName);
						SAFE_FREE(&pszFileName);
						pszFileName = szFileName;
					}
					else
					{  // Save Directory name for future use
						ft->szThisPath = pszFileName;
						// for multi-file transfer we do not display "folder" name, but create only a simple notice
						pszFileName = (char*)_alloca(64);

						char tmp[64];
						mir_snprintf(pszFileName, 64, ICQTranslateUtfStatic(LPGEN("%d Files"), tmp, SIZEOF(tmp)), ft->wFilesCount);
					}
				}
				// Total Size TLV (ICQ 6 and AIM 6)
				{ 
					oscar_tlv *tlv = chain->getTLV(0x2713, 1);

					if (tlv && tlv->wLen >= 8)
					{
						BYTE *tBuf = tlv->pData;

						unpackQWord(&tBuf, &ft->qwTotalSize);
					}
				}
				int bAdded;
				MCONTACT hContact = HContactFromUID(dwUin, szUID, &bAdded);

				ft->hContact = hContact;
				ft->fileId = -1;

				// Send chain event
				char *szBlob = (char*)_alloca(sizeof(DWORD) + strlennull(pszFileName) + strlennull(pszDescription) + 2);
				*(PDWORD)szBlob = 0;
				strcpy(szBlob + sizeof(DWORD), pszFileName);
				strcpy(szBlob + sizeof(DWORD) + strlennull(pszFileName) + 1, pszDescription);

				TCHAR* ptszFileName = mir_utf8decodeT(pszFileName);

				PROTORECVFILET pre = {0};
				pre.flags = PREF_TCHAR;
				pre.fileCount = 1;
				pre.timestamp = time(NULL);
				pre.tszDescription = mir_utf8decodeT(pszDescription);
				pre.ptszFiles = &ptszFileName;
				pre.lParam = (LPARAM)ft;
				ProtoChainRecvFile(hContact, &pre);

				mir_free(pre.tszDescription);
				mir_free(ptszFileName);
			}
			else if (wAckType == 2)
			{ // First attempt failed, reverse requested
				oscar_filetransfer *ft = FindOscarTransfer(hContact, dwID1, dwID2);

				if (ft)
				{
					NetLog_Direct("OFT: Redirect received (%d)", wAckType);

					ft->wReqNum = wAckType;

					if (ft->flags & OFTF_SENDING)
					{
						ReleaseOscarListener((oscar_listener**)&ft->listener);

						ft->bUseProxy = chain->getTLV(0x10, 1) ? 1 : 0;
						ft->dwProxyIP = chain->getDWord(0x02, 1);
						ft->dwRemoteInternalIP = chain->getDWord(0x03, 1);
						ft->dwRemoteExternalIP = chain->getDWord(0x04, 1);
						ft->wRemotePort = chain->getWord(0x05, 1);

						OpenOscarConnection(hContact, ft, ft->bUseProxy ? OCT_PROXY_RECV: OCT_REVERSE);
					}
					else
					{ // Just sanity
						ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ft, 0);
						// Release transfer
						SafeReleaseFileTransfer((void**)&ft);
					}
				}
				else
					debugLogA("Error: Invalid request, no such transfer");
			}
			else if (wAckType == 3)
			{ // Transfering thru proxy, join tunnel
				oscar_filetransfer *ft = FindOscarTransfer(hContact, dwID1, dwID2);

				if (ft)
				{ // release possible previous listener
					NetLog_Direct("OFT: Redirect received (%d)", wAckType);

					ft->wReqNum = wAckType;

					ReleaseOscarListener((oscar_listener**)&ft->listener);

					ft->bUseProxy = chain->getTLV(0x10, 1) ? 1 : 0;
					ft->dwProxyIP = chain->getDWord(0x02, 1);
					ft->wRemotePort = chain->getWord(0x05, 1);

					if (ft->bUseProxy && ft->dwProxyIP)
					{ // Init proxy connection
						OpenOscarConnection(hContact, ft, OCT_PROXY_RECV);
					}
					else
					{ // try Stage 4
						OpenOscarConnection(hContact, ft, OCT_PROXY);
					}
				}
				else
					debugLogA("Error: Invalid request, no such transfer");
			}
			else if (wAckType == 4)
			{
				oscar_filetransfer *ft = FindOscarTransfer(hContact, dwID1, dwID2);

				if (ft)
				{
					NetLog_Direct("OFT: Redirect received (%d)", wAckType);

					ft->wReqNum = wAckType;
					ft->bUseProxy = chain->getTLV(0x10, 1) ? 1 : 0;
					ft->dwProxyIP = chain->getDWord(0x02, 1);
					ft->wRemotePort = chain->getWord(0x05, 1);

					if (ft->bUseProxy && ft->dwProxyIP)
					{ // Init proxy connection
						OpenOscarConnection(hContact, ft, OCT_PROXY_RECV);
					}
					else
						debugLogA("Error: Invalid request, IP missing.");
				}
				else
					debugLogA("Error: Invalid request, no such transfer");
			}
			else
				debugLogA("Error: Uknown Stage %d request", wAckType);

			disposeChain(&chain);
		}
		else
			debugLogA("Error: Missing TLV chain in OFT request");
	}
	else if (wCommand == 1)
	{ // transfer cancelled/aborted
		oscar_filetransfer *ft = FindOscarTransfer(hContact, dwID1, dwID2);

		if (ft)
		{
			debugLogA("OFT: File transfer cancelled by %s", strUID(dwUin, szUID));

			ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ft, 0);
			// Notify user, that the FT was cancelled // TODO: new ACKRESULT_?
			icq_LogMessage(LOG_ERROR, LPGEN("The file transfer was aborted by the other user."));
			// Release transfer
			SafeReleaseFileTransfer((void**)&ft);
		}
		else
			debugLogA("Error: Invalid request, no such transfer");
	}
	else if (wCommand == 2)
	{ // transfer accepted - connection established
		oscar_filetransfer *ft = FindOscarTransfer(hContact, dwID1, dwID2);

		if (ft)
		{
			NetLog_Direct("OFT: Session established.");
			// Init connection
			if (ft->flags & OFTF_SENDING)
			{
				if (ft->connection && ft->connection->status == OCS_CONNECTED)
				{
					if (!(ft->flags & OFTF_FILE_REQUEST_SENT))
					{
						ft->flags |= OFTF_FILE_REQUEST_SENT;
						// proceed with first file
						oft_sendPeerInit(ft->connection);
					}
				}
				ft->flags |= OFTF_INITIALIZED; // accept was received
			}
			else
				debugLogA("Warning: Received invalid rendezvous accept");
		}
		else
			debugLogA("Error: Invalid request, no such transfer");
	}
	else
	{
		debugLogA("Error: Unknown wCommand=0x%x in OFT request", wCommand);
	}
}


void CIcqProto::handleRecvServResponseOFT(BYTE *buf, WORD wLen, DWORD dwUin, char *szUID, void* ft)
{
	WORD wDataLen;

	if (wLen < 2) return;

	unpackWord(&buf, &wDataLen);

	if (wDataLen == 2)
	{
		oscar_filetransfer *oft = (oscar_filetransfer*)ft;
		WORD wStatus;

		unpackWord(&buf, &wStatus);

		switch (wStatus)
		{
		case 1:
			{ // FT denied (icq5)
				debugLogA("OFT: File transfer denied by %s", strUID(dwUin, szUID));

				ProtoBroadcastAck(oft->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)oft, 0);
				// Release transfer
				SafeReleaseFileTransfer((void**)&oft);
			}
			break;

		case 4: // Proxy error
			{
				icq_LogMessage(LOG_ERROR, LPGEN("The file transfer failed: Proxy error"));

				ProtoBroadcastAck(oft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oft, 0);
				// Release transfer
				SafeReleaseFileTransfer((void**)&oft);
			}
			break;

		case 5: // Invalid request
			{
				icq_LogMessage(LOG_ERROR, LPGEN("The file transfer failed: Invalid request"));

				ProtoBroadcastAck(oft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oft, 0);
				// Release transfer
				SafeReleaseFileTransfer((void**)&oft);
			}
			break;

		case 6: // Proxy Failed (IP = 0)
			{
				icq_LogMessage(LOG_ERROR, LPGEN("The file transfer failed: Proxy unavailable"));

				ProtoBroadcastAck(oft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oft, 0);
				// Release transfer
				SafeReleaseFileTransfer((void**)&oft);
			}
			break;

		default:
			{
				debugLogA("OFT: Uknown request response code 0x%x", wStatus);

				ProtoBroadcastAck(oft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oft, 0);
				// Release transfer
				SafeReleaseFileTransfer((void**)&oft);
			}
		}
	}
}


// This function is called from the Netlib when someone is connecting to our oscar_listener
static void oft_newConnectionReceived(HANDLE hNewConnection, DWORD dwRemoteIP, void *pExtra)
{
	oscarthreadstartinfo *otsi = (oscarthreadstartinfo*)SAFE_MALLOC(sizeof(oscarthreadstartinfo));
	oscar_listener *listener = (oscar_listener*)pExtra;

	otsi->type = listener->ft->flags & OFTF_SENDING ? OCT_NORMAL : OCT_REVERSE;
	otsi->incoming = 1;
	otsi->hConnection = hNewConnection;
	otsi->dwRemoteIP = dwRemoteIP;
	otsi->listener = listener;

	// Start a new thread for the incomming connection
	listener->ppro->ForkThread((CIcqProto::MyThreadFunc)&CIcqProto::oft_connectionThread, otsi );
}


static char *oftGetFileContainer(oscar_filetransfer* oft, const char** files, int iFile)
{
	char szPath[MAX_PATH];
	char* szFileName = FindFilePathContainer(files, iFile, szPath);
	char *szPathUtf = ansi_to_utf8(szPath);
	int i;

	// try to find existing container
	for (i = 0; i < oft->containerCount; i++)
		if (!strcmpnull(szPathUtf, oft->file_containers[i]))
		{
			SAFE_FREE((void**)&szPathUtf);

			return oft->file_containers[i];
		}

		// create new container
		i = oft->containerCount++;
		oft->file_containers = (char**)SAFE_REALLOC(oft->file_containers, (sizeof(char*) * oft->containerCount));
		oft->file_containers[i] = szPathUtf;

		return oft->file_containers[i];
}


HANDLE CIcqProto::oftInitTransfer(MCONTACT hContact, DWORD dwUin, char* szUid, const TCHAR** files, const TCHAR* pszDesc)
{
	int i, filesCount;
	struct _stati64 statbuf;

	// Initialize filetransfer struct
	debugLogA("Init file send");

	oscar_filetransfer *ft = CreateOscarTransfer();
	ft->hContact = hContact;
	ft->pMessage.bMessageType = MTYPE_FILEREQ;
	InitMessageCookie(&ft->pMessage);

	for (filesCount = 0; files[filesCount]; filesCount++);
	ft->files = (oft_file_record *)SAFE_MALLOC(sizeof(oft_file_record) * filesCount);
	ft->files_list = (char**)SAFE_MALLOC(sizeof(TCHAR *) * filesCount);
	ft->qwTotalSize = 0;

	char **filesUtf = (char**)SAFE_MALLOC(sizeof(char *) * filesCount);
	for(i = 0; i < filesCount; i++) filesUtf[i] = FileNameToUtf(files[i]);

	// Prepare files arrays
	for (i = 0; i < filesCount; i++)
	{
		if (_tstati64(files[i], &statbuf))
			debugLogA("IcqSendFile() was passed invalid filename \"%s\"", files[i]);
		else
		{
			if (!(statbuf.st_mode&_S_IFDIR))
			{ // take only files
				ft->files[ft->wFilesCount].szFile = ft->files_list[ft->wFilesCount] = null_strdup(filesUtf[i]);
				ft->files[ft->wFilesCount].szContainer = oftGetFileContainer(ft, (LPCSTR*) filesUtf, i);

				ft->wFilesCount++;
				ft->qwTotalSize += statbuf.st_size;
			}
		}
	}

	for (i = 0; i < filesCount; i++)
		SAFE_FREE(&filesUtf[i]);
	SAFE_FREE((void**)&filesUtf);

	if (!ft->wFilesCount)
	{ // found no valid files to send
		icq_LogMessage(LOG_ERROR, LPGEN("Failed to Initialize File Transfer. No valid files were specified."));
		// Notify UI
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ft, 0);
		// Release transfer
		SafeReleaseFileTransfer((void**)&ft);

		return 0; // Failure
	}
#ifdef __GNUC__
#define OSCAR_MAX_SIZE 0x100000000ULL
#else
#define OSCAR_MAX_SIZE 0x100000000
#endif
	if (ft->qwTotalSize >= OSCAR_MAX_SIZE && ft->wFilesCount > 1)
	{ // file larger than 4GB can be send only as single
		icq_LogMessage(LOG_ERROR, LPGEN("The files are too big to be sent at once. Files bigger than 4 GB can be sent only separately."));
		// Notify UI
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ft, 0);
		// Release transfer
		SafeReleaseFileTransfer((void**)&ft);

		return 0; // Failure
	}

	debugLogA("OFT: Found %d files.", ft->wFilesCount);

	ft->szDescription = tchar_to_utf8(pszDesc);
	ft->flags = OFTF_SENDING;
	ft->fileId = -1;
	ft->iCurrentFile = 0;
	ft->dwCookie = AllocateCookie(CKT_FILE, ICQ_MSG_SRV_SEND, hContact, ft);

	// Init oscar fields
	{
		ft->wEncrypt = 0;
		ft->wCompress = 0;
		ft->wPartsCount = 1;
		ft->wPartsLeft = 1;
		strcpy(ft->rawIDString, "Cool FileXfer");
		ft->bHeaderFlags = 0x20;
		ft->bNameOff = 0x1C;
		ft->bSizeOff = 0x11;
		ft->dwRecvForkCheck = 0xFFFF0000;
		ft->dwThisForkCheck = 0xFFFF0000;
		ft->dwRecvFileCheck = 0xFFFF0000;
	}

	// Send file transfer request
	{
		char *pszFiles;

		if (ft->wFilesCount == 1)
		{ // transfering single file, give filename
			pszFiles = (char*)ExtractFileName(ft->files[0].szFile);
		}
		else 
		{ // check if transfering one directory
			char *szFirstDiv, *szFirstDir = ft->file_containers[0];
			int nFirstDirLen;

			// default is no root dir
			pszFiles = "";

			if ((szFirstDiv = strstrnull(szFirstDir, "\\")) || (szFirstDiv = strstrnull(szFirstDir, "/")))
				nFirstDirLen = szFirstDiv - szFirstDir;
			else
				nFirstDirLen = strlennull(szFirstDir);

			if (nFirstDirLen)
			{ // got root dir from first container, check if others are only sub-dirs
				for (i = 0; i < ft->containerCount; i++)
				{
					if (_strnicmp((char*)ft->file_containers[i], (char*)szFirstDir, nFirstDirLen))
					{
						szFirstDir = NULL;
						break;
					}
				}
				if (szFirstDir)
				{ // fine, we are sending only one directory
					pszFiles = szFirstDir;
					if (szFirstDiv) szFirstDiv[0] = '\0';
					nFirstDirLen++; // include backslash
					// cut all files container by root dir - it is transferred as root separately
					for (i = 0; i < ft->wFilesCount; i++)
						ft->files[i].szContainer += nFirstDirLen;
				}
			}
		}

		// Create listener
		ft->listener = CreateOscarListener(ft, oft_newConnectionReceived);

		// Send packet
		if (ft->listener)
		{
			oft_sendFileRequest(dwUin, szUid, ft, pszFiles, getDword("RealIP", 0));
		}
		else
		{ // try stage 1 proxy
			ft->szThisFile = null_strdup(pszFiles);
			OpenOscarConnection(hContact, ft, OCT_PROXY_INIT);
		}
	}

	return ft; // Success
}


HANDLE CIcqProto::oftFileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szPath)
{
	oscar_filetransfer *ft = (oscar_filetransfer*)hTransfer;
	DWORD dwUin;
	uid_str szUid;

	if (getContactUid(hContact, &dwUin, &szUid))
		return 0; // Invalid contact

	if (!IsValidOscarTransfer(ft))
		return 0; // Invalid transfer

	ft->szSavePath = tchar_to_utf8(szPath);

	if (ft->szThisPath)
	{ // Append Directory name to the save path, when transfering a directory
		ft->szSavePath = (char*)SAFE_REALLOC(ft->szSavePath, strlennull(ft->szSavePath) + strlennull(ft->szThisPath) + 4);
		NormalizeBackslash(ft->szSavePath);
		strcat(ft->szSavePath, ft->szThisPath);
		NormalizeBackslash(ft->szSavePath);
	}
#ifdef _DEBUG
	NetLog_Direct("OFT: Request accepted, saving to '%s'.", ft->szSavePath);
#endif

	// Create cookie
	ft->dwCookie = AllocateCookie(CKT_FILE, ICQ_MSG_SRV_SEND, hContact, ft);

	OpenOscarConnection(hContact, ft, ft->bUseProxy ? OCT_PROXY_RECV: OCT_NORMAL);
	return hTransfer; // Success
}


DWORD CIcqProto::oftFileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szReason)
{
	oscar_filetransfer *ft = (oscar_filetransfer*)hTransfer;
	DWORD dwUin;
	uid_str szUid;

	if (getContactUid(hContact, &dwUin, &szUid))
		return 1; // Invalid contact

	if (IsValidOscarTransfer(ft))
	{
		if (ft->hContact != hContact)
			return 1; // Bad contact or hTransfer

#ifdef _DEBUG
		NetLog_Direct("OFT: Request denied.");
#endif

		oft_sendFileDeny(dwUin, szUid, ft);

		// Release structure
		SafeReleaseFileTransfer((void**)&ft);

		return 0; // Success
	}
	return 1; // Invalid transfer
}


DWORD CIcqProto::oftFileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	oscar_filetransfer* ft = (oscar_filetransfer*)hTransfer;
	DWORD dwUin;
	uid_str szUid;

	if (getContactUid(hContact, &dwUin, &szUid))
		return 1; // Invalid contact

	if (IsValidOscarTransfer(ft))
	{
		if (ft->hContact != hContact)
			return 1; // Bad contact or hTransfer

#ifdef _DEBUG
		NetLog_Direct("OFT: Transfer cancelled.");
#endif

		oft_sendFileCancel(dwUin, szUid, ft);

		ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);

		// Release structure
		SafeReleaseFileTransfer((void**)&ft);

		return 0; // Success
	}
	return 1; // Invalid transfer
}


void CIcqProto::oftFileResume(oscar_filetransfer *ft, int action, const TCHAR *szFilename)
{
	int openFlags;

	if (ft->connection == NULL)
		return;

	oscar_connection *oc = ft->connection;

#ifdef _DEBUG
	NetLog_Direct("OFT: Resume Transfer, Action: %d, FileName: '%s'", action, szFilename);
#endif

	switch (action)
	{
	case FILERESUME_RESUME:
		openFlags = _O_BINARY | _O_RDWR;
		break;

	case FILERESUME_OVERWRITE:
		openFlags = _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY;
		ft->qwFileBytesDone = 0;
		break;

	case FILERESUME_SKIP:
		openFlags = _O_BINARY | _O_WRONLY;
		ft->qwFileBytesDone = ft->qwThisFileSize;
		break;

	case FILERESUME_RENAME:
		openFlags = _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY;
		SAFE_FREE(&ft->szThisFile);
		ft->szThisFile = tchar_to_utf8(szFilename);
		ft->qwFileBytesDone = 0;
		break;

	default: // workaround for bug in Miranda Core
		if (ft->resumeAction == FILERESUME_RESUME)
			openFlags = _O_BINARY | _O_RDWR;
		else
		{ // default to overwrite
			openFlags = _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY;
			ft->qwFileBytesDone = 0;
		}
	}
	ft->resumeAction = action;

	ft->fileId = OpenFileUtf(ft->szThisFile, openFlags, _S_IREAD | _S_IWRITE);
#ifdef _DEBUG
	NetLog_Direct("OFT: OpenFileUtf(%s, %u) returned %u", ft->szThisFile, openFlags, ft->fileId);
#endif
	if (ft->fileId == -1)
	{
#ifdef _DEBUG
		NetLog_Direct("OFT: errno=%d", errno);
#endif
		icq_LogMessage(LOG_ERROR, LPGEN("Your file receive has been aborted because Miranda could not open the destination file in order to write to it. You may be trying to save to a read-only folder."));

		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		// Release transfer
		SafeReleaseFileTransfer((void**)&oc->ft);
		return;
	}

	if (action == FILERESUME_RESUME)
		ft->qwFileBytesDone = _lseeki64(ft->fileId, 0, SEEK_END);
	else
		_lseeki64(ft->fileId, ft->qwFileBytesDone, SEEK_SET);

	ft->qwBytesDone += ft->qwFileBytesDone;

	if (action == FILERESUME_RESUME)
	{ // use smart-resume
		oc->status = OCS_RESUME;
		ft->dwRecvFileCheck = oft_calc_file_checksum(ft->fileId, ft->qwFileBytesDone);
		_lseek(ft->fileId, 0, SEEK_END);

#ifdef _DEBUG
		NetLog_Direct("OFT: Starting Smart-Resume");
#endif

		sendOFT2FramePacket(oc, OFT_TYPE_RESUMEREQUEST);

		return;
	}
	else if (action == FILERESUME_SKIP)
	{ // we are skipping the file, send "we are done"
		oc->status = OCS_NEGOTIATION;
	}
	else
	{ // Send "we are ready"
		oc->status = OCS_DATA;
		ft->flags |= OFTF_FILE_RECEIVING;

		sendOFT2FramePacket(oc, OFT_TYPE_READY);
	}
	ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);

	if (!ft->qwThisFileSize || action == FILERESUME_SKIP)
	{ // if the file is empty we will not receive any data
		BYTE buf;
		oft_handleFileData(oc, &buf, 0);
	}
}


static void oft_buildProtoFileTransferStatus(oscar_filetransfer* ft, PROTOFILETRANSFERSTATUS* pfts)
{
	ZeroMemory(pfts, sizeof(PROTOFILETRANSFERSTATUS));
	pfts->cbSize = sizeof(PROTOFILETRANSFERSTATUS);
	pfts->hContact = ft->hContact;
	pfts->flags = PFTS_UTF + ((ft->flags & OFTF_SENDING) ? PFTS_SENDING : PFTS_RECEIVING);
	if (ft->flags & OFTF_SENDING)
		pfts->pszFiles = ft->files_list;
	else
		pfts->pszFiles = NULL;  /* FIXME */
	pfts->totalFiles = ft->wFilesCount;
	pfts->currentFileNumber = ft->iCurrentFile;
	pfts->totalBytes = ft->qwTotalSize;
	pfts->totalProgress = ft->qwBytesDone;
	pfts->szWorkingDir = ft->szThisPath;
	pfts->szCurrentFile = ft->szThisFile; 
	pfts->currentFileSize = ft->qwThisFileSize;
	pfts->currentFileTime = ft->dwThisFileDate;
	pfts->currentFileProgress = ft->qwFileBytesDone;
}


void CIcqProto::CloseOscarConnection(oscar_connection *oc)
{
	icq_lock l(oftMutex);

	if (oc)
	{
		oc->type = OCT_CLOSING;

		if (oc->hConnection)
		{ // we need this for Netlib handle consistency
			NetLib_CloseConnection(&oc->hConnection, FALSE);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OpenOscarConnection(MCONTACT hContact, oscar_filetransfer *ft, int type)
{
	oscarthreadstartinfo *otsi = (oscarthreadstartinfo*)SAFE_MALLOC(sizeof(oscarthreadstartinfo));

	otsi->hContact = hContact;
	otsi->type = type;
	otsi->ft = ft;

	ForkThread((MyThreadFunc)&CIcqProto::oft_connectionThread, otsi );
}


int CIcqProto::CreateOscarProxyConnection(oscar_connection *oc)
{
	NETLIBOPENCONNECTION nloc = {0};

	// inform UI
	ProtoBroadcastAck(oc->ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTPROXY, oc->ft, 0);

	nloc.szHost = OSCAR_PROXY_HOST;
	nloc.wPort = getWord("OscarPort", m_bSecureConnection ? DEFAULT_SERVER_PORT_SSL : DEFAULT_SERVER_PORT);
	if (nloc.wPort == 0)
		nloc.wPort = RandRange(1024, 65535);
	if (m_bGatewayMode)
		nloc.flags |= NLOCF_HTTPGATEWAY;

	oc->hConnection = NetLib_OpenConnection(m_hNetlibUser, "Proxy ", &nloc);
	if (!oc->hConnection)
	{ // proxy connection failed
		return 0;
	}
	oc->type = OCT_PROXY;
	oc->status = OCS_PROXY;
	oc->ft->connection = oc;
	// init proxy
	proxy_sendInitTunnel(oc);

	return 1; // Success
}


void __cdecl CIcqProto::oft_connectionThread( oscarthreadstartinfo *otsi )
{
	oscar_connection oc = {0};
	oscar_listener *source;
	NETLIBPACKETRECVER packetRecv={0};
	HANDLE hPacketRecver;

	oc.hContact = otsi->hContact;
	oc.hConnection = otsi->hConnection;
	oc.type = otsi->type;
	oc.incoming = otsi->incoming;
	oc.ft = otsi->ft;
	source = otsi->listener;
	if (oc.incoming)
	{
		if (IsValidOscarTransfer(source->ft))
		{
			oc.ft = source->ft;
			oc.ft->dwRemoteExternalIP = otsi->dwRemoteIP;
			oc.hContact = oc.ft->hContact;
			oc.ft->connection = &oc;
			oc.status = OCS_CONNECTED;
		}
		else
		{ // FT is already over, kill listener
			NetLog_Direct("Received unexpected connection, closing.");

			CloseOscarConnection(&oc);
			ReleaseOscarListener(&source);

			SAFE_FREE((void**)&otsi);
			return;
		}
	}
	SAFE_FREE((void**)&otsi);

	if (oc.hContact)
	{ // Load contact information
		getContactUid(oc.hContact, &oc.dwUin, &oc.szUid);
	}

	// Load local IP information
	oc.dwLocalExternalIP = getDword("IP", 0);
	oc.dwLocalInternalIP = getDword("RealIP", 0);

	if (!oc.incoming)
	{ // create outgoing connection
		if (oc.type == OCT_NORMAL || oc.type == OCT_REVERSE)
		{ // create outgoing connection to peer
			NETLIBOPENCONNECTION nloc = {0};
			IN_ADDR addr = {0}, addr2 = {0};

			if (oc.ft->dwRemoteExternalIP == oc.dwLocalExternalIP && oc.ft->dwRemoteInternalIP)
				addr.S_un.S_addr = htonl(oc.ft->dwRemoteInternalIP);
			else if (oc.ft->dwRemoteExternalIP)
			{
				addr.S_un.S_addr = htonl(oc.ft->dwRemoteExternalIP);
				// for different internal, try it also (for LANs with multiple external IP, VPNs, etc.)
				if (oc.ft->dwRemoteInternalIP != oc.ft->dwRemoteExternalIP)
					addr2.S_un.S_addr = htonl(oc.ft->dwRemoteInternalIP);
			}
			else // try LAN
				addr.S_un.S_addr = htonl(oc.ft->dwRemoteInternalIP);

			// Inform UI that we will attempt to connect
			ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, oc.ft, 0);

			if (!addr.S_un.S_addr && oc.type == OCT_NORMAL)
			{ // IP to connect to is empty, request reverse
				oscar_listener* listener = CreateOscarListener(oc.ft, oft_newConnectionReceived);

				if (listener)
				{ // we got listening port, fine send request
					oc.ft->listener = listener;
					// notify UI
					ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_LISTENING, oc.ft, 0);

					oft_sendFileRedirect(oc.dwUin, oc.szUid, oc.ft, oc.dwLocalInternalIP, listener->wPort, FALSE);
					return;
				}
				if (!CreateOscarProxyConnection(&oc))
				{ // normal connection failed, notify peer, wait for error or stage 3 proxy
					oft_sendFileRedirect(oc.dwUin, oc.szUid, oc.ft, 0, 0, FALSE);
					// stage 3 can follow
					return;
				}
			}
			else if (addr.S_un.S_addr && oc.ft->wRemotePort)
			{
				nloc.szHost = inet_ntoa(addr);
				nloc.wPort = oc.ft->wRemotePort;
				nloc.timeout = 8; // 8 secs to connect
				oc.hConnection = NetLib_OpenConnection(m_hDirectNetlibUser, oc.type==OCT_REVERSE?"Reverse ":NULL, &nloc);
				if (!oc.hConnection && addr2.S_un.S_addr)
				{ // first address failed, try second one if available
					nloc.szHost = inet_ntoa(addr2);
					oc.hConnection = NetLib_OpenConnection(m_hDirectNetlibUser, oc.type==OCT_REVERSE?"Reverse ":NULL, &nloc);
				}
				if (!oc.hConnection)
				{
					if (oc.type == OCT_NORMAL)
					{ // connection failed, try reverse
						oscar_listener* listener = CreateOscarListener(oc.ft, oft_newConnectionReceived);

						if (listener)
						{ // we got listening port, fine send request
							oc.ft->listener = listener;
							// notify UI that we await connection
							ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_LISTENING, oc.ft, 0);

							oft_sendFileRedirect(oc.dwUin, oc.szUid, oc.ft, oc.dwLocalInternalIP, listener->wPort, FALSE);
							return;
						}
					}
					if (!CreateOscarProxyConnection(&oc))
					{ // proxy connection failed, notify peer, wait for error or stage 4 proxy
						oft_sendFileRedirect(oc.dwUin, oc.szUid, oc.ft, 0, 0, FALSE);
						// stage 3 or stage 4 can follow
						return;
					}
				}
				else
				{ 
					oc.status = OCS_CONNECTED;
					// ack normal connection
					oc.ft->connection = &oc;
					// acknowledge OFT - connection is ready
					oft_sendFileAccept(oc.dwUin, oc.szUid, oc.ft);
					// signal UI
					ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, oc.ft, 0);
				}
			}
			else
			{ // try proxy, stage 3 (sending)
				if (!CreateOscarProxyConnection(&oc))
				{ // proxy connection failed, notify peer, wait for error or stage 4 proxy
					oft_sendFileRedirect(oc.dwUin, oc.szUid, oc.ft, 0, 0, FALSE);
					// stage 4 can follow
					return;
				}
			}
		}
		else if (oc.type == OCT_PROXY_RECV)
		{ // stage 2 & stage 4
			if (oc.ft->dwProxyIP && oc.ft->wRemotePort)
			{ // create proxy connection, join tunnel
				NETLIBOPENCONNECTION nloc = {0};
				IN_ADDR addr = {0};

				// inform UI that we will connect to file proxy
				ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTPROXY, oc.ft, 0);

				addr.S_un.S_addr = htonl(oc.ft->dwProxyIP);
				nloc.szHost = inet_ntoa(addr);
				nloc.wPort = getWord("OscarPort", m_bSecureConnection ? DEFAULT_SERVER_PORT_SSL : DEFAULT_SERVER_PORT);
				if (nloc.wPort == 0)
					nloc.wPort = RandRange(1024, 65535);
				if (m_bGatewayMode)
					nloc.flags |= NLOCF_HTTPGATEWAY;
				oc.hConnection = NetLib_OpenConnection(m_hNetlibUser, "Proxy ", &nloc);
				if (!oc.hConnection)
				{ // proxy connection failed, we are out of possibilities
					ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, oc.ft, 0);
					// notify the other side, that we failed
					oft_sendFileResponse(oc.dwUin, oc.szUid, oc.ft, 0x04);
					// Release structure
					SafeReleaseFileTransfer((void**)&oc.ft);
					return;
				}
				oc.status = OCS_PROXY;
				oc.ft->connection = &oc;
				// Join proxy tunnel
				proxy_sendJoinTunnel(&oc, oc.ft->wRemotePort);
			}
			else // stage 2 failed (empty IP)
			{ // try stage 3, or send response error 0x06
				if (!CreateOscarProxyConnection(&oc))
				{
					oft_sendFileResponse(oc.dwUin, oc.szUid, oc.ft, 0x06);
					// notify UI
					ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, oc.ft, 0);
					// Release structure
					SafeReleaseFileTransfer((void**)&oc.ft);
					return;
				}
			}
		}
		else if (oc.type == OCT_PROXY)
		{ // stage 4
			if (!CreateOscarProxyConnection(&oc))
			{ // proxy connection failed, we are out of possibilities
				ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, oc.ft, 0);
				// notify the other side, that we failed
				oft_sendFileResponse(oc.dwUin, oc.szUid, oc.ft, 0x06);
				// Release structure
				SafeReleaseFileTransfer((void**)&oc.ft);
				return;
			}
		}
		else if (oc.type == OCT_PROXY_INIT)
		{ // stage 1
			if (!CreateOscarProxyConnection(&oc))
			{ // We failed to init transfer, notify UI
				icq_LogMessage(LOG_ERROR, LPGEN("Failed to Initialize File Transfer. Unable to bind local port and File proxy unavailable."));
				// Release transfer
				SafeReleaseFileTransfer((void**)&oc.ft);
				return;
			}
			else
				oc.type = OCT_PROXY_INIT;
		}
	}
	if (!oc.hConnection)
	{ // one more sanity check
		NetLog_Direct("Error: No OFT connection.");
		return;
	}
	if (oc.status != OCS_PROXY)
	{ // Connected, notify FT UI
		ProtoBroadcastAck(oc.ft->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, oc.ft, 0);

		// send init OFT frame - just for different order of packets (just like Trillian)
		if (oc.status == OCS_CONNECTED && (oc.ft->flags & OFTF_SENDING) && ((oc.ft->flags & OFTF_INITIALIZED) || oc.type == OCT_REVERSE) && !(oc.ft->flags & OFTF_FILE_REQUEST_SENT))
		{
			oc.ft->flags |= OFTF_FILE_REQUEST_SENT;
			// proceed with first file
			oft_sendPeerInit(&oc);
		}
	}
	hPacketRecver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)oc.hConnection, 8192);
	packetRecv.cbSize = sizeof(packetRecv);

	// Packet receiving loop

	while (oc.hConnection)
	{
		int recvResult;

		packetRecv.dwTimeout = oc.wantIdleTime ? 0 : 120000;

		recvResult = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hPacketRecver, (LPARAM)&packetRecv);
		if (!recvResult)
		{
			NetLog_Direct("Clean closure of oscar socket (%p)", oc.hConnection);
			break;
		}

		if (recvResult == SOCKET_ERROR)
		{
			if (GetLastError() == ERROR_TIMEOUT)
			{ // TODO: this will not work on some systems
				if (oc.wantIdleTime)
				{ // here we want to send file data packets
					oft_sendFileData(&oc);
				}
				else if (oc.status != OCS_WAITING)
				{
					NetLog_Direct("Connection timeouted, closing.");
					break;
				}
			}
			else if (oc.type != OCT_CLOSING || GetLastError() != 87)
			{ // log only significant errors, not "connection killed by us"
				NetLog_Direct("Abortive closure of oscar socket (%p) (%d)", oc.hConnection, GetLastError());
				break;
			}
		}

		if (oc.type == OCT_CLOSING)
			packetRecv.bytesUsed = packetRecv.bytesAvailable;
		else
			packetRecv.bytesUsed = oft_handlePackets(&oc, packetRecv.buffer, packetRecv.bytesAvailable);
	}

	// End of packet receiving loop

	NetLib_SafeCloseHandle(&hPacketRecver);

	CloseOscarConnection(&oc);

	{ // Clean up
		icq_lock l(oftMutex);

		if (getFileTransferIndex(oc.ft) != -1)
			oc.ft->connection = NULL; // release link
	}
	// Give server some time for abort/cancel to arrive
	SleepEx(1000, TRUE);
	// Error handling
	if (IsValidOscarTransfer(oc.ft))
	{
		if (oc.status == OCS_DATA)
		{
			ProtoBroadcastAck(oc.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, oc.ft, 0);

			icq_LogMessage(LOG_ERROR, LPGEN("Connection lost during file transfer."));
			// Release structure
			SafeReleaseFileTransfer((void**)&oc.ft);
		}
		else if (oc.status == OCS_NEGOTIATION)
		{
			ProtoBroadcastAck(oc.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, oc.ft, 0);

			icq_LogMessage(LOG_ERROR, LPGEN("File transfer negotiation failed for unknown reason."));
			// Release structure
			SafeReleaseFileTransfer((void**)&oc.ft);
		}
	}
}


void CIcqProto::sendOscarPacket(oscar_connection *oc, icq_packet *packet)
{
	if (oc->hConnection)
	{
		int nResult;

		nResult = Netlib_Send(oc->hConnection, (const char*)packet->pData, packet->wLen, 0);

		if (nResult == SOCKET_ERROR)
		{
			NetLog_Direct("Oscar %p socket error: %d, closing", oc->hConnection, GetLastError());
			CloseOscarConnection(oc);
		}
	}

	SAFE_FREE((void**)&packet->pData);
}


int CIcqProto::oft_handlePackets(oscar_connection *oc, BYTE *buf, int len)
{
	int bytesUsed = 0;

	while (len > 0)
	{
		if (oc->status == OCS_DATA && (oc->ft->flags & OFTF_FILE_RECEIVING))
		{
			return oft_handleFileData(oc, buf, len);
		}
		else if (oc->status == OCS_PROXY)
		{
			return oft_handleProxyData(oc, buf, len);
		}
		if (len < 6)
			break;

		BYTE *pBuf = buf;
		DWORD dwHead;
		unpackDWord(&pBuf, &dwHead);
		if (dwHead != 0x4F465432)
		{ // bad packet
			NetLog_Direct("OFT: Received invalid packet (dwHead = 0x%x).", dwHead);

			CloseOscarConnection(oc);
			break;
		}

		WORD datalen;
		unpackWord(&pBuf, &datalen);

		if (len < datalen) // wait for whole packet
			break;

		WORD datatype;
		unpackWord(&pBuf, &datatype);
#ifdef _DEBUG
		NetLog_Direct("OFT2: Type %u, Length %u bytes", datatype, datalen);
#endif
		handleOFT2FramePacket(oc, datatype, pBuf, (WORD)(datalen - 8));

		/* Increase pointers so we can check for more data */
		buf += datalen;
		len -= datalen;
		bytesUsed += datalen;
	}

	return bytesUsed;
}


int CIcqProto::oft_handleProxyData(oscar_connection *oc, BYTE *buf, int len)
{
	oscar_filetransfer *ft = oc->ft;
	BYTE *pBuf; 
	WORD datalen;
	WORD wCommand;
	int bytesUsed = 0;


	while (len > 2)
	{
		pBuf = buf;

		unpackWord(&pBuf, &datalen);
		datalen += 2;

		if (len < datalen)
			break; // packet is not complete

		if (datalen < 12)
		{ // malformed packet
			CloseOscarConnection(oc);
			break;
		}
		pBuf += 2; // packet version
		unpackWord(&pBuf, &wCommand);
		pBuf += 6;
		// handle packet
		switch (wCommand)
		{
		case 0x01: // Error
			{
				WORD wError;
				char* szError;

				unpackWord(&pBuf, &wError);
				switch(wError)
				{
				case 0x0D:
					szError = "Bad request";
					break;
				case 0x0E:
					szError = "Malformed packet";
					break;
				case 0x10:
					szError = "Initial request timeout";
					break;
				case 0x1A:
					szError = "Accept period timeout";
					break;
				case 0x1C:
					szError = "Invalid data";
					break;

				default:
					szError = "Unknown";
				}
				// Notify peer
				oft_sendFileResponse(oc->dwUin, oc->szUid, oc->ft, 0x06);

				debugLogA("Proxy Error: %s (0x%x)", szError, wError);
				// Notify UI
				ProtoBroadcastAck(oc->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, oc->ft, 0);
				// Release structure
				SafeReleaseFileTransfer((void**)&oc->ft);
			}
			break;

		case 0x03: // Tunnel created
			{
				WORD wCode;
				DWORD dwIP;

				unpackWord(&pBuf, &wCode);
				unpackDWord(&pBuf, &dwIP);

				if (oc->type == OCT_PROXY_INIT)
				{ // Proxy ready, send Stage 1 Request
					ft->bUseProxy = 1;
					ft->wRemotePort = wCode;
					ft->dwProxyIP = dwIP;
					oft_sendFileRequest(oc->dwUin, oc->szUid, ft, ft->szThisFile, 0);
					SAFE_FREE(&ft->szThisFile);
					// Notify UI
					ProtoBroadcastAck(oc->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, oc->ft, 0);
				}
				else
				{
					debugLogA("Proxy Tunnel ready, notify peer.");
					oft_sendFileRedirect(oc->dwUin, oc->szUid, ft, dwIP, wCode, TRUE);
				}
			}
			break;

		case 0x05: // Connection ready
			oc->status = OCS_CONNECTED; // connection ready to send packets
			// Notify UI
			ProtoBroadcastAck(oc->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, oc->ft, 0);
			// signal we are ready
			if (oc->type == OCT_PROXY_RECV)
			{
				oft_sendFileAccept(oc->dwUin, oc->szUid, ft);
				if (ft->flags & OFTF_SENDING) // connection is ready for transfer (sending only)
					ft->flags |= OFTF_INITIALIZED;
			}

			debugLogA("Proxy Tunnel established");

			if ((ft->flags & OFTF_INITIALIZED) && (ft->flags & OFTF_SENDING) && !(ft->flags & OFTF_FILE_REQUEST_SENT))
			{
				ft->flags |= OFTF_FILE_REQUEST_SENT;
				// proceed with first file
				oft_sendPeerInit(ft->connection);
			}
			break;

		default:
			debugLogA("Unknown proxy command 0x%x", wCommand);
		}

		buf += datalen;
		len -= datalen;
		bytesUsed += datalen;
	}

	return bytesUsed;
}


int CIcqProto::oft_handleFileData(oscar_connection *oc, BYTE *buf, int len)
{
	oscar_filetransfer *ft = oc->ft;
	DWORD dwLen = len;
	int bytesUsed = 0;

	// do not accept more data than expected
	if (ft->qwThisFileSize - ft->qwFileBytesDone < dwLen)
		dwLen = (int)(ft->qwThisFileSize - ft->qwFileBytesDone);

	if (ft->fileId == -1)
	{ // something went terribly bad
#ifdef _DEBUG
		NetLog_Direct("Error: handleFileData(%u bytes) without fileId!", len);
#endif
		CloseOscarConnection(oc);
		return 0;
	}
	_write(ft->fileId, buf, dwLen);
	// update checksum
	ft->dwRecvFileCheck = oft_calc_checksum((int)ft->qwFileBytesDone, buf, dwLen, ft->dwRecvFileCheck);
	bytesUsed += dwLen;
	ft->qwBytesDone += dwLen;
	ft->qwFileBytesDone += dwLen;

	if (GetTickCount() > ft->dwLastNotify + 700 || ft->qwFileBytesDone == ft->qwThisFileSize)
	{ // notify FT UI of our progress, at most every 700ms - do not be faster than Miranda
		PROTOFILETRANSFERSTATUS pfts;

		oft_buildProtoFileTransferStatus(ft, &pfts);
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&pfts);
		ft->dwLastNotify = GetTickCount();
	}
	if (ft->qwFileBytesDone == ft->qwThisFileSize)
	{
		/* EOF */
		ft->flags &= ~OFTF_FILE_RECEIVING;

#ifdef _DEBUG
		NetLog_Direct("OFT: _close(%u)", ft->fileId);
#endif
		_close(ft->fileId);
		ft->fileId = -1;

		if (ft->resumeAction != FILERESUME_SKIP && ft->dwRecvFileCheck != ft->dwThisFileCheck)
		{
			NetLog_Direct("Error: File checksums does not match!");
			{ // Notify UI
				char *pszMsg = ICQTranslateUtf(LPGEN("The checksum of file \"%s\" does not match, the file is probably damaged."));
				char szBuf[MAX_PATH];

				mir_snprintf(szBuf, MAX_PATH, pszMsg, ExtractFileName(ft->szThisFile));
				icq_LogMessage(LOG_ERROR, szBuf);

				SAFE_FREE(&pszMsg);
			}
		} // keep transfer going (icq6 ignores checksums completely)
		else if (ft->resumeAction == FILERESUME_SKIP)
			NetLog_Direct("OFT: File receive skipped.");
		else
			NetLog_Direct("OFT: File received successfully.");

		if ((DWORD)(ft->iCurrentFile + 1) == ft->wFilesCount)
		{
			ft->bHeaderFlags = 0x01; // the whole process is over
			// ack received file
			sendOFT2FramePacket(oc, OFT_TYPE_DONE);
			oc->type = OCT_CLOSING;
			NetLog_Direct("File Transfer completed successfully.");
			ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
			// Release transfer
			SafeReleaseFileTransfer((void**)&ft);
		}
		else
		{ // ack received file
			sendOFT2FramePacket(oc, OFT_TYPE_DONE);
			oc->status = OCS_NEGOTIATION;
		}

	}
	return bytesUsed;
}


void CIcqProto::handleOFT2FramePacket(oscar_connection *oc, WORD datatype, BYTE *pBuffer, WORD wLen)
{
	oscar_filetransfer *ft = oc->ft;
	DWORD dwID1;
	DWORD dwID2;

	if (wLen < 232)
	{ // allow shorter packets, but at least with filename
		NetLog_Direct("Error: Malformed OFT2 Frame, ignoring.");
		return;
	}

	unpackLEDWord(&pBuffer, &dwID1);
	wLen -= 4;
	unpackLEDWord(&pBuffer, &dwID2);
	wLen -= 4;

	if (datatype == OFT_TYPE_REQUEST && !(ft->flags & OFTF_FILE_REQUEST_RECEIVED))
	{ // first request does not contain MsgIDs we need to send them in ready packet
		dwID1 = ft->pMessage.dwMsgID1;
		dwID2 = ft->pMessage.dwMsgID2;
	}

	if (ft->pMessage.dwMsgID1 != dwID1 || ft->pMessage.dwMsgID2 != dwID2)
	{ // this is not the right packet - bad Message IDs
		NetLog_Direct("Error: Invalid Packet Cookie, closing.");
		CloseOscarConnection(oc);

		return;
	}

	switch (datatype) {

	case OFT_TYPE_REQUEST:
		{ // Sender ready
			if (ft->flags & OFTF_SENDING)
			{ // just sanity check - this is only for receiving client
				NetLog_Direct("Error: Invalid Packet, closing.");
				CloseOscarConnection(oc);
				return;
			}

			// Read Frame data
			if (!(ft->flags & OFTF_FILE_REQUEST_RECEIVED))
			{
				unpackWord(&pBuffer, &ft->wEncrypt);
				unpackWord(&pBuffer, &ft->wCompress);
				unpackWord(&pBuffer, &ft->wFilesCount);
			}
			else
				pBuffer += 6;
			unpackWord(&pBuffer, &ft->wFilesLeft);
			ft->iCurrentFile = ft->wFilesCount - ft->wFilesLeft;
			if (!(ft->flags & OFTF_FILE_REQUEST_RECEIVED))
				unpackWord(&pBuffer, &ft->wPartsCount);
			else
				pBuffer += 2;
			unpackWord(&pBuffer, &ft->wPartsLeft);
			if (!(ft->flags & OFTF_FILE_REQUEST_RECEIVED))
			{ // just check it
				DWORD dwSize;

				unpackDWord(&pBuffer, &dwSize);
				if (dwSize != (DWORD)ft->qwTotalSize)
				{ // the 32bits does not match, use them as full size
					ft->qwTotalSize = dwSize;

					debugLogA("Warning: Invalid total size.");
				}
			}
			else
				pBuffer += 4;
			{ // this allows us to receive single >4GB file correctly
				DWORD dwSize;

				unpackDWord(&pBuffer, &dwSize);
				if (dwSize == (DWORD)ft->qwTotalSize && ft->wFilesCount == 1)
					ft->qwThisFileSize = ft->qwTotalSize;
				else
					ft->qwThisFileSize = dwSize;
			}
			unpackDWord(&pBuffer, &ft->dwThisFileDate);
			unpackDWord(&pBuffer, &ft->dwThisFileCheck);
			unpackDWord(&pBuffer, &ft->dwRecvForkCheck);
			unpackDWord(&pBuffer, &ft->dwThisForkSize);
			unpackDWord(&pBuffer, &ft->dwThisFileCreation);
			unpackDWord(&pBuffer, &ft->dwThisForkCheck);
			pBuffer += 4; // File Bytes Done
			unpackDWord(&pBuffer, &ft->dwRecvFileCheck);
			if (!(ft->flags & OFTF_FILE_REQUEST_RECEIVED))
				unpackString(&pBuffer, ft->rawIDString, 32);
			else
				pBuffer += 32;
			unpackByte(&pBuffer, &ft->bHeaderFlags);
			unpackByte(&pBuffer, &ft->bNameOff);
			unpackByte(&pBuffer, &ft->bSizeOff);
			if (!(ft->flags & OFTF_FILE_REQUEST_RECEIVED))
			{
				unpackString(&pBuffer, (char*)ft->rawDummy, 69);
				unpackString(&pBuffer, (char*)ft->rawMacInfo, 16);
			}
			else
				pBuffer += 85;
			unpackWord(&pBuffer, &ft->wEncoding);
			unpackWord(&pBuffer, &ft->wSubEncoding);
			ft->cbRawFileName = wLen - 176;
			SAFE_FREE((void**)&ft->rawFileName); // release previous buffers
			SAFE_FREE(&ft->szThisFile);
			ft->rawFileName = (char*)SAFE_MALLOC(ft->cbRawFileName + 2);
			unpackString(&pBuffer, ft->rawFileName, ft->cbRawFileName);
			// Prepare file
			if (ft->wEncoding == 2)
			{ // UCS-2 encoding
				ft->szThisFile = ApplyEncoding(ft->rawFileName, "unicode-2-0");
			}
			else
			{
				ft->szThisFile = ansi_to_utf8(ft->rawFileName);
			}

			{ // convert dir markings to normal backslashes
				int i;

				for (i = 0; i < strlennull(ft->szThisFile); i++)
				{
					if (ft->szThisFile[i] == 0x01) ft->szThisFile[i] = '\\';
				}
			}

			ft->flags |= OFTF_FILE_REQUEST_RECEIVED; // First Frame Processed

			NetLog_Direct("File '%s', %I64u Bytes", ft->szThisFile, ft->qwThisFileSize);

			{ // Prepare Path Information
				char *szFile = strrchr(ft->szThisFile, '\\');

				SAFE_FREE(&ft->szThisPath); // release previous path
				if (szFile)
				{
					ft->szThisPath = ft->szThisFile;
					szFile[0] = '\0'; // split that strings
					ft->szThisFile = null_strdup(szFile + 1);
					// no cheating with paths
					if (!IsValidRelativePath(ft->szThisPath))
					{
						NetLog_Direct("Invalid path information");
						break;
					}
				}
				else
					ft->szThisPath = null_strdup("");
			}

			/* no cheating with paths */
			if (!IsValidRelativePath(ft->szThisFile))
			{
				NetLog_Direct("Invalid path information");
				break;
			}
			char *szFullPath = (char*)SAFE_MALLOC(strlennull(ft->szSavePath)+strlennull(ft->szThisPath)+strlennull(ft->szThisFile)+3);
			strcpy(szFullPath, ft->szSavePath);
			NormalizeBackslash(szFullPath);
			strcat(szFullPath, ft->szThisPath);
			NormalizeBackslash(szFullPath);
			// make sure the dest dir exists
			if (MakeDirUtf(szFullPath))
				NetLog_Direct("Failed to create destination directory!");

			strcat(szFullPath, ft->szThisFile);
			// we joined the full path to dest file
			SAFE_FREE(&ft->szThisFile);
			ft->szThisFile = szFullPath;

			ft->qwFileBytesDone = 0;

			{
				/* file resume */
				PROTOFILETRANSFERSTATUS pfts;

				oft_buildProtoFileTransferStatus(ft, &pfts);
				if (ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, ft, (LPARAM)&pfts))
				{
					oc->status = OCS_WAITING;
					break; /* UI supports resume: it will call PS_FILERESUME */
				}

				ft->fileId = OpenFileUtf(ft->szThisFile, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE);
#ifdef _DEBUG
				NetLog_Direct("OFT: OpenFileUtf(%s, %u) returned %u", ft->szThisFile, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, ft->fileId);
#endif
				if (ft->fileId == -1)
				{
#ifdef _DEBUG
					NetLog_Direct("OFT: errno=%d", errno);
#endif
					icq_LogMessage(LOG_ERROR, LPGEN("Your file receive has been aborted because Miranda could not open the destination file in order to write to it. You may be trying to save to a read-only folder."));

					ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
					// Release transfer
					SafeReleaseFileTransfer((void**)&oc->ft);
					return;
				}
			}
			// Send "we are ready"
			oc->status = OCS_DATA;
			ft->flags |= OFTF_FILE_RECEIVING;

			sendOFT2FramePacket(oc, OFT_TYPE_READY);
			ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
			if (!ft->qwThisFileSize)
			{ // if the file is empty we will not receive any data
				BYTE buf;

				oft_handleFileData(oc, &buf, 0);
			}
			return;
		}

	case OFT_TYPE_READY:
	case OFT_TYPE_RESUMEACK:
		{ // Receiver is ready
			oc->status = OCS_DATA;
			oc->wantIdleTime = 1;
			ft->flags |= OFTF_FILE_SENDING;

			NetLog_Direct("OFT: Receiver ready.");
		}
		break;

	case OFT_TYPE_RESUMEREQUEST:
		{ // Receiver wants to resume file transfer from point
			DWORD dwResumeCheck, dwResumeOffset, dwFileCheck;

			if (!(ft->flags & OFTF_SENDING))
			{ // just sanity check - this is only for sending client
				NetLog_Direct("Error: Invalid Packet, closing.");
				CloseOscarConnection(oc);

				return;
			}
			// Read Resume Frame data
			pBuffer += 44;
			unpackDWord(&pBuffer, &dwResumeOffset);
			unpackDWord(&pBuffer, &dwResumeCheck);

			dwFileCheck = oft_calc_file_checksum(ft->fileId, dwResumeOffset);
			if (dwFileCheck == dwResumeCheck && dwResumeOffset <= ft->qwThisFileSize)
			{ // resume seems ok
				ft->qwFileBytesDone = dwResumeOffset;
				ft->qwBytesDone += dwResumeOffset;
				_lseek(ft->fileId, dwResumeOffset, SEEK_SET);

				NetLog_Direct("OFT: Resume request, ready.");
			}
			else
				NetLog_Direct("OFT: Resume request, restarting.");

			// Ready for resume
			sendOFT2FramePacket(oc, OFT_TYPE_RESUMEREADY);
		}
		break;

	case OFT_TYPE_RESUMEREADY:
		{ // Process Smart-resume reply
			DWORD dwResumeOffset, dwResumeCheck;

			if (ft->flags & OFTF_SENDING)
			{ // just sanity check - this is only for receiving client
				NetLog_Direct("Error: Invalid Packet, closing.");
				CloseOscarConnection(oc);

				return;
			}
			// Read Resume Reply data
			pBuffer += 44;
			unpackDWord(&pBuffer, &dwResumeOffset);
			unpackDWord(&pBuffer, &dwResumeCheck);

			if (ft->qwFileBytesDone != dwResumeOffset)
			{
				ft->qwBytesDone -= (ft->qwFileBytesDone - dwResumeOffset);
				ft->qwFileBytesDone = dwResumeOffset;
				if (dwResumeOffset)
					ft->dwRecvFileCheck = dwResumeCheck;
				else // Restarted resume (data mismatch)
					ft->dwRecvFileCheck = 0xFFFF0000;
			}
			_lseek(ft->fileId, dwResumeOffset, SEEK_SET);

			if (ft->qwThisFileSize != ft->qwFileBytesDone)
				NetLog_Direct("OFT: Resuming from offset %u.", dwResumeOffset);

			// Prepare to receive data
			oc->status = OCS_DATA;

			ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);

			// Ready for receive
			sendOFT2FramePacket(oc, OFT_TYPE_RESUMEACK);

			if (ft->qwThisFileSize == ft->qwFileBytesDone)
			{ // all data already processed
				BYTE buf;

				oft_handleFileData(oc, &buf, 0);
			}
		}
		break;

	case OFT_TYPE_DONE:
		{ // File done
			oc->status = OCS_NEGOTIATION;
			oc->wantIdleTime = 0;

			ft->flags &= ~OFTF_FILE_SENDING;

			NetLog_Direct("OFT: File sent successfully.");

#ifdef _DEBUG
			NetLog_Direct("OFT: _close(%u)", ft->fileId);
#endif
			_close(ft->fileId); // FIXME: this needs fix for "skip file" feature
			ft->fileId = -1;
			ft->iCurrentFile++;
			// continue with next file
			oft_sendPeerInit(oc);
		}
		break;

	default:
		NetLog_Direct("Error: Uknown OFT frame type 0x%x", datatype);
	}
}


//
// Proxy packets
/////////////////////////////

void CIcqProto::proxy_sendInitTunnel(oscar_connection *oc)
{
	icq_packet packet;
	WORD wLen = 39 + getUINLen(m_dwLocalUIN);

	packet.wLen = wLen;
	init_generic_packet(&packet, 2);

	packWord(&packet, wLen);
	packWord(&packet, OSCAR_PROXY_VERSION);
	packWord(&packet, 0x02);                      // wCommand
	packDWord(&packet, 0);                        // Unknown
	packWord(&packet, 0);                         // Flags?
	packUIN(&packet, m_dwLocalUIN);
	packLEDWord(&packet, oc->ft->pMessage.dwMsgID1);
	packLEDWord(&packet, oc->ft->pMessage.dwMsgID2);
	packDWord(&packet, 0x00010010);               // TLV(1)
	packGUID(&packet, MCAP_FILE_TRANSFER);

	sendOscarPacket(oc, &packet);
}

void CIcqProto::proxy_sendJoinTunnel(oscar_connection *oc, WORD wPort)
{
	icq_packet packet;
	WORD wLen = 41 + getUINLen(m_dwLocalUIN);

	packet.wLen = wLen;
	init_generic_packet(&packet, 2);

	packWord(&packet, wLen);
	packWord(&packet, OSCAR_PROXY_VERSION);
	packWord(&packet, 0x04);                      // wCommand
	packDWord(&packet, 0);                        // Unknown
	packWord(&packet, 0);                         // Flags?
	packUIN(&packet, m_dwLocalUIN);
	packWord(&packet, wPort);
	packLEDWord(&packet, oc->ft->pMessage.dwMsgID1);
	packLEDWord(&packet, oc->ft->pMessage.dwMsgID2);
	packDWord(&packet, 0x00010010);               // TLV(1)
	packGUID(&packet, MCAP_FILE_TRANSFER);

	sendOscarPacket(oc, &packet);
}

//
// Direct packets
/////////////////////////////

void CIcqProto::oft_sendFileData(oscar_connection *oc)
{
	oscar_filetransfer *ft = oc->ft;
	BYTE buf[OFT_BUFFER_SIZE];

	if (ft->fileId == -1)
		return;

	int bytesRead = _read(ft->fileId, buf, sizeof(buf));
	if (bytesRead == -1)
		return;

	if (!bytesRead)
	{ //
		oc->wantIdleTime = 0;
		return;
	}

	if ((DWORD)bytesRead > (ft->qwThisFileSize - ft->qwFileBytesDone))
	{ // do not send more than expected, limit to known size
		bytesRead = (DWORD)(ft->qwThisFileSize - ft->qwFileBytesDone);
		oc->wantIdleTime = 0;
	}

	if (bytesRead)
	{
		icq_packet packet;

		packet.wLen = bytesRead;
		init_generic_packet(&packet, 0);
		packBuffer(&packet, buf, (WORD)bytesRead); // we are sending raw data
		sendOscarPacket(oc, &packet);

		ft->qwBytesDone += bytesRead;
		ft->qwFileBytesDone += bytesRead;
	}

	if (GetTickCount() > ft->dwLastNotify + 700 || oc->wantIdleTime == 0 || ft->qwFileBytesDone == ft->qwThisFileSize)
	{ // notify only once a while or after last data packet sent
		PROTOFILETRANSFERSTATUS pfts;

		oft_buildProtoFileTransferStatus(ft, &pfts);
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&pfts);
		ft->dwLastNotify = GetTickCount();
	}
}

void CIcqProto::oft_sendPeerInit(oscar_connection *oc)
{
	icq_lock l(oftMutex);

	oscar_filetransfer *ft = oc->ft;
	struct _stati64 statbuf;
	char *pszThisFileName;

	// prepare init frame
	if (ft->iCurrentFile >= (int)ft->wFilesCount)
	{ // All files done, great!
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		// Release transfer
		SafeReleaseFileTransfer((void**)&oc->ft);
		return;
	}

	SAFE_FREE(&ft->szThisFile);
	ft->szThisFile = null_strdup(ft->files[ft->iCurrentFile].szFile);
	if (FileStatUtf(ft->szThisFile, &statbuf))
	{
		icq_LogMessage(LOG_ERROR, LPGEN("Your file transfer has been aborted because one of the files that you selected to send is no longer readable from the disk. You may have deleted or moved it."));

		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		// Release transfer
		SafeReleaseFileTransfer((void**)&oc->ft);
		return;
	}

	{ // create full relative filename
		char* szThisContainer = ft->files[ft->iCurrentFile].szContainer;

		pszThisFileName = (char*)SAFE_MALLOC(strlennull(ft->szThisFile) + strlennull(szThisContainer) + 4);
		strcpy(pszThisFileName, szThisContainer);
		NormalizeBackslash(pszThisFileName);
		strcat(pszThisFileName, ExtractFileName(ft->szThisFile));
	}
	{ // convert backslashes to dir markings
		int i;
		for (i = 0; i < strlennull(pszThisFileName); i++)
			if (pszThisFileName[i] == '\\' || pszThisFileName[i] == '/')
				pszThisFileName[i] = 0x01;
	}

	ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);

	ft->fileId = OpenFileUtf(ft->szThisFile, _O_BINARY | _O_RDONLY, 0);
#ifdef _DEBUG
	NetLog_Direct("OFT: OpenFileUtf(%s, %u) returned %u", ft->szThisFile, _O_BINARY | _O_RDONLY, ft->fileId);
#endif
	if (ft->fileId == -1)
	{
#ifdef _DEBUG
		NetLog_Direct("OFT: errno=%d", errno);
#endif
		SAFE_FREE((void**)&pszThisFileName);
		icq_LogMessage(LOG_ERROR, LPGEN("Your file transfer has been aborted because one of the files that you selected to send is no longer readable from the disk. You may have deleted or moved it."));
		//
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		// Release transfer
		SafeReleaseFileTransfer((void**)&oc->ft);
		return;
	}

	ft->qwThisFileSize = statbuf.st_size;
	ft->dwThisFileDate = statbuf.st_mtime;
	ft->dwThisFileCreation = statbuf.st_ctime;
	ft->dwThisFileCheck = oft_calc_file_checksum(ft->fileId, ft->qwThisFileSize);
	ft->qwFileBytesDone = 0;
	ft->dwRecvFileCheck = 0xFFFF0000;
	SAFE_FREE((void**)&ft->rawFileName);

	if (IsUSASCII(pszThisFileName, strlennull(pszThisFileName)))
	{
		ft->wEncoding = 0; // ascii
		ft->cbRawFileName = strlennull(pszThisFileName) + 1;
		if (ft->cbRawFileName < 64) ft->cbRawFileName = 64;
		ft->rawFileName = (char*)SAFE_MALLOC(ft->cbRawFileName);
		strcpy(ft->rawFileName, (char*)pszThisFileName);
		SAFE_FREE((void**)&pszThisFileName);
	}
	else
	{
		ft->wEncoding = 2; // ucs-2
		WCHAR *pwsThisFile = make_unicode_string(pszThisFileName);
		SAFE_FREE((void**)&pszThisFileName);
		ft->cbRawFileName = strlennull(pwsThisFile) * (int)sizeof(WCHAR) + 2;
		if (ft->cbRawFileName < 64) ft->cbRawFileName = 64;
		ft->rawFileName = (char*)SAFE_MALLOC(ft->cbRawFileName);
		// convert to LE ordered string
		BYTE *pwsThisFileBuf = (BYTE*)pwsThisFile; // need this - unpackWideString moves the address!
		unpackWideString(&pwsThisFileBuf, (WCHAR*)ft->rawFileName, (WORD)(strlennull(pwsThisFile) * sizeof(WCHAR)));
		SAFE_FREE((void**)&pwsThisFile);
	}
	ft->wFilesLeft = (WORD)(ft->wFilesCount - ft->iCurrentFile);

	sendOFT2FramePacket(oc, OFT_TYPE_REQUEST);
}

void CIcqProto::sendOFT2FramePacket(oscar_connection *oc, WORD datatype)
{
	oscar_filetransfer *ft = oc->ft;
	icq_packet packet;

	packet.wLen = 192 + ft->cbRawFileName;
	init_generic_packet(&packet, 0);
	// Basic Oscar Frame
	packDWord(&packet, 0x4F465432); // Magic
	packWord(&packet, packet.wLen);
	packWord(&packet, datatype);
	// Cookie
	packLEDWord(&packet, ft->pMessage.dwMsgID1);
	packLEDWord(&packet, ft->pMessage.dwMsgID2);
	packWord(&packet, ft->wEncrypt);
	packWord(&packet, ft->wCompress);
	packWord(&packet, ft->wFilesCount);
	packWord(&packet, ft->wFilesLeft);
	packWord(&packet, ft->wPartsCount);
	packWord(&packet, ft->wPartsLeft);
	packDWord(&packet, (DWORD)ft->qwTotalSize);
	packDWord(&packet, (DWORD)ft->qwThisFileSize);
	packDWord(&packet, ft->dwThisFileDate);
	packDWord(&packet, ft->dwThisFileCheck);
	packDWord(&packet, ft->dwRecvForkCheck);
	packDWord(&packet, ft->dwThisForkSize);
	packDWord(&packet, ft->dwThisFileCreation);
	packDWord(&packet, ft->dwThisForkCheck);
	packDWord(&packet, (DWORD)ft->qwFileBytesDone);
	packDWord(&packet, ft->dwRecvFileCheck);
	packBuffer(&packet, (LPBYTE)ft->rawIDString, 32);
	packByte(&packet, ft->bHeaderFlags);
	packByte(&packet, ft->bNameOff);
	packByte(&packet, ft->bSizeOff);
	packBuffer(&packet, ft->rawDummy, 69);
	packBuffer(&packet, ft->rawMacInfo, 16);
	packWord(&packet, ft->wEncoding);
	packWord(&packet, ft->wSubEncoding);
	packBuffer(&packet, (LPBYTE)ft->rawFileName, ft->cbRawFileName);

	sendOscarPacket(oc, &packet);
}
