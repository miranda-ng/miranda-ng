/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

#define JABBER_FEAT_DISCO_INFO                  L"http://jabber.org/protocol/disco#info"
#define JABBER_CAPS_DISCO_INFO                  ((JabberCapsBits)1)
#define JABBER_FEAT_DISCO_ITEMS                 L"http://jabber.org/protocol/disco#items"
#define JABBER_CAPS_DISCO_ITEMS                 ((JabberCapsBits)1<<1)
#define JABBER_FEAT_ENTITY_CAPS                 L"http://jabber.org/protocol/caps"
#define JABBER_CAPS_ENTITY_CAPS                 ((JabberCapsBits)1<<2)
#define JABBER_FEAT_SI                          L"http://jabber.org/protocol/si"
#define JABBER_CAPS_SI                          ((JabberCapsBits)1<<3)
#define JABBER_FEAT_SI_FT                       L"http://jabber.org/protocol/si/profile/file-transfer"
#define JABBER_CAPS_SI_FT                       ((JabberCapsBits)1<<4)
#define JABBER_FEAT_BYTESTREAMS                 L"http://jabber.org/protocol/bytestreams"
#define JABBER_CAPS_BYTESTREAMS                 ((JabberCapsBits)1<<5)
#define JABBER_FEAT_IBB                         L"http://jabber.org/protocol/ibb"
#define JABBER_CAPS_IBB                         ((JabberCapsBits)1<<6)
#define JABBER_FEAT_OOB                         L"jabber:iq:oob"
#define JABBER_FEAT_OOB2                        L"jabber:x:oob"
#define JABBER_CAPS_OOB                         ((JabberCapsBits)1<<7)
#define JABBER_FEAT_COMMANDS                    L"http://jabber.org/protocol/commands"
#define JABBER_CAPS_COMMANDS                    ((JabberCapsBits)1<<8)
#define JABBER_FEAT_REGISTER                    L"jabber:iq:register"
#define JABBER_CAPS_REGISTER                    ((JabberCapsBits)1<<9)
#define JABBER_FEAT_MUC                         L"http://jabber.org/protocol/muc"
#define JABBER_CAPS_MUC                         ((JabberCapsBits)1<<10)
#define JABBER_FEAT_CHATSTATES                  L"http://jabber.org/protocol/chatstates"
#define JABBER_CAPS_CHATSTATES                  ((JabberCapsBits)1<<11)
#define JABBER_FEAT_LAST_ACTIVITY               L"jabber:iq:last"
#define JABBER_CAPS_LAST_ACTIVITY               ((JabberCapsBits)1<<12)
#define JABBER_FEAT_VERSION                     L"jabber:iq:version"
#define JABBER_CAPS_VERSION                     ((JabberCapsBits)1<<13)
#define JABBER_FEAT_ENTITY_TIME                 L"urn:xmpp:time"
#define JABBER_CAPS_ENTITY_TIME                 ((JabberCapsBits)1<<14)
#define JABBER_FEAT_PING                        L"urn:xmpp:ping"
#define JABBER_CAPS_PING                        ((JabberCapsBits)1<<15)
#define JABBER_FEAT_DATA_FORMS                  L"jabber:x:data"
#define JABBER_CAPS_DATA_FORMS                  ((JabberCapsBits)1<<16)
#define JABBER_FEAT_MESSAGE_EVENTS              L"jabber:x:event"
#define JABBER_CAPS_MESSAGE_EVENTS              ((JabberCapsBits)1<<17)
#define JABBER_FEAT_VCARD_TEMP                  L"vcard-temp"
#define JABBER_CAPS_VCARD_TEMP                  ((JabberCapsBits)1<<18)
#define JABBER_FEAT_AVATAR                      L"jabber:iq:avatar"
#define JABBER_FEAT_SERVER_AVATAR               L"storage:client:avatar"
#define JABBER_CAPS_AVATAR                      ((JabberCapsBits)1<<19)
#define JABBER_FEAT_XHTML                       L"http://jabber.org/protocol/xhtml-im"
#define JABBER_CAPS_XHTML                       ((JabberCapsBits)1<<20)
#define JABBER_FEAT_AGENTS                      L"jabber:iq:agents"
#define JABBER_CAPS_AGENTS                      ((JabberCapsBits)1<<21)
#define JABBER_FEAT_BROWSE                      L"jabber:iq:browse"
#define JABBER_CAPS_BROWSE                      ((JabberCapsBits)1<<22)
#define JABBER_FEAT_FEATURE_NEG                 L"http://jabber.org/protocol/feature-neg"
#define JABBER_CAPS_FEATURE_NEG                 ((JabberCapsBits)1<<23)
#define JABBER_FEAT_AMP                         L"http://jabber.org/protocol/amp"
#define JABBER_CAPS_AMP                         ((JabberCapsBits)1<<24)
#define JABBER_FEAT_USER_MOOD                   L"http://jabber.org/protocol/mood"
#define JABBER_CAPS_USER_MOOD                   ((JabberCapsBits)1<<25)
#define JABBER_FEAT_USER_MOOD_NOTIFY            L"http://jabber.org/protocol/mood+notify"
#define JABBER_CAPS_USER_MOOD_NOTIFY            ((JabberCapsBits)1<<26)
#define JABBER_FEAT_PUBSUB                      L"http://jabber.org/protocol/pubsub"
#define JABBER_CAPS_PUBSUB                      ((JabberCapsBits)1<<27)
#define JABBER_FEAT_SECUREIM                    L"http://miranda-ng.org/caps/secureim"
#define JABBER_CAPS_SECUREIM                    ((JabberCapsBits)1<<28)
#define JABBER_FEAT_MIROTR                      L"http://miranda-ng.org/caps/mirotr"
#define JABBER_CAPS_MIROTR                      ((JabberCapsBits)1<<42)
#define JABBER_FEAT_NEWGPG                      L"http://miranda-ng.org/caps/new_gpg"
#define JABBER_CAPS_NEWGPG                      ((JabberCapsBits)1<<43)
#define JABBER_FEAT_OMEMO						L"urn:xmpp:omemo:0"
#define JABBER_CAPS_OMEMO						((JabberCapsBits)1<<46)
#define JABBER_FEAT_OMEMO_DEVICELIST_NOTIFY				JABBER_FEAT_OMEMO L":devicelist+notify"
#define JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY				((JabberCapsBits)1<<47)
#define JABBER_CAPS_PLATFORMX86                 ((JabberCapsBits)1<<44)
#define JABBER_CAPS_PLATFORMX64                 ((JabberCapsBits)1<<45)
#define JABBER_FEAT_PRIVACY_LISTS               L"jabber:iq:privacy"
#define JABBER_CAPS_PRIVACY_LISTS               ((JabberCapsBits)1<<29)
#define JABBER_FEAT_MESSAGE_RECEIPTS            L"urn:xmpp:receipts"
#define JABBER_CAPS_MESSAGE_RECEIPTS            ((JabberCapsBits)1<<30)
#define JABBER_FEAT_USER_TUNE                   L"http://jabber.org/protocol/tune"
#define JABBER_CAPS_USER_TUNE                   ((JabberCapsBits)1<<31)
#define JABBER_FEAT_USER_TUNE_NOTIFY            L"http://jabber.org/protocol/tune+notify"
#define JABBER_CAPS_USER_TUNE_NOTIFY            ((JabberCapsBits)1<<32)
#define JABBER_FEAT_PRIVATE_STORAGE             L"jabber:iq:private"
#define JABBER_CAPS_PRIVATE_STORAGE             ((JabberCapsBits)1<<33)

