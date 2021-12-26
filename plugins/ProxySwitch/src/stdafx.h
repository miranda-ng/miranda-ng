#ifndef proxySwitch_h
#define proxySwitch_h

#include <winsock2.h>
#include <windows.h>
#include <iprtrmib.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <commctrl.h>
#include <Wininet.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <malloc.h>

#include "newpluginapi.h"
#include "m_system.h"
#include "m_options.h"
#include "m_clist.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_popup.h"
#include "m_database.h"
#include "m_netlib.h"
#include "m_utils.h"
#include "m_protocols.h"
#include "m_protosvc.h"

#include <m_proxySwitch.h>

#include "resource.h"
#include "version.h"

#define MODULENAME "ProxySwitch"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#define NVL(x) x == NULL ? "" : x
#define NVLW(x) x == NULL ? L"" : x

#define MAX_IPLIST_LENGTH 500

/**** Types ********************************************************************************/

// structure holding network interface description and information
struct NETWORK_INTERFACE : public MZeroedObject
{
	~NETWORK_INTERFACE()
	{
		mir_free(IP);
	}

	ptrA  AdapterName;
	ptrW  FriendlyName;
	ptrA  IPstr;
	LONG *IP;
	UCHAR IPcount;
	HGENMENU MenuItem;
	bool  Bound, Disabled;
};

typedef OBJLIST<NETWORK_INTERFACE> NETWORK_INTERFACE_LIST;

extern NETWORK_INTERFACE_LIST g_arNIF;
extern mir_cs csNIF_List;

// structure holding an information about local end of an active connections
struct ACTIVE_CONNECTION
{
	ACTIVE_CONNECTION(ULONG _ip, unsigned short _port) :
		IP(_ip),
		Port(_port)
	{}

	ULONG IP;
	unsigned short Port;
};

extern OBJLIST<ACTIVE_CONNECTION> g_arConnections;
extern mir_cs csConnection_List;

/**** Global variables *********************************************************************/

extern HANDLE hEventRebound;

/**** Options ******************************************************************************/

extern wchar_t opt_useProxy[MAX_IPLIST_LENGTH];
extern wchar_t opt_noProxy[MAX_IPLIST_LENGTH];
extern wchar_t opt_hideIntf[MAX_IPLIST_LENGTH];
extern UINT opt_defaultColors;
extern UINT opt_popups;
extern UINT opt_showProxyState;
extern UINT opt_miranda;
extern UINT opt_ie;
extern UINT opt_firefox;
extern UINT opt_alwayReconnect;
extern UINT opt_startup;
extern UINT opt_not_restarted;
extern COLORREF opt_bgColor;
extern COLORREF opt_txtColor;

void LoadSettings(void);
void SaveSettings(void);

/**** Service & Event handlers *************************************************************/

void PopupMyIPAddrs(const wchar_t *msg);

int OptInit(WPARAM wParam, LPARAM lParam);
int Init(WPARAM wParam, LPARAM lParam);
void UpdateInterfacesMenu(void);
void UpdatePopupMenu(BOOL State);

/**** Network ******************************************************************************/

#define NETORDER(a) ((((a) & 0xFFL)<<24) | (((a) & 0xFF00L)<<8) | (((a) & 0xFF0000L)>>8) | (((a) & 0xFF000000L)>>24))

#define INCUPD_INTACT     0
#define INCUPD_CONN_BIND  1
#define INCUPD_UPDATED    2
#define INCUPD_CONN_LOST  3
#define INCUPD(x, y) x = (x) > (y) ? (x) : (y)

#define DIGITS L"0123456789"
#define CMP_SKIP 0
#define CMP_MASK 1
#define CMP_SPAN 2
#define CMP_END  3

typedef struct {
	unsigned char cmpType;
	union { ULONG loIP; ULONG net; };
	union { ULONG hiIP; ULONG mask; };
} IP_RANGE, *PIP_RANGE;

typedef struct {
	PIP_RANGE item;
	UCHAR count;
} IP_RANGE_LIST;

void IP_WatchDog(void *arg);

int Create_NIF_List(NETWORK_INTERFACE_LIST *list);
int Create_NIF_List_Ex(NETWORK_INTERFACE_LIST *list);
int IncUpdate_NIF_List(NETWORK_INTERFACE_LIST *trg, NETWORK_INTERFACE_LIST &src);
wchar_t *Print_NIF_List(NETWORK_INTERFACE_LIST &list, const wchar_t *msg);
wchar_t *Print_NIF(NETWORK_INTERFACE *nif);

int Create_Range_List(IP_RANGE_LIST *list, wchar_t *str, BOOL prioritized);
int Match_Range_List(IP_RANGE_LIST range, NETWORK_INTERFACE_LIST &ip);
void Free_Range_List(IP_RANGE_LIST *list);

int ManageConnections(WPARAM wParam, LPARAM lParam);
void UnboundConnections(LONG *OldIP, LONG *NewIP);

/**** Proxy/Connection Modification and Query routines *************************************/

#define MAXLABELLENGTH 64

#define PROXY_NO_CONFIG -2
#define PROXY_MIXED     -1
#define PROXY_DISABLED   0
#define PROXY_ENABLED    1

typedef struct {
	char ModuleName[MAXLABELLENGTH];
	char SettingName[MAXLABELLENGTH];
} PROXY_SETTING, *PPROXY_SETTING;

typedef struct {
	PPROXY_SETTING item;
	UCHAR count;
	const char *_current_module;
	UCHAR _alloc;
} PROXY_SETTINGS, *PPROXY_SETTINGS;

typedef struct {
	char ProtoName[MAXLABELLENGTH];
	uint32_t Status;
} PROTO_SETTING, *PPROTO_SETTING;

typedef struct {
	PPROTO_SETTING item;
	UCHAR count;
} PROTO_SETTINGS, *PPROTO_SETTINGS;

void Create_Proxy_Settings_List(PPROXY_SETTINGS ps);
void Free_Proxy_Settings_List(PPROXY_SETTINGS ps);
char Get_Miranda_Proxy_Status(void);
void Set_Miranda_Proxy_Status(char proxy);
char Get_IE_Proxy_Status(void);
void Set_IE_Proxy_Status(char proxy);
char Get_Firefox_Proxy_Status(void);
void Set_Firefox_Proxy_Status(char proxy);
char Firefox_Installed(void);
void Disconnect_All_Protocols(PPROTO_SETTINGS settings, int disconnect);
void Connect_All_Protocols(PPROTO_SETTINGS settings);
#endif
