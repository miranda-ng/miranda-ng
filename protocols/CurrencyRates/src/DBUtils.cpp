#include "StdAfx.h"

std::wstring GetNodeText(const TiXmlElement *pNode)
{
	auto *pszText = pNode->GetText();
	if (pszText)
		return Utf2T(pszText).get();

	return std::wstring();
}

std::wstring CurrencyRates_DBGetStringW(MCONTACT hContact, const char *szModule, const char *szSetting, const wchar_t *pszDefValue)
{
	if (pszDefValue == nullptr)
		pszDefValue = L"";

	return std::wstring(ptrW(db_get_wsa(hContact, szModule, szSetting, pszDefValue)));
}

bool CurrencyRates_DBWriteDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double dValue)
{
	return 0 == db_set_blob(hContact, szModule, szSetting, &dValue, sizeof(dValue));
}

bool CurrencyRates_DBReadDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double& rdValue)
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;

	bool bResult = ((0 == db_get(hContact, szModule, szSetting, &dbv)) && (DBVT_BLOB == dbv.type));
	if (bResult)
		rdValue = *reinterpret_cast<double*>(dbv.pbVal);

	db_free(&dbv);
	return bResult;
}

void FixInvalidChars(std::wstring &s)
{
	for (auto &c : s)
		if (wcschr(L"\\/:*?\"<>|", c))
			c = '_';
}
