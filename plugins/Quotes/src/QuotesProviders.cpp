#include "StdAfx.h"

#define LAST_RUN_VERSION "LastRunVersion"

CQuotesProviders::CQuotesProviders()
{
	InitProviders();
}

CQuotesProviders::~CQuotesProviders()
{
	ClearProviders();
}

const CQuotesProviders::TQuotesProviders& CQuotesProviders::GetProviders()const
{
	return m_apProviders;
}

namespace
{
	template<class T>void create_provider(CQuotesProviders::TQuotesProviders& apProviders)
	{
		CQuotesProviders::TQuotesProviderPtr pProvider(new T);
		if (pProvider->Init())
		{
			apProviders.push_back(pProvider);
		}
	}
}

void CQuotesProviders::CreateProviders()
{
	create_provider<CQuotesProviderDukasCopy>(m_apProviders);
	create_provider<CQuotesProviderGoogle>(m_apProviders);
	create_provider<CQuotesProviderGoogleFinance>(m_apProviders);
	create_provider<CQuotesProviderYahoo>(m_apProviders);
}

void CQuotesProviders::ClearProviders()
{
	m_apProviders.clear();
}

namespace
{
	void convert_contact_settings(MCONTACT hContact)
	{
		WORD dwLogMode = db_get_w(hContact, QUOTES_PROTOCOL_NAME, DB_STR_QUOTE_LOG, static_cast<WORD>(lmDisabled));
		if ((dwLogMode&lmInternalHistory) || (dwLogMode&lmExternalFile))
		{
			db_set_b(hContact, QUOTES_PROTOCOL_NAME, DB_STR_CONTACT_SPEC_SETTINGS, 1);
		}
	}
}
void CQuotesProviders::InitProviders()
{
	CreateProviders();

	const WORD nCurrentVersion = 17;
	WORD nVersion = db_get_w(NULL, QUOTES_MODULE_NAME, LAST_RUN_VERSION, 1);

	for (MCONTACT hContact = db_find_first(QUOTES_MODULE_NAME); hContact; hContact = db_find_next(hContact, QUOTES_MODULE_NAME))
	{
		TQuotesProviderPtr pProvider = GetContactProviderPtr(hContact);
		if (pProvider)
		{
			pProvider->AddContact(hContact);
			if (nVersion < nCurrentVersion)
			{
				convert_contact_settings(hContact);
			}
		}
	}

	db_set_w(NULL, QUOTES_MODULE_NAME, LAST_RUN_VERSION, nCurrentVersion);
}

CQuotesProviders::TQuotesProviderPtr CQuotesProviders::GetContactProviderPtr(MCONTACT hContact)const
{
	char* szProto = GetContactProto(hContact);
	if (NULL == szProto || 0 != ::_stricmp(szProto, QUOTES_PROTOCOL_NAME))
	{
		return TQuotesProviderPtr();
	}

	tstring sProvider = Quotes_DBGetStringT(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_PROVIDER);
	if (true == sProvider.empty())
	{
		return TQuotesProviderPtr();
	}

	return FindProvider(sProvider);
}

CQuotesProviders::TQuotesProviderPtr CQuotesProviders::FindProvider(const tstring& rsName)const
{
	TQuotesProviderPtr pResult;
	for (TQuotesProviders::const_iterator i = m_apProviders.begin(); i != m_apProviders.end(); ++i)
	{
		const TQuotesProviderPtr& pProvider = *i;
		const IQuotesProvider::CProviderInfo& rInfo = pProvider->GetInfo();
		if (0 == ::quotes_stricmp(rsName.c_str(), rInfo.m_sName.c_str()))
		{
			pResult = pProvider;
			break;
		}
	}

	return pResult;
}
