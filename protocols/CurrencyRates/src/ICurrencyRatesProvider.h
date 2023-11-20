#pragma once

#ifndef __ac71e133_786c_41a7_ab07_625b76ff2a8c_CurrencyRatesProvider_h__
#define __ac71e133_786c_41a7_ab07_625b76ff2a8c_CurrencyRatesProvider_h__

class CCurrencyRatesProviderVisitor;

/////////////////////////////////////////////////////////////////////////////////////////
// CFormatSpecificator - array of variables to replace

using CFormatSpecificator = std::pair<const wchar_t *, const wchar_t *>;
typedef std::vector<CFormatSpecificator> TFormatSpecificators;

/////////////////////////////////////////////////////////////////////////////////////////
// ICurrencyRatesProvider - abstract interface

class ICurrencyRatesProvider : private boost::noncopyable
{
public:
	struct CProviderInfo
	{
		CMStringW m_sName;
		CMStringW m_sURL;
	};

public:
	ICurrencyRatesProvider() {}
	virtual ~ICurrencyRatesProvider() {}

	virtual bool Init() = 0;
	virtual const CProviderInfo& GetInfo() const = 0;

	virtual MCONTACT ImportContact(const TiXmlNode*) = 0;

	virtual void RefreshAllContacts() = 0;
	virtual void RefreshSettings() = 0;
	virtual void RefreshContact(MCONTACT hContact) = 0;

	virtual void FillFormat(TFormatSpecificators&) const = 0;
	virtual bool ParseSymbol(MCONTACT hContact, wchar_t c, double &d) const = 0;
	virtual CMStringW FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth = 0) const = 0;

	virtual void Run() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////

class CCurrencyRatesProviderBase;

struct TCurrencyRatesProviders : public OBJLIST<CCurrencyRatesProviderBase>
{
	TCurrencyRatesProviders() : 
		OBJLIST<CCurrencyRatesProviderBase>(1)
	{}

	void push(CCurrencyRatesProviderBase *pNew);
};

extern TCurrencyRatesProviders g_apProviders;
extern CCurrencyRatesProviderBase *g_pCurrentProvider;

CCurrencyRatesProviderBase* FindProvider(const CMStringW &rsName);

void InitProviders();
void ClearProviders();

#endif //__ac71e133_786c_41a7_ab07_625b76ff2a8c_CurrencyRatesProvider_h__
