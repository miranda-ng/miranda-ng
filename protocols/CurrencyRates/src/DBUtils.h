#ifndef __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
#define __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__

#define DB_KEY_RefreshRateType     "CC_RefreshRateType"
#define DB_KEY_RefreshRateValue    "CC_RefreshRateValue"

#define DB_KEY_StatusMsgFormat     "CC_StatusMessageFormat"
#define DB_DEF_StatusMsgFormat     L""

#define DB_DEF_IniFileName         L"CC.xml"

#define DB_KEY_ApiKey              "CC_ApiKey"

#define DB_KEY_DisplayNameFormat   "CC_DspNameFrmt"
#define DB_DEF_DisplayNameFormat   L"1 %f = %r %i"

#define DB_KEY_HistoryFormat       "CC_HistoryFormat"
#define DB_DEF_HistoryFormat       L"%s %r"

#define DB_KEY_HistoryCondition    "CC_AddToHistoryOnlyIfValueIsChanged"

#define DB_KEY_LogMode             "CC_LogMode"
#define DB_KEY_LogFile             "CC_LogFile"
#define DB_KEY_LogCondition        "CC_AddToLogOnlyIfValueIsChanged"

#define DB_KEY_LogFormat           "CC_LogFileFormat"
#define DB_DEF_LogFormat           L"%s\\t%t\\t%r\\n"

#define DB_KEY_PopupFormat         "CC_PopupFormat"
#define DB_DEF_PopupFormat         L"\\nCurrent = %r\\nPrevious = %p"

#define DB_KEY_PopupCondition      "CC_ShowPopupOnlyIfValueChanged"

#define DB_KEY_PopupColourMode     "CC_PopupColourMode"
#define DB_KEY_PopupBkColour       "CC_PopupColourBk"
#define DB_KEY_PopupTextColour     "CC_PopupColourText"
#define DB_KEY_PopupDelayMode      "CC_PopupDelayMode"
#define DB_KEY_PopupDelayTimeout   "CC_PopupDelayTimeout"
#define DB_KEY_PopupHistoryFlag    "CC_PopupHistoryFlag"

#define DB_KEY_TendencyFormat      "CC_TendencyFormat"
#define DB_DEF_TendencyFormat      L"%r>%p"

void FixInvalidChars(CMStringW &s);

CMStringW GetNodeText(const TiXmlElement*);

bool CurrencyRates_DBWriteDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double dValue);
bool CurrencyRates_DBReadDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double& rdValue);

#endif //__54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
