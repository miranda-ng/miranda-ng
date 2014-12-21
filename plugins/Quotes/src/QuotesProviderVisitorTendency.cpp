#include "stdafx.h"

CQuotesProviderVisitorTendency::CQuotesProviderVisitorTendency(MCONTACT hContact, TCHAR chr)
	: m_hContact(hContact), m_chr(chr), m_bValid(false), m_dResult(0.0)
{
}

void CQuotesProviderVisitorTendency::Visit(const CQuotesProviderBase&)
{
	switch (m_chr)
	{
	case _T('r'):
	case _T('R'):
		GetValue(DB_STR_QUOTE_CURR_VALUE);
		break;
	case _T('p'):
		GetValue(DB_STR_QUOTE_PREV_VALUE);
		break;
	}
}

void CQuotesProviderVisitorTendency::Visit(const CQuotesProviderGoogleFinance&)
{
	switch (m_chr)
	{
	case _T('o'):
		GetValue(DB_STR_GOOGLE_FINANCE_OPEN_VALUE);
		break;
	case _T('d'):
		GetValue(DB_STR_GOOGLE_FINANCE_DIFF);
		break;
	case _T('y'):
		GetValue(DB_STR_GOOGLE_FINANCE_PERCENT_CHANGE_TO_YERSTERDAY_CLOSE);
		break;
	}
}

void CQuotesProviderVisitorTendency::Visit(const CQuotesProviderYahoo&)
{
	switch (m_chr)
	{
	case _T('o'):
		GetValue(DB_STR_YAHOO_OPEN_VALUE);
		break;
	case _T('h'):
		GetValue(DB_STR_YAHOO_DAY_HIGH);
		break;
	case _T('P'):
		GetValue(DB_STR_YAHOO_PREVIOUS_CLOSE);
		break;
	case _T('c'):
		GetValue(DB_STR_YAHOO_CHANGE);
		break;
	case _T('g'):
		GetValue(DB_STR_YAHOO_DAY_LOW);
		break;
	}

}


void CQuotesProviderVisitorTendency::GetValue(LPCSTR pszDbKeyName)
{
	m_bValid = Quotes_DBReadDouble(m_hContact, QUOTES_MODULE_NAME, pszDbKeyName, m_dResult);
}
