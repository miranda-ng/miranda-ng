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

#ifndef _JABBER_H_
#define _JABBER_H_

#pragma warning(disable:4706 4121 4127)

/*******************************************************************
 * Global header files
 *******************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <richedit.h>
#define SECURITY_WIN32
#include <Security.h>

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <locale.h>
#include <malloc.h>
#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <newpluginapi.h>
#include <m_system.h>

#include <m_avatars.h>
#include <m_awaymsg.h>
#include <m_button.h>
#include <m_chat_int.h>
#include <m_clist.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_extraicons.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_genmenu.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_messagestate.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_srmm_int.h>
#include <m_timezones.h>
#include <m_tipper.h>
#include <m_toptoolbar.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_xstatus.h>
#include <m_imgsrvc.h>
#include <m_clc.h>
#include <m_xml.h>

#include <m_folders.h>
#include <m_fingerprint.h>
#include <m_jabber.h>
#include <m_popup.h>
#include <m_proto_listeningto.h>
#include <m_nudge.h>
#include <m_skin_eng.h>
#include <m_gui.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include "../../libs/zlib/src/zlib.h"

#include "resource.h"
#include "version.h"

#include "jabber_xml.h"
#include "jabber_byte.h"
#include "jabber_ibb.h"

struct CJabberProto;

struct CMPlugin : public ACCPROTOPLUGIN<CJabberProto>
{
	bool bMessageState = false, bSecureIM = false, bMirOTR = false, bNewGPG = false, bPlatform = false;
	char szRandom[17];

	CMPlugin();

	int Load() override;
	int Unload() override;
};

class CJabberDlgBase : public CProtoDlgBase<CJabberProto>
{
	typedef CProtoDlgBase<CJabberProto> CSuper;
	friend struct CJabberProto;

protected:
	__inline CJabberDlgBase(CJabberProto *proto, int idDialog) :
		CSuper(proto, idDialog)
	{
	}

	int Resizer(UTILRESIZECONTROL *urc)
	{
		switch (urc->wId) {
		case IDC_HEADERBAR:
			urc->rcItem.right = urc->dlgNewSize.cx;
			return 0;
		}

		return CSuper::Resizer(urc);
	}
};

/*******************************************************************
 * Global constants
 *******************************************************************/

#define GLOBAL_SETTING_MODULE	"JABBER"

#define JABBER_DEFAULT_PORT 5222
#define JABBER_MAX_JID_LEN  1024

#define JABBER_GC_MSG_QUIT				LPGEN("I'm happy Miranda NG user. Get it at https://miranda-ng.org/.")
#define JABBER_GC_MSG_SLAP				LPGEN("/me slaps %s around a bit with a large trout")
#define JABBER_SERVER_URL				"https://xmpp.net/services.php"

#define PGP_PROLOG "-----BEGIN PGP MESSAGE-----\r\n\r\n"
#define PGP_EPILOG "\r\n-----END PGP MESSAGE-----\r\n"

// registered db event types
#define EVENTTYPE_JABBER_CHATSTATES          2000
#define JS_DB_GETEVENTTEXT_CHATSTATES            "/GetEventText2000"
#define JABBER_DB_EVENT_CHATSTATES_GONE          1
#define EVENTTYPE_JABBER_PRESENCE            2001
#define JS_DB_GETEVENTTEXT_PRESENCE              "/GetEventText2001"
#define JABBER_DB_EVENT_PRESENCE_SUBSCRIBE       1
#define JABBER_DB_EVENT_PRESENCE_SUBSCRIBED      2
#define JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBE     3
#define JABBER_DB_EVENT_PRESENCE_UNSUBSCRIBED    4
#define JABBER_DB_EVENT_PRESENCE_ERROR           5

// User-defined message
#define WM_JABBER_TRANSPORT_REFRESH    (WM_PROTO_LAST + 102)
#define WM_JABBER_REFRESH_VCARD        (WM_PROTO_LAST + 117)

#define STATUS_TITLE_MAX 16
#define STATUS_DESC_MAX  64

