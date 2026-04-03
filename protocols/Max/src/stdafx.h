#pragma once

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <map>
#include <string>

#include <newpluginapi.h>
#include <m_chat_int.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_srmm_int.h>
#include <m_system.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <m_clist.h>
#include <m_http.h>
#include "../../libs/zlib/src/zlib.h"

#include "resource.h"
#include "version.h"

#include "max_proto.h"

#define MODULENAME "Max"

#define DB_KEY_DEVICEID "DeviceId"
#define DB_KEY_PHONE    "Phone"
#define DB_KEY_TOKEN_TMP "TokenTmp"
#define DB_KEY_LOGIN_TOKEN "LoginToken"
#define DB_KEY_MOBILE_DEVICEID "MobileDeviceId"
#define DB_KEY_MTINSTANCE "MtInstanceId"
