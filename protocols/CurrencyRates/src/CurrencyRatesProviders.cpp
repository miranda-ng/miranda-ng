#include "StdAfx.h"

#define LAST_RUN_VERSION "LastRunVersion"

void InitCC();
void InitExchangeRates();

TCurrencyRatesProviders g_apProviders;

CCurrencyRatesProviderBase *g_pCurrentProvider = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

void InitProviders()
{
	InitCC();
	InitExchangeRates();

	g_pCurrentProvider = FindProvider(g_plugin.getMStringW(DB_STR_PROVIDER));
	if (g_pCurrentProvider == nullptr)
		g_pCurrentProvider = &g_apProviders[0];
}

/////////////////////////////////////////////////////////////////////////////////////////

void ClearProviders()
{
	g_apProviders.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void convert_contact_settings(MCONTACT hContact)
{
	uint16_t dwLogMode = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG, static_cast<uint16_t>(lmDisabled));
	if ((dwLogMode&lmInternalHistory) || (dwLogMode&lmExternalFile))
		g_plugin.setByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, 1);
}

void TCurrencyRatesProviders::push(CCurrencyRatesProviderBase *pNew)
{
	if (pNew->Init())
		insert(pNew);
	else
		delete pNew;
}

/////////////////////////////////////////////////////////////////////////////////////////

CCurrencyRatesProviderBase* FindProvider(const CMStringW &rsName)
{
	for (auto &pProvider : g_apProviders) {
		const ICurrencyRatesProvider::CProviderInfo& rInfo = pProvider->GetInfo();
		if (!mir_wstrcmpi(rsName.c_str(), rInfo.m_sName.c_str()))
			return pProvider;
	}

	return nullptr;
}
