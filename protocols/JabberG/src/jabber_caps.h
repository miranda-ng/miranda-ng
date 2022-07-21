/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

#define JABBER_FEAT_DISCO_INFO                  "http://jabber.org/protocol/disco#info"
#define JABBER_CAPS_DISCO_INFO                  ((JabberCapsBits)1)

#define JABBER_FEAT_DISCO_ITEMS                 "http://jabber.org/protocol/disco#items"
#define JABBER_CAPS_DISCO_ITEMS                 ((JabberCapsBits)1<<1)

#define JABBER_FEAT_ENTITY_CAPS                 "http://jabber.org/protocol/caps"
#define JABBER_CAPS_ENTITY_CAPS                 ((JabberCapsBits)1<<2)

#define JABBER_FEAT_SI                          "http://jabber.org/protocol/si"
#define JABBER_CAPS_SI                          ((JabberCapsBits)1<<3)

#define JABBER_FEAT_SI_FT                       "http://jabber.org/protocol/si/profile/file-transfer"
#define JABBER_CAPS_SI_FT                       ((JabberCapsBits)1<<4)

#define JABBER_FEAT_BYTESTREAMS                 "http://jabber.org/protocol/bytestreams"
#define JABBER_CAPS_BYTESTREAMS                 ((JabberCapsBits)1<<5)

#define JABBER_FEAT_IBB                         "http://jabber.org/protocol/ibb"
#define JABBER_CAPS_IBB                         ((JabberCapsBits)1<<6)

#define JABBER_FEAT_OOB                         "jabber:iq:oob"
#define JABBER_FEAT_OOB2                        "jabber:x:oob"
#define JABBER_CAPS_OOB                         ((JabberCapsBits)1<<7)

#define JABBER_FEAT_COMMANDS                    "http://jabber.org/protocol/commands"
#define JABBER_CAPS_COMMANDS                    ((JabberCapsBits)1<<8)

#define JABBER_FEAT_REGISTER                    "jabber:iq:register"
#define JABBER_CAPS_REGISTER                    ((JabberCapsBits)1<<9)

#define JABBER_FEAT_MUC                         "http://jabber.org/protocol/muc"
#define JABBER_CAPS_MUC                         ((JabberCapsBits)1<<10)

#define JABBER_FEAT_CHATSTATES                  "http://jabber.org/protocol/chatstates"
#define JABBER_CAPS_CHATSTATES                  ((JabberCapsBits)1<<11)

#define JABBER_FEAT_LAST_ACTIVITY               "jabber:iq:last"
#define JABBER_CAPS_LAST_ACTIVITY               ((JabberCapsBits)1<<12)

#define JABBER_FEAT_VERSION                     "jabber:iq:version"
#define JABBER_CAPS_VERSION                     ((JabberCapsBits)1<<13)

#define JABBER_FEAT_ENTITY_TIME                 "urn:xmpp:time"
#define JABBER_CAPS_ENTITY_TIME                 ((JabberCapsBits)1<<14)

#define JABBER_FEAT_PING                        "urn:xmpp:ping"
#define JABBER_CAPS_PING                        ((JabberCapsBits)1<<15)

#define JABBER_FEAT_DATA_FORMS                  "jabber:x:data"
#define JABBER_CAPS_DATA_FORMS                  ((JabberCapsBits)1<<16)

#define JABBER_FEAT_CHAT_MARKERS                "urn:xmpp:chat-markers:0"
#define JABBER_CAPS_CHAT_MARKERS                ((JabberCapsBits)1<<17)

#define JABBER_FEAT_VCARD_TEMP                  "vcard-temp"
#define JABBER_CAPS_VCARD_TEMP                  ((JabberCapsBits)1<<18)

#define JABBER_FEAT_MAM                         "urn:xmpp:mam:2"
#define JABBER_CAPS_MAM                         ((JabberCapsBits)1<<19)

#define JABBER_FEAT_XHTML                       "http://jabber.org/protocol/xhtml-im"
#define JABBER_CAPS_XHTML                       ((JabberCapsBits)1<<20)

