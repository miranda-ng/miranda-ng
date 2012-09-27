#include "skype_proto.h"

TCHAR* CSkypeProto::GetSettingString(const char *szSetting, TCHAR* defVal)
{
	return this->GetSettingString(NULL, szSetting, defVal);
}

TCHAR* CSkypeProto::GetSettingString(HANDLE hContact, const char *szSetting, TCHAR* defVal)
{
	TCHAR* result = DBGetStringW(hContact, this->m_szModuleName, szSetting);
	return result != NULL ? result : defVal;
}

TCHAR* CSkypeProto::GetDecodeSettingString(const char *szSetting, TCHAR* defVal)
{
	return this->GetSettingString(NULL, szSetting, defVal);
}

TCHAR* CSkypeProto::GetDecodeSettingString(HANDLE hContact, const char *szSetting, TCHAR* defVal)
{
	DBVARIANT dbv;
	TCHAR* result = NULL;
	if ( !DBGetContactSettingString(0, this->m_szModuleName, szSetting, &dbv))
	{
		CallService(
			MS_DB_CRYPT_DECODESTRING,
			wcslen(dbv.ptszVal) + 1,
			reinterpret_cast<LPARAM>(dbv.ptszVal));
		result = mir_wstrdup(dbv.pwszVal);
		DBFreeVariant(&dbv);
	}
	return result;
}