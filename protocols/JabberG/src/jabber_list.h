/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov
Copyright (C) 2012-13  Miranda NG Project

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

#ifndef _JABBER_LIST_H_
#define _JABBER_LIST_H_

#include "jabber_caps.h"

typedef enum {
	LIST_ROSTER,        // Roster list
	LIST_CHATROOM,      // Groupchat room currently joined
	LIST_ROOM,          // Groupchat room list to show on the Jabber groupchat dialog
	LIST_FILE,          // Current file transfer session
	LIST_BYTE,          // Bytestream sending connection
	LIST_FTRECV,
	LIST_BOOKMARK,
	LIST_VCARD_TEMP,
	LIST_FTIQID
} JABBER_LIST;

typedef enum {
	SUB_NONE,
	SUB_TO,
	SUB_FROM,
	SUB_BOTH
} JABBER_SUBSCRIPTION;

typedef enum {
	AFFILIATION_NONE,
	AFFILIATION_OUTCAST,
	AFFILIATION_MEMBER,
	AFFILIATION_ADMIN,
	AFFILIATION_OWNER
} JABBER_GC_AFFILIATION;

typedef enum {
	ROLE_NONE,
	ROLE_VISITOR,
	ROLE_PARTICIPANT,
	ROLE_MODERATOR
} JABBER_GC_ROLE;

typedef enum {			// initial default to RSMODE_LASTSEEN
	RSMODE_SERVER,		// always let server decide (always send correspondence without resouce name)
	RSMODE_LASTSEEN,	// use the last seen resource (or let server decide if haven't seen anything yet)
	RSMODE_MANUAL		// specify resource manually (see the defaultResource field - must not be NULL)
} JABBER_RESOURCE_MODE;


struct JABBER_XEP0232_SOFTWARE_INFO : public MZeroedObject
{
	ptrT tszOs, tszOsVersion, tszSoftware, tszSoftwareVersion, tszXMirandaCoreVersion;
};

struct JABBER_RESOURCE_STATUS : public MZeroedObject
{
	~JABBER_RESOURCE_STATUS();

	int status;
	TCHAR* resourceName;
	TCHAR* nick;
	TCHAR* statusMessage;
	TCHAR* software;
	TCHAR* version;
	TCHAR* system;
	signed char priority; // resource priority, -128..+127
	time_t idleStartTime;// XEP-0012 support
	JABBER_GC_AFFILIATION affiliation;
	JABBER_GC_ROLE role;
	TCHAR* szRealJid; // real jid for jabber conferences

	// XEP-0115 support
	TCHAR* szCapsNode;
	TCHAR* szCapsVer;
	TCHAR* szCapsExt;
	DWORD dwVersionRequestTime;
	DWORD dwDiscoInfoRequestTime;
	JabberCapsBits jcbCachedCaps;
	JabberCapsBits jcbManualDiscoveredCaps;

	// XEP-0085 gone event support
	BOOL bMessageSessionActive;

	JABBER_XEP0232_SOFTWARE_INFO* pSoftwareInfo;
};

struct JABBER_LIST_ITEM : public MZeroedObject
{
	JABBER_LIST_ITEM();
	~JABBER_LIST_ITEM();

	JABBER_LIST list;
	TCHAR* jid;

	// LIST_ROSTER
	// jid = jid of the contact
	TCHAR* nick;

	JABBER_RESOURCE_STATUS* findResource(const TCHAR *resourceName) const;
	JABBER_RESOURCE_STATUS* getBestResource() const;
	JABBER_RESOURCE_MODE resourceMode;
	LIST<JABBER_RESOURCE_STATUS> arResources; // array of resources
	JABBER_RESOURCE_STATUS
		*pLastSeenResource, // resource which was last seen active
		*pManualResource,   // manually set resource
		*m_pItemResource;      // resource for jids without /resource node

	JABBER_SUBSCRIPTION subscription;
	TCHAR* group;
	TCHAR* photoFileName;
	TCHAR* messageEventIdStr;

	// LIST_AGENT
	// jid = jid of the agent
	TCHAR* name;
	TCHAR* service;

	// LIST_ROOM
	// jid = room JID
	TCHAR* type;	// room type

	// LIST_CHATROOM
	// jid = room JID
	BOOL bChatActive;
	HWND hwndGcListBan;
	HWND hwndGcListAdmin;
	HWND hwndGcListOwner;
	int  iChatState;

	// LIST_FILE
	// jid = string representation of port number
	filetransfer *ft;
	WORD port;

	// LIST_BYTE
	// jid = string representation of port number
	JABBER_BYTE_TRANSFER *jbt;

	JABBER_IBB_TRANSFER *jibb;

	// LIST_FTRECV
	// jid = string representation of stream id (sid)
	// ft = file transfer data

	//LIST_BOOKMARK
	// jid = room JID
	TCHAR* password;	// password for room
	BOOL bAutoJoin;

	BOOL bUseResource;
	BOOL bHistoryRead;
};

struct JABBER_HTTP_AVATARS
{
	char * Url;
	HANDLE hContact;

	JABBER_HTTP_AVATARS(const TCHAR *tUrl, HANDLE thContact)
		: Url(mir_t2a(tUrl)), hContact(thContact) {}

	~JABBER_HTTP_AVATARS() { mir_free(Url); }

	static int compare(const JABBER_HTTP_AVATARS *p1, const JABBER_HTTP_AVATARS *p2)
	{ return strcmp(p1->Url, p2->Url); }
};

#endif