#define JABBER_FEAT_AGENTS                      "jabber:iq:agents"
#define JABBER_CAPS_AGENTS                      ((JabberCapsBits)1<<21)

#define JABBER_FEAT_BROWSE                      "jabber:iq:browse"
#define JABBER_CAPS_BROWSE                      ((JabberCapsBits)1<<22)

#define JABBER_FEAT_FEATURE_NEG                 "http://jabber.org/protocol/feature-neg"
#define JABBER_CAPS_FEATURE_NEG                 ((JabberCapsBits)1<<23)

#define JABBER_FEAT_AMP                         "http://jabber.org/protocol/amp"
#define JABBER_CAPS_AMP                         ((JabberCapsBits)1<<24)

#define JABBER_FEAT_USER_MOOD                   "http://jabber.org/protocol/mood"
#define JABBER_CAPS_USER_MOOD                   ((JabberCapsBits)1<<25)

#define JABBER_FEAT_USER_MOOD_NOTIFY            "http://jabber.org/protocol/mood+notify"
#define JABBER_CAPS_USER_MOOD_NOTIFY            ((JabberCapsBits)1<<26)

#define JABBER_FEAT_PUBSUB                      "http://jabber.org/protocol/pubsub"
#define JABBER_CAPS_PUBSUB                      ((JabberCapsBits)1<<27)

#define JABBER_FEAT_SECUREIM                    "http://miranda-ng.org/caps/secureim"
#define JABBER_CAPS_SECUREIM                    ((JabberCapsBits)1<<28)

#define JABBER_FEAT_PRIVACY_LISTS               "jabber:iq:privacy"
#define JABBER_CAPS_PRIVACY_LISTS               ((JabberCapsBits)1<<29)

#define JABBER_FEAT_MESSAGE_RECEIPTS            "urn:xmpp:receipts"
#define JABBER_CAPS_MESSAGE_RECEIPTS            ((JabberCapsBits)1<<30)

#define JABBER_FEAT_USER_TUNE                   "http://jabber.org/protocol/tune"
#define JABBER_CAPS_USER_TUNE                   ((JabberCapsBits)1<<31)

#define JABBER_FEAT_USER_TUNE_NOTIFY            "http://jabber.org/protocol/tune+notify"
#define JABBER_CAPS_USER_TUNE_NOTIFY            ((JabberCapsBits)1<<32)

#define JABBER_FEAT_PRIVATE_STORAGE             "jabber:iq:private"
#define JABBER_CAPS_PRIVATE_STORAGE             ((JabberCapsBits)1<<33)

#define JABBER_FEAT_ARCHIVE_AUTO                "urn:xmpp:archive:auto"
#define JABBER_CAPS_ARCHIVE_AUTO                ((JabberCapsBits)1<<34)

#define JABBER_FEAT_ARCHIVE_MANAGE              "urn:xmpp:archive:manage"
#define JABBER_CAPS_ARCHIVE_MANAGE              ((JabberCapsBits)1<<35)

#define JABBER_FEAT_ATTENTION                   "urn:xmpp:attention:0"
#define JABBER_CAPS_ATTENTION                   ((JabberCapsBits)1<<36)

#define JABBER_FEAT_USER_ACTIVITY               "http://jabber.org/protocol/activity"
#define JABBER_CAPS_USER_ACTIVITY               ((JabberCapsBits)1<<37)

#define JABBER_FEAT_USER_ACTIVITY_NOTIFY        "http://jabber.org/protocol/activity+notify"
#define JABBER_CAPS_USER_ACTIVITY_NOTIFY        ((JabberCapsBits)1<<38)

#define JABBER_FEAT_MIRANDA_NOTES               "http://miranda-ng.org/storage#notes"
#define JABBER_CAPS_MIRANDA_NOTES               ((JabberCapsBits)1<<39)

#define JABBER_FEAT_JINGLE                      "urn:xmpp:jingle:1"
#define JABBER_CAPS_JINGLE                      ((JabberCapsBits)1<<40)

