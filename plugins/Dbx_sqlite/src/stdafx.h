#pragma once

#include <windows.h>
#include <sqlite3.h>

#include <memory>

#include <malloc.h>
#include <win2k.h>
#include <assert.h>

#include <newpluginapi.h>
#include <m_crypto.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_metacontacts.h>

#include "dbintf.h"
#include "resource.h"
#include "version.h"

struct CQuery
{
	const char *szQuery;
	sqlite3_stmt *pQuery;
};

/////////////////////////////////////////////////////////////////////////////////////////

constexpr auto SQLITE_HEADER_STR = "SQLite format 3";

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};
