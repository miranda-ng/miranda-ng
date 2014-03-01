/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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

#ifndef _JABBER_CAPS_H_
#define _JABBER_CAPS_H_

#include "jabber_iq.h"

typedef unsigned __int64 JabberCapsBits;

#define JABBER_RESOURCE_CAPS_QUERY_TIMEOUT	10000

#ifdef __GNUC__
#define JABBER_RESOURCE_CAPS_ERROR        0x8000000000000000ULL
#define JABBER_RESOURCE_CAPS_IN_PROGRESS  0x8000000000000001ULL
#define JABBER_RESOURCE_CAPS_TIMEOUT      0x8000000000000002ULL
#define JABBER_RESOURCE_CAPS_UNINIT       0x8000000000000003ULL
#define JABBER_RESOURCE_CAPS_NONE         0x0000000000000000ULL
#else
#define JABBER_RESOURCE_CAPS_ERROR        0x8000000000000000
#define JABBER_RESOURCE_CAPS_IN_PROGRESS  0x8000000000000001
#define JABBER_RESOURCE_CAPS_TIMEOUT      0x8000000000000002
#define JABBER_RESOURCE_CAPS_UNINIT       0x8000000000000003
#define JABBER_RESOURCE_CAPS_NONE         0x0000000000000000
#endif