#define JABBER_FEAT_ARCHIVE                     L"urn:xmpp:archive"
#define JABBER_FEAT_ARCHIVE_AUTO                L"urn:xmpp:archive:auto"
#define JABBER_CAPS_ARCHIVE_AUTO                ((JabberCapsBits)1<<34)
#define JABBER_FEAT_ARCHIVE_MANAGE              L"urn:xmpp:archive:manage"
#define JABBER_CAPS_ARCHIVE_MANAGE              ((JabberCapsBits)1<<35)

#define JABBER_FEAT_CAPTCHA                     L"urn:xmpp:captcha"

#define JABBER_FEAT_ATTENTION                   L"urn:xmpp:attention:0"
#define JABBER_CAPS_ATTENTION                   ((JabberCapsBits)1<<36)

// deferred
#define JABBER_FEAT_USER_ACTIVITY               L"http://jabber.org/protocol/activity"
#define JABBER_CAPS_USER_ACTIVITY               ((JabberCapsBits)1<<37)
#define JABBER_FEAT_USER_ACTIVITY_NOTIFY        L"http://jabber.org/protocol/activity+notify"
#define JABBER_CAPS_USER_ACTIVITY_NOTIFY        ((JabberCapsBits)1<<38)
#define JABBER_FEAT_MIRANDA_NOTES               L"http://miranda-ng.org/storage#notes"
#define JABBER_CAPS_MIRANDA_NOTES               ((JabberCapsBits)1<<39)
#define JABBER_FEAT_JINGLE                      L"urn:xmpp:jingle:1"
#define JABBER_CAPS_JINGLE                      ((JabberCapsBits)1<<40)
#define JABBER_FEAT_ROSTER_EXCHANGE             L"http://jabber.org/protocol/rosterx"
#define JABBER_CAPS_ROSTER_EXCHANGE             ((JabberCapsBits)1<<41)