// Error code
#define JABBER_ERROR_REDIRECT                    302
#define JABBER_ERROR_BAD_REQUEST                 400
#define JABBER_ERROR_UNAUTHORIZED                401
#define JABBER_ERROR_PAYMENT_REQUIRED            402
#define JABBER_ERROR_FORBIDDEN                   403
#define JABBER_ERROR_NOT_FOUND                   404
#define JABBER_ERROR_NOT_ALLOWED                 405
#define JABBER_ERROR_NOT_ACCEPTABLE              406
#define JABBER_ERROR_REGISTRATION_REQUIRED       407
#define JABBER_ERROR_REQUEST_TIMEOUT             408
#define JABBER_ERROR_CONFLICT                    409
#define JABBER_ERROR_INTERNAL_SERVER_ERROR       500
#define JABBER_ERROR_NOT_IMPLEMENTED             501
#define JABBER_ERROR_REMOTE_SERVER_ERROR         502
#define JABBER_ERROR_SERVICE_UNAVAILABLE         503
#define JABBER_ERROR_REMOTE_SERVER_TIMEOUT       504

// Vcard flags
#define JABBER_VCEMAIL_HOME                        1
#define JABBER_VCEMAIL_WORK                        2
#define JABBER_VCEMAIL_INTERNET                    4
#define JABBER_VCEMAIL_X400                        8

#define JABBER_VCTEL_HOME                     0x0001
#define JABBER_VCTEL_WORK                     0x0002
#define JABBER_VCTEL_VOICE                    0x0004
#define JABBER_VCTEL_FAX                      0x0008
#define JABBER_VCTEL_PAGER                    0x0010
#define JABBER_VCTEL_MSG                      0x0020
#define JABBER_VCTEL_CELL                     0x0040
#define JABBER_VCTEL_VIDEO                    0x0080
#define JABBER_VCTEL_BBS                      0x0100
#define JABBER_VCTEL_MODEM                    0x0200
#define JABBER_VCTEL_ISDN                     0x0400
#define JABBER_VCTEL_PCS                      0x0800

// File transfer setting
#define JABBER_OPTION_FT_DIRECT    0	// Direct connection
#define JABBER_OPTION_FT_PASS      1	// Use PASS server
#define JABBER_OPTION_FT_PROXY     2	// Use proxy with local port forwarding

// Font style saved in DB
#define JABBER_FONT_BOLD           1
#define JABBER_FONT_ITALIC         2

// Font for groupchat log dialog
#define JABBER_GCLOG_NUM_FONT      6	// 6 fonts (0:send, 1:msg, 2:time, 3:nick, 4:sys, 5:/me)

// Icon list
enum {
	JABBER_IDI_GCOWNER = 0,
	JABBER_IDI_GCADMIN,
	JABBER_IDI_GCMODERATOR,
	JABBER_IDI_GCVOICE,
	JABBER_ICON_TOTAL
};

// Services and Events
#define JS_PARSE_XMPP_URI          "/ParseXmppURI"

// Called when contact changes custom status and extra icon is set to clist_mw
//wParam = hContact    // contact changing status
//lParam = hIcon       // HANDLE to clist extra icon set as custom status
#define JE_CUSTOMSTATUS_EXTRAICON_CHANGED "/XStatusExtraIconChanged"
#define JE_CUSTOMSTATUS_CHANGED						"/XStatusChanged"

#define LR_BIGICON                 0x40

#define JS_SENDXML                 "/SendXML" // Warning: This service is obsolete. Use IJabberNetInterface::SendXmlNode() instead.

#define JS_HTTP_AUTH               "/HttpAuthRequest"
#define JS_INCOMING_NOTE_EVENT     "/IncomingNoteEvent"

#define DBSETTING_DISPLAY_UID      "display_uid"
#define DBSETTING_XSTATUSID        "XStatusId"
#define DBSETTING_XSTATUSNAME      "XStatusName"
#define DBSETTING_XSTATUSMSG       "XStatusMsg"

#define ADVSTATUS_MOOD             "mood"
#define ADVSTATUS_ACTIVITY         "activity"
#define ADVSTATUS_TUNE             "tune"

#define ADVSTATUS_VAL_ID           "id"
#define ADVSTATUS_VAL_ICON         "icon"
#define ADVSTATUS_VAL_TITLE        "title"
#define ADVSTATUS_VAL_TEXT         "text"

struct CJabberHttpAuthParams
{
	enum {IQ = 1, MSG = 2} m_nType;
	char *m_szFrom;
	char *m_szIqId;
	char *m_szThreadId;
	char *m_szId;
	char *m_szMethod;
	char *m_szUrl;
	CJabberHttpAuthParams()
	{
		memset(this, 0, sizeof(CJabberHttpAuthParams));
	}
	~CJabberHttpAuthParams()
	{
		Free();
	}
	void Free()
	{
		mir_free(m_szFrom);
		mir_free(m_szIqId);
		mir_free(m_szThreadId);
		mir_free(m_szId);
		mir_free(m_szMethod);
		mir_free(m_szUrl);
		memset(this, 0, sizeof(CJabberHttpAuthParams));
	}
};

