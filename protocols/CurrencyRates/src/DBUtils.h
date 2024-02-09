#ifndef __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
#define __54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__

void FixInvalidChars(CMStringW &s);

CMStringW GetNodeText(const TiXmlElement*);

bool CurrencyRates_DBWriteDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double dValue);
bool CurrencyRates_DBReadDouble(MCONTACT hContact, const char *szModule, const char *szSetting, double& rdValue);

#endif //__54294385_3fdd_4f0c_98c3_c583a96e7fb4_DBUtils_h__
