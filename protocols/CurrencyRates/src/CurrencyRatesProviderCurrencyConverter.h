#pragma once

#define DB_STR_FROM_ID "FromID"
#define DB_STR_TO_ID "ToID"
#define DB_STR_FROM_DESCRIPTION "FromDesc"
#define DB_STR_TO_DESCRIPTION "ToDesc"


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
	void FillFormat(TFormatSpecificators &) const override;
	void RefreshCurrencyRates(TContacts &anContacts) override;
	void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE &odp) override;

	MCONTACT ImportContact(const TiXmlNode*) override;
	CMStringW FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth) const override;
};