/*******************************************************************
 * Global data structures and data type definitions
 *******************************************************************/

#define CAPS_BOOKMARK         0x0001
#define CAPS_BOOKMARKS_LOADED 0x8000

#define ZLIB_CHUNK_SIZE 2048

#include "jabber_caps.h"

extern CJabberClientCapsManager g_clientCapsManager;

#define JABBER_LOGIN_ROSTER				0x0001
#define JABBER_LOGIN_BOOKMARKS			0x0002
#define JABBER_LOGIN_SERVERINFO			0x0004
#define JABBER_LOGIN_BOOKMARKS_AJ		0x0008

struct JABBER_CONN_DATA : public MZeroedObject
{
	char username[512];
	char password[512];
	char server[128];
	char manualHost[128];
	int  port;
	BOOL useSSL;

	class CJabberDlgRegister *pDlg;
	void  SetProgress(int progress, const wchar_t *pwszText);
};

struct ThreadData
{
	ThreadData(CJabberProto *_pro, JABBER_CONN_DATA *_connData);
	~ThreadData();

	ptrA     szStreamId;
	char*    buffer;
	uint32_t lastWriteTime; // in ticks

	// network support
	HNETLIBCONN s;
	CJabberProto *proto;

	// XEP-0138 (Compression support)
	BOOL     useZlib;
	z_stream zStreamIn,zStreamOut;
	bool     zRecvReady;
	int      zRecvDatalen;
	char*    zRecvData;

	void     xmpp_client_query(void);

	bool     zlibInit(void);
	void     zlibUninit();
	int      zlibSend(char* data, int datalen);
	int      zlibRecv(char* data, long datalen);

	// for nick names resolving
	int      resolveID;
	MCONTACT resolveContact;

	// features & registration
	bool     bIsReg;
	bool     reg_done, bIsSessionAvailable;
	bool     bBookmarksLoaded;
	uint32_t	dwLoginRqs;

	// connection & login data
	JABBER_CONN_DATA conn;
	char     resource[128];
	char     fullJID[JABBER_MAX_JID_LEN];
	ptrA     tszNewPassword;

	char *gssapiHostName;

	CJabberIqInfo *pPendingQuery;
	JabberCapsBits jabberServerCaps;

	void  close(void);
	void  shutdown(void);
	int   recv(char* buf, size_t len);
	int   send(char* buffer, int bufsize = -1);
	int   send(TiXmlElement *node);
	int   send_no_strm_mgmt(TiXmlElement *node);

private:
	bool  bShutdown;
};

struct JABBER_MODEMSGS
{
	char *szOnline;
	char *szAway;
	char *szNa;
	char *szDnd;
	char *szFreechat;
};

enum JABBER_FT_TYPE { FT_SI, FT_OOB, FT_BYTESTREAM, FT_IBB, FT_HTTP };
enum JABBER_FILE_STATE { FT_CONNECTING, FT_INITIALIZING, FT_RECEIVING, FT_DONE, FT_ERROR, FT_DENIED };

struct filetransfer : public MZeroedObject
{
	filetransfer(CJabberProto *proto, struct JABBER_LIST_ITEM *item);
	~filetransfer();

	void close();
	void complete();
	int  create();

	PROTOFILETRANSFERSTATUS std;

	JABBER_FT_TYPE type;
	HNETLIBCONN s;
	JABBER_FILE_STATE state;
	JABBER_LIST_ITEM *pItem;
	char*  jid;
	int    fileId = -1;
	char*  szId;
	char*  sid;
	int    bCompleted;
	HANDLE hWaitEvent = INVALID_HANDLE_VALUE;

	// For type == FT_BYTESTREAM
	JABBER_BYTE_TRANSFER *jbt;

	JABBER_IBB_TRANSFER *jibb;

	// Used by file receiving only
	char* httpHostName;
	uint16_t httpPort;
	char* httpPath;
	unsigned __int64 dwExpectedRecvFileSize;

	// Used by file sending only
	HANDLE hFileEvent;
	unsigned __int64 *fileSize;
	wchar_t *szDescription;

	CJabberProto *ppro;
};

struct JABBER_GCLOG_FONT
{
	char face[LF_FACESIZE];		// LF_FACESIZE is from LOGFONT struct
	uint8_t style;
	char size;	// signed
	uint8_t charset;
	COLORREF color;
};

struct JABBER_FIELD_MAP
{
	int id;
	char* name;
};

