#ifndef _COMMON_H
#define _COMMON_H

#define MAX_HISTORY		(1440)		// 12 hrs at 30 sec intervals

#define PLUG	"PING"

#define DEFAULT_PING_PERIOD		30
#define DEFAULT_PING_TIMEOUT	2
#define DEFAULT_SHOW_POPUP		true
#define DEFAULT_SHOW_POPUP2		false
#define DEFAULT_BLOCK_REPS		true
#define DEFAULT_LOGGING_ENABLED	false
#define DEFAULT_LOG_FILENAME	"ping_log.txt"
#define DEFAULT_NO_TEST_ICON	true
#define DEFAULT_ATTACH_TO_CLIST	false

#define MAX_PINGADDRESS_STRING_LENGTH	256

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#define VC_EXTRALEAN
//#define _WIN32_WINNT 0x0500

#define WINVER	0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0300

#define MIRANDA_VER		0x0600
#include <windows.h>

//#include <prsht.h>
#include <shellapi.h>
#include <commdlg.h>
#include <commctrl.h>
#include <time.h>

#include <stdio.h>

#include <newpluginapi.h>
#include <statusmodes.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_system.h>
#include <m_skin.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_ignore.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_genmenu.h>
#include <m_cluiframes.h>
#include <m_utils.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <win2k.h>

#include "collection.h"

typedef struct {
	int ping_period, ping_timeout;
	bool show_popup, show_popup2, block_reps, logging;
	char log_filename[MAX_PATH];
	bool no_test_icon;
	int row_height;
	int indent;
	int retries;
	bool attach_to_clist;
	bool log_csv;
} PingOptions;

#pragma warning( disable : 4786 )

// a deque of pairs - ping time and timestamp
struct HistPair {
	short first;
	time_t second;

	const bool operator==(const HistPair &other) const {return first == other.first && second == other.second;}
};
typedef LinkedList< HistPair > HistoryList;


#define PS_RESPONDING		1
#define PS_NOTRESPONDING	2
#define PS_TESTING			3
#define PS_DISABLED			4

struct PINGADDRESS {
	int cbSize;	//size in bytes of this structure
	DWORD item_id;
	char pszName[MAX_PINGADDRESS_STRING_LENGTH];		//IP address or domain name
	char pszLabel[MAX_PINGADDRESS_STRING_LENGTH];
	bool responding;
	int status;
	short round_trip_time;
	int miss_count;
	int port; // -1 for ICMP, non-zero for TCP
	char pszProto[MAX_PINGADDRESS_STRING_LENGTH];
	char pszCommand[MAX_PATH];
	char pszParams[MAX_PATH];
	unsigned int get_status;		// on success, if status equals this
	unsigned int set_status;	// set it to this
	int index;

	const bool operator==(const PINGADDRESS &b) const;
	const bool operator<(const PINGADDRESS &b) const;
};

typedef Map<DWORD, HistoryList> HistoryMap;


extern HANDLE hNetlibUser;
extern HINSTANCE hInst;

extern bool use_raw_ping;

#ifndef MIID_PING
#define MIID_PING	{0x9cd1684e, 0xc520, 0x4b58, { 0x9a, 0x52, 0xae, 0x3d, 0x7a, 0x72, 0x4, 0x46}}
#endif

#endif
