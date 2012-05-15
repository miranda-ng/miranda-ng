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
#define VERSIONSTRING		"mirandaversion"
#define CODETOSTATUS		"code2status"
#define CONTACT				"contact"
#define CONTACTCOUNT		"ccount"
#define CONTACTINFO			"cinfo"
#define DBPROFILENAME		"dbprofile"
#define DBPROFILEPATH		"dbprofilepath"
#define DBSETTING			"dbsetting"
#define LSDATE				"lsdate"
#define LSTIME				"lstime"
#define LSSTATUS			"lsstatus"
//#define SUBJECT				"subject" // defined in variables.h
#define MIRANDAPATH			"mirandapath"
#define MYSTATUS			"mstatus"
#define DBEVENT				"message" // may be extended later
//#define PROTONAME			"protoname" // depreciated
#define PROTOINFO			"pinfo"
#define TRANSLATE			"translate"
#define CONTACT_NAME		"contactname"
#define MIR_DATE			"date"
#define SRVEXISTS			"srvexists"

#define STR_PINAME			"name"
#define STR_PIUIDTEXT		"uidtext"
#define STR_PIUIDSETTING	"uidsetting"

#define CEX_MODULE			"ContactsEx"
#define SEEN_MODULE			"SeenModule"

#define STR_PROTOID			"protoid"
#define STR_FIRST			"first"
#define STR_LAST			"last"
#define STR_SENT			"sent"
#define STR_RCVD			"recv"
#define STR_READ			"read"
#define	STR_UNREAD			"unread"

/* dbevent flags */
/* these flags must contain:
DBE_FIRST|DBE_LAST|DBE_NEXT|DBE_PREV
and
DBE_SENT|DBE_RCVD
and
DBE_READ|DBE_UNREAD
and
DBE_MESSAGE|DBE_URL|DBE_CONTACTS|DBE_ADDED|DBE_AUTHREQUEST|DBE_FILE|DBE_OTHER
*/
#define DBE_FIRST			0x00000001	// first event (conforming the rest of the flags)
#define DBE_LAST			0x00000002  // last event (conforming the rest of the flags)
#define DBE_NEXT			0x00000004  // next event (conforming the rest of the flags), hDbEvent must be set
#define DBE_PREV			0x00000008	// prev event (conforming the rest of the flags), hDbEvent must be set
#define DBE_SENT			0x00000010  // event was sent
#define DBE_RCVD			0x00000020	// event was received
#define DBE_READ			0x00000040	// event is read
#define DBE_UNREAD			0x00000080	// event is not read
/* type */
#define DBE_MESSAGE			0x00000100  // event is a message, etc (pBlob = message)
#define DBE_URL				0x00000200	// pBlob = message
#define DBE_CONTACTS		0x00000400	// pBlob = 'some format', no string
#define DBE_ADDED			0x00000800	// pBlob = 'some format', no string
#define DBE_AUTHREQUEST		0x00001000	// pBlob = message
#define DBE_FILE			0x00002000	// pBlob = DWORD + file + description
#define DBE_STATUSCHANGE	0x00004000  // pBlob = description
#define DBE_OTHER			0x00008000
#define DBE_ANYTYPE			(DBE_MESSAGE|DBE_URL|DBE_CONTACTS|DBE_ADDED|DBE_AUTHREQUEST|DBE_FILE|DBE_STATUSCHANGE|DBE_OTHER)
#define DBE_ANYFIRSTUNREAD	(DBE_ANYTYPE|DBE_UNREAD|DBE_RCVD)

#ifndef EVENTTYPE_STATUSCHANGE
#define EVENTTYPE_STATUSCHANGE 25368
#endif