#define JABBER_FEAT_DIRECT_MUC_INVITE           L"jabber:x:conference"
#define JABBER_CAPS_DIRECT_MUC_INVITE           ((JabberCapsBits)1<<42)

#define JABBER_FEAT_PUBSUB_EVENT                L"http://jabber.org/protocol/pubsub#event"
#define JABBER_FEAT_PUBSUB_NODE_CONFIG          L"http://jabber.org/protocol/pubsub#node_config"

#define JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY  ((JabberCapsBits)1<<63)
#define JABBER_CAPS_OTHER_SPECIAL               (JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY|JABBER_RESOURCE_CAPS_ERROR) // must contain all the caps not listed in g_JabberFeatCapPairs, to prevent using these bits for features registered through IJabberNetInterface::RegisterFeature()

#define JABBER_CAPS_MIRANDA_NODE    L"http://miranda-ng.org/caps"
#define JABBER_CAPS_MIRANDA_PARTIAL (JABBER_CAPS_DISCO_INFO | JABBER_CAPS_DISCO_ITEMS | JABBER_CAPS_MUC | JABBER_CAPS_ENTITY_CAPS | JABBER_CAPS_SI | JABBER_CAPS_SI_FT | \
                                     JABBER_CAPS_BYTESTREAMS | JABBER_CAPS_IBB | JABBER_CAPS_OOB | JABBER_CAPS_CHATSTATES | JABBER_CAPS_AGENTS | JABBER_CAPS_BROWSE | \
											    JABBER_CAPS_VERSION | JABBER_CAPS_LAST_ACTIVITY | JABBER_CAPS_DATA_FORMS | JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_VCARD_TEMP | \
												 JABBER_CAPS_ENTITY_TIME | JABBER_CAPS_PING | JABBER_CAPS_PRIVACY_LISTS | JABBER_CAPS_MESSAGE_RECEIPTS | JABBER_CAPS_PRIVATE_STORAGE | \
												 JABBER_CAPS_ROSTER_EXCHANGE | JABBER_CAPS_DIRECT_MUC_INVITE)

#define JABBER_CAPS_MIRANDA_ALL     (JABBER_CAPS_MIRANDA_PARTIAL | JABBER_CAPS_COMMANDS | \
                                     JABBER_CAPS_USER_MOOD_NOTIFY | JABBER_CAPS_USER_TUNE_NOTIFY | JABBER_CAPS_USER_ACTIVITY_NOTIFY  \
									 | JABBER_CAPS_PLATFORMX86 | JABBER_CAPS_PLATFORMX64)


#define JABBER_EXT_SECUREIM                     L"secureim"
#define JABBER_EXT_MIROTR                       L"mirotr"
#define JABBER_EXT_JINGLE                       L"jingle"
#define JABBER_EXT_NEWGPG                       L"new_gpg"
#define JABBER_EXT_OMEMO                       L"omemo"
#define JABBER_EXT_NUDGE                        L"nudge"
#define JABBER_EXT_COMMANDS                     L"cmds"
#define JABBER_EXT_USER_MOOD                    L"mood"
#define JABBER_EXT_USER_TUNE                    L"tune"
#define JABBER_EXT_USER_ACTIVITY                L"activity"
#define JABBER_EXT_MIR_NOTES                    L"mir_notes"
#define JABBER_EXT_PLATFORMX86                  L"x86"
#define JABBER_EXT_PLATFORMX64                  L"x64"

#define JABBER_FEAT_EXT_ADDRESSING              L"http://jabber.org/protocol/address"
#define JABBER_FEAT_NESTED_ROSTER_GROUPS        L"roster:delimiter"

#define JABBER_FEAT_RC                          L"http://jabber.org/protocol/rc"
#define JABBER_FEAT_RC_SET_STATUS               L"http://jabber.org/protocol/rc#set-status"
#define JABBER_FEAT_RC_SET_OPTIONS              L"http://jabber.org/protocol/rc#set-options"
#define JABBER_FEAT_RC_FORWARD                  L"http://jabber.org/protocol/rc#forward"
#define JABBER_FEAT_RC_LEAVE_GROUPCHATS			L"http://jabber.org/protocol/rc#leave-groupchats"
#define JABBER_FEAT_RC_WS_LOCK                  L"http://miranda-ng.org/rc#lock_workstation"
#define JABBER_FEAT_RC_QUIT_MIRANDA             L"http://miranda-ng.org/rc#quit"

