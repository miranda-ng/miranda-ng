/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-08  George Hazan
Copyright ( C ) 2007     Maxim Mluhov
Copyright ( C ) 2008-09  Dmitriy Chervov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL: https://miranda.svn.sourceforge.net/svnroot/miranda/trunk/miranda/include/m_jabber.h $
Revision       : $Revision: 9996 $
Last change on : $Date: 2009-11-15 13:27:32 +0200 $
Last change by : $Author: dchervov $

*/

#ifndef M_JABBER_H__
#define M_JABBER_H__

#ifdef __cplusplus

#include <m_xml.h>

struct IJabberInterface;

struct IJabberSysInterface
{
	// Returns version of IJabberSysInterface.
	virtual int STDMETHODCALLTYPE		GetVersion() const = 0;

	// Compares JIDs by their node@domain part (without resource name).
	virtual int STDMETHODCALLTYPE		CompareJIDs(LPCTSTR jid1, LPCTSTR jid2) = 0;

	// Returns contact handle for given JID, or NULL on error.
	virtual HANDLE STDMETHODCALLTYPE	ContactFromJID(LPCTSTR jid) = 0;

	// Returns JID of hContact, or NULL on error. You must free the result using mir_free().
	virtual LPTSTR STDMETHODCALLTYPE	ContactToJID(HANDLE hContact) = 0;

	// Returns best resource name for given JID, or NULL on error. You must free the result using mir_free().
	virtual LPTSTR STDMETHODCALLTYPE	GetBestResourceName(LPCTSTR jid) = 0;

	// Returns all resource names for a given JID in format "resource1\0resource2\0resource3\0\0" (all resources are separated by \0 character and the whole string is terminated with two \0 characters), or NULL on error. You must free returned string using mir_free().
	virtual LPTSTR STDMETHODCALLTYPE	GetResourceList(LPCTSTR jid) = 0;

	// Returns Jabber module name. DO NOT free the returned string.
	virtual char* STDMETHODCALLTYPE		GetModuleName() const = 0;
};

// Iq type flags
enum
{
	JABBER_IQ_TYPE_FAIL				= 0,
	JABBER_IQ_TYPE_RESULT			= 1,
	JABBER_IQ_TYPE_ERROR			= 2,
	JABBER_IQ_TYPE_GET				= 4,
	JABBER_IQ_TYPE_SET				= 8,

	JABBER_IQ_TYPE_RESULT_ERROR		= JABBER_IQ_TYPE_RESULT | JABBER_IQ_TYPE_ERROR,
	JABBER_IQ_TYPE_GOOD				= JABBER_IQ_TYPE_RESULT | JABBER_IQ_TYPE_GET | JABBER_IQ_TYPE_SET,
	JABBER_IQ_TYPE_ANY				= JABBER_IQ_TYPE_RESULT | JABBER_IQ_TYPE_ERROR | JABBER_IQ_TYPE_GET | JABBER_IQ_TYPE_SET
};

// Message type flags
enum
{
	JABBER_MESSAGE_TYPE_FAIL		= 0,
	JABBER_MESSAGE_TYPE_NORMAL		= 1,
	JABBER_MESSAGE_TYPE_ERROR		= 2,
	JABBER_MESSAGE_TYPE_CHAT		= 4,
	JABBER_MESSAGE_TYPE_GROUPCHAT	= 8,
	JABBER_MESSAGE_TYPE_HEADLINE	= 0x10,

	JABBER_MESSAGE_TYPE_ANY			= JABBER_MESSAGE_TYPE_NORMAL | JABBER_MESSAGE_TYPE_ERROR | JABBER_MESSAGE_TYPE_CHAT | JABBER_MESSAGE_TYPE_GROUPCHAT | JABBER_MESSAGE_TYPE_HEADLINE
};

// Handler priority values.
// All handler calls are ordered by priority specified when adding a handler.
// For recv, it will go from lower to higher, so in this case:  check ignore, default.
// For send handlers, it will go in the opposite order:  default, check ignore, send.
// You may also use other values than specified here for your handler priority, if it's necessary.

