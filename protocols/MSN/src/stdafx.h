/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2016 Miranda NG Team
Copyright (c) 2006-2011 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <commctrl.h>

#include <ctype.h>
#include <malloc.h>
#include <process.h>
#include <stdio.h>
#include <time.h>

#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <newpluginapi.h>

#include <m_clist.h>
#include <m_clui.h>
#include <m_contacts.h>
#include <m_idle.h>
#include <m_icolib.h>
#include <m_message.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <win2k.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_chat.h>
#include <m_avatars.h>
#include <m_timezones.h>
#include <m_extraicons.h>
#include <m_nudge.h>
#include <m_string.h>

#include "m_proto_listeningto.h"
#include "m_folders.h"
#include "m_metacontacts.h"

#include "ezxml.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
//	MSN error codes

#define ERR_SYNTAX_ERROR                 200
#define ERR_INVALID_PARAMETER            201
#define ERR_INVALID_FEDERATED_USER		 203
#define ERR_UNROUTABLE_USER				 204
#define ERR_INVALID_USER                 205
#define ERR_FQDN_MISSING                 206
#define ERR_ALREADY_LOGIN                207
#define ERR_INVALID_USERNAME             208
#define ERR_INVALID_FRIENDLY_NAME        209
#define ERR_LIST_FULL                    210
#define ERR_ALREADY_THERE                215
#define ERR_NOT_ON_LIST                  216
#define ERR_NOT_ONLINE                   217
#define ERR_ALREADY_IN_THE_MODE          218
#define ERR_ALREADY_IN_OPPOSITE_LIST     219
#define ERR_NOT_ACCEPTING_PAGES			 220
#define ERR_INVALID_MSISDN				 232
#define ERR_UNKNOWN_MSISDN				 233
#define ERR_UNKNOWN_KEITAI_DOMAIN		 234
#define ERR_INVALID_XML_DATA			 240
#define ERR_DETAILED_ERR_IN_PAYLOAD      241
#define ERR_SWITCHBOARD_FAILED           280
#define ERR_NOTIFY_XFR_FAILED            281
#define ERR_UNKNOWN_P2P_APP				 282
#define ERR_UNKNOWN_UUN_APP				 283
#define ERR_MESSAGE_TOO_LONG			 285
#define ERR_SMS_JUST_OUT_OF_FUNDS		 290
#define ERR_REQUIRED_FIELDS_MISSING      300
#define ERR_NOT_LOGGED_IN                302
#define ERR_ADDRESS_BOOK	 			 403
#define ERR_SMS_SUBSCRIPTION_REQUIRED	 413
#define ERR_SMS_SUBSCRIPTION_DISABLED	 414
#define ERR_SMS_OUT_OF_FUNDS			 415
#define ERR_SMS_DISABLED_MARKET			 416
#define ERR_SMS_DISABLED_GLOBAL			 417
#define ERR_TRY_AGAIN_LATER				 418
#define ERR_NO_MARKET_SPECIFIED			 419
#define ERR_INTERNAL_SERVER              500
#define ERR_DB_SERVER                    501
#define ERR_UPSDOWN                      504
#define ERR_LIST_UNAVAILABLE             508
#define ERR_PAGE_MODE_MESSAGE			 509
#define ERR_FILE_OPERATION               510
#define ERR_DETAILED_ERR				 511
#define ERR_MEMORY_ALLOC                 520
#define ERR_OIM_NOT_SUPPORTED			 537
#define ERR_SMS_ACCOUNT_MUTED			 550
#define ERR_SMS_ACCOUNT_DISABLED		 551
#define ERR_SMS_ACCOUNT_MAXED			 552
#define ERR_SMS_INTERNAL_SERVER			 580
#define ERR_SMS_CARRIER_INVALID			 590
#define ERR_SMS_CARRIER_NO_ROUTE		 591
#define ERR_SMS_CARRIER_ERRORED			 592
#define ERR_SMS_ADDRESS_MAPPING_FULL	 593
#define ERR_SMS_INCORRECT_SOURCE_CTRY	 594
#define ERR_SMS_MOBILE_CACHE_FULL		 595
#define ERR_SMS_INCORRECT_FORMAT		 596
#define ERR_SMS_INVALID_TEXT			 597
#define ERR_SMS_MESSAGE_TOO_LONG		 598
#define ERR_SERVER_BUSY                  600
#define ERR_SERVER_UNAVAILABLE           601
#define ERR_PEER_NS_DOWN                 602
#define ERR_DB_CONNECT                   603
#define ERR_SERVER_GOING_DOWN            604
#define ERR_PAGING_UNAVAILABLE			 606
#define ERR_CREATE_CONNECTION            707
#define ERR_INVALID_LOCALE               710
#define ERR_BLOCKING_WRITE               711
#define ERR_SESSION_OVERLOAD             712
#define ERR_USER_TOO_ACTIVE              713
#define ERR_TOO_MANY_SESSIONS            714
#define ERR_NOT_EXPECTED                 715
#define ERR_BAD_FRIEND_FILE              717
#define ERR_USER_RESTRICTED				 718
#define ERR_SESSION_FEDERATED			 719
#define ERR_USER_FEDERATED				 726
#define ERR_ROAMING_LOGOFF				 733
#define ERR_TOO_MANY_ENDPOINTS			 734
#define ERR_RATE_LIMIT_EXCEEDED			 800
#define ERR_PER_USER_LOGIN_LIMIT_EXCEED	 803
#define ERR_AUTHENTICATION_FAILED        911
#define ERR_NOT_ALLOWED_WHEN_OFFLINE     913
#define ERR_ACCOUNT_COMPROMISED			 915
#define ERR_NOT_ACCEPTING_NEW_USERS      920
#define ERR_TIMEDOUT					 921
#define ERR_CHILD_WITHOUT_FULL_CONSENT	 923
#define ERR_EMAIL_ADDRESS_NOT_VERIFIED   924
#define ERR_MGED_USER_LTD_ACC_WRONG_CLNT 926
#define ERR_MGED_USER_ACCESS_DENIED		 927
#define ERR_AUTH						 928
#define ERR_DOMAIN_RESERVED				 931
#define ERR_INVALID_CONTACT_LIST		 933
#define ERR_INVALID_SIGNATURE			 935

