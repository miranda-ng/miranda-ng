#ifndef __87d726e0_26c6_485d_8016_1fba819b037d_EconomicRateInfo__
#define __87d726e0_26c6_485d_8016_1fba819b037d_EconomicRateInfo__

// db settings module path
#define MODULENAME "CurrencyRates" 

enum ERefreshRateType
{
	RRT_SECONDS = 0,
	RRT_MINUTES = 1,
	RRT_HOURS = 2
};

#define DB_STR_FROM_ID "FromID"
#define DB_STR_TO_ID "ToID"
#define DB_STR_PROVIDER "Provider"
#define DB_STR_FROM_DESCRIPTION "FromDesc"
#define DB_STR_TO_DESCRIPTION "ToDesc"
#define DB_STR_ENABLE_LOG "EnableLog"
#define DB_STR_CURRENCYRATE_PROVIDER "CurrencyRateProvider"
#define DB_STR_CURRENCYRATE_ID "CurrencyRateID"
#define DB_STR_CURRENCYRATE_SYMBOL "CurrencyRateSymbol"
#define DB_STR_CURRENCYRATE_DESCRIPTION "CurrencyRateDescription"
#define DB_STR_CURRENCYRATE_PREV_VALUE "PreviousCurrencyRateValue"
#define DB_STR_CURRENCYRATE_CURR_VALUE "CurrentCurrencyRateValue"
#define DB_STR_CURRENCYRATE_FETCH_TIME "FetchTime"

enum ELogMode
{
	lmDisabled = 0x0000,
	lmInternalHistory = 0x0001,
	lmExternalFile = 0x0002,
	lmPopup = 0x0004,
};

#define DB_STR_CONTACT_SPEC_SETTINGS "ContactSpecSettings"
#define DB_STR_CURRENCYRATE_LOG "Log"
#define DB_STR_CURRENCYRATE_LOG_FILE "LogFile"
#define DB_STR_CURRENCYRATE_FORMAT_LOG_FILE "LogFileFormat"
#define DB_STR_CURRENCYRATE_FORMAT_HISTORY "HistoryFormat"
#define DB_STR_CURRENCYRATE_LOG_FILE_CONDITION "AddToLogOnlyIfValueIsChanged"
#define DB_STR_CURRENCYRATE_HISTORY_CONDITION "AddToHistoryOnlyIfValueIsChanged"
#define DB_STR_CURRENCYRATE_EXTRA_IMAGE_SLOT "ExtraImageSlot"
#define DB_STR_CURRENCYRATE_FORMAT_POPUP "PopupFormat"
#define DB_STR_CURRENCYRATE_POPUP_CONDITION "ShowPopupOnlyIfValueIsChanged"

#define DB_STR_CURRENCYRATE_POPUP_COLOUR_MODE "PopupColourMode"
#define DB_STR_CURRENCYRATE_POPUP_COLOUR_BK "PopupColourBk"
#define DB_STR_CURRENCYRATE_POPUP_COLOUR_TEXT "PopupColourText"
#define DB_STR_CURRENCYRATE_POPUP_DELAY_MODE "PopupDelayMode"
#define DB_STR_CURRENCYRATE_POPUP_DELAY_TIMEOUT "PopupDelayTimeout"
#define DB_STR_CURRENCYRATE_POPUP_HISTORY_FLAG "PopupHistoryFlag"


// #define DB_STR_NICK "Nick"
#define DB_STR_STATUS "Status"

#define LIST_MODULE_NAME "CList"
#define CONTACT_LIST_NAME "MyHandle"
#define STATUS_MSG_NAME "StatusMsg"

#endif //__87d726e0_26c6_485d_8016_1fba819b037d_EconomicRateInfo__
