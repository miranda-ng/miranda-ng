#ifndef _STDAFX_H_
#define _STDAFX_H_

#include "../../../miranda-private-keys/Telegram/api.h"

#include <windows.h>
#include <time.h>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <thread>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protosvc.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
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
#include <m_string.h>
#include <m_json.h>
#include <m_gui.h>
#include <m_imgsrvc.h>
#include <m_xml.h>
#include <m_assocmgr.h>
#include <m_file.h>

extern "C" 
{
	#include "tgl/tgl.h"
	#include "tgl/tgl-net.h"
	#include "tgl/tgl-timers.h"
	#include "tgl/tgl-binlog.h"
	#include "tgl/config.h"
	#include "tgl/mtproto-client.h"
}


struct MirTLS : public tgl_state, public MZeroedObject
{
	struct CTelegramProto *m_proto;
	MirTLS(CTelegramProto *p) : m_proto(p) {}
};

#include "version.h"
#include "t_proto.h"
#include "t_network.h"
#include "t_callback.h"

extern char g_szMirVer[];
extern HANDLE hQueue;

#define MODULE "Telegram"

struct CTelegramProto;




#endif //_STDAFX_H_