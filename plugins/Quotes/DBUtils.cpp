#include "StdAfx.h"
#include "DBUtils.h"

std::string Quotes_DBGetStringA(HANDLE hContact,const char* szModule,const char* szSetting,const char* pszDefValue /*= NULL*/)
{
	std::string sResult;
	char* pszSymbol = DBGetString(hContact,szModule,szSetting);
	if(NULL != pszSymbol)
	{
		sResult = pszSymbol;
		mir_free(pszSymbol);
	}
	else if(NULL != pszDefValue)
	{
		sResult = pszDefValue;
	}

	return sResult;
}

std::wstring Quotes_DBGetStringW(HANDLE hContact,const char* szModule,const char* szSetting,const wchar_t* pszDefValue/* = NULL*/)
{
	std::wstring sResult;
	wchar_t* pszSymbol = DBGetStringW(hContact,szModule,szSetting);
	if(NULL != pszSymbol)
	{
		sResult = pszSymbol;
		mir_free(pszSymbol);
	}
	else if(NULL != pszDefValue)
	{
		sResult = pszDefValue;
	}

	return sResult;
}

bool Quotes_DBWriteDouble(HANDLE hContact,const char* szModule,const char* szSetting,double dValue)
{
	DBCONTACTWRITESETTING cws = {0};

	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BLOB;
	cws.value.cpbVal = sizeof(dValue);
	cws.value.pbVal = reinterpret_cast<BYTE*>(&dValue);
	return 0 == CallService(MS_DB_CONTACT_WRITESETTING,reinterpret_cast<WPARAM>(hContact),reinterpret_cast<LPARAM>(&cws));
}

bool Quotes_DBReadDouble(HANDLE hContact,const char* szModule,const char* szSetting,double& rdValue)
{
	DBVARIANT dbv = {0};
	DBCONTACTGETSETTING cgs;
	cgs.szModule=szModule;
	cgs.szSetting=szSetting;
	cgs.pValue = &dbv;
	dbv.type = DBVT_BLOB;

	bool bResult = ((0 == CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&cgs))
		&& (DBVT_BLOB == dbv.type));

	if(bResult)
	{
		rdValue = *reinterpret_cast<double*>(dbv.pbVal);
	}

	DBFreeVariant(&dbv);
	return bResult;
}

