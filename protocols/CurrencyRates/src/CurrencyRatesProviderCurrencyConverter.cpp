#include "stdafx.h"

static CMStringW build_url(const CMStringW &rsURL, const CMStringW &from, const CMStringW &to)
{
	CMStringW res = rsURL + L"?q=" + from + L"_" + to + L"&compact=ultra";
	if (mir_wstrlen(g_plugin.wszApiKey))
		res.AppendFormat(L"&apiKey=%S", (wchar_t*)g_plugin.wszApiKey);
	return res;
}

static CMStringW build_url(MCONTACT hContact, const CMStringW &rsURL)
{
	CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
	CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	return build_url(rsURL, sFrom, sTo);
}

static bool parse_response(const CMStringW &rsJSON, double &dRate)
{
	JSONNode root = JSONNode::parse(_T2A(rsJSON));
	if (!root)
		return false;

	dRate = root.at(json_index_t(0)).as_float();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRatesProviderCurrencyConverter implementation

struct CCurrencyRatesProviderCurrencyConverter : public CCurrencyRatesProviderBase
{
	CCurrencyRatesProviderCurrencyConverter()
	{}

	double Convert(double dAmount, const CCurrencyRate &from, const CCurrencyRate &to) const override
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

private:
	void RefreshCurrencyRates(TContacts &anContacts) override
	{
		CHTTPSession http;
		CMStringW sURL = GetURL();

		for (TContacts::const_iterator i = anContacts.begin(); i != anContacts.end(); ++i) {
			MCONTACT hContact = *i;

			CMStringW sFullURL = build_url(hContact, sURL);
			if (true == http.OpenURL(sFullURL)) {
				CMStringW sHTML;
				if (true == http.ReadResponce(sHTML)) {
					double dRate = 0.0;
					if (true == parse_response(sHTML, dRate)) {
						WriteContactRate(hContact, dRate);
						continue;
					}
				}
			}

			SetContactStatus(hContact, ID_STATUS_NA);
		}
	}

	wchar_t *GetXmlFilename() const override
	{
		return L"CC.xml";
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitCC()
{
	g_apProviders.push(new CCurrencyRatesProviderCurrencyConverter());
}
