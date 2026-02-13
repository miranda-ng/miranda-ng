#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <CommCtrl.h>
#include <list>
#include <algorithm>

#include "newpluginapi.h"
#include "m_system.h"
#include "m_langpack.h"
#include "m_database.h"
#include "m_message.h"
#include "m_clist.h"
#include "m_clistint.h"
#include "m_options.h"

#include "AsSingleWindow.h"
#include "Options.h"
#include "resource.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
