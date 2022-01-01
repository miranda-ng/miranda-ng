/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#pragma once
#pragma warning(disable:4996 4290 4200)

#include <malloc.h>
#include <time.h>
#include <windows.h>

#include <list>
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
#include <m_json.h>
#include <m_gui.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "../../libs/libqrencode/src/qrencode.h"

#include "../../libs/libsignal/src/curve.h"
#include "../../libs/libsignal/src/hkdf.h"
#include "../../libs/libsignal/src/signal_protocol.h"

/////////////////////////////////////////////////////////////////////////////////////////
// to obtain protobuf library do the following
// - install vcpkg (https://github.com/microsoft/vcpkg);
// - install static libraries of protobuf:
// >vcpkg.exe install protobuf:x86-windows-static-md
// >vcpkg.exe install protobuf:x64-windows-static-md

#include "google/protobuf/message.h"

/////////////////////////////////////////////////////////////////////////////////////////

struct signal_buffer
{
	size_t len;
	uint8_t data[];
};

struct signal_type_base
{
	unsigned int ref_count = 0;
	void (*destroy)(signal_type_base *instance) = 0;
};

struct ec_public_key : public signal_type_base
{
	uint8_t data[32];
};

struct ec_private_key : public signal_type_base
{
	uint8_t data[32];
};

#include "db.h"
#include "utils.h"
#include "proto.h"
#include "resource.h"
#include "pmsg.pb.h"

#pragma comment(lib, "libcrypto.lib")
