#include "StdAfx.h"

std::string Quotes_DBGetStringA(MCONTACT hContact, const char* szModule, const char* szSetting, const char* pszDefValue /*= NULL*/)
{
	std::string sResult;
	char* pszSymbol = db_get_sa(hContact, szModule, szSetting);
	if (NULL != pszSymbol)
	{
		sResult = pszSymbol;
		mir_free(pszSymbol);
	}
	else if (NULL != pszDefValue)
	{
		sResult = pszDefValue;
	}

	return sResult;
}

std::wstring Quotes_DBGetStringW(MCONTACT hContact, const char* szModule, const char* szSetting, const wchar_t* pszDefValue/* = NULL*/)
{
	std::wstring sResult;
	wchar_t* pszSymbol = db_get_wsa(hContact, szModule, szSetting);
	if (NULL != pszSymbol)
	{
		sResult = pszSymbol;
		mir_free(pszSymbol);
	}
	else if (NULL != pszDefValue)
	{
		sResult = pszDefValue;
	}

	return sResult;
}

bool Quotes_DBWriteDouble(MCONTACT hContact, const char* szModule, const char* szSetting, double dValue)
{
	return 0 == db_set_blob(hContact, szModule, szSetting, &dValue, sizeof(dValue));
}

bool Quotes_DBReadDouble(MCONTACT hContact, const char* szModule, const char* szSetting, double& rdValue)
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_BLOB;

	bool bResult = ((0 == db_get(hContact, szModule, szSetting, &dbv)) && (DBVT_BLOB == dbv.type));
	if (bResult)
		rdValue = *reinterpret_cast<double*>(dbv.pbVal);

	db_free(&dbv);
	return bResult;
}

