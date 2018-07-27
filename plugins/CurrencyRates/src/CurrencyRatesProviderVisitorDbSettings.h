#ifndef __97cd432a_1119_4803_a96f_0abc1cc2653f_CurrencyRatesProviderVisitorDbSettings_h__
#define __97cd432a_1119_4803_a96f_0abc1cc2653f_CurrencyRatesProviderVisitorDbSettings_h__

class CCurrencyRatesProviderVisitorDbSettings : public CCurrencyRatesProviderVisitor
{
public:
	CCurrencyRatesProviderVisitorDbSettings();
	~CCurrencyRatesProviderVisitorDbSettings();

private:
	virtual void Visit(const CCurrencyRatesProviderBase& rProvider)override;
	virtual void Visit(const CCurrencyRatesProviderCurrencyConverter& rProvider)override;
public:
	LPCSTR m_pszDbRefreshRateType;
	LPCSTR m_pszDbRefreshRateValue;
	LPCSTR m_pszDbDisplayNameFormat;
	LPCTSTR m_pszDefDisplayFormat;
	LPCTSTR m_pszDefLogFileFormat;
	LPCTSTR m_pszDefHistoryFormat;
	LPCTSTR m_pszXMLIniFileName;
	LPCSTR m_pszDbStatusMsgFormat;
	LPCTSTR m_pszDefStatusMsgFormat;
	LPCTSTR m_pszDefPopupFormat;
	LPCSTR m_pszDbTendencyFormat;
	LPCTSTR m_pszDefTendencyFormat;

	//global settings
	LPCSTR m_pszDbLogMode;
	LPCSTR m_pszDbHistoryFormat;
	LPCSTR m_pszDbHistoryCondition;
	LPCSTR m_pszDbLogFile;
	LPCSTR m_pszDbLogFormat;
	LPCSTR m_pszDbLogCondition;
	LPCSTR m_pszDbPopupFormat;
	LPCSTR m_pszDbPopupCondition;
	LPCSTR m_pszDbPopupColourMode;
	LPCSTR m_pszDbPopupBkColour;
	LPCSTR m_pszDbPopupTextColour;
	LPCSTR m_pszDbPopupDelayMode;
	LPCSTR m_pszDbPopupDelayTimeout;
	LPCSTR m_pszDbPopupHistoryFlag;
};

#endif //__97cd432a_1119_4803_a96f_0abc1cc2653f_CurrencyRatesProviderVisitorDbSettings_h__