#define JABBER_FEAT_ROSTER_EXCHANGE             "http://jabber.org/protocol/rosterx"
#define JABBER_CAPS_ROSTER_EXCHANGE             ((JabberCapsBits)1<<41)

#define JABBER_FEAT_MIROTR                      "http://miranda-ng.org/caps/mirotr"
#define JABBER_CAPS_MIROTR                      ((JabberCapsBits)1<<42)

#define JABBER_FEAT_NEWGPG                      "http://miranda-ng.org/caps/new_gpg"
#define JABBER_CAPS_NEWGPG                      ((JabberCapsBits)1<<43)

#define JABBER_CAPS_PLATFORMX86                 ((JabberCapsBits)1<<44)
#define JABBER_CAPS_PLATFORMX64                 ((JabberCapsBits)1<<45)

#define JABBER_FEAT_OMEMO                       "eu.siacs.conversations.axolotl"
#define JABBER_CAPS_OMEMO                       ((JabberCapsBits)1<<46)

#define JABBER_FEAT_OMEMO_DEVICELIST_NOTIFY     JABBER_FEAT_OMEMO ".devicelist+notify"
#define JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY     ((JabberCapsBits)1<<47)

#define JABBER_FEAT_DIRECT_MUC_INVITE           "jabber:x:conference"
#define JABBER_CAPS_DIRECT_MUC_INVITE           ((JabberCapsBits)1<<48)

#define JABBER_FEAT_CARBONS                     "urn:xmpp:carbons:2"
#define JABBER_CAPS_CARBONS                     ((JabberCapsBits)1<<49)

#define JABBER_FEAT_BITS                        "urn:xmpp:bob"
#define JABBER_CAPS_BITS                        ((JabberCapsBits)1<<50)

#define JABBER_FEAT_ARCHIVE                     "urn:xmpp:archive"
#define JABBER_FEAT_BIND                        "urn:ietf:params:xml:ns:xmpp-bind"
#define JABBER_FEAT_CAPTCHA                     "urn:xmpp:captcha"
#define JABBER_FEAT_CSI                         "urn:xmpp:csi:0"
#define JABBER_FEAT_JUD                         "jabber:iq:search"
#define JABBER_FEAT_IDLE                        "urn:xmpp:idle:1"
#define JABBER_FEAT_SERVER_AVATAR               "storage:client:avatar"
#define JABBER_FEAT_SID                         "urn:xmpp:sid:0"
#define JABBER_FEAT_SOFTWARE_INFO               "urn:xmpp:dataforms:softwareinfo"
#define JABBER_FEAT_UPLOAD                      "urn:xmpp:http:upload"
#define JABBER_FEAT_UPLOAD0                     "urn:xmpp:http:upload:0"

#define JABBER_FEAT_PUBSUB_EVENT                "http://jabber.org/protocol/pubsub#event"
#define JABBER_FEAT_PUBSUB_NODE_CONFIG          "http://jabber.org/protocol/pubsub#node_config"

#define JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY  ((JabberCapsBits)1<<63)
#define JABBER_CAPS_OTHER_SPECIAL               (JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY|JABBER_RESOURCE_CAPS_ERROR) // must contain all the caps not listed in g_JabberFeatCapPairs, to prevent using these bits for features registered through IJabberNetInterface::RegisterFeature()

#define JABBER_CAPS_MIRANDA_NODE    "http://miranda-ng.org/caps"
#define JABBER_CAPS_MIRANDA_PARTIAL (JABBER_CAPS_DISCO_INFO | JABBER_CAPS_DISCO_ITEMS | JABBER_CAPS_MUC | JABBER_CAPS_ENTITY_CAPS | JABBER_CAPS_SI | JABBER_CAPS_SI_FT | \
                                     JABBER_CAPS_BYTESTREAMS | JABBER_CAPS_IBB | JABBER_CAPS_OOB | JABBER_CAPS_CHATSTATES | JABBER_CAPS_AGENTS | JABBER_CAPS_BROWSE | \
											    JABBER_CAPS_VERSION | JABBER_CAPS_LAST_ACTIVITY | JABBER_CAPS_DATA_FORMS | JABBER_CAPS_VCARD_TEMP | \
												 JABBER_CAPS_ENTITY_TIME | JABBER_CAPS_PING | JABBER_CAPS_PRIVACY_LISTS | JABBER_CAPS_MESSAGE_RECEIPTS | JABBER_CAPS_PRIVATE_STORAGE | \
												 JABBER_CAPS_ROSTER_EXCHANGE | JABBER_CAPS_DIRECT_MUC_INVITE | JABBER_CAPS_CHAT_MARKERS | JABBER_CAPS_BITS | JABBER_CAPS_XHTML)

