#ifndef __00c159f3_525a_41e9_8fc5_00464b6fafa3_QuotesProviderVisitorFormatSpecificator_h__
#define __00c159f3_525a_41e9_8fc5_00464b6fafa3_QuotesProviderVisitorFormatSpecificator_h__

class CQuotesProviderVisitorFormatSpecificator : public CQuotesProviderVisitor
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
	CQuotesProviderVisitorFormatSpecificator();
	~CQuotesProviderVisitorFormatSpecificator();

	const TFormatSpecificators& GetSpecificators()const;

private:
	virtual void Visit(const CQuotesProviderDukasCopy& rProvider)override;
	virtual void Visit(const CQuotesProviderBase& rProvider)override;
	virtual void Visit(const CQuotesProviderGoogleFinance& rProvider)override;
	virtual void Visit(const CQuotesProviderYahoo& rProvider)override;
	virtual void Visit(const CQuotesProviderCurrencyConverter& rProvider)override;
private:
	TFormatSpecificators m_aSpecificators;
};

#endif//__00c159f3_525a_41e9_8fc5_00464b6fafa3_QuotesProviderVisitorFormatSpecificator_h__
