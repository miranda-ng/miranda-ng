#ifndef _STDAFX_H_
#define _STDAFX_H_

#define APP_SHORTNAME __PLUGIN_NAME

/* Common header files */
#include <assert.h>

#include <Windows.h>
#include <CommCtrl.h>
#include <time.h>
#include <string>
#include <vector>
#include <queue>
#include <list>

using namespace std;

#include <newpluginapi.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clistint.h>
#include <m_protocols.h>
#include <m_skin.h>
#include <m_chat_int.h>
#include <m_idle.h>
#include <m_metacontacts.h>
#include <m_langpack.h>

#define IDF_SHORT		0x2 // short idle mode
#define IDF_LONG		0x4 // long idle mode
#define IDF_ONFORCE	   0x10

#define ASSERT assert

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#include "LCDFramework/LCDFramework.h"

#include "CEvent.h"

#include "resource.h"
#include "version.h"

#endif