#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protosvc.h>

#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_message.h>
#include <m_avatars.h>
#include <m_skin.h>
#include <m_chat.h>
#include <m_genmenu.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_gui.h>
#include <m_http.h>
#include <m_json.h>

struct CSlackProto;

#define MODULE "Slack"

#define SLACK_URL "https://slack.com"
#define SLACK_API_URL SLACK_URL "/api"
#define SLACK_REDIRECT_URL "https://www.miranda-ng.org/slack"

#define SLACK_CLIENT_ID "149178180673.150539976630"
#include "../../../miranda-private-keys/Slack/client_secret.h"

#include "http_request.h"

#include "version.h"
#include "resource.h"
#include "slack_menus.h"
#include "slack_dialogs.h"
#include "slack_options.h"
#include "slack_proto.h"

#include "api\api_oauth.h"
#include "api\api_users.h"
#include "api\api_chat.h"
#include "api\api_im.h"

extern HINSTANCE g_hInstance;

#endif //_COMMON_H_