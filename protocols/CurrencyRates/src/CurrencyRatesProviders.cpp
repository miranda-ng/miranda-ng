#include "StdAfx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"

#define LAST_RUN_VERSION "LastRunVersion"

TCurrencyRatesProviders g_apProviders;

/////////////////////////////////////////////////////////////////////////////////////////

void CreateProviders()
{
	g_apProviders.push(new CCurrencyRatesProviderCurrencyConverter());
}

/////////////////////////////////////////////////////////////////////////////////////////

void ClearProviders()
{
	g_apProviders.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void convert_contact_settings(MCONTACT hContact)
{
	WORD dwLogMode = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG, static_cast<WORD>(lmDisabled));
	if ((dwLogMode&lmInternalHistory) || (dwLogMode&lmExternalFile))
		g_plugin.setByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, 1);
}

void InitProviders()
{
	CreateProviders();

	const WORD nCurrentVersion = 17;
	WORD nVersion = g_plugin.getWord(LAST_RUN_VERSION, 1);

	for (auto &hContact : Contacts(MODULENAME)) {
		ICurrencyRatesProvider *pProvider = GetContactProviderPtr(hContact);
		if (pProvider) {
			pProvider->AddContact(hContact);
			if (nVersion < nCurrentVersion)
				convert_contact_settings(hContact);
		}
	}

	g_plugin.setWord(LAST_RUN_VERSION, nCurrentVersion);
}

/////////////////////////////////////////////////////////////////////////////////////////

ICurrencyRatesProvider* GetContactProviderPtr(MCONTACT hContact)
{
	char* szProto = Proto_GetBaseAccountName(hContact);
	if (nullptr == szProto || 0 != ::_stricmp(szProto, MODULENAME))
		return nullptr;

	CMStringW sProvider = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_PROVIDER);
	if (sProvider.IsEmpty())
		return nullptr;

	return FindProvider(sProvider);
}

/////////////////////////////////////////////////////////////////////////////////////////

ICurrencyRatesProvider* FindProvider(const CMStringW &rsName)
{
	for (auto &pProvider : g_apProviders) {
		const ICurrencyRatesProvider::CProviderInfo& rInfo = pProvider->GetInfo();
		if (!mir_wstrcmpi(rsName.c_str(), rInfo.m_sName.c_str()))
			return pProvider;
	}

	return nullptr;
}