#define JABBER_CAPS_MIRANDA_ALL     (JABBER_CAPS_MIRANDA_PARTIAL | JABBER_CAPS_COMMANDS | \
                                     JABBER_CAPS_USER_MOOD_NOTIFY | JABBER_CAPS_USER_TUNE_NOTIFY | JABBER_CAPS_USER_ACTIVITY_NOTIFY | \
												 JABBER_CAPS_PLATFORMX86 | JABBER_CAPS_PLATFORMX64)

#define JABBER_XMLNS_FORWARD                    "urn:xmpp:forward:0"

#define JABBER_EXT_SECUREIM                     "secureim"
#define JABBER_EXT_MIROTR                       "mirotr"
#define JABBER_EXT_JINGLE                       "jingle"
#define JABBER_EXT_NEWGPG                       "new_gpg"
#define JABBER_EXT_OMEMO                        "omemo"
#define JABBER_EXT_NUDGE                        "nudge"
#define JABBER_EXT_COMMANDS                     "cmds"
#define JABBER_EXT_USER_MOOD                    "mood"
#define JABBER_EXT_USER_TUNE                    "tune"
#define JABBER_EXT_USER_ACTIVITY                "activity"
#define JABBER_EXT_MIR_NOTES                    "mir_notes"
#define JABBER_EXT_PLATFORMX86                  "x86"
#define JABBER_EXT_PLATFORMX64                  "x64"

#define JABBER_FEAT_EXT_ADDRESSING              "http://jabber.org/protocol/address"
#define JABBER_FEAT_NESTED_ROSTER_GROUPS        "roster:delimiter"

#define JABBER_FEAT_RC                          "http://jabber.org/protocol/rc"
#define JABBER_FEAT_RC_SET_STATUS               "http://jabber.org/protocol/rc#set-status"
#define JABBER_FEAT_RC_SET_OPTIONS              "http://jabber.org/protocol/rc#set-options"
#define JABBER_FEAT_RC_FORWARD                  "http://jabber.org/protocol/rc#forward"
#define JABBER_FEAT_RC_LEAVE_GROUPCHATS			"http://jabber.org/protocol/rc#leave-groupchats"
#define JABBER_FEAT_RC_WS_LOCK                  "http://miranda-ng.org/rc#lock_workstation"
#define JABBER_FEAT_RC_QUIT_MIRANDA             "http://miranda-ng.org/rc#quit"

#define JABBER_FEAT_IQ_ROSTER                   "jabber:iq:roster"
#define JABBER_FEAT_DELAY                       "jabber:x:delay"
#define JABBER_FEAT_ENTITY_TIME_OLD             "jabber:iq:time"

#define JABBER_FEAT_MUC_ADMIN                   "http://jabber.org/protocol/muc#admin"
#define JABBER_FEAT_MUC_OWNER                   "http://jabber.org/protocol/muc#owner"
#define JABBER_FEAT_MUC_USER                    "http://jabber.org/protocol/muc#user"

#define JABBER_FEAT_NICK                        "http://jabber.org/protocol/nick"

#define JABBER_FEAT_HTTP_AUTH                   "http://jabber.org/protocol/http-auth"


class CJabberClientPartialCaps
{
	ptrA m_szHash, m_szOs, m_szOsVer, m_szSoft, m_szSoftVer, m_szSoftMir;
	JabberCapsBits m_jcbCaps = JABBER_RESOURCE_CAPS_UNINIT;
	int m_nIqId = -1, m_iTime;
	uint32_t m_dwRequestTime = 0;

