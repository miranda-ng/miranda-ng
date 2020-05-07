#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#define PRIVATE_KEY_FILENAME "otr.private_key"
#define FINGERPRINT_STORE_FILENAME "otr.fingerprints"
#define INSTAG_FILENAME "otr.instag"

extern wchar_t g_fingerprint_store_filename[MAX_PATH];
extern wchar_t g_private_key_filename[MAX_PATH];
extern wchar_t g_instag_filename[MAX_PATH];

#define CONTACT_DEFAULT_POLICY 	0xFFFF

typedef enum {ED_POP, ED_BAL, ED_MB} ErrorDisplay;

#define OPTIONS_PREFIXLEN 64
#define OPTIONS_DEFAULT_PREFIX "OTR: "
//#define OPTIONS_DEFAULT_PREFIX "?OTR? "

typedef struct {
	OtrlPolicy default_policy;
	ErrorDisplay err_method;
	bool prefix_messages;
	bool msg_inline;
	bool msg_popup;
	char prefix[OPTIONS_PREFIXLEN];

	bool delete_history;
	bool delete_systeminfo;
	bool autoshow_verify;
	
	bool end_offline, end_window_close;

	// temporary options
	bool bHaveSecureIM;

} Options;

extern Options options;

#define CONTACT_DEFAULT_POLICY 	0xFFFF

#include <map>
enum HTMLConvSetting {
	HTMLCONV_DISABLE,
	HTMLCONV_ENABLE
};
struct CONTACT_DATA {
	OtrlPolicy policy;
	int htmlconv;
};

typedef enum {FPM_VERIFY, FPM_NOTRUST, FPM_DELETE } FPModify;

int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();
void SaveOptions();
void InitOptions();

void ReadPrivkeyFiles();

#endif
