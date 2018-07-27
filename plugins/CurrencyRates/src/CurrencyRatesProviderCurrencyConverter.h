#pragma once

#define DB_STR_FROM_ID "FromID"
#define DB_STR_TO_ID "ToID"
#define DB_STR_FROM_DESCRIPTION "FromDesc"
#define DB_STR_TO_DESCRIPTION "ToDesc"


class CCurrencyRatesProviderCurrencyConverter : public CCurrencyRatesProviderBase
{
public:
	using TRateInfo = std::pair<CCurrencyRate, CCurrencyRate>;

public:
	CCurrencyRatesProviderCurrencyConverter();
	~CCurrencyRatesProviderCurrencyConverter();

	double Convert(double dAmount, const CCurrencyRate& from, const CCurrencyRate& to)const;
	size_t GetWatchedRateCount()const;
	bool GetWatchedRateInfo(size_t nIndex, TRateInfo& rRateInfo);
	bool WatchForRate(const TRateInfo& ri, bool bWatch);
	MCONTACT GetContactByID(const tstring& rsFromID, const tstring& rsToID)const;

private:
	virtual void Accept(CCurrencyRatesProviderVisitor& visitor)const override;
	virtual void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp)override;
	virtual void RefreshCurrencyRates(TContracts& anContacts)override;
};

