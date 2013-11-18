#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#include "common.h"
#include "session.h"
#include "userlist.h"
#include "utils.h"

#include "resource.h"

#define LSTRINGLEN		256

typedef enum {ED_MB, ED_POP, ED_BAL} ErrorDisplay;
typedef enum {CPT_USER, CPT_ANSI, CPT_UTF8, CPT_OEM, CPT_UTF7} CodePageType;

typedef struct Options_tag {
	char server_name[LSTRINGLEN];
	char id[LSTRINGLEN];
	char pword[LSTRINGLEN];
	int port;
	bool get_server_contacts;
	int client_id;
	ErrorDisplay err_method;
	bool add_contacts;
	bool encrypt_session;
	bool idle_as_away;
	bool use_old_default_client_ver;
} Options;

extern Options options;

#define DEFAULT_PORT 1533

BOOL CALLBACK DlgProcOptNet(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int OptInit(WPARAM wParam,LPARAM lParam);
void LoadOptions();
void SaveOptions();

extern HWND hWndOptions;

#define WMU_STORECOMPLETE		(WM_USER + 110)

#endif
