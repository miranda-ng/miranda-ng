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

#ifndef __COOKIES_H
#define __COOKIES_H

#define CKT_MESSAGE           0x01
#define CKT_FILE              0x02
#define CKT_SEARCH            0x04
#define CKT_SERVERLIST        0x08
#define CKT_SERVICEREQUEST    0x0A
#define CKT_REVERSEDIRECT     0x0C
#define CKT_FAMILYSPECIAL     0x10
#define CKT_OFFLINEMESSAGE    0x12
#define CKT_DIRECTORY_QUERY   0x18
#define CKT_DIRECTORY_UPDATE  0x19
#define CKT_AVATAR            0x20

struct CIcqProto;

/* Basic structure used to hold operation cookies list */
struct icq_cookie_info
{
  DWORD dwCookie;
  MCONTACT hContact;
  void *pvExtra;
  time_t dwTime;
  BYTE bType;
};


/* Specific structures to hold request specific data - pvExtra */

struct cookie_family_request
{
  WORD wFamily;
  void (CIcqProto::*familyHandler)(HANDLE hConn, char* cookie, size_t cookieLen);
};


struct cookie_offline_messages
{
  int nMessages;
  int nMissed;
};


#define ACKTYPE_NONE   0
#define ACKTYPE_SERVER 1
#define ACKTYPE_CLIENT 2

struct cookie_message_data
{
  DWORD dwMsgID1;
  DWORD dwMsgID2;
  WORD bMessageType;
  BYTE nAckType;
  BYTE isOffline;
};

#define REQUESTTYPE_OWNER        0
#define REQUESTTYPE_USERAUTO     1
#define REQUESTTYPE_USERMINIMAL  2
#define REQUESTTYPE_USERDETAILED 3
#define REQUESTTYPE_PROFILE      4

struct cookie_fam15_data
{
  BYTE bRequestType;
};


#define SEARCHTYPE_UID     0
#define SEARCHTYPE_EMAIL   1
#define SEARCHTYPE_NAMES   2
#define SEARCHTYPE_DETAILS 4

struct cookie_search
{
  BYTE bSearchType;
  char* szObject;
  DWORD dwMainId;
  DWORD dwStatus;
};


struct cookie_avatar
{
  DWORD dwUin;
  MCONTACT hContact;
  size_t hashlen;
  BYTE *hash;
  size_t cbData;
  TCHAR *szFile;
};


struct cookie_reverse_connect: public cookie_message_data
{
  MCONTACT hContact;
  DWORD dwUin;
  int type;
  void *ft;
};


#define DIRECTORYREQUEST_INFOUSER       0x01
#define DIRECTORYREQUEST_INFOOWNER      0x02
#define DIRECTORYREQUEST_INFOMULTI      0x03
#define DIRECTORYREQUEST_SEARCH         0x08
#define DIRECTORYREQUEST_UPDATEOWNER    0x10
#define DIRECTORYREQUEST_UPDATENOTE     0x11
#define DIRECTORYREQUEST_UPDATEPRIVACY  0x12

struct cookie_directory_data
{
  BYTE bRequestType;
};


#endif /* __COOKIES_H */