// IMPORTANT: In all incoming stanza handlers, return FALSE to continue processing the stanza (Jabber plugin will then call other handlers), or TRUE to stop processing the stanza (if you're sure noone else needs to process this stanza).
// In an outgoing stanza handler it works in the same way: return TRUE to stop processing the stanza and abort sending.
// All handlers may modify the stanza if it's necessary; these modifications will be passed to remaining handlers.
enum
{
	JH_PRIORITY_IGNORE = 50,
	JH_PRIORITY_DEFAULT = 1000,
	JH_PRIORITY_UNHANDLED = 5000 // useful for example to catch unhandled Iq stanzas to send a correct reply.
};

typedef void* HJHANDLER;

typedef BOOL (*JABBER_HANDLER_FUNC)(IJabberInterface *ji, HXML node, void *pUserData);

struct IJabberNetInterface
{
	// Returns version of IJabberNetInterface.
	virtual int STDMETHODCALLTYPE			GetVersion() const = 0;

	// Returns id that can be used for next message sent through SendXmlNode().
	virtual unsigned int STDMETHODCALLTYPE	SerialNext() = 0;

	// Sends XML node.
	virtual int STDMETHODCALLTYPE			SendXmlNode(HXML node) = 0;


	// Registers incoming <presence/> handler. Returns handler handle on success or NULL on error.
	virtual HJHANDLER STDMETHODCALLTYPE		AddPresenceHandler(JABBER_HANDLER_FUNC Func, void *pUserData = NULL, int iPriority = JH_PRIORITY_DEFAULT) = 0;

	// Registers incoming <message/> handler for messages of types specified by iMsgTypes. iMsgTypes is a combination of JABBER_MESSAGE_TYPE_* flags. Returns handler handle on success or NULL on error.
	virtual HJHANDLER STDMETHODCALLTYPE		AddMessageHandler(JABBER_HANDLER_FUNC Func, int iMsgTypes, LPCTSTR szXmlns, LPCTSTR szTag, void *pUserData = NULL, int iPriority = JH_PRIORITY_DEFAULT) = 0;

	// Registers incoming <iq/> handler. iIqTypes is a combination of JABBER_IQ_TYPE_* flags. Returns handler handle on success or NULL on error.
	virtual HJHANDLER STDMETHODCALLTYPE		AddIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, LPCTSTR szXmlns, LPCTSTR szTag, void *pUserData = NULL, int iPriority = JH_PRIORITY_DEFAULT) = 0;

	// Registers temporary handler for incoming <iq/> stanza of type iIqType with id iIqId. iIqTypes is a combination of JABBER_IQ_TYPE_* flags. Returns handler handle on success or NULL on error.
	// If dwTimeout milliseconds pass and no Iq stanza with the specified iIqId is received, Jabber plugin will call Func() with NULL node.
	virtual HJHANDLER STDMETHODCALLTYPE		AddTemporaryIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, int iIqId, void *pUserData = NULL, DWORD dwTimeout = 30000, int iPriority = JH_PRIORITY_DEFAULT) = 0;

	// Registers handler for outgoing nodes. Returns handler handle on success or NULL on error.
	// Return FALSE in the handler to continue, or TRUE to abort sending.
	virtual HJHANDLER STDMETHODCALLTYPE		AddSendHandler(JABBER_HANDLER_FUNC Func, void *pUserData = NULL, int iPriority = JH_PRIORITY_DEFAULT) = 0;

	// Unregisters handler by its handle.
	virtual int STDMETHODCALLTYPE			RemoveHandler(HJHANDLER hHandler) = 0;


// Entity capabilities support (see xep-0115)
	// Registers feature so that it's displayed with proper description in other users' details. Call this function in your ME_SYSTEM_MODULESLOADED handler. Returns TRUE on success or FALSE on error.
	virtual int STDMETHODCALLTYPE			RegisterFeature(LPCTSTR szFeature, LPCTSTR szDescription) = 0;

	// Adds features to the list of features supported by the client. szFeatures is a list of features separated by \0 character and terminated with two \0 characters. You can call this function at any time. Returns TRUE on success or FALSE on error.
	virtual int STDMETHODCALLTYPE			AddFeatures(LPCTSTR szFeatures) = 0;

	// Removes features from the list of features supported by the client. szFeatures is a list of features separated by \0 character and terminated with two \0 characters. You can call this function at any time.
	virtual int STDMETHODCALLTYPE			RemoveFeatures(LPCTSTR szFeatures) = 0;

	// Returns features supported by JID in format "feature1\0feature2\0...\0featureN\0\0" (a list of features separated by \0 character and terminated with two \0 characters), or NULL on error. JID may contain resource name to get features of a specific resource. If there's no resource name, GetResourceFeatures() returns features for the same resource as IJabberSysInterface::GetBestResourceName() returns. If a feature you're checking for is not supported by Jabber plugin natively, you must register it with RegisterFeature(), otherwise GetContactFeatures() won't be able to return it. You must free returned string using mir_free().
	virtual LPTSTR STDMETHODCALLTYPE		GetResourceFeatures(LPCTSTR jid) = 0;
};

