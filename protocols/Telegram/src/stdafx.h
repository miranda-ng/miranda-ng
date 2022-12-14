#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>

#include <map>
#include <memory>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protosvc.h>

#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_extraicons.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>

#include "td/telegram/Client.h"
#include "td/telegram/td_api.h"
#include "td/telegram/td_api.hpp"

#define MODULE "Telegram"

#include "version.h"
#include "resource.h"
#include "mt_proto.h"

struct CMPlugin : public ACCPROTOPLUGIN<CMTProto>
{
	CMPlugin();

	HANDLE m_hIcon;

	int Load() override;
};

#endif //_COMMON_H_