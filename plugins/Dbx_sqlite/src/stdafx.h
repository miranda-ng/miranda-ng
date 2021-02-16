#pragma once

#include <windows.h>
#include <malloc.h>
#include <crtdbg.h>

#include <memory>

#include <newpluginapi.h>
#include <m_crypto.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_metacontacts.h>

#include <sqlite3.h>

#include "dbintf.h"
#include "resource.h"
#include "version.h"

struct CQuery
{
	const char *szQuery;
	sqlite3_stmt *pQuery;
};

void logError(int rc, const char *szFile, int line);

/////////////////////////////////////////////////////////////////////////////////////////

constexpr auto SQLITE_HEADER_STR = "SQLite format 3";

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};
