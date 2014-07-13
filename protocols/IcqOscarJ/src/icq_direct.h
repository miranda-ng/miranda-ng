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

#ifndef __ICQ_DIRECT_H
#define __ICQ_DIRECT_H

struct filetransfer: public basic_filetransfer
{
  int status;
  int sending;
  int iCurrentFile;
  int currentIsDir;
  DWORD dwCookie;
  DWORD dwUin;
  DWORD dwRemotePort;
  MCONTACT hContact;
  char *szFilename;
  char *szDescription;
  char *szSavePath;
  char *szThisFile;
  char *szThisSubdir;
  char **pszFiles;
  DWORD dwThisFileSize;
  DWORD dwThisFileDate;
  DWORD dwTotalSize;
  DWORD dwFileCount;
  DWORD dwTransferSpeed;
  DWORD dwBytesDone, dwFileBytesDone;
  int fileId;
  HANDLE hConnection;
  DWORD dwLastNotify;
  int   nVersion;   // Was this sent with a v7 or a v8 packet?
  BOOL bDC;         // Was this received over a DC or through server?
  BOOL bEmptyDesc;  // Was the description empty ?
};

#define DIRECTCONN_STANDARD   0
#define DIRECTCONN_FILE       1
#define DIRECTCONN_CHAT       2
#define DIRECTCONN_REVERSE    10
#define DIRECTCONN_CLOSING    15

struct directconnect
{
  MCONTACT hContact;
  HANDLE hConnection;
  DWORD dwConnectionCookie;
  int type;
  WORD wVersion;
  int incoming;
  int wantIdleTime;
  int packetPending;
  DWORD dwRemotePort;
  DWORD dwRemoteUin;
  DWORD dwRemoteExternalIP;
  DWORD dwRemoteInternalIP;
  DWORD dwLocalExternalIP;
  DWORD dwLocalInternalIP;
  int initialised;
  int handshake;
  DWORD dwThreadId;
  filetransfer *ft;
  DWORD dwReqId;  // Reverse Connect request cookie
};

int  DecryptDirectPacket(directconnect* dc, PBYTE buf, WORD wLen);

#endif /* __ICQ_DIRECT_H */
