#pragma once

class CCurrencyRatesProviderCurrencyConverter : public CCurrencyRatesProviderBase
{
public:
	typedef CCurrencyRatesProviderBase CSuper;
	using TRateInfo = std::pair<CCurrencyRate, CCurrencyRate>;

public:
	CCurrencyRatesProviderCurrencyConverter();
	~CCurrencyRatesProviderCurrencyConverter();

	double Convert(double dAmount, const CCurrencyRate &from, const CCurrencyRate &to) const;
	size_t GetWatchedRateCount() const;
	bool GetWatchedRateInfo(size_t nIndex, TRateInfo &rRateInfo);
	bool WatchForRate(const TRateInfo &ri, bool bWatch);
	MCONTACT GetContactByID(const CMStringW &rsFromID, const CMStringW &rsToID) const;

private:
	void RefreshCurrencyRates(TContacts &anContacts) override;

	wchar_t* GetXmlFilename() const override;

	MCONTACT ImportContact(const TiXmlNode*) override;
};
