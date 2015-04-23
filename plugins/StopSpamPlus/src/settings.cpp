#include "headers.h"

//reading from database-------------
tstring db_usage::DBGetPluginSetting(std::string const &name, tstring const &defValue)
{
	DBVARIANT dbv;
	if (db_get_ts(NULL, pluginName, name.c_str(), &dbv))
		return defValue;
	tstring value = dbv.ptszVal;
	db_free(&dbv);
	return value;
}


std::string db_usage::DBGetPluginSetting(std::string const &name, std::string const &defValue)
{
	DBVARIANT dbv;
	if (db_get_s(NULL, pluginName, name.c_str(), &dbv))
		return defValue;
	std::string value = dbv.pszVal;
	db_free(&dbv);
	return value;
}


bool db_usage::DBGetPluginSetting(std::string const &name, bool const &defValue)
{
	return(0 != db_get_b(NULL, pluginName, name.c_str(), defValue ? 1 : 0));
}

DWORD db_usage::DBGetPluginSetting(std::string const &name, DWORD const &defValue)
{
	return db_get_dw(NULL, pluginName, name.c_str(), defValue);
}

//writting to database--------------
void db_usage::DBSetPluginSetting(std::string const &name, tstring const &value)
{
	db_set_ts(NULL, pluginName, name.c_str(), value.c_str());
}


void db_usage::DBSetPluginSetting(std::string const &name, std::string const &value)
{
	db_set_s(NULL, pluginName, name.c_str(), value.c_str());
}


void db_usage::DBSetPluginSetting(std::string const &name, bool const &value)
{
	db_set_b(NULL, pluginName, name.c_str(), value ? 1 : 0);
}

void db_usage::DBSetPluginSetting(std::string const &name, DWORD const &value)
{
	db_set_dw(NULL, pluginName, name.c_str(), value);
}

Settings *plSets;
