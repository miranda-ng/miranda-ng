#ifndef __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
#define __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__

#define DB_KEY_RefreshRateType     "CC_RefreshRateType"
#define DB_KEY_RefreshRateValue    "CC_RefreshRateValue"

#define DB_KEY_StatusMsgFormat     "CC_StatusMessageFormat"
#define DB_DEF_StatusMsgFormat     L""

#define DB_KEY_ApiKey              "CC_ApiKey"

#define DB_KEY_DisplayNameFormat   "CC_DspNameFrmt"
#define DB_DEF_DisplayNameFormat   L"1 %f = %r %i"

#define DB_KEY_TendencyFormat      "CC_TendencyFormat"
#define DB_DEF_TendencyFormat      L"%r>%p"

void FixInvalidChars(CMStringW &s);

CMStringW GetNodeText(const TiXmlElement*);

bool CurrencyRates_DBWriteDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double dValue);
bool CurrencyRates_DBReadDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double& rdValue);

#endif //__54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
