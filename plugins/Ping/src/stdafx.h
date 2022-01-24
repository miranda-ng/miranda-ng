#pragma once

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
#include <m_clistint.h>
#include <m_cluiframes.h>
#include <m_fontservice.h>
#include <m_icolib.h>

#include "resource.h"
#include "collection.h"
#include "version.h"
#include "icmp.h"
#include "log.h"
#include "menu.h"
#include "pinggraph.h"
#include "pinglist.h"
#include "pingthread.h"
#include "rawping.h"
#include "utils.h"

#define MAX_HISTORY		(1440)		// 12 hrs at 30 sec intervals

#define MODULENAME	"PING"

#define DEFAULT_PING_PERIOD		30
#define DEFAULT_PING_TIMEOUT	2
#define DEFAULT_SHOW_POPUP		true
#define DEFAULT_SHOW_POPUP2		false
#define DEFAULT_BLOCK_REPS		true
#define DEFAULT_LOGGING_ENABLED	false
#define DEFAULT_LOG_FILENAME	L"ping_log.txt"
#define DEFAULT_NO_TEST_ICON	true
#define DEFAULT_ATTACH_TO_CLIST	false

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

typedef struct {
	int ping_period, ping_timeout;
	bool show_popup, show_popup2, block_reps, logging;
	wchar_t log_filename[MAX_PATH];
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

#include "options.h"

typedef Map<uint32_t, HistoryList> HistoryMap;

extern HNETLIBUSER hNetlibUser;

extern bool use_raw_ping;

// wake event for ping thread
extern HANDLE hWakeEvent;

extern PingOptions options;
extern PINGADDRESS add_edit_addr;
extern HistoryMap history_map;
extern PINGLIST list_items;
extern HANDLE reload_event_handle;
extern mir_cs list_cs;
extern HANDLE hWakeEvent;
extern mir_cs thread_finished_cs, list_changed_cs, data_list_cs;

extern PINGLIST data_list;

int ReloadIcons(WPARAM, LPARAM);
