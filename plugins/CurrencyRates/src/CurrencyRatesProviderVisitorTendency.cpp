#include "stdafx.h"

CCurrencyRatesProviderVisitorTendency::CCurrencyRatesProviderVisitorTendency(MCONTACT hContact, wchar_t chr)
	: m_hContact(hContact), m_chr(chr), m_bValid(false), m_dResult(0.0)
{
}

void CCurrencyRatesProviderVisitorTendency::Visit(const CCurrencyRatesProviderBase&)
{
	switch (m_chr) {
	case 'r':
	case 'R':
		GetValue(DB_STR_CURRENCYRATE_CURR_VALUE);
		break;
	case 'p':
		GetValue(DB_STR_CURRENCYRATE_PREV_VALUE);
		break;
	}
}

void CCurrencyRatesProviderVisitorTendency::GetValue(LPCSTR pszDbKeyName)
{
	m_bValid = CurrencyRates_DBReadDouble(m_hContact, CURRENCYRATES_MODULE_NAME, pszDbKeyName, m_dResult);
}
