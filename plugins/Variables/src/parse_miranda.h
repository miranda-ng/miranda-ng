/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#define VERSIONSTRING      L"mirandaversion"
#define CODETOSTATUS       L"code2status"
#define CONTACT            L"contact"
#define CONTACTCOUNT       L"ccount"
#define MIR_CONTACTINFO    L"cinfo"
#define DBPROFILENAME      L"dbprofile"
#define DBPROFILEPATH      L"dbprofilepath"
#define DBSETTING          L"dbsetting"
#define LSDATE             L"lsdate"
#define LSTIME             L"lstime"
#define LSSTATUS           L"lsstatus"
#define MIRANDAPATH        L"mirandapath"
#define MYSTATUS           L"mstatus"
#define DBEVENT            L"message" // may be extended later
#define PROTOINFO          L"pinfo"
#define TRANSLATE          L"translate"
#define CONTACT_NAME       L"contactname"
#define MIR_DATE           L"date"
#define SRVEXISTS          L"srvexists"

#define STR_PINAME         "name"
#define STR_PIUIDTEXT      "uidtext"
#define STR_PIUIDSETTING   "uidsetting"
#define STR_PINICK         "nick"

#define SEEN_MODULE        "SeenModule"

#define STR_PROTOID        L"protoid"
#define STR_FIRST          L"first"
#define STR_LAST           L"last"
#define STR_SENT           L"sent"
#define STR_RCVD           L"recv"
#define STR_READ           L"read"
#define STR_UNREAD         L"unread"

/* dbevent flags */
#define DBE_FIRST          0x00000001  // first event (conforming the rest of the flags)
#define DBE_LAST           0x00000002  // last event (conforming the rest of the flags)
#define DBE_NEXT           0x00000004  // next event (conforming the rest of the flags), hDbEvent must be set
#define DBE_PREV           0x00000008  // prev event (conforming the rest of the flags), hDbEvent must be set
#define DBE_SENT           0x00000010  // event was sent
#define DBE_RCVD           0x00000020  // event was received
#define DBE_READ           0x00000040  // event is read
#define DBE_UNREAD         0x00000080  // event is not read

/* type */
#define DBE_MESSAGE        0x00000100  // event is a message, etc (pBlob = message)
#define DBE_CONTACTS       0x00000400  // pBlob = 'some format', no string
#define DBE_ADDED          0x00000800  // pBlob = 'some format', no string
#define DBE_AUTHREQUEST    0x00001000  // pBlob = message
#define DBE_FILE           0x00002000  // pBlob = uint32_t + file + description
#define DBE_OTHER          0x00008000