#define JABBER_FEAT_DISCO_INFO                  _T("http://jabber.org/protocol/disco#info")
#define JABBER_CAPS_DISCO_INFO                  ((JabberCapsBits)1)
#define JABBER_FEAT_DISCO_ITEMS                 _T("http://jabber.org/protocol/disco#items")
#define JABBER_CAPS_DISCO_ITEMS                 ((JabberCapsBits)1<<1)
#define JABBER_FEAT_ENTITY_CAPS                 _T("http://jabber.org/protocol/caps")
#define JABBER_CAPS_ENTITY_CAPS                 ((JabberCapsBits)1<<2)
#define JABBER_FEAT_SI                          _T("http://jabber.org/protocol/si")
#define JABBER_CAPS_SI                          ((JabberCapsBits)1<<3)
#define JABBER_FEAT_SI_FT                       _T("http://jabber.org/protocol/si/profile/file-transfer")
#define JABBER_CAPS_SI_FT                       ((JabberCapsBits)1<<4)
#define JABBER_FEAT_BYTESTREAMS                 _T("http://jabber.org/protocol/bytestreams")
#define JABBER_CAPS_BYTESTREAMS                 ((JabberCapsBits)1<<5)
#define JABBER_FEAT_IBB                         _T("http://jabber.org/protocol/ibb")
#define JABBER_CAPS_IBB                         ((JabberCapsBits)1<<6)
#define JABBER_FEAT_OOB                         _T("jabber:iq:oob")
#define JABBER_FEAT_OOB2                        _T("jabber:x:oob")
#define JABBER_CAPS_OOB                         ((JabberCapsBits)1<<7)
#define JABBER_FEAT_COMMANDS                    _T("http://jabber.org/protocol/commands")
#define JABBER_CAPS_COMMANDS                    ((JabberCapsBits)1<<8)
#define JABBER_FEAT_REGISTER                    _T("jabber:iq:register")
#define JABBER_CAPS_REGISTER                    ((JabberCapsBits)1<<9)
#define JABBER_FEAT_MUC                         _T("http://jabber.org/protocol/muc")
#define JABBER_CAPS_MUC                         ((JabberCapsBits)1<<10)
#define JABBER_FEAT_CHATSTATES                  _T("http://jabber.org/protocol/chatstates")
#define JABBER_CAPS_CHATSTATES                  ((JabberCapsBits)1<<11)
#define JABBER_FEAT_LAST_ACTIVITY               _T("jabber:iq:last")
#define JABBER_CAPS_LAST_ACTIVITY               ((JabberCapsBits)1<<12)
#define JABBER_FEAT_VERSION                     _T("jabber:iq:version")
#define JABBER_CAPS_VERSION                     ((JabberCapsBits)1<<13)
#define JABBER_FEAT_ENTITY_TIME                 _T("urn:xmpp:time")
#define JABBER_CAPS_ENTITY_TIME                 ((JabberCapsBits)1<<14)
#define JABBER_FEAT_PING                        _T("urn:xmpp:ping")
#define JABBER_CAPS_PING                        ((JabberCapsBits)1<<15)
#define JABBER_FEAT_DATA_FORMS                  _T("jabber:x:data")
#define JABBER_CAPS_DATA_FORMS                  ((JabberCapsBits)1<<16)
#define JABBER_FEAT_MESSAGE_EVENTS              _T("jabber:x:event")
#define JABBER_CAPS_MESSAGE_EVENTS              ((JabberCapsBits)1<<17)
#define JABBER_FEAT_VCARD_TEMP                  _T("vcard-temp")
#define JABBER_CAPS_VCARD_TEMP                  ((JabberCapsBits)1<<18)
#define JABBER_FEAT_AVATAR                      _T("jabber:iq:avatar")
#define JABBER_FEAT_SERVER_AVATAR               _T("storage:client:avatar")
#define JABBER_CAPS_AVATAR                      ((JabberCapsBits)1<<19)
#define JABBER_FEAT_XHTML                       _T("http://jabber.org/protocol/xhtml-im")
#define JABBER_CAPS_XHTML                       ((JabberCapsBits)1<<20)
#define JABBER_FEAT_AGENTS                      _T("jabber:iq:agents")
#define JABBER_CAPS_AGENTS                      ((JabberCapsBits)1<<21)
#define JABBER_FEAT_BROWSE                      _T("jabber:iq:browse")
#define JABBER_CAPS_BROWSE                      ((JabberCapsBits)1<<22)
#define JABBER_FEAT_FEATURE_NEG                 _T("http://jabber.org/protocol/feature-neg")
#define JABBER_CAPS_FEATURE_NEG                 ((JabberCapsBits)1<<23)
#define JABBER_FEAT_AMP                         _T("http://jabber.org/protocol/amp")
#define JABBER_CAPS_AMP                         ((JabberCapsBits)1<<24)
#define JABBER_FEAT_USER_MOOD                   _T("http://jabber.org/protocol/mood")
#define JABBER_CAPS_USER_MOOD                   ((JabberCapsBits)1<<25)
#define JABBER_FEAT_USER_MOOD_NOTIFY            _T("http://jabber.org/protocol/mood+notify")
#define JABBER_CAPS_USER_MOOD_NOTIFY            ((JabberCapsBits)1<<26)
#define JABBER_FEAT_PUBSUB                      _T("http://jabber.org/protocol/pubsub")
#define JABBER_CAPS_PUBSUB                      ((JabberCapsBits)1<<27)
#define JABBER_FEAT_SECUREIM                    _T("http://miranda-ng.org/caps/secureim")
#define JABBER_CAPS_SECUREIM                    ((JabberCapsBits)1<<28)
#define JABBER_FEAT_MIROTR                      _T("http://miranda-ng.org/caps/mirotr")
#define JABBER_CAPS_MIROTR                      ((JabberCapsBits)1<<42)
#define JABBER_FEAT_NEWGPG                      _T("http://miranda-ng.org/caps/new_gpg")
#define JABBER_CAPS_NEWGPG                      ((JabberCapsBits)1<<43)
#define JABBER_CAPS_PLATFORMX86                 ((JabberCapsBits)1<<44)
#define JABBER_CAPS_PLATFORMX64                 ((JabberCapsBits)1<<45)
#define JABBER_FEAT_PRIVACY_LISTS               _T("jabber:iq:privacy")
#define JABBER_CAPS_PRIVACY_LISTS               ((JabberCapsBits)1<<29)
#define JABBER_FEAT_MESSAGE_RECEIPTS            _T("urn:xmpp:receipts")
#define JABBER_CAPS_MESSAGE_RECEIPTS            ((JabberCapsBits)1<<30)
#define JABBER_FEAT_USER_TUNE                   _T("http://jabber.org/protocol/tune")
#define JABBER_CAPS_USER_TUNE                   ((JabberCapsBits)1<<31)
#define JABBER_FEAT_USER_TUNE_NOTIFY            _T("http://jabber.org/protocol/tune+notify")
#define JABBER_CAPS_USER_TUNE_NOTIFY            ((JabberCapsBits)1<<32)
#define JABBER_FEAT_PRIVATE_STORAGE             _T("jabber:iq:private")
#define JABBER_CAPS_PRIVATE_STORAGE             ((JabberCapsBits)1<<33)

#define JABBER_FEAT_ARCHIVE                     _T("urn:xmpp:archive")
#define JABBER_FEAT_ARCHIVE_AUTO                _T("urn:xmpp:archive:auto")
#define JABBER_CAPS_ARCHIVE_AUTO                ((JabberCapsBits)1<<34)
#define JABBER_FEAT_ARCHIVE_MANAGE              _T("urn:xmpp:archive:manage")
#define JABBER_CAPS_ARCHIVE_MANAGE              ((JabberCapsBits)1<<35)

#define JABBER_FEAT_CAPTCHA                     _T("urn:xmpp:captcha")

#define JABBER_FEAT_ATTENTION                   _T("urn:xmpp:attention:0")
#define JABBER_CAPS_ATTENTION                   ((JabberCapsBits)1<<36)

