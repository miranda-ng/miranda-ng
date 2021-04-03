#include "stdafx.h"

void logError(int rc, const char *szFile, int line)
{
	switch (rc) {
	case SQLITE_OK:
	case SQLITE_ROW:
	case SQLITE_DONE:
		return;
	}

	_ASSERT(rc == 0);
	Netlib_Logf(0, "SQLITE error %d (%s, %d)", rc, szFile, line);
}

/////////////////////////////////////////////////////////////////////////////////////////

sqlite3_stmt* CDbxSQLite::InitQuery(const char *szQuery, CQuery &stmt)
{
	if (stmt.pQuery == nullptr) 
		sqlite3_prepare_v3(m_db, szQuery, -1, SQLITE_PREPARE_PERSISTENT, &stmt.pQuery, nullptr);

	return stmt.pQuery;
}

CQuery::~CQuery()
{
	if (pQuery)
		sqlite3_finalize(pQuery);
}
