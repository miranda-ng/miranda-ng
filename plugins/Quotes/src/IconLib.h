#ifndef __8821d334_afac_439e_9a81_76318e1ac4ef_IconLib_h__
#define __8821d334_afac_439e_9a81_76318e1ac4ef_IconLib_h__

#define ICON_STR_MAIN  "main"
#define ICON_STR_QUOTE "quote"

void Quotes_IconsInit();
HICON Quotes_LoadIconEx(int iconId, bool bBig = false);
HANDLE Quotes_GetIconHandle(int iconId);

#endif //__8821d334_afac_439e_9a81_76318e1ac4ef_IconLib_h__
