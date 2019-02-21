#pragma once

#ifndef __ac71e133_786c_41a7_ab07_625b76ff2a8c_CurrencyRatesProvider_h__
#define __ac71e133_786c_41a7_ab07_625b76ff2a8c_CurrencyRatesProvider_h__

class CCurrencyRatesProviderVisitor;

/////////////////////////////////////////////////////////////////////////////////////////
// CFormatSpecificator - array of variables to replace

using CFormatSpecificator = std::pair<tstring, tstring>;
typedef std::vector<CFormatSpecificator> TFormatSpecificators;

/////////////////////////////////////////////////////////////////////////////////////////
// ICurrencyRatesProvider - abstract interface

class ICurrencyRatesProvider : private boost::noncopyable
{
public:
	struct CProviderInfo
	{
		tstring m_sName;
		tstring m_sURL;
	};

public:
	ICurrencyRatesProvider() {}
	virtual ~ICurrencyRatesProvider() {}

	virtual bool Init() = 0;
	virtual const CProviderInfo& GetInfo() const = 0;

	virtual void AddContact(MCONTACT hContact) = 0;
	virtual void DeleteContact(MCONTACT hContact) = 0;

	virtual void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp) = 0;

	virtual void RefreshAllContacts() = 0;
	virtual void RefreshSettings() = 0;
	virtual void RefreshContact(MCONTACT hContact) = 0;

	virtual void FillFormat(TFormatSpecificators&) const = 0;

	virtual void Run() = 0;
	
	virtual void Accept(CCurrencyRatesProviderVisitor &visitor) const = 0;
};

#endif //__ac71e133_786c_41a7_ab07_625b76ff2a8c_CurrencyRatesProvider_h__