// deferred
#define JABBER_FEAT_USER_ACTIVITY               _T("http://jabber.org/protocol/activity")
#define JABBER_CAPS_USER_ACTIVITY               ((JabberCapsBits)1<<37)
#define JABBER_FEAT_USER_ACTIVITY_NOTIFY        _T("http://jabber.org/protocol/activity+notify")
#define JABBER_CAPS_USER_ACTIVITY_NOTIFY        ((JabberCapsBits)1<<38)
#define JABBER_FEAT_MIRANDA_NOTES               _T("http://miranda-ng.org/storage#notes")
#define JABBER_CAPS_MIRANDA_NOTES               ((JabberCapsBits)1<<39)
#define JABBER_FEAT_JINGLE                      _T("urn:xmpp:jingle:1")
#define JABBER_CAPS_JINGLE                      ((JabberCapsBits)1<<40)
#define JABBER_FEAT_ROSTER_EXCHANGE             _T("http://jabber.org/protocol/rosterx")
#define JABBER_CAPS_ROSTER_EXCHANGE             ((JabberCapsBits)1<<41)

#define JABBER_FEAT_DIRECT_MUC_INVITE           _T("jabber:x:conference")
#define JABBER_CAPS_DIRECT_MUC_INVITE           ((JabberCapsBits)1<<42)

#define JABBER_FEAT_PUBSUB_EVENT                _T("http://jabber.org/protocol/pubsub#event")
#define JABBER_FEAT_PUBSUB_NODE_CONFIG          _T("http://jabber.org/protocol/pubsub#node_config")

#define JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY  ((JabberCapsBits)1<<62)
#define JABBER_CAPS_OTHER_SPECIAL               (JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY|JABBER_RESOURCE_CAPS_ERROR) // must contain all the caps not listed in g_JabberFeatCapPairs, to prevent using these bits for features registered through IJabberNetInterface::RegisterFeature()

#define JABBER_CAPS_MIRANDA_NODE    _T("http://miranda-ng.org/caps")
#define JABBER_CAPS_MIRANDA_PARTIAL (JABBER_CAPS_DISCO_INFO | JABBER_CAPS_DISCO_ITEMS | JABBER_CAPS_MUC | JABBER_CAPS_ENTITY_CAPS | JABBER_CAPS_SI | JABBER_CAPS_SI_FT | \
                                     JABBER_CAPS_BYTESTREAMS | JABBER_CAPS_IBB | JABBER_CAPS_OOB | JABBER_CAPS_CHATSTATES | JABBER_CAPS_AGENTS | JABBER_CAPS_BROWSE | \
											    JABBER_CAPS_VERSION | JABBER_CAPS_LAST_ACTIVITY | JABBER_CAPS_DATA_FORMS | JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_VCARD_TEMP | \
												 JABBER_CAPS_ENTITY_TIME | JABBER_CAPS_PING | JABBER_CAPS_PRIVACY_LISTS | JABBER_CAPS_MESSAGE_RECEIPTS | JABBER_CAPS_PRIVATE_STORAGE | \
												 JABBER_CAPS_ROSTER_EXCHANGE | JABBER_CAPS_DIRECT_MUC_INVITE)

#define JABBER_CAPS_MIRANDA_ALL     (JABBER_CAPS_MIRANDA_PARTIAL | JABBER_CAPS_COMMANDS | \
                                     JABBER_CAPS_USER_MOOD_NOTIFY | JABBER_CAPS_USER_TUNE_NOTIFY | JABBER_CAPS_USER_ACTIVITY_NOTIFY | JABBER_CAPS_PLATFORMX86 | JABBER_CAPS_PLATFORMX64)


#define JABBER_EXT_SECUREIM                     _T("secureim")
#define JABBER_EXT_MIROTR                       _T("mirotr")
#define JABBER_EXT_JINGLE                       _T("jingle")
#define JABBER_EXT_NEWGPG                       _T("new_gpg")
#define JABBER_EXT_NUDGE                        _T("nudge")
#define JABBER_EXT_COMMANDS                     _T("cmds")
#define JABBER_EXT_USER_MOOD                    _T("mood")
#define JABBER_EXT_USER_TUNE                    _T("tune")
#define JABBER_EXT_USER_ACTIVITY                _T("activity")
#define JABBER_EXT_MIR_NOTES                    _T("mir_notes")
#define JABBER_EXT_PLATFORMX86                  _T("x86")
#define JABBER_EXT_PLATFORMX64                  _T("x64")

#define JABBER_FEAT_EXT_ADDRESSING              _T("http://jabber.org/protocol/address")
#define JABBER_FEAT_NESTED_ROSTER_GROUPS        _T("roster:delimiter")

