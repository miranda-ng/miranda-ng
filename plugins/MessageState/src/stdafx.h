#pragma once

#include <windows.h>
#include <time.h>
#include <locale.h>

#include <m_messagestate.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_metacontacts.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_extraicons.h>
#include <m_message.h>
#include "resource.h"
#include "version.h"

#include "global.h"

#define MODULENAME LPGEN("MessageState")

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

