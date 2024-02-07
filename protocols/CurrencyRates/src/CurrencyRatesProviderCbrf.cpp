#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRatesProviderExchangeRates implementation

class CCurrencyRatesProviderCbrf : public CCurrencyRatesProviderBase
{
	std::map<CMStringA, double> m_lastRates;

	double GetRate(const CMStringA &id1, const CMStringA &id2) const
	{
		if (!id1.IsEmpty() && !id2.IsEmpty()) {
			auto r1 = m_lastRates.find(id1), r2 = m_lastRates.find(id2);
			if (r1 != m_lastRates.end() && r2 != m_lastRates.end())
				return r2->second / r1->second;
		}

		return 0.0;
	}

public:
	CCurrencyRatesProviderCbrf()
	{}

	void RefreshCurrencyRates(TContacts &anContacts) override
	{
		CHTTPSession http;
		if (true == http.OpenURL(GetURL())) {
			CMStringW sHTML;
			if (true == http.ReadResponce(sHTML)) {
				JSONNode root = JSONNode::parse(_T2A(sHTML));
				if (!root)
					return;

				auto rates = root["rates"];
				if (!rates)
					return;

				auto &qs = GetSection();

				m_lastRates.clear();
				for (auto &it : qs.GetCurrencyRates()) {
					CMStringA id = it.GetID();
					m_lastRates[id] = (id == "RUB") ? 1.0 : rates[id].as_float();
				}
			}
		}

		for (auto &hContact : anContacts) {
			double rate = GetRate(g_plugin.getMStringA(hContact, DB_STR_FROM_ID), g_plugin.getMStringA(hContact, DB_STR_TO_ID));
			if (rate != 0.0) {
				WriteContactRate(hContact, rate);
				continue;
			}
			SetContactStatus(hContact, ID_STATUS_NA);
		}
	}

	double Convert(double dAmount, const CCurrencyRate &from, const CCurrencyRate &to) const override
	{
		CMStringA id1 = from.GetID(), id2 = to.GetID();
		return dAmount * GetRate(id1, id2);
	}

	wchar_t* GetXmlFilename() const override
	{
		return L"cbrf.xml";
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitCbrf()
{
	g_apProviders.push(new CCurrencyRatesProviderCbrf());
}
