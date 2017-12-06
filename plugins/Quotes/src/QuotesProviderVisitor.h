#ifndef __7fca59e7_17b2_4849_bd7a_02c7675f2d76_QuotesProviderVisitor_h__
#define __7fca59e7_17b2_4849_bd7a_02c7675f2d76_QuotesProviderVisitor_h__

class CQuotesProviderBase;
class CQuotesProviderFinance;
class CQuotesProviderDukasCopy;
class CQuotesProviderGoogle;
class CQuotesProviderGoogleFinance;
class CQuotesProviderYahoo;

class CQuotesProviderVisitor
{
public:
	CQuotesProviderVisitor() {}
	virtual ~CQuotesProviderVisitor() {}

	virtual void Visit(const CQuotesProviderBase&){}
	virtual void Visit(const CQuotesProviderFinance&){}
	virtual void Visit(const CQuotesProviderDukasCopy&){}
	virtual void Visit(const CQuotesProviderGoogle&){}
	virtual void Visit(const CQuotesProviderGoogleFinance&){}
	virtual void Visit(const CQuotesProviderYahoo&){}
};

#endif //__7fca59e7_17b2_4849_bd7a_02c7675f2d76_QuotesProviderVisitor_h__
