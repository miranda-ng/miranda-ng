/*

WhatsApp plugin for Miranda NG
Copyright © 2019-25 George Hazan

*/

#pragma once
#pragma warning(disable:4996 4290 4200 4239 4324)

#include <fcntl.h>
#include <malloc.h>
#include <io.h>
#include <time.h>
#include <windows.h>

#include <list>
#include <map>
#include <memory>
#include <string>

#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_chat_int.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_history.h>
#include <m_imgsrvc.h>
#include <m_ignore.h>
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
#include <m_string.h>
#include <statusmodes.h>
#include <m_userinfo.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_xml.h>
#include <m_hotkeys.h>
#include <m_folders.h>
#include <m_gui.h>
#include <m_messagestate.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/kdf.h>

#include "../../libs/libqrencode/src/qrencode.h"
#include "../../libs/zlib/src/zlib.h"

#include "../../utils/mir_signal.h"

#include "pmsg.proto.h"

/////////////////////////////////////////////////////////////////////////////////////////

#include "db.h"
#include "utils.h"
#include "proto.h"
#include "resource.h"

#pragma comment(lib, "libcrypto.lib")
