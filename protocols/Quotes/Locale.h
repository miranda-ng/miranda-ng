#ifndef __11f7afd0_5a66_4029_8bf3_e3c66346b349_Locale_h_
#define __11f7afd0_5a66_4029_8bf3_e3c66346b349_Locale_h_

// std::string GetLocaleInfoString(LCTYPE LCType,LCID Locale = LOCALE_USER_DEFAULT);
const std::locale GetSystemLocale();
tstring Quotes_GetDateFormat(bool bShort);
tstring Quotes_GetTimeFormat(bool bShort);

#endif //__11f7afd0_5a66_4029_8bf3_e3c66346b349_Locale_h_
