#include "StdAfx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"

CCurrencyRatesProviderVisitorFormater::CCurrencyRatesProviderVisitorFormater(MCONTACT hContact, wchar_t chr, int nWidth)
	: m_hContact(hContact),
	m_chr(chr),
	m_nWidth(nWidth)
{
}

CCurrencyRatesProviderVisitorFormater::~CCurrencyRatesProviderVisitorFormater()
{
}

const tstring& CCurrencyRatesProviderVisitorFormater::GetResult()const
{
	return m_sResult;
}

static bool get_fetch_time(MCONTACT hContact, time_t& rTime)
{
	DBVARIANT dbv;
	if (db_get(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_FETCH_TIME, &dbv) || (DBVT_DWORD != dbv.type))
		return false;

	rTime = dbv.dVal;
	return true;
}

static tstring format_fetch_time(const CCurrencyRatesProviderBase&, MCONTACT hContact, const tstring& rsFormat)
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

void CCurrencyRatesProviderVisitorFormater::Visit(const CCurrencyRatesProviderBase& rProvider)
{
	switch (m_chr) {
	case '%':
	case '\t':
	case '\\':
		m_sResult = m_chr;
		break;
	case 'S':
		m_sResult = CurrencyRates_DBGetStringW(m_hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_PROVIDER);
		break;
	case 's':
		m_sResult = CurrencyRates_DBGetStringW(m_hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_SYMBOL);
		break;
	case 'X':
		m_sResult = format_fetch_time(rProvider, m_hContact, CurrencyRates_GetTimeFormat(true));
		break;
	case 'x':
		m_sResult = format_fetch_time(rProvider, m_hContact, CurrencyRates_GetDateFormat(true));
		break;
	case 't':
		{
			tstring sFrmt = CurrencyRates_GetDateFormat(true);
			sFrmt += L" ";
			sFrmt += CurrencyRates_GetTimeFormat(true);
			m_sResult = format_fetch_time(rProvider, m_hContact, sFrmt);
		}
		break;
	case 'r':
	case 'R':
		FormatDoubleHelper(DB_STR_CURRENCYRATE_CURR_VALUE);
		break;
	case 'p':
		FormatDoubleHelper(DB_STR_CURRENCYRATE_PREV_VALUE);
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

void CCurrencyRatesProviderVisitorFormater::FormatDoubleHelper(LPCSTR pszDbSet,
	const tstring sInvalid/* = L"-"*/)
{
	double d = 0.0;
	if (true == CurrencyRates_DBReadDouble(m_hContact, CURRENCYRATES_MODULE_NAME, pszDbSet, d))
		m_sResult = format_double(d, m_nWidth);
	else
		m_sResult = sInvalid;
}

void CCurrencyRatesProviderVisitorFormater::Visit(const CCurrencyRatesProviderCurrencyConverter&)
{
	switch (m_chr) {
	case 'F':
		m_sResult = CurrencyRates_DBGetStringW(m_hContact, CURRENCYRATES_MODULE_NAME, DB_STR_FROM_DESCRIPTION);
		break;
	case 'f':
		m_sResult = CurrencyRates_DBGetStringW(m_hContact, CURRENCYRATES_MODULE_NAME, DB_STR_FROM_ID);
		break;
	case 'I':
		m_sResult = CurrencyRates_DBGetStringW(m_hContact, CURRENCYRATES_MODULE_NAME, DB_STR_TO_DESCRIPTION);
		break;
	case 'i':
		m_sResult = CurrencyRates_DBGetStringW(m_hContact, CURRENCYRATES_MODULE_NAME, DB_STR_TO_ID);
		break;
	}
}