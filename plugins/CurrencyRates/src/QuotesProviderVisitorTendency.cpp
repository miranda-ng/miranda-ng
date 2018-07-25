#include "stdafx.h"

CQuotesProviderVisitorTendency::CQuotesProviderVisitorTendency(MCONTACT hContact, wchar_t chr)
	: m_hContact(hContact), m_chr(chr), m_bValid(false), m_dResult(0.0)
{
}

void CQuotesProviderVisitorTendency::Visit(const CQuotesProviderBase&)
{
	switch (m_chr) {
	case 'r':
	case 'R':
		GetValue(DB_STR_QUOTE_CURR_VALUE);
		break;
	case 'p':
		GetValue(DB_STR_QUOTE_PREV_VALUE);
		break;
	}
}

void CQuotesProviderVisitorTendency::GetValue(LPCSTR pszDbKeyName)
{
	m_bValid = Quotes_DBReadDouble(m_hContact, QUOTES_MODULE_NAME, pszDbKeyName, m_dResult);
}