	class CJabberClientCaps *m_parent;
	CJabberClientPartialCaps *m_pNext = nullptr;

public:
	CJabberClientPartialCaps(CJabberClientCaps *pParent, const char *szHash, const char *szVer);
	~CJabberClientPartialCaps();

	CJabberClientPartialCaps* SetNext(CJabberClientPartialCaps *pCaps);
	__inline CJabberClientPartialCaps* GetNext()
	{	return m_pNext;
	}

	void SetCaps(JabberCapsBits jcbCaps, int nIqId = -1);
	JabberCapsBits GetCaps();

	__inline int GetTime() const { return m_iTime; }
	__inline const char* GetHash() const { return m_szHash.get(); }
	__inline const char* GetNode() const;

	__inline const char* GetOs() const { return m_szOs.get(); }
	__inline const char* GetOsVer() const { return m_szOsVer.get(); }
	__inline const char* GetSoft() const { return m_szSoft.get(); }
	__inline const char* GetSoftVer() const { return m_szSoftVer.get(); }
	__inline const char* GetSoftMir() const { return m_szSoftMir.get(); }

	__inline void SetTime(int val) { m_iTime = val; }
	__inline void SetOs(const char *str) { m_szOs = mir_strdup(str); }
	__inline void SetOsVer(const char *str) { m_szOsVer = mir_strdup(str); }
	__inline void SetSoft(const char *str) { m_szSoft = mir_strdup(str); }
	__inline void SetSoftVer(const char *str) { m_szSoftVer = mir_strdup(str); }
	__inline void SetSoftMir(const char *str) { m_szSoftMir = mir_strdup(str); }

	__inline int GetIqId() const { return m_nIqId; }

	__inline void SetVer(const char *szVer)
	{
		m_szSoft = mir_strdup(szVer);
	}
};

class CJabberClientCaps
{
	char *m_szNode;
	CJabberClientPartialCaps *m_pCaps;

public:
	CJabberClientCaps(const char *szNode);
	~CJabberClientCaps();

	CJabberClientPartialCaps* FindByVersion(const char *szHash);
	CJabberClientPartialCaps* FindById(int nIqId);

	JabberCapsBits            GetPartialCaps(const char *szVer);
	CJabberClientPartialCaps* SetPartialCaps(const char *szHash, const char *szVer, JabberCapsBits jcbCaps, int nIqId = -1);

	__inline char *GetNode() const { return m_szNode; }
	__inline CJabberClientPartialCaps *GetFirst() const { return m_pCaps; }
};

__inline const char* CJabberClientPartialCaps::GetNode() const { return m_parent->GetNode(); }

class CJabberClientCapsManager
{
	mir_cs m_cs;
	OBJLIST<CJabberClientCaps> m_arCaps;

protected:
	CJabberClientCaps* FindClient(const char *szNode);

public:
	CJabberClientCapsManager();
	~CJabberClientCapsManager();

	void Load();
	void Save();

	CJabberClientPartialCaps* GetPartialCaps(const char *szNode, const char *szHash);

	CJabberClientPartialCaps* SetClientCaps(const char *szNode, const char *szHash, const char *szVer, JabberCapsBits jcbCaps, int nIqId = -1);
};

struct JabberFeatCapPair
{
	const char *szFeature;
	JabberCapsBits jcbCap;
	const char *tszDescription;
};

struct JabberFeatCapPairExt
{
	const char *szFeature;
	JabberCapsBits jcbCap;
	LPCSTR szService;

	int Valid() const
	{	return (szService == nullptr) ? true : ServiceExists(szService);
	}
};

struct JabberFeatCapPairDynamic
{
	char *szExt;
	char *szFeature;
	JabberCapsBits jcbCap;
	char *szDescription;
};

extern const int g_cJabberFeatCapPairs;
extern const JabberFeatCapPair g_JabberFeatCapPairs[];

extern const int g_cJabberFeatCapPairsExt;
extern const JabberFeatCapPairExt g_JabberFeatCapPairsExt[];

#endif
