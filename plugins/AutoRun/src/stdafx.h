#pragma once

#include <windows.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_database.h>

#include "resource.h"
#include "version.h"

#define SUB_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

#define MODULENAME LPGEN("Autorun")

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};
