// Tipper API
// note: Tipper is internally unicode and requires unicows.dll to function correctly on 95/98/ME
//		so you'll find a lot of wchar_t stuff in here

// translation function type
// use hContact, module and setting to read your db value(s) and put the resulting string into buff
// return buff if the translation was successful, or return 0 for failure
typedef TCHAR *(TranslateFunc)(HANDLE hContact, const char *module, const char *setting_or_prefix, TCHAR *buff, int bufflen);

typedef struct {
	TranslateFunc *tfunc;	// address of your translation function (see typedef above)
	const TCHAR *name;	// make sure this is unique, and DO NOT translate it
	DWORD id;					// will be overwritten by Tipper - do not use
} DBVTranslation;

// add a translation to tipper
// wParam not used
// lParam = (DBVTranslation *)translation
#define MS_TIPPER_ADDTRANSLATION	 "Tipper/AddTranslation"
