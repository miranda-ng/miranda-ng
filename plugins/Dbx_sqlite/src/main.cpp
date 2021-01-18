#include "stdafx.h"

CMPlugin g_plugin;

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_DATABASE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {DA223468-5F8E-4513-88B0-E52CE8A8B33B}
	{ 0xda223468, 0x5f8e, 0x4513, { 0x88, 0xb0, 0xe5, 0x2c, 0xe8, 0xa8, 0xb3, 0x3b } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

static DATABASELINK dblink =
{
	MDB_CAPS_CREATE | MDB_CAPS_COMPACT,
	"dbx_sqlite",
	L"SQLite database driver",
	&CDbxSQLite::Create,
	&CDbxSQLite::Check,
	&CDbxSQLite::Load,
};

STDMETHODIMP_(DATABASELINK *) CDbxSQLite::GetDriver()
{
	return &dblink;
}

int CMPlugin::Load()
{
	RegisterDatabasePlugin(&dblink);
	return 0;
}
