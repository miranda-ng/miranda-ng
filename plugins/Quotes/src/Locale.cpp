#include "StdAfx.h"

const std::locale GetSystemLocale()
{
	return std::locale("");
}

namespace
{
	tstring get_int_registry_value(LPCTSTR pszValueName)
	{
		tstring sResult;
		HKEY hKey = NULL;
		LONG lResult = ::RegOpenKeyEx(HKEY_CURRENT_USER,
			_T("Control Panel\\International"), 0, KEY_QUERY_VALUE, &hKey);
		if ((ERROR_SUCCESS == lResult) && (NULL != hKey))
		{
			DWORD dwType = 0;
			DWORD dwSize = 0;
			lResult = ::RegQueryValueEx(hKey, pszValueName, nullptr, &dwType, nullptr, &dwSize);
			if ((ERROR_SUCCESS == lResult) && ((REG_SZ == dwType) || (REG_EXPAND_SZ == dwType)))
			{
				std::vector<TCHAR> aBuffer(dwSize);
				lResult = ::RegQueryValueEx(hKey, pszValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&*aBuffer.begin()), &dwSize);
				if (ERROR_SUCCESS == lResult)
				{
					std::copy(aBuffer.begin(), aBuffer.end(), std::back_inserter(sResult));
				}
			}
		}

		if (NULL != hKey)
		{
			lResult = ::RegCloseKey(hKey);
			assert(ERROR_SUCCESS == lResult);
		}

		return sResult;
	}

	tstring date_win_2_boost(const tstring& sFrmt)
	{
		tstring sResult(_T("%d.%m.%y"));
		if (sFrmt == _T("dd/MM/yy"))
		{
			sResult = _T("%d/%m/%y");
		}
		else if (sFrmt == _T("yyyy-MM-dd"))
		{
			sResult = _T("%y-%m-%d");
		}
		return sResult;
	}

	tstring time_win_2_boost(const tstring& sFrmt)
	{
		tstring sResult = _T("%H:%M:%S");
		if (sFrmt == _T("H:mm") || sFrmt == _T("HH:mm"))
		{
			sResult = _T("%H:%M");
		}
		return sResult;
	}
}

tstring Quotes_GetDateFormat(bool bShort)
{
	return date_win_2_boost(get_int_registry_value(bShort ? _T("sShortDate") : _T("sLongDate")));
}

tstring Quotes_GetTimeFormat(bool bShort)
{
	return time_win_2_boost(get_int_registry_value(bShort ? _T("sShortTime") : _T("sTimeFormat")));
}