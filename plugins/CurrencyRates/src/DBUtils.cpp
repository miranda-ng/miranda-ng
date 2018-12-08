#include "StdAfx.h"

std::string CurrencyRates_DBGetStringA(MCONTACT hContact, const char* szModule, const char* szSetting, const char* pszDefValue)
{
	return std::string(ptrA(db_get_sa(hContact, szModule, szSetting, pszDefValue)));
}

std::wstring CurrencyRates_DBGetStringW(MCONTACT hContact, const char* szModule, const char* szSetting, const wchar_t* pszDefValue)
{
	return std::wstring(ptrW(db_get_wsa(hContact, szModule, szSetting, pszDefValue)));
}

bool CurrencyRates_DBWriteDouble(MCONTACT hContact, const char* szModule, const char* szSetting, double dValue)
{
	return 0 == db_set_blob(hContact, szModule, szSetting, &dValue, sizeof(dValue));
}

bool CurrencyRates_DBReadDouble(MCONTACT hContact, const char* szModule, const char* szSetting, double& rdValue)
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_BLOB;

	bool bResult = ((0 == db_get(hContact, szModule, szSetting, &dbv)) && (DBVT_BLOB == dbv.type));
	if (bResult)
		rdValue = *reinterpret_cast<double*>(dbv.pbVal);

	db_free(&dbv);
	return bResult;
}
