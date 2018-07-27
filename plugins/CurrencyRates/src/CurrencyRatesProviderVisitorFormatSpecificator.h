#ifndef __00c159f3_525a_41e9_8fc5_00464b6fafa3_CurrencyRatesProviderVisitorFormatSpecificator_h__
#define __00c159f3_525a_41e9_8fc5_00464b6fafa3_CurrencyRatesProviderVisitorFormatSpecificator_h__

class CCurrencyRatesProviderVisitorFormatSpecificator : public CCurrencyRatesProviderVisitor
{
public:
	struct CFormatSpecificator
	{
		CFormatSpecificator(const tstring& rsSymbol = L"", const tstring& rsDec = L"")
			: m_sSymbol(rsSymbol), m_sDesc(rsDec){}

		tstring m_sSymbol;
		tstring m_sDesc;
	};
	typedef std::vector<CFormatSpecificator> TFormatSpecificators;

public:
	CCurrencyRatesProviderVisitorFormatSpecificator();
	~CCurrencyRatesProviderVisitorFormatSpecificator();

	const TFormatSpecificators& GetSpecificators()const;

private:
	virtual void Visit(const CCurrencyRatesProviderDukasCopy& rProvider)override;
	virtual void Visit(const CCurrencyRatesProviderBase& rProvider)override;
	virtual void Visit(const CCurrencyRatesProviderGoogleFinance& rProvider)override;
	virtual void Visit(const CCurrencyRatesProviderYahoo& rProvider)override;
	virtual void Visit(const CCurrencyRatesProviderCurrencyConverter& rProvider)override;
private:
	TFormatSpecificators m_aSpecificators;
};

#endif//__00c159f3_525a_41e9_8fc5_00464b6fafa3_CurrencyRatesProviderVisitorFormatSpecificator_h__
