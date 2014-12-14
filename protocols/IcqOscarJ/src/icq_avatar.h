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
//  Avatars connection support declarations
// -----------------------------------------------------------------------------

#ifndef __ICQ_AVATAR_H
#define __ICQ_AVATAR_H

extern BYTE hashEmptyAvatar[9];

#define AVATAR_HASH_MINI    0x00
#define AVATAR_HASH_STATIC  0x01
#define AVATAR_HASH_FLASH   0x08
#define AVATAR_HASH_PHOTO   0x0C

struct CIcqProto;

struct avatars_server_connection : public lockable_struct
{
protected:
	CIcqProto *ppro;
	HANDLE hConnection;  // handle to the connection
	HANDLE hPacketRecver;
	WORD   wLocalSequence;
	icq_critical_section *localSeqMutex;

	BOOL   isLoggedIn;
	BOOL   isActive;
	BOOL   stopThread; // horrible, but simple - signal for thread to stop

	char  *pCookie;     // auth to server
	size_t wCookieLen;

	int    sendServerPacket(icq_packet *pPacket);

	int    handleServerPackets(BYTE *buf, size_t buflen);

	void   handleLoginChannel(BYTE *buf);
	void   handleDataChannel(BYTE *buf, size_t datalen);

	void   handleServiceFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader);
	void   handleAvatarFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader);

	rates *m_rates;
	icq_critical_section *m_ratesMutex;

	MCONTACT runContact[4];
	DWORD    runTime[4];
	int      runCount;
	void     checkRequestQueue();

public:
	avatars_server_connection(CIcqProto *ppro, HANDLE hConnection, char *pCookie, size_t wCookieLen);
	virtual ~avatars_server_connection();

	void connectionThread();
	void closeConnection();
	void shutdownConnection();

	__inline BOOL isPending() { return !isLoggedIn; };
	__inline BOOL isReady() { return isLoggedIn && isActive && !stopThread; };

	DWORD  sendGetAvatarRequest(MCONTACT hContact, DWORD dwUin, char *szUid, const BYTE *hash, size_t hashlen, const TCHAR *file);
	DWORD  sendUploadAvatarRequest(MCONTACT hContact, WORD wRef, const BYTE *data, size_t datalen);
};

__inline static void SAFE_DELETE(avatars_server_connection **p) { SAFE_DELETE((lockable_struct**)p); };

struct avatars_request : public MZeroedObject
{
	int      type;
	MCONTACT hContact;
	DWORD    dwUin;
	uid_str  szUid;
	BYTE    *hash;
	size_t   hashlen;
	TCHAR   *szFile;
	BYTE    *pData;
	size_t   cbData;
	WORD     wRef;
	DWORD    timeOut;
	avatars_request *pNext;

public:
	avatars_request(int type);
	virtual ~avatars_request();
};

__inline static void SAFE_DELETE(avatars_request **p) { SAFE_DELETE((MZeroedObject**)p); };

#define ART_GET     1
#define ART_UPLOAD  2
#define ART_BLOCK   4

void AddAvatarExt(int dwFormat, TCHAR *pszDest);

BYTE* calcMD5HashOfFile(const TCHAR *szFile);

#endif /* __ICQ_AVATAR_H */
