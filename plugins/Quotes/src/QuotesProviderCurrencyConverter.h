#pragma once

#define DB_STR_FROM_ID "FromID"
#define DB_STR_TO_ID "ToID"
#define DB_STR_FROM_DESCRIPTION "FromDesc"
#define DB_STR_TO_DESCRIPTION "ToDesc"


class CQuotesProviderCurrencyConverter : public CQuotesProviderBase
{
public:
	using TRateInfo = std::pair<CQuote, CQuote>;

public:
	CQuotesProviderCurrencyConverter();
	~CQuotesProviderCurrencyConverter();

	double Convert(double dAmount, const CQuote& from, const CQuote& to)const;
	size_t GetWatchedRateCount()const;
	bool GetWatchedRateInfo(size_t nIndex, TRateInfo& rRateInfo);
	bool WatchForRate(const TRateInfo& ri, bool bWatch);
	MCONTACT GetContactByID(const tstring& rsFromID, const tstring& rsToID)const;

private:
	virtual void Accept(CQuotesProviderVisitor& visitor)const override;
	virtual void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp)override;
	virtual void RefreshQuotes(TContracts& anContacts)override;
};

