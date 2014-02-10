#pragma once

#ifndef __ac71e133_786c_41a7_ab07_625b76ff2a8c_QuotesProvider_h__
#define __ac71e133_786c_41a7_ab07_625b76ff2a8c_QuotesProvider_h__

class CQuotesProviderVisitor;

class IQuotesProvider : private boost::noncopyable
{
public:
	struct CProviderInfo 
	{		
		tstring m_sName;
		tstring m_sURL;

	};

public:
	IQuotesProvider() {}
	virtual ~IQuotesProvider() {}

	virtual bool Init() = 0;
	virtual const CProviderInfo& GetInfo()const = 0;

	virtual void AddContact(HCONTACT hContact) = 0;
	virtual void DeleteContact(HCONTACT hContact) = 0;

	virtual void ShowPropertyPage(WPARAM wp,OPTIONSDIALOGPAGE& odp) = 0;
	virtual void RefreshAll() = 0;
 	virtual void RefreshContact(HCONTACT hContact) = 0;
	virtual void SetContactExtraIcon(HCONTACT hContact)const = 0;

	virtual void Run() = 0;

	virtual void Accept(CQuotesProviderVisitor& visitor)const = 0;
};

#endif //__ac71e133_786c_41a7_ab07_625b76ff2a8c_QuotesProvider_h__
