#include "stdafx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"

CMStringW build_url(const CMStringW &rsURL, const CMStringW &from, const CMStringW &to)
{
	CMStringW res = rsURL + L"?q=" + from + L"_" + to + L"&compact=ultra";
	ptrA szApiKey(g_plugin.getStringA(DB_KEY_ApiKey));
	if (szApiKey != nullptr)
		res.AppendFormat(L"&apiKey=%S", szApiKey.get());
	return res;
}

CMStringW build_url(MCONTACT hContact, const CMStringW &rsURL)
{
	CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
	CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	return build_url(rsURL, sFrom, sTo);
}

bool parse_response(const CMStringW &rsJSON, double &dRate)
{
	JSONNode root = JSONNode::parse(_T2A(rsJSON));
	if (!root)
		return false;

	dRate = root.at(json_index_t(0)).as_float();
	return true;
}

CCurrencyRatesProviderCurrencyConverter::CCurrencyRatesProviderCurrencyConverter()
{
}

CCurrencyRatesProviderCurrencyConverter::~CCurrencyRatesProviderCurrencyConverter()
{
}

void CCurrencyRatesProviderCurrencyConverter::RefreshCurrencyRates(TContacts &anContacts)
{
	CHTTPSession http;
	CMStringW sURL = GetURL();

	for (TContacts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i) {
		MCONTACT hContact = *i;

		CMStringW sFullURL = build_url(hContact, sURL);
		if ((true == http.OpenURL(sFullURL)) && (true == IsOnline())) {
			CMStringW sHTML;
			if ((true == http.ReadResponce(sHTML)) && (true == IsOnline())) {
				double dRate = 0.0;
				if ((true == parse_response(sHTML, dRate)) && (true == IsOnline())) {
					WriteContactRate(hContact, dRate);
					continue;
				}
			}
		}

		SetContactStatus(hContact, ID_STATUS_NA);
	}
}

double CCurrencyRatesProviderCurrencyConverter::Convert(double dAmount, const CCurrencyRate &from, const CCurrencyRate &to) const
{
	CMStringW sFullURL = build_url(GetURL(), from.GetID(), to.GetID());

	CHTTPSession http;
	if ((true == http.OpenURL(sFullURL))) {
		CMStringW sHTML;
		if ((true == http.ReadResponce(sHTML))) {
			double dResult = 0.0;
			if ((true == parse_response(sHTML, dResult)))
				return dResult * dAmount;

			throw std::runtime_error(Translate("Error occurred during HTML parsing."));
		}
		else throw std::runtime_error(Translate("Error occurred during site access."));
	}
	else throw std::runtime_error(Translate("Error occurred during site access."));

	return 0.0;
}

size_t CCurrencyRatesProviderCurrencyConverter::GetWatchedRateCount() const
{
	return m_aContacts.size();
}

bool CCurrencyRatesProviderCurrencyConverter::GetWatchedRateInfo(size_t nIndex, TRateInfo &rRateInfo)
{
	if (nIndex >= m_aContacts.size())
		return false;

	MCONTACT hContact = m_aContacts[nIndex];
	CMStringW sSymbolFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
	CMStringW sSymbolTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	CMStringW sDescFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_DESCRIPTION);
	CMStringW sDescTo = g_plugin.getMStringW(hContact, DB_STR_TO_DESCRIPTION);

	rRateInfo.first = CCurrencyRate(sSymbolFrom, sSymbolFrom, sDescFrom);
	rRateInfo.second = CCurrencyRate(sSymbolTo, sSymbolTo, sDescTo);
	return true;
}

bool CCurrencyRatesProviderCurrencyConverter::WatchForRate(const TRateInfo &ri, bool bWatch)
{
	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [&ri](auto hContact)->bool
	{
		CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
		CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
		return !mir_wstrcmpi(ri.first.GetID().c_str(), sFrom.c_str()) && !mir_wstrcmpi(ri.second.GetID().c_str(), sTo.c_str());
	});

	auto make_contact_name = [](const CMStringW &rsSymbolFrom, const CMStringW &rsSymbolTo)->CMStringW
	{
		return rsSymbolFrom + L"/" + rsSymbolTo;
	};

	if ((true == bWatch) && (i == m_aContacts.end())) {
		CMStringW sName = make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
		MCONTACT hContact = CreateNewContact(sName);
		if (hContact) {
			g_plugin.setWString(hContact, DB_STR_FROM_ID, ri.first.GetID().c_str());
			g_plugin.setWString(hContact, DB_STR_TO_ID, ri.second.GetID().c_str());
			if (false == ri.first.GetName().IsEmpty()) {
				g_plugin.setWString(hContact, DB_STR_FROM_DESCRIPTION, ri.first.GetName().c_str());
			}
			if (false == ri.second.GetName().IsEmpty()) {
				g_plugin.setWString(hContact, DB_STR_TO_DESCRIPTION, ri.second.GetName().c_str());
			}

			return true;
		}
	}
	else if ((false == bWatch) && (i != m_aContacts.end())) {
		MCONTACT hContact = *i;
		{// for CCritSection
			mir_cslock lck(m_cs);
			m_aContacts.erase(i);
		}

		db_delete_contact(hContact, true);
		return true;
	}

	return false;
}

MCONTACT CCurrencyRatesProviderCurrencyConverter::GetContactByID(const CMStringW &rsFromID, const CMStringW &rsToID) const
{
	mir_cslock lck(m_cs);

	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [rsFromID, rsToID](MCONTACT hContact)->bool
	{
		CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
		CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
		return !mir_wstrcmpi(rsFromID.c_str(), sFrom.c_str()) && !mir_wstrcmpi(rsToID.c_str(), sTo.c_str());
	});

	if (i != m_aContacts.end())
		return *i;

	return NULL;
}

void CCurrencyRatesProviderCurrencyConverter::FillFormat(TFormatSpecificators &array) const
{
	CSuper::FillFormat(array);

	array.push_back(CFormatSpecificator(L"%F", LPGENW("From Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%f", LPGENW("From Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%I", LPGENW("Into Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%i", LPGENW("Into Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%s", LPGENW("Short notation for \"%f/%i\"")));
}

CMStringW CCurrencyRatesProviderCurrencyConverter::FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth) const
{
	switch (c) {
	case 'F':
		return g_plugin.getMStringW(hContact, DB_STR_FROM_DESCRIPTION);

	case 'f':
		return g_plugin.getMStringW(hContact, DB_STR_FROM_ID);

	case 'I':
		return g_plugin.getMStringW(hContact, DB_STR_TO_DESCRIPTION);

	case 'i':
		return g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	}

	return CSuper::FormatSymbol(hContact, c, nWidth);
}

MCONTACT CCurrencyRatesProviderCurrencyConverter::ImportContact(const TiXmlNode *pRoot)
{
	const char *sFromID = nullptr, *sToID = nullptr;

	for (auto *pNode : TiXmlFilter(pRoot, "Setting")) {
		TNameValue Item = parse_setting_node(pNode);
		if (!mir_strcmpi(Item.first, DB_STR_FROM_ID))
			sFromID = Item.second;
		else if (!mir_strcmpi(Item.first, DB_STR_TO_ID))
			sToID = Item.second;
	}

	if (sFromID && sToID)
		return GetContactByID(Utf2T(sFromID).get(), Utf2T(sToID).get());

	return 0;
}