/////////////////////////////////////////////////////////////////////////////////////////
//	Global definitions

#define MSN_MAX_EMAIL_LEN        128
#define MSN_GUID_LEN              40

#define MSN_PACKETS_COMBINE         7
#define MSN_DEFAULT_PORT          443
#define MSN_DEFAULT_GATEWAY_PORT  443
const char MSN_DEFAULT_LOGIN_SERVER[] = "s.gateway.messenger.live.com";
const char MSN_DEFAULT_GATEWAY[] =      "geo.gateway.messenger.live.com";
const char MSN_USER_AGENT[] =           NETLIB_USER_AGENT;

#define MSN_BLOCK        "/BlockCommand"
#define MSN_INVITE       "/InviteCommand"
#define MSN_NETMEETING   "/NetMeeting"
#define MSN_VIEW_PROFILE "/ViewProfile"

#define MS_GOTO_INBOX		"/GotoInbox"
#define MS_EDIT_PROFILE		"/EditProfile"
#define MS_EDIT_ALERTS		"/EditAlerts"
#define MS_SET_NICKNAME_UI  "/SetNicknameUI"

extern const char sttVoidUid[];

/////////////////////////////////////////////////////////////////////////////////////////
//	MSN plugin functions

struct CMsnProto;

#define	MSN_ALLOW_MSGBOX    1
#define	MSN_ALLOW_ENTER	    2
#define	MSN_HOTMAIL_POPUP   4
#define MSN_SHOW_ERROR      8
#define	MSN_ALERT_POPUP	    16

void        HtmlDecode(char* str);
char*       HtmlEncode(const char* str);
bool		txtParseParam (const char* szData, const char* presearch, const char* start, const char* finish, char* param, const int size);
void		stripBBCode(char* src);
void		stripColorCode(char* src);
void		stripHTML(char* str);
void		parseWLID(char* wlid, char** net, char** email, char** inst);

char*		GetGlobalIp(void);

template <class chartype> void UrlDecode(chartype* str);

void		__cdecl MSN_ConnectionProc(HANDLE hNewConnection, DWORD dwRemoteIP, void*);

