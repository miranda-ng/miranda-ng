#ifndef _SAMETIME_H
#define _SAMETIME_H


//sametime defines

#define FILE_BUFF_SIZE					(1024 * 32)

#define MS_SAMETIME_MENULEAVECHAT		"/LeaveChat"
#define MS_SAMETIME_MENUCREATECHAT		"/CreateChat"

#define MAX_MESSAGE_SIZE				(10 * 1024)				// verified limit in official client, thx Periferral

#define LSTRINGLEN						256

#define DEFAULT_PORT					1533

#define WMU_STORECOMPLETE				(WM_USER + 110)



typedef enum {SAMETIME_POPUP_ERROR = 1, SAMETIME_POPUP_INFO = 2 } SametimePopupEnum;
typedef enum {ED_MB = 1, ED_POP = 2, ED_BAL = 3} ErrorDisplay;
typedef enum {CPT_USER, CPT_ANSI, CPT_UTF8, CPT_OEM, CPT_UTF7} CodePageType;



//stl typedef's
typedef std::queue<std::string> InviteQueue;					///for conference.cpp
typedef std::queue<std::string> MessageQueue;					///for messaging.cpp
typedef std::map<MCONTACT, MessageQueue> ContactMessageQueue;	///for messaging.cpp


//protocol includes
#include "resource.h"


//methods
INT_PTR CALLBACK SessionAnnounceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CALLBACK SearchDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void mwResolve_handler_callback(mwServiceResolve* srvc, guint32 id, guint32 code, GList* results, gpointer data);

void SametimeInitIcons(void);
HANDLE GetIconHandle(int iconId);
HICON LoadIconEx(const char* name, BOOL big);
void ReleaseIconEx(const char* name, BOOL big);


// services (async thread functions)
void __cdecl sttFakeAckInfoSuccessThread(void* param);
void __cdecl sttFakeAckMessageFailedThread(void* param);
void __cdecl sttFakeAckMessageSuccessThread(void* param);
void __cdecl sttRecvAwayThread(void* param);


//sametime structs

typedef struct Options_tag {
	char server_name[LSTRINGLEN]; // utf8
	char id[LSTRINGLEN];          // utf8
	char pword[LSTRINGLEN];       // utf8
	int port;
	bool get_server_contacts;
	int client_id;
	ErrorDisplay err_method;
	bool add_contacts;
	bool encrypt_session;
	bool idle_as_away;
	bool use_old_default_client_ver;
} SametimeOptions;

typedef struct {
	int cbSize;
	char* nick;
	char* firstName;
	char* lastName;
	char* email;
	char reserved[16];
	char name[256];
	char stid[256];
	bool group;
} MYPROTOSEARCHRESULT;

typedef struct {
	size_t nSize;
	int nFieldCount;
	TCHAR** pszFields;
	MYPROTOSEARCHRESULT psr;
} MYCUSTOMSEARCHRESULTS;

typedef struct FileTransferClientData_tag {
	char* save_path;
	HANDLE hFile;
	bool sending;
	MCONTACT hContact;
	struct FileTransferClientData_tag* first;
	struct FileTransferClientData_tag* next;
	HANDLE hFt;
	char* buffer;
	int ft_number;
	int ft_count;	// number of nodes in list - only valid in first node
	int totalSize;	// total for all files in the list - only valid in first node
	int sizeToHere;	// in a link list of file transfers, the sum of the filesizes of all prior nodes in the list
	mwFileTransfer* ft;
} FileTransferClientData;



// Global variables
struct CSametimeProto;

extern HINSTANCE hInst;
extern PLUGININFOEX pluginInfo;
extern LIST<CSametimeProto> g_Instances;


#include "sametime_proto.h"


#endif //#ifndef _SAMETIME_H

