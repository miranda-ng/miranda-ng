// Tipper API
// note: Tipper is internally unicode and requires unicows.dll to function correctly on 95/98/ME
//		so you'll find a lot of wchar_t stuff in here

// use hContact, module and setting to read your db value(s) and put the resulting string into buff
// return buff if the translation was successful, or return 0 for failure
typedef wchar_t *(TranslateFunc)(HANDLE hContact, const char *module, const char *setting_or_prefix, wchar_t *buff, int bufflen);

typedef struct {
	TranslateFunc *tfunc;	// address of your translation function (see typedef above)
	const wchar_t *name;	// make sure this is unique, and DO NOT translate it
	DWORD id;					// will be overwritten by Tipper - do not use
} DBVTranslation;

// add a translation to tipper
// wParam not used
// lParam = (DBVTranslation *)translation
#define MS_TIPPER_ADDTRANSLATION	 "Tipper/AddTranslation"

// unicode extension to the basic functionality
// wParam - optional (wchar_t *)text for text-only tips
// lParam - (CLCINFOTIP *)infoTip
#define MS_TIPPER_SHOWTIPW				"mToolTip/ShowTipW"
