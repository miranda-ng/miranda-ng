// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#include <Windows.h>
#include <Shlwapi.h>
#include <Wincrypt.h>
#include <stdio.h>
#include <direct.h>
#include <tchar.h>
#include <time.h>

#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
using std::string;
using std::wstring;
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <list>
using std::list;
#include <algorithm>
using std::min;
#include <sstream>

typedef std::basic_string<TCHAR> tstring;

#include "resource.h"

#pragma warning(push)
#	pragma warning(disable:4312)
#include <m_system_cpp.h>
#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_database.h>
#include <m_folders.h>
#include <m_history.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <statusmodes.h>
#include <m_system.h>
#include <m_userinfo.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_hotkeys.h>
#include <win2k.h>
#pragma warning(pop)

#include "StringUtil.h"

extern HINSTANCE g_hInstance;

#define TWITTER_KEY_NICK		   "Nick"  // we need one called Nick for the chat thingo to work
#define TWITTER_KEY_UN             "Username"
#define TWITTER_KEY_PASS           "Password"
#define TWITTER_KEY_OAUTH_PIN	   "OAuthPIN"
#define TWITTER_KEY_OAUTH_TOK	   "OAuthToken"
#define TWITTER_KEY_OAUTH_TOK_SECRET "OAuthTokenSecret"
#define TWITTER_KEY_OAUTH_ACCESS_TOK "OAuthAccessToken"
#define TWITTER_KEY_OAUTH_ACCESS_TOK_SECRET "OAuthAccessTokenSecret"
#define TWITTER_KEY_BASEURL        "BaseURL"
#define TWITTER_KEY_CHATFEED       "ChatFeed"
#define TWITTER_KEY_POLLRATE       "PollRate"
#define TWITTER_KEY_GROUP		   "DefaultGroup"

#define TWITTER_KEY_POPUP_SHOW     "Popup/Show"
#define TWITTER_KEY_POPUP_SIGNON   "Popup/Signon"
#define TWITTER_KEY_POPUP_COLBACK  "Popup/ColorBack"
#define TWITTER_KEY_POPUP_COLTEXT  "Popup/ColorText"
#define TWITTER_KEY_POPUP_TIMEOUT  "Popup/Timeout"

#define TWITTER_KEY_TWEET_TO_MSG   "TweetToMsg"

#define TWITTER_KEY_SINCEID        "SinceID"
#define TWITTER_KEY_DMSINCEID      "DMSinceID"
#define TWITTER_KEY_NEW            "NewAcc"

#define TWITTER_KEY_AV_URL         "AvatarURL"

#define TWITTER_DB_EVENT_TYPE_TWEET 2718

#define WM_SETREPLY   WM_APP+10
