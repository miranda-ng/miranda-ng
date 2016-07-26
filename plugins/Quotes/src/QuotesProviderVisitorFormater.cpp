#include "StdAfx.h"

CQuotesProviderVisitorFormater::CQuotesProviderVisitorFormater(MCONTACT hContact, wchar_t chr, int nWidth)
	: m_hContact(hContact),
	m_chr(chr),
	m_nWidth(nWidth)
{
}

CQuotesProviderVisitorFormater::~CQuotesProviderVisitorFormater()
{
}

const tstring& CQuotesProviderVisitorFormater::GetResult()const
{
	return m_sResult;
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderDukasCopy&)
{
	if ('d' == m_chr || 'D' == m_chr)
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION);
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderGoogle&)
{
	switch (m_chr) {
	case 'F':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_FROM_DESCRIPTION);
		break;
	case 'f':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_FROM_ID);
		break;
	case 'I':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_TO_DESCRIPTION);
		break;
	case 'i':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_TO_ID);
		break;
	}
}

static bool get_fetch_time(MCONTACT hContact, time_t& rTime)
{
	DBVARIANT dbv;
	if (db_get(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_FETCH_TIME, &dbv) || (DBVT_DWORD != dbv.type))
		return false;

	rTime = dbv.dVal;
	return true;
}

static tstring format_fetch_time(const CQuotesProviderBase&, MCONTACT hContact, const tstring& rsFormat)
{
	time_t nTime;
	if (true == get_fetch_time(hContact, nTime)) {
		boost::posix_time::ptime time = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(boost::posix_time::from_time_t(nTime));
		tostringstream k;
		k.imbue(std::locale(GetSystemLocale(), new ttime_facet(rsFormat.c_str())));
		k << time;
		return k.str();
	}

	return tstring();
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderBase& rProvider)
{
	switch (m_chr) {
	case '%':
	case '\t':
	case '\\':
		m_sResult = m_chr;
		break;
	case 'S':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_PROVIDER);
		break;
	case 's':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_SYMBOL);
		break;
	case 'X':
		m_sResult = format_fetch_time(rProvider, m_hContact, Quotes_GetTimeFormat(true));
		break;
	case 'x':
		m_sResult = format_fetch_time(rProvider, m_hContact, Quotes_GetDateFormat(true));
		break;
	case 't':
		{
			tstring sFrmt = Quotes_GetDateFormat(true);
			sFrmt += L" ";
			sFrmt += Quotes_GetTimeFormat(true);
			m_sResult = format_fetch_time(rProvider, m_hContact, sFrmt);
		}
		break;
	case 'r':
	case 'R':
		FormatDoubleHelper(DB_STR_QUOTE_CURR_VALUE);
		break;
	case 'p':
		FormatDoubleHelper(DB_STR_QUOTE_PREV_VALUE);
		break;
	}
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderGoogleFinance&/* rProvider*/)
{
	switch (m_chr) {
	case 'o':
		FormatDoubleHelper(DB_STR_GOOGLE_FINANCE_OPEN_VALUE);
		break;
	case 'd':
		FormatDoubleHelper(DB_STR_GOOGLE_FINANCE_DIFF, L"0");
		break;
	case 'y':
		FormatDoubleHelper(DB_STR_GOOGLE_FINANCE_PERCENT_CHANGE_TO_YERSTERDAY_CLOSE, L"0");
		break;
	case 'n':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION);
		break;
	}
}

tstring format_double(double dValue, int nWidth)
{
	tostringstream o;
	o.imbue(GetSystemLocale());

	if (nWidth > 0 && nWidth <= 9)
		o << std::setprecision(nWidth) << std::showpoint << std::fixed;

	o << dValue;

	return o.str();
}

void CQuotesProviderVisitorFormater::FormatDoubleHelper(LPCSTR pszDbSet,
	const tstring sInvalid/* = L"-"*/)
{
	double d = 0.0;
	if (true == Quotes_DBReadDouble(m_hContact, QUOTES_MODULE_NAME, pszDbSet, d))
		m_sResult = format_double(d, m_nWidth);
	else
		m_sResult = sInvalid;
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderYahoo&)
{
	switch (m_chr) {
	case 'o':
		FormatDoubleHelper(DB_STR_YAHOO_OPEN_VALUE);
		break;
	case 'h':
		FormatDoubleHelper(DB_STR_YAHOO_DAY_HIGH);
		break;
	case 'P':
		FormatDoubleHelper(DB_STR_YAHOO_PREVIOUS_CLOSE);
		break;
	case 'c':
		FormatDoubleHelper(DB_STR_YAHOO_CHANGE);
		break;
	case 'g':
		FormatDoubleHelper(DB_STR_YAHOO_DAY_LOW);
		break;
	case 'n':
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION);
		break;
	}
}
