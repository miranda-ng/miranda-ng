#ifndef __8821d334_afac_439e_9a81_76318e1ac4ef_IconLib_h__
#define __8821d334_afac_439e_9a81_76318e1ac4ef_IconLib_h__

#define ICON_STR_MAIN "main"
#define ICON_STR_AUTO_UPDATE_DISABLED "auto_update_disabled"
#define ICON_STR_QUOTE_UP "quote_up"
#define ICON_STR_QUOTE_DOWN "quote_down"
#define ICON_STR_QUOTE_NOT_CHANGED "quote_not_changed"
#define ICON_STR_SECTION "quote_section"
#define ICON_STR_QUOTE "quote"
#define ICON_STR_CURRENCY_CONVERTER "currency_converter"
#define ICON_STR_REFRESH "refresh"
#define ICON_STR_IMPORT "import"
#define ICON_STR_EXPORT "export"
#define ICON_STR_SWAP "swap"

void Quotes_IconsInit();
HICON Quotes_LoadIconEx(const char* name, bool bBig = false);
HANDLE Quotes_GetIconHandle(int iconId);
std::string Quotes_MakeIconName(const char* name);

#endif //__8821d334_afac_439e_9a81_76318e1ac4ef_IconLib_h__
