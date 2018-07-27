#include "StdAfx.h"

CCurrencyRatesProviderVisitorFormatSpecificator::CCurrencyRatesProviderVisitorFormatSpecificator()
{
}

CCurrencyRatesProviderVisitorFormatSpecificator::~CCurrencyRatesProviderVisitorFormatSpecificator()
{
}

void CCurrencyRatesProviderVisitorFormatSpecificator::Visit(const CCurrencyRatesProviderDukasCopy&/* rProvider*/)
{
	m_aSpecificators.push_back(CFormatSpecificator(L"%s", TranslateT("CurrencyRate Symbol")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%d", TranslateT("CurrencyRate Name")));
}

void CCurrencyRatesProviderVisitorFormatSpecificator::Visit(const CCurrencyRatesProviderBase&/* rProvider*/)
{
	m_aSpecificators.push_back(CFormatSpecificator(L"%S", TranslateT("Source of Information")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%r", TranslateT("Rate Value")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%p", TranslateT("Previous Rate Value")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%X", TranslateT("Fetch Time")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%x", TranslateT("Fetch Date")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%t", TranslateT("Fetch Time and Date")));
	m_aSpecificators.push_back(CFormatSpecificator(L"\\%", TranslateT("Percentage Character (%)")));
	m_aSpecificators.push_back(CFormatSpecificator(L"\\t", TranslateT("Tabulation")));
	m_aSpecificators.push_back(CFormatSpecificator(L"\\\\", TranslateT("Left slash (\\)")));
}

void CCurrencyRatesProviderVisitorFormatSpecificator::Visit(const CCurrencyRatesProviderGoogleFinance&/* rProvider*/)
{
	m_aSpecificators.push_back(CFormatSpecificator(L"%s", TranslateT("CurrencyRate Symbol")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%n", TranslateT("CurrencyRate Name")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%o", TranslateT("Open Price")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%d", TranslateT("Percent Change to After Hours")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%y", TranslateT("Percent Change to Yesterday Close")));
}

const CCurrencyRatesProviderVisitorFormatSpecificator::TFormatSpecificators& CCurrencyRatesProviderVisitorFormatSpecificator::GetSpecificators()const
{
	return m_aSpecificators;
}

void CCurrencyRatesProviderVisitorFormatSpecificator::Visit(const CCurrencyRatesProviderYahoo&)
{
	m_aSpecificators.push_back(CFormatSpecificator(L"%s", TranslateT("CurrencyRate Symbol")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%n", TranslateT("CurrencyRate Name")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%o", TranslateT("Open Price")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%h", TranslateT("Day's High")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%g", TranslateT("Day's Low")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%P", TranslateT("Previous Close")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%c", TranslateT("Change")));
}

void CCurrencyRatesProviderVisitorFormatSpecificator::Visit(const CCurrencyRatesProviderCurrencyConverter&)
{
	m_aSpecificators.push_back(CFormatSpecificator(L"%F", TranslateT("From Currency Full Name")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%f", TranslateT("From Currency Short Name")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%I", TranslateT("Into Currency Full Name")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%i", TranslateT("Into Currency Short Name")));
	m_aSpecificators.push_back(CFormatSpecificator(L"%s", TranslateT("Short notation for \"%f/%i\"")));
}