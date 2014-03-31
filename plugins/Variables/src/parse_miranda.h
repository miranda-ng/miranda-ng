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

#define VERSIONSTRING		_T("mirandaversion")
#define CODETOSTATUS		_T("code2status")
#define CONTACT				_T("contact")
#define CONTACTCOUNT		_T("ccount")
#define MIR_CONTACTINFO		_T("cinfo")
#define DBPROFILENAME		_T("dbprofile")
#define DBPROFILEPATH		_T("dbprofilepath")
#define DBSETTING			_T("dbsetting")
#define LSDATE				_T("lsdate")
#define LSTIME				_T("lstime")
#define LSSTATUS			_T("lsstatus")
//#define SUBJECT				"subject" // defined in variables.h
#define MIRANDAPATH			_T("mirandapath")
#define MYSTATUS			_T("mstatus")
#define DBEVENT				_T("message") // may be extended later
//#define PROTONAME			"protoname" // depreciated
#define PROTOINFO			_T("pinfo")
#define TRANSLATE			_T("translate")
#define CONTACT_NAME		_T("contactname")
#define MIR_DATE			_T("date")
#define SRVEXISTS			_T("srvexists")

#define STR_PINAME			"name"
#define STR_PIUIDTEXT		"uidtext"
#define STR_PIUIDSETTING	"uidsetting"
#define STR_PINICK			"nick"

#define SEEN_MODULE			"SeenModule"

#define STR_PROTOID			_T("protoid")
#define STR_FIRST			_T("first")
#define STR_LAST			_T("last")
#define STR_SENT			_T("sent")
#define STR_RCVD			_T("recv")
#define STR_READ			_T("read")
#define	STR_UNREAD			_T("unread")

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
#define DBE_OTHER			0x00008000
#define DBE_ANYTYPE			(DBE_MESSAGE|DBE_URL|DBE_CONTACTS|DBE_ADDED|DBE_AUTHREQUEST|DBE_FILE|DBE_STATUSCHANGE|DBE_OTHER)
#define DBE_ANYFIRSTUNREAD	(DBE_ANYTYPE|DBE_UNREAD|DBE_RCVD)
