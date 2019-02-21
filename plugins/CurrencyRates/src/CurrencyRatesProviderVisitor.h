#ifndef __7fca59e7_17b2_4849_bd7a_02c7675f2d76_CurrencyRatesProviderVisitor_h__
#define __7fca59e7_17b2_4849_bd7a_02c7675f2d76_CurrencyRatesProviderVisitor_h__

class CCurrencyRatesProviderBase;
class CCurrencyRatesProviderFinance;
class CCurrencyRatesProviderDukasCopy;
class CCurrencyRatesProviderGoogleFinance;
class CCurrencyRatesProviderYahoo;
class CCurrencyRatesProviderCurrencyConverter;

class CCurrencyRatesProviderVisitor
{
public:
	CCurrencyRatesProviderVisitor() {}
	virtual ~CCurrencyRatesProviderVisitor() {}

	virtual void Visit(const CCurrencyRatesProviderBase&) {}
	virtual void Visit(const CCurrencyRatesProviderFinance&) {}
	virtual void Visit(const CCurrencyRatesProviderDukasCopy& ){}
	virtual void Visit(const CCurrencyRatesProviderGoogleFinance&) {}
	virtual void Visit(const CCurrencyRatesProviderYahoo&) {}
	virtual void Visit(const CCurrencyRatesProviderCurrencyConverter&) {}
};

#endif //__7fca59e7_17b2_4849_bd7a_02c7675f2d76_CurrencyRatesProviderVisitor_h__
