#ifndef __AD721194_E9944366_9CF1_0307460EF32F_CurrencyRatesProviderVisitorTendency_h__
#define __AD721194_E9944366_9CF1_0307460EF32F_CurrencyRatesProviderVisitorTendency_h__

class CCurrencyRatesProviderVisitorTendency : public CCurrencyRatesProviderVisitor
{
public:
	CCurrencyRatesProviderVisitorTendency(MCONTACT hContact, wchar_t chr);

	bool IsValid() const{ return m_bValid; }
	double GetResult() const{ return m_dResult; }

private:
	virtual void Visit(const CCurrencyRatesProviderBase& rProvider);

private:
	void GetValue(LPCSTR pszDbKeyName);

private:
	MCONTACT m_hContact;
	wchar_t m_chr;
	bool m_bValid = false;
	double m_dResult = 0.0;
};

#endif //__AD721194_E9944366_9CF1_0307460EF32F_CurrencyRatesProviderVisitorTendency_h__
