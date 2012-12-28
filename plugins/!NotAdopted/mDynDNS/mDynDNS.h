
// Windows includes
#include <windows.h>
//#include <winsock2.h>

// Standard includes
#include <stdio.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>

// Miranda IM SDK includes
#include <win2k.h>
#include <newpluginapi.h> // This must be included first
#include <m_popup.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_netlib.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_database.h>

// plugins SDK
#include <m_updater.h>

// Project resources
#include "resource.h"
#include "version.h"

#define POPUP_DEFAULT_COLORBKG 0xDCBDA5
#define POPUP_DEFAULT_COLORTXT 0x000000

#define RESPONSE_PATTERN	   "<html><head><title>Current IP Check</title></head><body>Current IP Address: %d.%d.%d.%d</body></html>"
#define SETTINGS_MODULE		   "mDynDNS.dll"
#define DESCRIPTIVE_NAME	   "DynDNS Plugin"
#define MDYNDNS_ERROR_TITLE    "mDynDns - Error"
#define MDYNDNS_ERROR_CODE     "F"
#define MDYNDNS_BLOCK_CODE	   "Yes"
#define MDYNDNS_UNBLOCK_CODE   "No"
#define DYN_DNS_SERVER		   "http://checkip.dyndns.org"
#define URL_DYNDNS             "https://www.dyndns.com/account/create.html"
#define MDYNDNS_KEY_USERNAME   "Username"
#define MDYNDNS_KEY_PASSWORD   "Password"
#define MDYNDNS_KEY_DOMAIN	   "Domain"
#define MDYNDNS_KEY_POPUPENAB  "PopupEnabled"
#define MDYNDNS_KEY_POPUPUPD   "UpdatePopupsEnabled"
#define MDYNDNS_KEY_POPUPBACK  "PopupBack"
#define MDYNDNS_KEY_POPUPTEXT  "PopupText"
#define MDYNDNS_KEY_POPUPDELAY "PopupDelay"
#define MDYNDNS_KEY_POPUPDELM  "PopupDelayMode"
#define MDYNDNS_KEY_BLOCKED	   "Blocked"
#define MDYNDNS_KEY_UPDATETIME "UpdateTime"

#ifndef MIID_MDYNDNS
#define MIID_MDYNDNS	{ 0x91e005c7, 0x8760, 0x479f, { 0xa2, 0xc9, 0x1c, 0x9e, 0x31, 0x7, 0x20, 0x1c } } // {91E005C7-8760-479f-A2C9-1C9E3107201C}
#endif
