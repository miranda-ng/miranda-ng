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

void CQuotesProviderVisitorDbSettings::Visit(const CQuotesProviderDukasCopy&/* rProvider*/)
{
	m_pszDbRefreshRateType = DB_STR_REFRESH_RATE_TYPE;
	m_pszDbRefreshRateValue = DB_STR_REFRESH_RATE_VALUE;
	m_pszDbDisplayNameFormat = DB_STR_DC_DISPLAY_NAME_FORMAT;
	m_pszDefDisplayFormat = L"%s %r";
	m_pszXMLIniFileName = L"Dukascopy.xml";
	m_pszDbStatusMsgFormat = "DC_StatusMessageFormat";

	m_pszDbLogMode = "DC_LogMode";
	m_pszDbHistoryFormat = "DC_HistoryFormat";
	m_pszDbHistoryCondition = "DC_AddToHistoryOnlyIfValueIsChanged";
	m_pszDbLogFile = "DC_LogFile";
	m_pszDbLogFormat = "DC_LogFileFormat";
	m_pszDbLogCondition = "DC_AddToLogOnlyIfValueIsChanged";
	m_pszDbPopupFormat = "DC_PopupFormat";
	m_pszDbPopupCondition = "DC_ShowPopupOnlyIfValueChanged";

	m_pszDbPopupColourMode = "DC_PopupColourMode";
	m_pszDbPopupBkColour = "DC_PopupColourBk";
	m_pszDbPopupTextColour = "DC_PopupColourText";
	m_pszDbPopupDelayMode = "DC_PopupDelayMode";
	m_pszDbPopupDelayTimeout = "DC_PopupDelayTimeout";
	m_pszDbPopupHistoryFlag = "DC_PopupHistoryFlag";
	m_pszDbTendencyFormat = "DC_TendencyFormat";
}

void CQuotesProviderVisitorDbSettings::Visit(const CQuotesProviderGoogle&/* rProvider*/)
{
	m_pszDbRefreshRateType = DB_STR_GOOGLE_REFRESH_RATE_TYPE;
	m_pszDbRefreshRateValue = DB_STR_GOOGLE_REFRESH_RATE_VALUE;
	m_pszDbDisplayNameFormat = DB_STR_GOOGLE_DISPLAY_NAME_FORMAT;
	m_pszDefDisplayFormat = L"1 %f = %r %i";
	m_pszXMLIniFileName = L"Google.xml";
	m_pszDbStatusMsgFormat = "Google_StatusMessageFormat";

	m_pszDbLogMode = "Google_LogMode";
	m_pszDbHistoryFormat = "Google_HistoryFormat";
	m_pszDbHistoryCondition = "Google_AddToHistoryOnlyIfValueIsChanged";
	m_pszDbLogFile = "Google_LogFile";
	m_pszDbLogFormat = "Google_LogFileFormat";
	m_pszDbLogCondition = "Google_AddToLogOnlyIfValueIsChanged";
	m_pszDbPopupFormat = "Google_PopupFormat";
	m_pszDbPopupCondition = "Google_ShowPopupOnlyIfValueChanged";

	m_pszDbPopupColourMode = "Google_PopupColourMode";
	m_pszDbPopupBkColour = "Google_PopupColourBk";
	m_pszDbPopupTextColour = "Google_PopupColourText";
	m_pszDbPopupDelayMode = "Google_PopupDelayMode";
	m_pszDbPopupDelayTimeout = "Google_PopupDelayTimeout";
	m_pszDbPopupHistoryFlag = "Google_PopupHistoryFlag";

	m_pszDbTendencyFormat = "Google_TendencyFormat";
}

void CQuotesProviderVisitorDbSettings::Visit(const CQuotesProviderGoogleFinance&/* rProvider*/)
{
	m_pszDbRefreshRateType = "GoogleFinance_RefreshRateType";
	m_pszDbRefreshRateValue = "GoogleFinance_RefreshRateValue";
	m_pszDbDisplayNameFormat = "GoogleFinance_DspNameFrmt";
	m_pszDefDisplayFormat = L"%s %r";
	m_pszXMLIniFileName = L"GoogleFinance.xml";
	m_pszDbStatusMsgFormat = "GoogleFinance_StatusMessageFormat";

	m_pszDbLogMode = "GoogleFinance_LogMode";
	m_pszDbHistoryFormat = "GoogleFinance_HistoryFormat";
	m_pszDbHistoryCondition = "GoogleFinance_AddToHistoryOnlyIfValueIsChanged";
	m_pszDbLogFile = "GoogleFinance_LogFile";
	m_pszDbLogFormat = "GoogleFinance_LogFileFormat";
	m_pszDbLogCondition = "GoogleFinance_AddToLogOnlyIfValueIsChanged";
	m_pszDbPopupFormat = "GoogleFinance_PopupFormat";
	m_pszDbPopupCondition = "GoogleFinance_ShowPopupOnlyIfValueChanged";

	m_pszDbPopupColourMode = "GoogleFinance_PopupColourMode";
	m_pszDbPopupBkColour = "GoogleFinance_PopupColourBk";
	m_pszDbPopupTextColour = "GoogleFinance_PopupColourText";
	m_pszDbPopupDelayMode = "GoogleFinance_PopupDelayMode";
	m_pszDbPopupDelayTimeout = "GoogleFinance_PopupDelayTimeout";
	m_pszDbPopupHistoryFlag = "GoogleFinance_PopupHistoryFlag";

	m_pszDbTendencyFormat = "GoogleFinance_TendencyFormat";
}

void CQuotesProviderVisitorDbSettings::Visit(const CQuotesProviderYahoo&)
{
	m_pszDbRefreshRateType = "Yahoo_RefreshRateType";
	m_pszDbRefreshRateValue = "Yahoo_RefreshRateValue";
	m_pszDbDisplayNameFormat = "Yahoo_DspNameFrmt";
	m_pszDefDisplayFormat = L"%s %r";
	m_pszXMLIniFileName = L"Yahoo.xml";
	m_pszDbStatusMsgFormat = "Yahoo_StatusMessageFormat";

	m_pszDbLogMode = "Yahoo_LogMode";
	m_pszDbHistoryFormat = "Yahoo_HistoryFormat";
	m_pszDbHistoryCondition = "Yahoo_AddToHistoryOnlyIfValueIsChanged";
	m_pszDbLogFile = "Yahoo_LogFile";
	m_pszDbLogFormat = "Yahoo_LogFileFormat";
	m_pszDbLogCondition = "Yahoo_AddToLogOnlyIfValueIsChanged";
	m_pszDbPopupFormat = "Yahoo_PopupFormat";
	m_pszDbPopupCondition = "Yahoo_ShowPopupOnlyIfValueChanged";

	m_pszDbPopupColourMode = "Yahoo_PopupColourMode";
	m_pszDbPopupBkColour = "Yahoo_PopupColourBk";
	m_pszDbPopupTextColour = "Yahoo_PopupColourText";
	m_pszDbPopupDelayMode = "Yahoo_PopupDelayMode";
	m_pszDbPopupDelayTimeout = "Yahoo_PopupDelayTimeout";
	m_pszDbPopupHistoryFlag = "Yahoo_PopupHistoryFlag";

	m_pszDbTendencyFormat = "Yahoo_TendencyFormat";
}
