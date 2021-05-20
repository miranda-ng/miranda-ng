#include "StdAfx.h"

const std::locale GetSystemLocale()
{
	return std::locale("");
}

std::wstring get_int_registry_value(LPCTSTR pszValueName)
{
	std::wstring sResult;
	HKEY hKey = nullptr;
	LONG lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Control Panel\\International", 0, KEY_QUERY_VALUE, &hKey);
	if ((ERROR_SUCCESS == lResult) && (nullptr != hKey)) {
		DWORD dwType = 0;
		DWORD dwSize = 0;
		lResult = ::RegQueryValueEx(hKey, pszValueName, nullptr, &dwType, nullptr, &dwSize);
		if ((ERROR_SUCCESS == lResult) && ((REG_SZ == dwType) || (REG_EXPAND_SZ == dwType))) {
			std::vector<wchar_t> aBuffer(dwSize);
			lResult = ::RegQueryValueEx(hKey, pszValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&*aBuffer.begin()), &dwSize);
			if (ERROR_SUCCESS == lResult)
				std::copy(aBuffer.begin(), aBuffer.end(), std::back_inserter(sResult));
		}
	}

	if (nullptr != hKey) {
		lResult = ::RegCloseKey(hKey);
		assert(ERROR_SUCCESS == lResult);
	}

	return sResult;
}

LPCTSTR date_win_2_boost(const std::wstring& sFrmt)
{
	if (sFrmt == L"dd/MM/yy")
		return L"%d/%m/%y";
	if (sFrmt == L"yyyy-MM-dd")
		return L"%y-%m-%d";
	return L"%d.%m.%y";
}

LPCTSTR time_win_2_boost(const std::wstring& sFrmt)
{
	if (sFrmt == L"H:mm" || sFrmt == L"HH:mm")
		return L"%H:%M";

	return L"%H:%M:%S";
}

LPCTSTR CurrencyRates_GetDateFormat(bool bShort)
{
	return date_win_2_boost(get_int_registry_value(bShort ? L"sShortDate" : L"sLongDate"));
}

LPCTSTR CurrencyRates_GetTimeFormat(bool bShort)
{
	return time_win_2_boost(get_int_registry_value(bShort ? L"sShortTime" : L"sTimeFormat"));
}
