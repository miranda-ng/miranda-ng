/*
 * $Id: yahoo.h 14085 2012-02-13 10:55:56Z george.hazan $
 *
 * myYahoo Miranda Plugin
 *
 * Authors: Gennady Feldman (aka Gena01)
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#ifndef _YAHOO_YAHOO_H_
#define _YAHOO_YAHOO_H_

#include <windows.h>
#include <shlwapi.h>

#include <malloc.h>
#include <sys/stat.h>
#include <io.h>
#include <time.h>

/*
 * Yahoo Services
 */
#define USE_STRUCT_CALLBACKS

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"
#include "libyahoo2/yahoo_util.h"

extern "C"
{
	#include <newpluginapi.h>
	#include <m_system.h>
};

#include <m_system_cpp.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_netlib.h>
#include <m_idle.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_langpack.h>
#include <win2k.h>

//=======================================================
//	Definitions
//=======================================================
// Build is a cvs build
//
// If defined, the build will add cvs info to the plugin info
#define YAHOO_CVSBUILD

#define YAHOO_LOGINSERVER					"LoginServer"
#define YAHOO_LOGINPORT						"LoginPort"
#define YAHOO_LOGINID						"yahoo_id"
#define YAHOO_PASSWORD						"Password"
#define YAHOO_PWTOKEN						"PWToken"
#define YAHOO_CHECKMAIL						"CheckMail"
#define YAHOO_CUSTSTATDB					"CustomStat"
#define YAHOO_DEFAULT_PORT					5050
#define YAHOO_DEFAULT_LOGIN_SERVER			"mcs.msg.yahoo.com"
#define YAHOO_DEFAULT_JAPAN_LOGIN_SERVER	"cs.yahoo.co.jp"
#define YAHOO_CUSTOM_STATUS					99

extern int do_yahoo_debug;

#define LOG(x) if (do_yahoo_debug) { debugLogA("%s:%d: ", __FILE__, __LINE__); \
	debugLogA x; \
	debugLogA(" ");}

#define YAHOO_SET_CUST_STAT    "/SetCustomStatCommand"
#define YAHOO_EDIT_MY_PROFILE  "/YahooEditMyProfileCommand"
#define YAHOO_SHOW_PROFILE     "/YahooShowProfileCommand"
#define YAHOO_SHOW_MY_PROFILE  "/YahooShowMyProfileCommand"
#define YAHOO_YAHOO_MAIL       "/YahooGotoMailboxCommand"
#define YAHOO_REFRESH          "/YahooRefreshCommand"
#define YAHOO_AB               "/YahooAddressBook"
#define YAHOO_CALENDAR         "/YahooCalendar"

#define STYLE_DEFAULTBGCOLOUR     RGB(173,206,247)

struct _conn {
	unsigned int tag;
	int id;
	INT_PTR fd;
	yahoo_input_condition cond;
	void *data;
	int remove;
};

#include "proto.h"

//=======================================================
//	Defines
//=======================================================
extern HINSTANCE		hInstance;

#ifdef HTTP_GATEWAY
extern int 				iHTTPGateway;
#endif

struct YAHOO_SEARCH_RESULT : public PROTOSEARCHRESULT
{
	yahoo_im_protocols protocol;
};

#define YAHOO_hasnotification() ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)

int YAHOO_util_dbsettingchanged(WPARAM wParam, LPARAM lParam);

//Services.c
int SetStatus(WPARAM wParam,LPARAM lParam);
int GetStatus(WPARAM wParam,LPARAM lParam);

yahoo_status miranda_to_yahoo(int myyahooStatus);

void register_callbacks();

#ifdef __GNUC__
	int debugLogA( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
#else
	int debugLogA( const char *fmt, ... );
#endif

void SetButtonCheck(HWND hwndDlg, int CtrlID, BOOL bCheck);

char * yahoo_status_code(enum yahoo_status s);
void yahoo_callback(struct _conn *c, yahoo_input_condition cond);

CYahooProto* __fastcall getProtoById( int id );
#define GETPROTOBYID(A) CYahooProto* ppro = getProtoById(A); if ( ppro ) ppro

#endif