// IJabberInterface::dwFlags values
enum
{
	JIF_UNICODE = 1
};

// Overall Jabber interface
struct IJabberInterface
{
	// Set of JIF_* flags.
	virtual DWORD STDMETHODCALLTYPE					GetFlags() const = 0;

	// Returns version of IJabberInterface.
	virtual int STDMETHODCALLTYPE					GetVersion() const = 0;

	// Returns Jabber plugin version.
	virtual DWORD STDMETHODCALLTYPE					GetJabberVersion() const = 0;

	// Jabber system utilities.
	virtual IJabberSysInterface* STDMETHODCALLTYPE	Sys() const = 0;

	// Jabber network interface.
	virtual IJabberNetInterface* STDMETHODCALLTYPE	Net() const = 0;
};

/*
A service to obtain Jabber API for a given account.
If you store the pointer to the interface for later use, you must also hook ME_PROTO_ACCLISTCHANGED and update the pointer to prevent calling API for a removed account.

wParam = 0;
lParam = (LPARAM)(IJabberInterface**).

Returns FALSE if all is Ok, and TRUE otherwise.
*/
#define JS_GETJABBERAPI		"/GetJabberApi"

__forceinline IJabberInterface *getJabberApi(const char *szAccount)
{
	IJabberInterface *ji;
	if (!CallProtoService(szAccount, JS_GETJABBERAPI, 0, (LPARAM)&ji))
	{
		return ji;
	}
	return NULL;
}

/* Custom Status helper API *
   DB settings and write values to them (UTF-8 strings best). (obsolete)
 - use JS_GETXSTATUSEX and JS_SETXSTATUSEX for controling Custom Status
 */
#define CSSF_MASK_STATUS    0x0001  // status member valid for set/get
#define CSSF_MASK_NAME      0x0002  // pszName member valid for set/get
#define CSSF_MASK_MESSAGE   0x0004  // pszMessage member valid for set/get
#define CSSF_DISABLE_MENU   0x0020  // disable default custom status menu, wParam = bEnable
#define CSSF_DISABLE_UI     0x0040  // disable default custom status UI, wParam = bEnable
#define CSSF_DEFAULT_NAME   0x0080  // only with CSSF_MASK_NAME and get API to get default custom status name (wParam = status)
#define CSSF_STATUSES_COUNT 0x0100  // returns number of custom statuses in wParam, only get API
#define CSSF_STR_SIZES      0x0200  // returns sizes of custom status name & message (wParam & lParam members) in chars
#define CSSF_UNICODE        0x1000  // strings are in UCS-2

#if defined(_UNICODE)
  #define CSSF_TCHAR  CSSF_UNICODE
#else
  #define CSSF_TCHAR  0
#endif

typedef struct {
  int cbSize;         // size of the structure
  int flags;          // combination of CSSF_*
  int *status;        // custom status id
  union {
    char *pszMessage; // buffer for custom status message
    TCHAR *ptszMessage;
    WCHAR *pwszMessage;
  };
  WPARAM *wParam;     // extra params, see flags
  LPARAM *lParam;
} JABBER_CUSTOM_STATUS;

// Sets owner current custom status
//wParam = 0                          // reserved
//lParam = (JABBER_CUSTOM_STATUS*)pData  // contains what to set and new values
//return = 0 (for success)
#define JS_SETXSTATUSEX "/SetXStatusEx"

// Retrieves custom status details for specified hContact
//wParam = (HANDLE)hContact
//lParam = (JABBER_CUSTOM_STATUS*)pData  // receives details (members must be prepared)
//return = 0 (for success)

// not needed anymore and therefore commented out
//#define JS_GETXSTATUSEX "/GetXStatusEx"

#endif // __cplusplus

#endif // M_JABBER_H__