char*		MSN_GetAvatarHash(char* szContext, char** pszUrl = NULL);
bool		MSN_MsgWndExist(MCONTACT hContact);

#define		MSN_SendNickname(a) MSN_SendNicknameUtf(UTF8(a))

unsigned    MSN_GenRandom(void);

void        MSN_InitContactMenu(void);
void        MSN_RemoveContactMenus(void);

HANDLE      GetIconHandle(int iconId);
HICON       LoadIconEx(const char* name, bool big = false);
void        ReleaseIconEx(const char* name, bool big = false);

void        MsnInitIcons(void);

int         sttDivideWords(char* parBuffer, int parMinItems, char** parDest);
void		MSN_MakeDigest(const char* chl, char* dgst);
char*		getNewUuid(void);
time_t		IsoToUnixTime(const char *stamp);
time_t		MsnTSToUnixtime(const char *pszTS);

TCHAR* EscapeChatTags(const TCHAR* pszText);
TCHAR* UnEscapeChatTags(TCHAR* str_in);

void   overrideStr(TCHAR*& dest, const TCHAR* src, bool unicode, const TCHAR* def = NULL);

char* arrayToHex(BYTE* data, size_t datasz);

inline unsigned short _htons(unsigned short s)
{
	return s>>8|s<<8;
}

inline unsigned long _htonl(unsigned long s)
{
	return s<<24|(s&0xff00)<<8|((s>>8)&0xff00)|s>>24;
}

