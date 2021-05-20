#include "StdAfx.h"

const std::locale GetSystemLocale()
{
	return std::locale("");
}

static CMStringW get_int_registry_value(LPCTSTR pszValueName)
{
	CMStringW sResult;
	HKEY hKey = nullptr;
	LONG lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Control Panel\\International", 0, KEY_QUERY_VALUE, &hKey);
	if ((ERROR_SUCCESS == lResult) && (nullptr != hKey)) {
		DWORD dwType = 0;
		DWORD dwSize = 0;
		lResult = ::RegQueryValueEx(hKey, pszValueName, nullptr, &dwType, nullptr, &dwSize);
		if ((ERROR_SUCCESS == lResult) && ((REG_SZ == dwType) || (REG_EXPAND_SZ == dwType))) {
			sResult.Truncate(dwSize);
			::RegQueryValueEx(hKey, pszValueName, nullptr, nullptr, LPBYTE(sResult.c_str()), &dwSize);
		}
	}

	if (hKey) {
		lResult = ::RegCloseKey(hKey);
		assert(ERROR_SUCCESS == lResult);
	}

	return sResult;
}

LPCTSTR CurrencyRates_GetDateFormat(bool bShort)
{
	CMStringW sFrmt = get_int_registry_value(bShort ? L"sShortDate" : L"sLongDate");
	if (sFrmt == L"dd/MM/yy")
		return L"%d/%m/%y";
	if (sFrmt == L"yyyy-MM-dd")
		return L"%y-%m-%d";
	return L"%d.%m.%y";
}

LPCTSTR CurrencyRates_GetTimeFormat(bool bShort)
{
	CMStringW sFrmt = get_int_registry_value(bShort ? L"sShortTime" : L"sTimeFormat");
	if (sFrmt == L"H:mm" || sFrmt == L"HH:mm")
		return L"%H:%M";
	return L"%H:%M:%S";
}
