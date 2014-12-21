#include "StdAfx.h"

CQuotesProviderVisitorFormater::CQuotesProviderVisitorFormater(MCONTACT hContact, TCHAR chr, int nWidth)
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
	if (_T('d') == m_chr || _T('D') == m_chr)
	{
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION);
	}
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderGoogle&)
{
	switch (m_chr)
	{
	case _T('F'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_FROM_DESCRIPTION);
		break;
	case _T('f'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_FROM_ID);
		break;
	case _T('I'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_TO_DESCRIPTION);
		break;
	case _T('i'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_TO_ID);
		break;
	}
}

namespace
{
	bool get_fetch_time(MCONTACT hContact, time_t& rTime)
	{
		DBVARIANT dbv;
		if (db_get(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_FETCH_TIME, &dbv) || (DBVT_DWORD != dbv.type))
			return false;

		rTime = dbv.dVal;
		return true;
	}

	tstring format_fetch_time(const CQuotesProviderBase&, MCONTACT hContact, const tstring& rsFormat)
	{
		time_t nTime;
		if (true == get_fetch_time(hContact, nTime))
		{
			boost::posix_time::ptime time = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(boost::posix_time::from_time_t(nTime));
			tostringstream k;
			k.imbue(std::locale(GetSystemLocale(), new ttime_facet(rsFormat.c_str())));
			k << time;
			return k.str();
		}

		return tstring();
	}
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderBase& rProvider)
{
	switch (m_chr)
	{
		// 	default:
		// 		m_sResult = m_chr;
		// 		break;
	case _T('%'):
	case _T('\t'):
	case _T('\\'):
		m_sResult = m_chr;
		break;
	case _T('S'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_PROVIDER);
		break;
	case _T('s'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_SYMBOL);
		break;
	case _T('X'):
		//m_sResult = format_fetch_time(rProvider,m_hContact,_T("%H:%M:%S"));
		m_sResult = format_fetch_time(rProvider, m_hContact, Quotes_GetTimeFormat(true));
		break;
	case _T('x'):
		//m_sResult = format_fetch_time(rProvider,m_hContact,_T("%d.%m.%y"));
		m_sResult = format_fetch_time(rProvider, m_hContact, Quotes_GetDateFormat(true));
		break;
	case _T('t'):
	{
		tstring sFrmt = Quotes_GetDateFormat(true);
		sFrmt += _T(" ");
		sFrmt += Quotes_GetTimeFormat(true);
		m_sResult = format_fetch_time(rProvider, m_hContact, sFrmt);

		//m_sResult = format_fetch_time(rProvider,m_hContact,_T("%d.%m.%y %H:%M:%S"));
	}
	break;
	case _T('r'):
	case _T('R'):
		FormatDoubleHelper(DB_STR_QUOTE_CURR_VALUE);
		break;
	case _T('p'):
		FormatDoubleHelper(DB_STR_QUOTE_PREV_VALUE);
		break;
		// 	case _T('c'):
		// 		FormatChangeValueHelper(false);
		// 		break;
		// 	case _T('C'):
		// 		FormatChangeValueHelper(true);
		// 		break;
	}
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderGoogleFinance&/* rProvider*/)
{
	switch (m_chr)
	{
	case _T('o'):
		FormatDoubleHelper(DB_STR_GOOGLE_FINANCE_OPEN_VALUE);
		break;
	case _T('d'):
		FormatDoubleHelper(DB_STR_GOOGLE_FINANCE_DIFF, _T("0"));
		break;
	case _T('y'):
		FormatDoubleHelper(DB_STR_GOOGLE_FINANCE_PERCENT_CHANGE_TO_YERSTERDAY_CLOSE, _T("0"));
		break;
	case _T('n'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION);
		break;
	}
}

namespace
{
	tstring format_double(double dValue, int nWidth)
	{
		tostringstream o;
		o.imbue(GetSystemLocale());

		if (nWidth > 0 && nWidth <= 9)
		{
			o << std::setprecision(nWidth) << std::showpoint << std::fixed;
		}
		o << dValue;

		return o.str();
	}
}

void CQuotesProviderVisitorFormater::FormatDoubleHelper(LPCSTR pszDbSet,
	const tstring sInvalid/* = _T("-")*/)
{
	double d = 0.0;
	if (true == Quotes_DBReadDouble(m_hContact, QUOTES_MODULE_NAME, pszDbSet, d))
	{
		m_sResult = format_double(d, m_nWidth);
	}
	else
	{
		m_sResult = sInvalid;
	}
}

void CQuotesProviderVisitorFormater::Visit(const CQuotesProviderYahoo&)
{
	switch (m_chr)
	{
	case _T('o'):
		FormatDoubleHelper(DB_STR_YAHOO_OPEN_VALUE);
		break;
	case _T('h'):
		FormatDoubleHelper(DB_STR_YAHOO_DAY_HIGH);
		break;
	case _T('P'):
		FormatDoubleHelper(DB_STR_YAHOO_PREVIOUS_CLOSE);
		break;
	case _T('c'):
		FormatDoubleHelper(DB_STR_YAHOO_CHANGE);
		break;
	case _T('g'):
		FormatDoubleHelper(DB_STR_YAHOO_DAY_LOW);
		break;
	case _T('n'):
		m_sResult = Quotes_DBGetStringT(m_hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION);
		break;
	}

}
