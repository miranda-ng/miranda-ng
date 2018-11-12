#include "StdAfx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"

#define LAST_RUN_VERSION "LastRunVersion"

CCurrencyRatesProviders::CCurrencyRatesProviders()
{
	InitProviders();
}

CCurrencyRatesProviders::~CCurrencyRatesProviders()
{
	ClearProviders();
}

const CCurrencyRatesProviders::TCurrencyRatesProviders& CCurrencyRatesProviders::GetProviders()const
{
	return m_apProviders;
}

template<class T>void create_provider(CCurrencyRatesProviders::TCurrencyRatesProviders& apProviders)
{
	CCurrencyRatesProviders::TCurrencyRatesProviderPtr pProvider(new T);
	if (pProvider->Init())
		apProviders.push_back(pProvider);
};

void CCurrencyRatesProviders::CreateProviders()
{
	create_provider<CCurrencyRatesProviderCurrencyConverter>(m_apProviders);
}

void CCurrencyRatesProviders::ClearProviders()
{
	m_apProviders.clear();
}

void convert_contact_settings(MCONTACT hContact)
{
	WORD dwLogMode = db_get_w(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_LOG, static_cast<WORD>(lmDisabled));
	if ((dwLogMode&lmInternalHistory) || (dwLogMode&lmExternalFile))
		db_set_b(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CONTACT_SPEC_SETTINGS, 1);
}

void CCurrencyRatesProviders::InitProviders()
{
	CreateProviders();

	const WORD nCurrentVersion = 17;
	WORD nVersion = db_get_w(0, CURRENCYRATES_MODULE_NAME, LAST_RUN_VERSION, 1);

	for (auto &hContact : Contacts(CURRENCYRATES_MODULE_NAME)) {
		TCurrencyRatesProviderPtr pProvider = GetContactProviderPtr(hContact);
		if (pProvider) {
			pProvider->AddContact(hContact);
			if (nVersion < nCurrentVersion)
				convert_contact_settings(hContact);
		}
	}

	db_set_w(0, CURRENCYRATES_MODULE_NAME, LAST_RUN_VERSION, nCurrentVersion);
}

CCurrencyRatesProviders::TCurrencyRatesProviderPtr CCurrencyRatesProviders::GetContactProviderPtr(MCONTACT hContact)const
{
	char* szProto = GetContactProto(hContact);
	if (nullptr == szProto || 0 != ::_stricmp(szProto, CURRENCYRATES_PROTOCOL_NAME))
		return TCurrencyRatesProviderPtr();

	tstring sProvider = CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_PROVIDER);
	if (true == sProvider.empty())
		return TCurrencyRatesProviderPtr();

	return FindProvider(sProvider);
}

CCurrencyRatesProviders::TCurrencyRatesProviderPtr CCurrencyRatesProviders::FindProvider(const tstring& rsName)const
{
	TCurrencyRatesProviderPtr pResult;
	for (TCurrencyRatesProviders::const_iterator i = m_apProviders.begin(); i != m_apProviders.end(); ++i) {
		const TCurrencyRatesProviderPtr& pProvider = *i;
		const ICurrencyRatesProvider::CProviderInfo& rInfo = pProvider->GetInfo();
		if (0 == ::mir_wstrcmpi(rsName.c_str(), rInfo.m_sName.c_str())) {
			pResult = pProvider;
			break;
		}
	}

	return pResult;
}
