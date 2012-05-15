#include "../headers.h"

//reading from database-------------
tstring db_usage::DBGetPluginSetting(std::string const &name, tstring const &defValue)
{
	DBVARIANT dbv;
	if(DBGetContactSettingTString(NULL, pluginName, name.c_str(), &dbv))
		return defValue;
	tstring value = dbv.ptszVal;
	DBFreeVariant(&dbv);
	return value;
}

#ifdef _UNICODE
std::string db_usage::DBGetPluginSetting(std::string const &name, std::string const &defValue)
{
	DBVARIANT dbv;
	if(DBGetContactSettingString(NULL, pluginName, name.c_str(), &dbv))
		return defValue;
	std::string value = dbv.pszVal;
	DBFreeVariant(&dbv);
	return value;
}
#endif

bool db_usage::DBGetPluginSetting(std::string const &name, bool const &defValue)
{
	return(0 != DBGetContactSettingByte(NULL, pluginName, name.c_str(), defValue?1:0));
}

DWORD db_usage::DBGetPluginSetting(std::string const &name, DWORD const &defValue)
{
	return DBGetContactSettingDword(NULL, pluginName, name.c_str(), defValue);
}

//writting to database--------------
void db_usage::DBSetPluginSetting(std::string const &name, tstring const &value)
{
	DBWriteContactSettingTString(NULL, pluginName, name.c_str(), value.c_str());
}

#ifdef _UNICODE
void db_usage::DBSetPluginSetting(std::string const &name, std::string const &value)
{
	DBWriteContactSettingString(NULL, pluginName, name.c_str(), value.c_str());
}
#endif

void db_usage::DBSetPluginSetting(std::string const &name, bool const &value)
{
	DBWriteContactSettingByte(NULL, pluginName, name.c_str(), value?1:0);
}

void db_usage::DBSetPluginSetting(std::string const &name, DWORD const &value)
{
	DBWriteContactSettingDword(NULL, pluginName, name.c_str(),value);
}

Settings *plSets;