inline unsigned __int64 _htonl64(unsigned __int64 s)
{
	return (unsigned __int64)_htonl(s & 0xffffffff) << 32 | _htonl(s >> 32);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup interface

typedef struct _tag_PopupData
{
	unsigned flags;
	char* url;
	TCHAR* title;
	TCHAR* text;
	CMsnProto* proto;
} PopupData;

struct STRLIST : public LIST<char>
{
	static int compare(const char* p1, const char* p2)
	{ return _stricmp(p1, p2); }

	STRLIST() : LIST<char>(2, compare) {}
	~STRLIST() { destroy(); }

	void destroy( void )
	{
		for (int i=0; i < count; i++)
			mir_free(items[i]);

		List_Destroy((SortedList*)this);
	}

	int insertn(const char* p) { return insert(mir_strdup(p)); }

	int remove(int idx)
	{
		mir_free(items[idx]);
		return List_Remove((SortedList*)this, idx);
	}

	int remove(const char* p)
	{
		int idx;
		return  List_GetIndex((SortedList*)this, (char*)p, &idx) == 1 ? remove(idx) : -1;
	}
};


/////////////////////////////////////////////////////////////////////////////////////////
//	MIME headers processing

class MimeHeaders
{
public:

	MimeHeaders();
	MimeHeaders(unsigned);
	~MimeHeaders();

	void        clear(void);
	char*       decodeMailBody(char* msgBody);
	const char* find(const char* fieldName);
	char*       flipStr(const char* src, size_t len, char* dest);
	size_t      getLength(void);
	char*       readFromBuffer(char* src);
	char*       writeToBuffer(char* dest);

	void        addString(const char* name, const char* szValue, unsigned flags = 0);
	void        addLong(const char* name, long lValue, unsigned flags = 0);
	void        addULong(const char* name, unsigned lValue);
	void	    addBool(const char* name, bool lValue);

	const char* operator[](const char* fieldName) { return find(fieldName); }

	static wchar_t* decode(const char* val);

private:
	typedef struct tag_MimeHeader
	{
		const char* name;
		const char* value;
		unsigned flags;
	} MimeHeader;

	unsigned	mCount;
	unsigned	mAllocCount;
	MimeHeader* mVals;

	unsigned allocSlot(void);
};

/////////////////////////////////////////////////////////////////////////////////////////
//	File transfer helper

struct ThreadData;

struct HReadBuffer
{
	HReadBuffer(ThreadData* info, int iStart = 0);
	~HReadBuffer();

	BYTE* surelyRead(size_t parBytes);

	ThreadData* owner;
	BYTE* buffer;
	size_t totalDataSize;
	size_t startOffset;
};

enum TInfoType
{
	SERVER_NOTIFICATION,
	SERVER_SWITCHBOARD,
	SERVER_FILETRANS,
	SERVER_P2P_DIRECT,
	SERVER_HTTP
};



struct filetransfer
{
	filetransfer(CMsnProto* prt);
	~filetransfer(void);

	void close(void);
	void complete(void);
	int  create(void);
	int openNext(void);

	CMsnProto* proto;

	PROTOFILETRANSFERSTATUS std;

	bool        bCanceled;		// flag to interrupt a transfer
	bool        bCompleted;		// was a FT ever completed?
	bool        bAccepted;		// was a FT ever completed?

	int			fileId;			// handle of file being transferring (r/w)

	HANDLE		hLockHandle;
	HANDLE		hResumeEvt;

	ThreadData  *info;
	TInfoType	tType;
	TInfoType	tTypeReq;
	time_t		ts;
	clock_t     nNotify;
	unsigned	cf;

	bool        p2p_waitack;    // wait for ack
	bool        p2p_isV2;       // P2P V2

	unsigned    p2p_sessionid;	// session id
	unsigned    p2p_acksessid;	// acknowledged session id
	unsigned    p2p_sendmsgid;  // send message id
	unsigned    p2p_byemsgid;   // bye message id
	unsigned    p2p_ackID;		// number of ack's state
	unsigned    p2p_appID;		// application id: 1 = avatar, 2 = file transfer
	unsigned    p2p_type;		// application id: 1 = avatar, 2 = file transfer, 3 = custom emoticon
	char*       p2p_branch;		// header Branch: field
	char*       p2p_callID;		// header Call-ID: field
	char*       p2p_dest;		// destination e-mail address
	char*       p2p_object;     // MSN object for a transfer

	//---- receiving a file
	char*       szInvcookie;	// cookie for receiving

	unsigned __int64 lstFilePtr;
};

struct directconnection
{
	directconnection(const char* CallID, const char* Wlid);
	~directconnection();

	char* calcHashedNonce(UUID* nonce);
	char* mNonceToText(void);
	char* mNonceToHash(void) { return calcHashedNonce(mNonce); }
	void  xNonceToBin(UUID* nonce);

	UUID* mNonce;
	char* xNonce;

	char* callId;
	char* wlid;

	time_t ts;

	bool useHashedNonce;
	bool bAccepted;

	CMsnProto* proto;
};


#pragma pack(1)

typedef struct _tag_HFileContext
{
	unsigned len;
	unsigned ver;
	unsigned __int64 dwSize;
	unsigned type;
	wchar_t wszFileName[MAX_PATH];
	char unknown[30];
	unsigned id;
	char unknown2[64];
} HFileContext;

struct P2PB_Header
{
	virtual char* parseMsg(char *buf) = 0;
	virtual char* createMsg(char *buf, const char* wlid, CMsnProto *ppro) = 0;
	virtual bool isV2Hdr(void) = 0;
	virtual void logHeader(CMsnProto *ppro) = 0;
};

struct P2P_Header : P2PB_Header
{
	unsigned          mSessionID;
	unsigned          mID;
	unsigned __int64  mOffset;
	unsigned __int64  mTotalSize;
	unsigned          mPacketLen;
	unsigned          mFlags;
	unsigned          mAckSessionID;
	unsigned          mAckUniqueID;
	unsigned __int64  mAckDataSize;

	P2P_Header() { memset(&mSessionID, 0, 48); }
	P2P_Header(char *buf) { parseMsg(buf); }

	char* parseMsg(char *buf)  { memcpy(&mSessionID, buf, 48); return buf + 48; }
	char* createMsg(char *buf, const char* wlid, CMsnProto *ppro);
	bool isV2Hdr(void) { return false; }
	void logHeader(CMsnProto *ppro);
} ;

struct P2PV2_Header : P2PB_Header
{
	unsigned          mSessionID;
	unsigned          mID;
	const char*       mCap;
	unsigned __int64  mRemSize;
	unsigned          mPacketLen;
	unsigned          mPacketNum;
	unsigned          mAckUniqueID;
	unsigned char     mOpCode;
	unsigned char     mTFCode;

	P2PV2_Header() { memset(&mSessionID, 0, ((char*)&mTFCode - (char*)&mSessionID) + sizeof(mTFCode)); }
	P2PV2_Header(char *buf) { parseMsg(buf); }

	char* parseMsg(char *buf);
	char* createMsg(char *buf, const char* wlid, CMsnProto *ppro);
	bool isV2Hdr(void) { return true; }
	void logHeader(CMsnProto *ppro);
};

#pragma pack()

bool p2p_IsDlFileOk(filetransfer* ft);

/////////////////////////////////////////////////////////////////////////////////////////
//	Thread handling functions and datatypes

#define MSG_DISABLE_HDR      1
#define MSG_REQUIRE_ACK      2
#define MSG_RTL              4
#define MSG_OFFLINE          8
#define MSG_CONTACT			16
#define MSG_NUDGE			32

struct CMsnProto;
typedef void (__cdecl CMsnProto::*MsnThreadFunc)(void*);

/* Groupchat threadlist entry. As there is no more SB in MSNP21+, 
 * this is no longer in ThreadData and there are no more new 
 * Threads, but for code compatibility, we still have ThreadData
 * as a "main connection"
 */
struct GCUserItem
{
	char	WLID[MSN_MAX_EMAIL_LEN];
	TCHAR   role[8];
	BYTE    btag;
};

struct GCThreadData
{
   GCThreadData();
   ~GCThreadData();

   LIST<GCUserItem> mJoinedContacts;
   GCUserItem*   mCreator;
   GCUserItem*   mMe;
   TCHAR         mChatID[MSN_MAX_EMAIL_LEN];
   int			 netId;			// from mChatID
   char			 szEmail[MSN_MAX_EMAIL_LEN];	// frim mChatID
};

struct ThreadData
{
   ThreadData();
   ~ThreadData();

   STRLIST       mJoinedContactsWLID;
   STRLIST       mJoinedIdentContactsWLID;
   char*         mInitialContactWLID;

   TInfoType     mType;            // thread type
   MsnThreadFunc mFunc;            // thread entry point
   char          mServer[80];      // server name
   char			 mState[128];	   // state on XFR

   HANDLE        s;               // NetLib connection for the thread
   HANDLE        mIncomingBoundPort; // Netlib listen for the thread
   HANDLE        hWaitEvent;
   WORD          mIncomingPort;
   TCHAR         mChatID[10];
   bool          mIsMainThread;
   clock_t       mWaitPeriod;

   CMsnProto*    proto;

   //----| for gateways |----------------------------------------------------------------
   char          mSessionID[50]; // Gateway session ID
   char          mGatewayIP[80]; // Gateway IP address
   int           mGatewayTimeout;
   bool          sessionClosed;
   bool          termPending;
   bool          gatewayType;

   //----| for switchboard servers only |------------------------------------------------
   bool          firstMsgRecv;
   int           mCaller;
   char          mCookie[130];     // for switchboard servers only
   LONG          mTrid;            // current message ID
   UINT          mTimerId;         // typing notifications timer id

   //----| for file transfers only |-----------------------------------------------------
   filetransfer* mMsnFtp;          // file transfer block
   bool          mBridgeInit;

   //----| internal data buffer |--------------------------------------------------------
   int           mBytesInData;     // bytes available in data buffer
   char          *mData;           // data buffer for connection
   size_t        mDataSize;

   //----| methods |---------------------------------------------------------------------
   void          applyGatewayData(HANDLE hConn, bool isPoll);
   void          getGatewayUrl(char* dest, int destlen, bool isPoll);
   void          processSessionData(const char* xMsgr, const char* xHost);
   void          startThread(MsnThreadFunc , CMsnProto *prt);

   int           send(const char data[], size_t datalen);
   int           recv(char* data, size_t datalen);

   void          resetTimeout(bool term = false);
   bool          isTimeout(void);

   void          sendTerminate(void);
   void          sendCaps(void);
   int           sendMessage(int msgType, const char* email, int netId, const char* msg, int parFlags);
   int           sendRawMessage(int msgType, const char* data, int datLen);
   int           sendPacket(const char* cmd, const char* fmt, ...);
   int			 sendPacketPayload(const char* cmd, const char *param, const char* fmt, ...);

   int           contactJoined(const char* email);
   int           contactLeft(const char* email);
   MCONTACT      getContactHandle(void);
};


/////////////////////////////////////////////////////////////////////////////////////////
// MSN P2P session support

#define MSN_APPID_AVATAR		1
#define MSN_APPID_AVATAR2   	12
#define MSN_APPID_FILE			2
#define MSN_APPID_WEBCAM		4
#define MSN_APPID_MEDIA_SHARING	35
#define MSN_APPID_IMAGE			33

#define MSN_APPID_CUSTOMSMILEY  3
#define MSN_APPID_CUSTOMANIMATEDSMILEY  4

#define MSN_TYPEID_FTPREVIEW		0
#define MSN_TYPEID_FTNOPREVIEW		1
#define MSN_TYPEID_CUSTOMSMILEY		2
#define MSN_TYPEID_DISPLAYPICT		3
#define MSN_TYPEID_BKGNDSHARING		4
#define MSN_TYPEID_BKGNDIMG			5
#define MSN_TYPEID_WINK				8



inline bool IsChatHandle(MCONTACT hContact) { return (INT_PTR)hContact < 0; }


/////////////////////////////////////////////////////////////////////////////////////////
//	Message queue

#define MSGQUE_RAW	1

struct MsgQueueEntry
{
	char*          wlid;
	char*          message;
	filetransfer*  ft;
	STRLIST*       cont;
	int            msgType;
	int            msgSize;
	int            seq;
	int            allocatedToThread;
	time_t         ts;
	int            flags;
};

/////////////////////////////////////////////////////////////////////////////////////////
//	Avatars' queue
struct AvatarQueueEntry
{
	MCONTACT hContact;
	char *pszUrl;

	__forceinline AvatarQueueEntry(MCONTACT _contact, LPCSTR _url) :
		hContact(_contact),
		pszUrl( mir_strdup(_url))
	{}

	__forceinline ~AvatarQueueEntry()
	{	mir_free(pszUrl);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//	OAuth Token
class GenericToken {
public:
	GenericToken(const char *pszTokenName);
	~GenericToken();

	__forceinline void Init(CMsnProto* proto) { m_proto = proto; }
	bool Load();
	void Save();
	virtual bool Refresh(bool = false) { return false; }
	bool Expired(time_t t = time(NULL));
	void Clear();
	const char *Token();
	__forceinline operator char*() const { return m_pszToken; }
	void SetToken(const char *pszToken, time_t tExpires);
	//__forceinline const char *RefreshToken() { return m_pszRefreshToken; }

protected:
	const char *m_pszTokenName;
	char *m_pszToken;
	//char *m_pszRefreshToken;
	time_t m_tExpires;
	CMsnProto*    m_proto;
};

class OAuthToken : public GenericToken {
public:
	OAuthToken(const char *pszTokenName, const char *pszService, bool bPreprendT = false);

	bool Refresh(bool bForce = false);

private:
	const char *m_pszService;
	bool m_bPreprendT;
};

class SkypeToken : public GenericToken {
public:
	SkypeToken(const char *pszTokenName);
	bool Refresh(bool bForce = false);
	const char *XSkypetoken();
};


/////////////////////////////////////////////////////////////////////////////////////////
//	User lists

template< class T >  int CompareId(const T* p1, const T* p2)
{
	return _stricmp(p1->id, p2->id);
}

struct ServerGroupItem
{
	char* id;
	char* name; // in UTF8
};

struct MsnPlace
{
	char *id;
	unsigned cap1;
	unsigned cap2;
	unsigned client;
	char szClientVer[32];
	unsigned p2pMsgId;
	unsigned short p2pPktNum;

	~MsnPlace() {
		mir_free(id);
	}
};

struct MsnContact
{
	char *email;
	char *invite;
	char *nick;
	MCONTACT hContact;
	int list;
	int netId;
	int p2pMsgId;
	unsigned cap1;
	unsigned cap2;

	OBJLIST<MsnPlace> places;

	MsnContact() : email(0), invite(0), nick(0),
		hContact(0), list(0), netId(0), p2pMsgId(0),
		cap1(0), cap2(0), places(1, CompareId) {
	}

	~MsnContact() {
		mir_free(email);
		mir_free(nick);
		mir_free(invite);
	}
};

#define cap_OnlineViaMobile                 0x00000001
#define cap_OnlineMSN8User                  0x00000002
#define cap_SupportsGifInk                  0x00000004
#define cap_SupportsIsfInk                  0x00000008
#define cap_WebCamDetected                  0x00000010
#define cap_SupportsChunking                0x00000020
#define cap_MobileEnabled                   0x00000040
#define cap_WebWatchEnabled                 0x00000080
#define cap_SupportsActivities              0x00000100
#define cap_OnlineViaWebIM                  0x00000200
#define cap_MobileDevice                    0x00000400
#define cap_OnlineViaTGW                    0x00000800
#define cap_HasSpace                        0x00001000
#define cap_IsMceUser                       0x00002000
#define cap_SupportsDirectIM                0x00004000
#define cap_SupportsWinks                   0x00008000
#define cap_SupportsSharedSearch            0x00010000
#define cap_IsBot                           0x00020000
#define cap_SupportsVoiceIM                 0x00040000
#define cap_SupportsSChannel                0x00080000
#define cap_SupportsSipInvite               0x00100000
#define cap_SupportsMultipartyMedia         0x00200000
#define cap_SupportsSDrive                  0x00400000
#define cap_SupportsPageModeMessaging       0x00800000
#define cap_HasOneCare                      0x01000000
#define cap_SupportsTurn                    0x02000000
#define cap_SupportsP2PBootstrap            0x04000000
#define cap_UsingAlias                      0x08000000

#define capex_IsSmsOnly                     0x00000001
#define capex_SupportsVoiceOverMsnp         0x00000002
#define capex_SupportsUucpSipStack          0x00000004
#define capex_SupportsApplicationMsg        0x00000008
#define capex_RTCVideoEnabled               0x00000010
#define capex_SupportsPeerToPeerV2          0x00000020
#define capex_IsAuthWebIMUser               0x00000040
#define capex_Supports1On1ViaGroup          0x00000080
#define capex_SupportsOfflineIM             0x00000100
#define capex_SupportsSharingVideo          0x00000200
#define capex_SupportsNudges                0x00000400
#define capex_CircleVoiceIMEnabled          0x00000800
#define capex_SharingEnabled                0x00001000
#define capex_MobileSuspendIMFanoutDisable  0x00002000
#define capex_SupportsP2PMixerRelay         0x00008000
#define capex_ConvWindowFileTransfer        0x00020000
#define capex_VideoCallSupports16x9         0x00040000
#define capex_SupportsP2PEnveloping         0x00080000
#define capex_YahooIMDisabled               0x00400000
#define capex_SIPTunnelVersion2             0x00800000
#define capex_VoiceClipSupportsWMAFormat    0x01000000
#define capex_VoiceClipSupportsCircleIM     0x02000000
#define capex_SupportsSocialNewsObjectTypes 0x04000000
#define capex_CustomEmoticonsCapable        0x08000000
#define capex_SupportsUTF8MoodMessages      0x10000000
#define capex_FTURNCapable                  0x20000000
#define capex_SupportsP4Activity            0x40000000
#define capex_SupportsChats                 0x80000000

#define capexex_SupportsMissedConversations	0x00000001
#define capexex_SupportsShortCircuit		0x00000020

#define NETID_UNKNOWN	0
#define NETID_MSN		1
#define NETID_LCS		2
#define NETID_ALIAS		3
#define NETID_MOB		4
#define NETID_DOMAIN	5
#define NETID_SINK		6
#define NETID_CONTACT	7
#define NETID_SKYPE		8
#define NETID_CIRCLE	9
#define NETID_TMPCIRCLE	10
#define NETID_CID		11
#define NETID_APPID		12
#define NETID_CONNECT	13
#define NETID_REMOTE	14
#define NETID_SMTP		16
#define NETID_LVIDSINK	17
#define NETID_MULTICAST	18
#define NETID_THREAD	19
#define NETID_1TO1TEXT	21
#define NETID_GROUPTEXT	22
#define NETID_BOT		28
#define NETID_YAHOO		32
#define NETID_PUBSUBTPC	33
#define NETID_PUBSUBSUB	34
#define NETID_WNSSID	35

#define	LIST_FL         0x0001
#define	LIST_AL		    0x0002
#define	LIST_BL		    0x0004
#define	LIST_RL		    0x0008
#define LIST_PL		    0x0010
#define LIST_LL		    0x0080

#define	LIST_REMOVE     0x0100
#define	LIST_REMOVENH   0x0300

//	MSN plugin options
typedef struct _tag_MYOPTIONS
{
	bool		EnableSounds;

	bool		ShowErrorsAsPopups;
	bool		SlowSend;
	bool		ManageServer;

	char		szEmail[MSN_MAX_EMAIL_LEN];
	int			netId;
	char		szMachineGuid[MSN_GUID_LEN];
	char		szMachineGuidP2P[MSN_GUID_LEN];
}
MYOPTIONS;

//	Windows error class
struct TWinErrorCode
{
	WINAPI	TWinErrorCode();
	WINAPI	~TWinErrorCode();

	char*		WINAPI getText();

	long		mErrorCode;
	char*		mErrorText;
};

/////////////////////////////////////////////////////////////////////////////////////////
//	External variables

#define MSN_NUM_MODES 9

const char msnProtChallenge[] = "YMM8C_H7KCQ2S_KL";
const char msnProductID[] = "PROD0090YUAUV{2B";
const char msnAppID[] = "F6D2794D-501F-443A-ADBE-8F1490FF30FD";
const int  msnP24Ver = 2;
const char msnStoreAppId[] = "Skype";
const char msnProductVer[] = "0/6.16.0.105/259/";
const char msnProtID[] = "MSNP24";

extern HINSTANCE g_hInst, g_hOpenssl;
extern bool g_bTerminated;

///////////////////////////////////////////////////////////////////////////////
// UTF8 encode helper

class UTFEncoder
{
private:
	char* m_body;

public:
	UTFEncoder(const char* pSrc) :
		m_body(mir_utf8encode(pSrc)) {}

	UTFEncoder(const wchar_t* pSrc) :
		m_body(mir_utf8encodeW(pSrc)) {}

	~UTFEncoder() {  mir_free(m_body);	}
	const char* str() const { return m_body; }
};

#define UTF8(A) UTFEncoder(A).str()


typedef enum _tag_ConEnum
{
	conUnknown,
	conDirect,
	conUnknownNAT,
	conIPRestrictNAT,
	conPortRestrictNAT,
	conSymmetricNAT,
	conFirewall,
	conISALike
} ConEnum;

#pragma pack(1)
typedef struct _tag_UDPProbePkt
{
	unsigned char  version;
	unsigned char  serviceCode;
	unsigned short clientPort;
	unsigned	   clientIP;
	unsigned short discardPort;
	unsigned short testPort;
	unsigned	   testIP;
	unsigned       trId;
} UDPProbePkt;
#pragma pack()

extern const char* conStr[];

typedef struct _tag_MyConnectionType
{
	unsigned intIP;
	unsigned extIP;
	ConEnum udpConType;
	ConEnum tcpConType;
	unsigned weight;
	bool upnpNAT;
	bool icf;

	const IN_ADDR GetMyExtIP(void) { return *((PIN_ADDR)&extIP); }
	const char* GetMyExtIPStr(void) { return inet_ntoa(GetMyExtIP()); }
	const char* GetMyUdpConStr(void) { return conStr[udpConType]; }
	void SetUdpCon(const char* str);
	void CalculateWeight(void);
} MyConnectionType;

struct chunkedmsg
{
	char* id;
	char* msg;
	size_t size;
	size_t recvsz;
	bool bychunk;

	chunkedmsg(const char* tid, const size_t totsz, const bool bychunk);
	~chunkedmsg();

	void add(const char* msg, size_t offset, size_t portion);
	bool get(char*& tmsg, size_t& tsize);
};

struct DeleteParam
{
	CMsnProto *proto;
	MCONTACT hContact;
};

INT_PTR CALLBACK DlgDeleteContactUI(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

struct InviteChatParam
{
	TCHAR* id;
	MCONTACT hContact;
	CMsnProto* ppro;

	InviteChatParam(const TCHAR* id, MCONTACT hContact, CMsnProto* ppro)
		: id(mir_tstrdup(id)), hContact(hContact), ppro(ppro) {}

	~InviteChatParam() {
		mir_free(id);
	}
};

INT_PTR CALLBACK DlgInviteToChat(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);