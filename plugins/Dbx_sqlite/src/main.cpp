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
	// {7C3D0A33-2646-4001-9107-F35EA299D292}
	{ 0x7c3d0a33, 0x2646, 0x4001, { 0x91, 0x7, 0xf3, 0x5e, 0xa2, 0x99, 0xd2, 0x92 } }
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

int CMPlugin::Load()
{
	RegisterDatabasePlugin(&dblink);
	return 0;
}