#define JABBER_FEAT_RC                          _T("http://jabber.org/protocol/rc")
#define JABBER_FEAT_RC_SET_STATUS               _T("http://jabber.org/protocol/rc#set-status")
#define JABBER_FEAT_RC_SET_OPTIONS              _T("http://jabber.org/protocol/rc#set-options")
#define JABBER_FEAT_RC_FORWARD                  _T("http://jabber.org/protocol/rc#forward")
#define JABBER_FEAT_RC_LEAVE_GROUPCHATS			_T("http://jabber.org/protocol/rc#leave-groupchats")
#define JABBER_FEAT_RC_WS_LOCK                  _T("http://miranda-ng.org/rc#lock_workstation")
#define JABBER_FEAT_RC_QUIT_MIRANDA             _T("http://miranda-ng.org/rc#quit")

#define JABBER_FEAT_IQ_ROSTER                   _T("jabber:iq:roster")
#define JABBER_FEAT_DELAY                       _T("jabber:x:delay")
#define JABBER_FEAT_ENTITY_TIME_OLD             _T("jabber:iq:time")

#define JABBER_FEAT_MUC_ADMIN                   _T("http://jabber.org/protocol/muc#admin")
#define JABBER_FEAT_MUC_OWNER                   _T("http://jabber.org/protocol/muc#owner")
#define JABBER_FEAT_MUC_USER                    _T("http://jabber.org/protocol/muc#user")

#define JABBER_FEAT_NICK                        _T("http://jabber.org/protocol/nick")

#define JABBER_FEAT_HTTP_AUTH                   _T("http://jabber.org/protocol/http-auth")


class CJabberClientPartialCaps
{

protected:
	TCHAR *m_szVer;
	JabberCapsBits m_jcbCaps;
	CJabberClientPartialCaps *m_pNext;
	int m_nIqId;
	DWORD m_dwRequestTime;

public:
	CJabberClientPartialCaps(const TCHAR *szVer);
	~CJabberClientPartialCaps();

	CJabberClientPartialCaps* SetNext(CJabberClientPartialCaps *pCaps);
	__inline CJabberClientPartialCaps* GetNext()
	{	return m_pNext;
	}

	void SetCaps(JabberCapsBits jcbCaps, int nIqId = -1);
	JabberCapsBits GetCaps();

	__inline TCHAR* GetVersion()
	{	return m_szVer;
	}

	__inline int GetIqId()
	{	return m_nIqId;
	}
};

class CJabberClientCaps
{

protected:
	TCHAR *m_szNode;
	CJabberClientPartialCaps *m_pCaps;
	CJabberClientCaps *m_pNext;

protected:
	CJabberClientPartialCaps* FindByVersion(const TCHAR *szVer);
	CJabberClientPartialCaps* FindById(int nIqId);

public:
	CJabberClientCaps(const TCHAR *szNode);
	~CJabberClientCaps();

	CJabberClientCaps* SetNext(CJabberClientCaps *pClient);
	__inline CJabberClientCaps* GetNext()
	{	return m_pNext;
	}

	JabberCapsBits GetPartialCaps(TCHAR *szVer);
	BOOL SetPartialCaps(const TCHAR *szVer, JabberCapsBits jcbCaps, int nIqId = -1);
	BOOL SetPartialCaps(int nIqId, JabberCapsBits jcbCaps);

	__inline TCHAR* GetNode()
	{	return m_szNode;
	}
};

class CJabberClientCapsManager
{

protected:
	CRITICAL_SECTION m_cs;
	CJabberClientCaps *m_pClients;
	CJabberProto *ppro;

protected:
	CJabberClientCaps *FindClient(const TCHAR *szNode);

public:
	CJabberClientCapsManager(CJabberProto* proto);
	~CJabberClientCapsManager();

	__inline void Lock()
	{	EnterCriticalSection(&m_cs);
	}
	__inline void Unlock()
	{	LeaveCriticalSection(&m_cs);
	}

	void AddDefaultCaps();

	JabberCapsBits GetClientCaps(TCHAR *szNode, TCHAR *szVer);
	BOOL SetClientCaps(const TCHAR *szNode, const TCHAR *szVer, JabberCapsBits jcbCaps, int nIqId = -1);
	BOOL SetClientCaps(int nIqId, JabberCapsBits jcbCaps);

	BOOL HandleInfoRequest(HXML iqNode, CJabberIqInfo *pInfo, const TCHAR *szNode);
};

struct JabberFeatCapPair
{
	LPCTSTR szFeature;
	JabberCapsBits jcbCap;
	LPCTSTR tszDescription;
};

struct JabberFeatCapPairExt
{
	LPCTSTR szFeature;
	JabberCapsBits jcbCap;
	LPCSTR szService;

	int Valid() const
	{	return (szService == NULL) ? true : ServiceExists(szService);
	}
};

struct JabberFeatCapPairDynamic
{
	TCHAR *szExt;
	TCHAR *szFeature;
	JabberCapsBits jcbCap;
	TCHAR *szDescription;
};

extern const JabberFeatCapPair    g_JabberFeatCapPairs[];
extern const JabberFeatCapPairExt g_JabberFeatCapPairsExt[];

#endif
