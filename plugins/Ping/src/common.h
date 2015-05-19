#ifndef _COMMON_H
#define _COMMON_H

#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <shellapi.h>
#include <commdlg.h>
#include <commctrl.h>
#include <iphlpapi.h>
#include <Icmpapi.h>
#include <list>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_skin.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_clui.h>
#include <m_cluiframes.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <win2k.h>

#include "resource.h"
#include "collection.h"
#include "Version.h"
#include "icmp.h"
#include "log.h"
#include "menu.h"
#include "pinggraph.h"
#include "pinglist.h"
#include "pingthread.h"
#include "rawping.h"
#include "utils.h"

#define MAX_HISTORY		(1440)		// 12 hrs at 30 sec intervals

#define PLUG	"PING"

#define DEFAULT_PING_PERIOD		30
#define DEFAULT_PING_TIMEOUT	2
#define DEFAULT_SHOW_POPUP		true
#define DEFAULT_SHOW_POPUP2		false
#define DEFAULT_BLOCK_REPS		true
#define DEFAULT_LOGGING_ENABLED	false
#define DEFAULT_LOG_FILENAME	_T("ping_log.txt")
#define DEFAULT_NO_TEST_ICON	true
#define DEFAULT_ATTACH_TO_CLIST	false

#define MAX_PINGADDRESS_STRING_LENGTH	256


typedef struct {
	int ping_period, ping_timeout;
	bool show_popup, show_popup2, block_reps, logging;
	TCHAR log_filename[MAX_PATH];
	bool no_test_icon;
	int row_height;
	int indent;
	int retries;
	bool attach_to_clist;
	bool log_csv;
} PingOptions;

// a deque of pairs - ping time and timestamp
struct HistPair {
	short first;
	time_t second;

	const bool operator==(const HistPair &other) const { return first == other.first && second == other.second; }
};
typedef LinkedList< HistPair > HistoryList;


#define PS_RESPONDING		1
#define PS_NOTRESPONDING	2
#define PS_TESTING			3
#define PS_DISABLED			4

struct PINGADDRESS {
	int cbSize;	//size in bytes of this structure
	DWORD item_id;
	TCHAR pszName[MAX_PINGADDRESS_STRING_LENGTH];		//IP address or domain name
	TCHAR pszLabel[MAX_PINGADDRESS_STRING_LENGTH];
	bool responding;
	int status;
	short round_trip_time;
	int miss_count;
	int port; // -1 for ICMP, non-zero for TCP
	char pszProto[MAX_PINGADDRESS_STRING_LENGTH];
	TCHAR pszCommand[MAX_PATH];
	TCHAR pszParams[MAX_PATH];
	unsigned int get_status;		// on success, if status equals this
	unsigned int set_status;	// set it to this
	int index;

	const bool operator==(const PINGADDRESS &b) const;
	const bool operator<(const PINGADDRESS &b) const;
};

#include "options.h"
typedef Map<DWORD, HistoryList> HistoryMap;
typedef std::list<PINGADDRESS> PINGLIST;
typedef std::list<PINGADDRESS>::iterator pinglist_it;


extern HANDLE hNetlibUser;
extern HINSTANCE hInst;

extern bool use_raw_ping;

// wake event for ping thread
extern HANDLE hWakeEvent;

extern PingOptions options;
extern PINGADDRESS add_edit_addr;
extern HistoryMap history_map;
extern PINGLIST list_items;
extern HANDLE reload_event_handle;
extern mir_cs list_cs;
extern HANDLE mainThread;
extern HANDLE hWakeEvent;
extern mir_cs thread_finished_cs, list_changed_cs, data_list_cs;

extern PINGLIST data_list;

int ReloadIcons(WPARAM, LPARAM);

#endif
