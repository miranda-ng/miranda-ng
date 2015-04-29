// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

void CIcqProto::handleFileAck(PBYTE buf, size_t wLen, DWORD dwUin, DWORD dwCookie, WORD wStatus)
{
	char* pszFileName = NULL;
	DWORD dwFileSize;
	MCONTACT hCookieContact;
	WORD wPort;
	filetransfer* ft;

	// Find the filetransfer that belongs to this response
	if (!FindCookie(dwCookie, &hCookieContact, (void**)&ft)) {
		NetLog_Direct("Error: Received unexpected file transfer request response");
		return;
	}

	FreeCookie(dwCookie);

	if (hCookieContact != HContactFromUIN(dwUin, NULL)) {
		NetLog_Direct("Error: UINs do not match in file transfer request response");
		return;
	}

	// If status != 0, a request has been denied
	if (wStatus != 0) {
		NetLog_Direct("File transfer denied by %u.", dwUin);
		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)ft, 0);

		FreeCookie(dwCookie);

		return;
	}

	if (wLen < 6) { // sanity check
		NetLog_Direct("Ignoring malformed file transfer request response");
		return;
	}

	// Port to connect to
	unpackWord(&buf, &wPort);
	ft->dwRemotePort = wPort;
	wLen -= 2;

	// Unknown
	buf += 2;
	wLen -= 2;

	// Filename
	size_t wFilenameLength;
	unpackLEWord(&buf, &wFilenameLength);
	if (wFilenameLength > 0) {
		if (wFilenameLength > wLen - 2)
			wFilenameLength = wLen - 2;
		pszFileName = (char*)_alloca(wFilenameLength + 1);
		unpackString(&buf, pszFileName, wFilenameLength);
		pszFileName[wFilenameLength] = '\0';
	}
	wLen -= wFilenameLength+2;

	if (wLen >= 4)
		unpackLEDWord(&buf, &dwFileSize);
	else
		dwFileSize = 0;

	NetLog_Direct("File transfer ack from %u, port %u, name %s, size %u", dwUin, ft->dwRemotePort, pszFileName, dwFileSize);

	ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)ft, 0);

	OpenDirectConnection(ft->hContact, DIRECTCONN_FILE, ft);
}

filetransfer* CIcqProto::CreateFileTransfer(MCONTACT hContact, DWORD dwUin, int nVersion)
{
	filetransfer *ft = CreateIcqFileTransfer();

	ft->dwUin = dwUin;
	ft->hContact = hContact;
	ft->nVersion = nVersion;
	ft->pMessage.bMessageType = MTYPE_FILEREQ;
	InitMessageCookie(&ft->pMessage);

	return ft;
}

// pszDescription points to a string with the reason
// buf points to the first data after the string
void CIcqProto::handleFileRequest(PBYTE buf, DWORD dwUin, DWORD dwCookie, DWORD dwID1, DWORD dwID2, char* pszDescription, int nVersion, BOOL bDC)
{
	BOOL bEmptyDesc = FALSE;
	if (mir_strlen(pszDescription) == 0) {
		pszDescription = Translate("No description given");
		bEmptyDesc = TRUE;
	}

	// Empty port+pad
	buf += 4;

	// Filename
	size_t wFilenameLength;
	unpackLEWord(&buf, &wFilenameLength);
	if (!wFilenameLength) {
		NetLog_Direct("Ignoring malformed file send request");
		return;
	}

	char *pszFileName = (char*)_alloca(wFilenameLength + 1);
	unpackString(&buf, pszFileName, wFilenameLength);
	pszFileName[wFilenameLength] = '\0';

	// Total filesize
	DWORD dwFileSize;
	unpackLEDWord(&buf, &dwFileSize);

	int bAdded;
	MCONTACT hContact = HContactFromUIN(dwUin, &bAdded);

	// Initialize a filetransfer struct
	filetransfer *ft = CreateFileTransfer(hContact, dwUin, nVersion);
	ft->dwCookie = dwCookie;
	ft->szFilename = mir_strdup(pszFileName);
	ft->szDescription = 0;
	ft->fileId = -1;
	ft->dwTotalSize = dwFileSize;
	ft->pMessage.dwMsgID1 = dwID1;
	ft->pMessage.dwMsgID2 = dwID2;
	ft->bDC = bDC;
	ft->bEmptyDesc = bEmptyDesc;

	// Send chain event
	TCHAR* ptszFileName = mir_utf8decodeT(pszFileName);

	PROTORECVFILET pre = { 0 };
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

void CIcqProto::handleDirectCancel()
{
	NetLog_Direct("handleDirectCancel: Unhandled cancel");
}

// *******************************************************************************

void CIcqProto::icq_CancelFileTransfer(filetransfer* ft)
{
	DWORD dwCookie;
	if (FindCookieByData(ft, &dwCookie, NULL))
		FreeCookie(dwCookie);      /* this bit stops a send that's waiting for acceptance */

	if (IsValidFileTransfer(ft)) { // Transfer still out there, end it
		NetLib_CloseConnection(&ft->hConnection, FALSE);

		ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);

		if (!FindFileTransferDC(ft)) { // Release orphan structure only
			SafeReleaseFileTransfer((void**)&ft);
		}
	}
}