#define JABBER_FEAT_IQ_ROSTER                   L"jabber:iq:roster"
#define JABBER_FEAT_DELAY                       L"jabber:x:delay"
#define JABBER_FEAT_ENTITY_TIME_OLD             L"jabber:iq:time"

#define JABBER_FEAT_MUC_ADMIN                   L"http://jabber.org/protocol/muc#admin"
#define JABBER_FEAT_MUC_OWNER                   L"http://jabber.org/protocol/muc#owner"
#define JABBER_FEAT_MUC_USER                    L"http://jabber.org/protocol/muc#user"

#define JABBER_FEAT_NICK                        L"http://jabber.org/protocol/nick"

#define JABBER_FEAT_HTTP_AUTH                   L"http://jabber.org/protocol/http-auth"


class CJabberClientPartialCaps
{
	friend struct CJabberProto;

	ptrW m_szHash, m_szOs, m_szOsVer, m_szSoft, m_szSoftVer, m_szSoftMir;
	JabberCapsBits m_jcbCaps;
	int m_nIqId;
	DWORD m_dwRequestTime;

	class CJabberClientCaps *m_parent;
	CJabberClientPartialCaps *m_pNext;

public:
	CJabberClientPartialCaps(CJabberClientCaps *pParent, const wchar_t *szHash, const wchar_t *szVer);
	~CJabberClientPartialCaps();

	CJabberClientPartialCaps* SetNext(CJabberClientPartialCaps *pCaps);
	__inline CJabberClientPartialCaps* GetNext()
	{	return m_pNext;
	}

	void SetCaps(JabberCapsBits jcbCaps, int nIqId = -1);
	JabberCapsBits GetCaps();

	__inline const wchar_t* GetHash() const { return m_szHash.get(); }
	__inline const wchar_t* GetNode() const;

	__inline const wchar_t* GetOs() const { return m_szOs.get(); }
	__inline const wchar_t* GetOsVer() const { return m_szOsVer.get(); }
	__inline const wchar_t* GetSoft() const { return m_szSoft.get(); }
	__inline const wchar_t* GetSoftVer() const { return m_szSoftVer.get(); }
	__inline const wchar_t* GetSoftMir() const { return m_szSoftMir.get(); }

	__inline int GetIqId() const { return m_nIqId; }

	__inline void SetVer(const wchar_t *szVer)
	{
		m_szSoft = mir_wstrdup(szVer);
	}
};

class CJabberClientCaps
{
	wchar_t *m_szNode;
	CJabberClientPartialCaps *m_pCaps;

public:
	CJabberClientCaps(const wchar_t *szNode);
	~CJabberClientCaps();

	CJabberClientPartialCaps* FindByVersion(const wchar_t *szHash);
	CJabberClientPartialCaps* FindById(int nIqId);

	JabberCapsBits            GetPartialCaps(const wchar_t *szVer);
	CJabberClientPartialCaps* SetPartialCaps(const wchar_t *szHash, const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId = -1);

	__inline wchar_t* GetNode() const { return m_szNode; }
};

__inline const wchar_t* CJabberClientPartialCaps::GetNode() const { return m_parent->GetNode(); }

class CJabberClientCapsManager
{
	mir_cs m_cs;
	OBJLIST<CJabberClientCaps> m_arCaps;

	CJabberProto *ppro;
	CMStringW m_szFeaturesCrc;

protected:
	CJabberClientCaps *FindClient(const wchar_t *szNode);

public:
	CJabberClientCapsManager(CJabberProto *proto);
	~CJabberClientCapsManager();

	const wchar_t* GetFeaturesCrc();
	void UpdateFeatHash();

	JabberCapsBits GetClientCaps(const wchar_t *szNode, const wchar_t *szHash);
	CJabberClientPartialCaps* GetPartialCaps(const wchar_t *szNode, const wchar_t *szHash);

	CJabberClientPartialCaps* SetClientCaps(const wchar_t *szNode, const wchar_t *szHash, const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId = -1);
	__inline CJabberClientPartialCaps* SetOwnCaps(const wchar_t *szNode, const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId = -1)
	{
		return SetClientCaps(szNode, m_szFeaturesCrc, szVer, jcbCaps, nIqId);
	}

	bool HandleInfoRequest(HXML iqNode, CJabberIqInfo *pInfo, const wchar_t *szNode);
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
	{	return (szService == nullptr) ? true : ServiceExists(szService);
	}
};

struct JabberFeatCapPairDynamic
{
	wchar_t *szExt;
	wchar_t *szFeature;
	JabberCapsBits jcbCap;
	wchar_t *szDescription;
};

extern const JabberFeatCapPair    g_JabberFeatCapPairs[];
extern const JabberFeatCapPairExt g_JabberFeatCapPairsExt[];

#endif
