#include "StdAfx.h"

CQuotesProviderVisitorDbSettings::CQuotesProviderVisitorDbSettings()
	: m_pszDbRefreshRateType(nullptr),
	m_pszDbRefreshRateValue(nullptr),
	m_pszDbDisplayNameFormat(nullptr),
	m_pszDefDisplayFormat(nullptr),
	m_pszDefLogFileFormat(nullptr),
	m_pszDefHistoryFormat(nullptr),
	m_pszXMLIniFileName(nullptr),
	m_pszDbStatusMsgFormat(nullptr),
	m_pszDefStatusMsgFormat(nullptr),
	m_pszDbLogMode(nullptr),
	m_pszDbHistoryFormat(nullptr),
	m_pszDbHistoryCondition(nullptr),
	m_pszDbLogFile(nullptr),
	m_pszDbLogFormat(nullptr),
	m_pszDbLogCondition(nullptr),
	m_pszDbPopupFormat(nullptr),
	m_pszDefPopupFormat(nullptr),
	m_pszDbPopupCondition(nullptr),
	m_pszDbPopupColourMode(nullptr),
	m_pszDbPopupBkColour(nullptr),
	m_pszDbPopupTextColour(nullptr),
	m_pszDbPopupDelayMode(nullptr),
	m_pszDbPopupDelayTimeout(nullptr),
	m_pszDbPopupHistoryFlag(nullptr),
	m_pszDbTendencyFormat(nullptr),
	m_pszDefTendencyFormat(L"%r>%p")
{
}

CQuotesProviderVisitorDbSettings::~CQuotesProviderVisitorDbSettings()
{
}

void CQuotesProviderVisitorDbSettings::Visit(const CQuotesProviderBase&/* rProvider*/)
{
	m_pszDefLogFileFormat = L"%s\\t%t\\t%r\\n";
	m_pszDefHistoryFormat = L"%s %r";
	m_pszDefPopupFormat = L"\\nCurrent = %r\\nPrevious = %p";
}

void CQuotesProviderVisitorDbSettings::Visit(const CQuotesProviderCurrencyConverter&)
{
	m_pszDbRefreshRateType = "CC_RefreshRateType";
	m_pszDbRefreshRateValue = "CC_RefreshRateValue";
	m_pszDbDisplayNameFormat = "CC_DspNameFrmt";
	m_pszDefDisplayFormat = L"1 %f = %r %i";
	m_pszXMLIniFileName = L"CC.xml";
	m_pszDbStatusMsgFormat = "CC_StatusMessageFormat";

	m_pszDbLogMode = "CC_LogMode";
	m_pszDbHistoryFormat = "CC_HistoryFormat";
	m_pszDbHistoryCondition = "CC_AddToHistoryOnlyIfValueIsChanged";
	m_pszDbLogFile = "CC_LogFile";
	m_pszDbLogFormat = "CC_LogFileFormat";
	m_pszDbLogCondition = "CC_AddToLogOnlyIfValueIsChanged";
	m_pszDbPopupFormat = "CC_PopupFormat";
	m_pszDbPopupCondition = "CC_ShowPopupOnlyIfValueChanged";

	m_pszDbPopupColourMode = "CC_PopupColourMode";
	m_pszDbPopupBkColour = "CC_PopupColourBk";
	m_pszDbPopupTextColour = "CC_PopupColourText";
	m_pszDbPopupDelayMode = "CC_PopupDelayMode";
	m_pszDbPopupDelayTimeout = "CC_PopupDelayTimeout";
	m_pszDbPopupHistoryFlag = "CC_PopupHistoryFlag";

	m_pszDbTendencyFormat = "CC_TendencyFormat";
}