enum JABBER_MUC_JIDLIST_TYPE
{
	MUC_VOICELIST,
	MUC_MEMBERLIST,
	MUC_MODERATORLIST,
	MUC_BANLIST,
	MUC_ADMINLIST,
	MUC_OWNERLIST
};

struct JABBER_MUC_JIDLIST_INFO : public MZeroedObject
{
	~JABBER_MUC_JIDLIST_INFO();

	JABBER_MUC_JIDLIST_TYPE type;
	char *roomJid;	// filled-in by the WM_JABBER_REFRESH code
	TiXmlDocument doc;
	TiXmlElement *iqNode;
	CJabberProto *ppro;
	void *pUserData;

	wchar_t* type2str(void) const;
};

//---- jabber_treelist.c ------------------------------------------------

typedef struct TTreeList_ItemInfo *HTREELISTITEM;
enum { TLM_TREE, TLM_REPORT };

#include "jabber_list.h"
#include "jabber_proto.h"

/*******************************************************************
 * Global variables
 *******************************************************************/
extern HANDLE hExtraMood;
extern HANDLE hExtraActivity;

extern char szCoreVersion[];

extern unsigned int g_nTempFileId;
extern int g_cbCountries;
extern struct CountryListEntry* g_countries;

extern HANDLE hExtListInit, hDiscoInfoResult;

/*******************************************************************
 * Function declarations
 *******************************************************************/

//---- jabber_treelist.c ------------------------------------------------

void TreeList_Create(HWND hwnd);
void TreeList_Destroy(HWND hwnd);
void TreeList_Reset(HWND hwnd);
void TreeList_SetMode(HWND hwnd, int mode);
HTREELISTITEM TreeList_GetActiveItem(HWND hwnd);
void TreeList_SetSortMode(HWND hwnd, int col, BOOL descending);
void TreeList_SetFilter(HWND hwnd, wchar_t *filter);
HTREELISTITEM TreeList_AddItem(HWND hwnd, HTREELISTITEM hParent, wchar_t *text, LPARAM data);
void TreeList_ResetItem(HWND hwnd, HTREELISTITEM hParent);
void TreeList_MakeFakeParent(HTREELISTITEM hItem, BOOL flag);
void TreeList_AppendColumn(HTREELISTITEM hItem, wchar_t *text);
int TreeList_AddIcon(HWND hwnd, HICON hIcon, int iOverlay);
void TreeList_SetIcon(HTREELISTITEM hItem, int iIcon, int iOverlay);
LPARAM TreeList_GetData(HTREELISTITEM hItem);
HTREELISTITEM TreeList_GetRoot(HWND hwnd);
int TreeList_GetChildrenCount(HTREELISTITEM hItem);
HTREELISTITEM TreeList_GetChild(HTREELISTITEM hItem, int i);
void TreeList_Update(HWND hwnd);
BOOL TreeList_ProcessMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT idc, BOOL *result);

//---- jabber_form.c ------------------------------------------------

enum TJabberFormControlType
{
	JFORM_CTYPE_NONE, JFORM_CTYPE_TEXT_PRIVATE, JFORM_CTYPE_TEXT_MULTI,
	JFORM_CTYPE_BOOLEAN, JFORM_CTYPE_LIST_SINGLE, JFORM_CTYPE_LIST_MULTI,
	JFORM_CTYPE_FIXED, JFORM_CTYPE_HIDDEN, JFORM_CTYPE_TEXT_SINGLE
};

struct TJabberFormControlInfo;

struct TJabberFormLayoutInfo
{
	TJabberFormLayoutInfo(HWND hwndCtrl, bool bCompact);

	TJabberFormControlInfo* AppendControl(TJabberFormControlType type, const char *labelStr, const char *valueStr);
	HWND CreateLabel(const wchar_t *pwszLabel);
	void OrderControls(int *formHeight);
	void PositionControl(TJabberFormControlInfo *item, const char *labelStr, const char *valueStr);

	HWND m_hwnd;
	int  m_ctrlHeight;
	int  m_offset, m_width, m_maxLabelWidth;
	int  m_yPos, m_ySpacing;
	int  m_id;
	bool m_bCompact;
};

void JabberFormCreateUI(HWND hwndStatic, const TiXmlElement *xNode, int *formHeight, BOOL bCompact = FALSE);
void JabberFormDestroyUI(HWND hwndStatic);
void JabberFormGetData(HWND hwndStatic, TiXmlElement* pRoot, const TiXmlElement *xNode);
void JabberFormSetInstruction(HWND hwndForm, const char *text);

class CJabberFormDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	TiXmlDocument m_doc;
	TiXmlElement *m_xNode;

	ptrA m_defTitle;	// Default title if no <title/> in xNode
	JABBER_FORM_SUBMIT_FUNC m_pfnSubmit, m_pfnCancel = nullptr;
	void *m_pUserdata;

	RECT m_frameRect;		// Clipping region of the frame to scroll
	int m_frameHeight;	// Height of the frame (can be eliminated, redundant to frameRect)
	int m_formHeight;		// Actual height of the form
	int m_curPos;			// Current scroll position

public:
	CJabberFormDlg(CJabberProto *ppro, const TiXmlElement *xNode, char *defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata);

	void Display();
	void GetData(TiXmlElement *xDest);

	void SetCancel(JABBER_FORM_SUBMIT_FUNC pFunc) {
		m_pfnCancel = pFunc;
	}

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};

//---- jabber_icolib.c ----------------------------------------------

void   g_IconsInit();
void   g_XstatusIconsInit();
void   g_XstatusIconsUninit();

//---- jabber_libstr.c ----------------------------------------------

int lstrcmp_null(const wchar_t *s1, const wchar_t *s2);

//---- jabber_menu.c ------------------------------------------------

void   g_MenuInit();
void   g_MenuUninit();
int    g_OnToolbarInit(WPARAM, LPARAM);

//---- jabber_adhoc.cpp	---------------------------------------------

struct CJabberAdhocStartupParams
{
	char *m_szJid;
	char *m_szNode;
	CJabberProto *m_pProto;

	CJabberAdhocStartupParams(CJabberProto *proto, const char *szJid, const char *szNode = nullptr)
	{
		m_pProto = proto;
		m_szJid = mir_strdup(szJid);
		m_szNode = szNode ? mir_strdup(szNode) : nullptr;
	}
	~CJabberAdhocStartupParams()
	{
		mir_free(m_szJid);
		mir_free(m_szNode);
	}
};

struct JabberAdHocData : public MZeroedObject
{
	CJabberProto *proto;

	int CurrentHeight;
	int curPos;
	int frameHeight;
	RECT frameRect;
	TiXmlDocument doc;
	TiXmlElement *AdHocNode;
	TiXmlElement *CommandsNode;
	char* ResponderJID;
};

//---- jabber_util.cpp ------------------------------------------------------------------

struct TStringPairsElem
{
	const char *name, *value;
};

struct TStringPairs
{
	TStringPairs(char*);
	~TStringPairs();

	const char* operator[](const char* name) const;

	int numElems;
	TStringPairsElem *elems;
};

typedef char JabberShaStrBuf[2*MIR_SHA1_HASH_SIZE + 1];

CMStringA MakeJid(const char *jid, const char *resource);

char*  JabberNickFromJID(const char *jid);
char*  JabberPrepareJid(const char *jid);
char*  JabberSha1(const char *str, JabberShaStrBuf buf);
void   JabberHttpUrlDecode(wchar_t *str);
int    JabberCombineStatus(int status1, int status2);
time_t JabberIsoToUnixTime(const char *stamp);
char*  JabberStripJid(const char *jid, char *dest, size_t destLen);
int    JabberGetPacketID(const char*);
char*  JabberId2string(int id);

__inline int JabberGetPacketID(const TiXmlElement *n)
{	return JabberGetPacketID(XmlGetAttr(n, "id"));
}

char*  time2str(time_t _time, char *buf, size_t bufLen);
time_t str2time(const char*);

wchar_t* JabberStrFixLines(const wchar_t *str);

wchar_t* JabberErrorStr(int errorCode);
CMStringW JabberErrorMsg(const TiXmlElement *errorNode, int *errorCode = nullptr);

void JabberCopyText(HWND hwnd, const char *text);
void JabberCopyText(HWND hwnd, const wchar_t *text);

const wchar_t *JabberStrIStr(const wchar_t *str, const wchar_t *substr);
CJabberProto*  JabberChooseInstance(bool bIsLink=false);

bool JabberReadXep203delay(const TiXmlElement *node, time_t &msgTime);

void SetWindowTextUtf(HWND hwndDlg, const char *szValue);
void SetDlgItemTextUtf(HWND hwndDlg, int ctrlId, const char *szValue);

int  UIEmulateBtnClick(HWND hwndDlg, UINT idcButton);
void UIShowControls(HWND hwndDlg, int *idList, int nCmdShow);

//---- jabber_userinfo.cpp --------------------------------------------------------------

void JabberUserInfoUpdate(MCONTACT hContact);

#endif
