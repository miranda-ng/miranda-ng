/*
    KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __KEEPSTATUS_HEADER
#define __KEEPSTATUS_HEADER

#include <process.h>
#include <winsock.h>
#include <wininet.h>
#include <ipexport.h>
#include <Icmpapi.h>

#include <m_options.h>
#include <m_skin.h>
#include <m_utils.h>
#include "version.h"

#define __FILENAME                   "KeepStatus.dll"
#define MODULENAME                   "KeepStatus"
#define SETTING_CHECKCONNECTION      "CheckConnection"
#define SETTING_MAXRETRIES           "MaxRetries"
#define SETTING_INCREASEEXPONENTIAL  "IncreaseExponential"
#define SETTING_INITDELAY            "InitDelay"
#define SETTING_MAXDELAY             "MaxDelay"
#define SETTING_SHOWCONNECTIONPOPUPS "ShowConnectionPopups"
#define SETTING_CHKINET              "CheckInet"
#define SETTING_CNCOTHERLOC          "CancelIfOtherLocation"
#define SETTING_LOGINERR             "OnLoginErr"
#define SETTING_LOGINERR_DELAY       "OnLoginErrDelay"
#define SETTING_CONTCHECK            "ContinueslyCheck"
#define SETTING_BYPING               "ByPingingHost"
#define SETTING_PINGHOST             "HostToPing"
#define SETTING_CHECKAPMRESUME       "CheckAPMResume"
#define SETTING_FIRSTOFFLINE         "FirstOffline"
#define SETTING_NOLOCKED             "NoLocked"
#define SETTING_MAXCONNECTINGTIME    "MaxConnectingTime"
#define SETTING_PINGCOUNT            "PingCount"
#define DEFAULT_PINGCOUNT            1
#define SETTING_CNTDELAY             "CntDelay"
#define STATUSCHANGEDELAY            500 // ms
#define DEFAULT_MAXRETRIES           0
#define DEFAULT_INITDELAY            10 // s
#define DEFAULT_MAXDELAY             900 // s
#define AFTERCHECK_DELAY             10000 //ms (divided by 2)
#define CHECKCONTIN_DELAY            10 // s
#define SETTING_POPUP_DELAYTYPE      "PopupDelayType"
#define SETTING_POPUP_USEWINCOLORS   "PopupUseWinColors"
#define SETTING_POPUP_USEDEFCOLORS   "PopupUseDefColors"
#define SETTING_POPUP_BACKCOLOR      "PopupBackColor"
#define SETTING_POPUP_TEXTCOLOR      "PopupTextColor"
#define SETTING_POPUP_TIMEOUT        "PopupTimeout"
#define SETTING_POPUP_LEFTCLICK      "PopupLeftClickAction"
#define SETTING_POPUP_RIGHTCLICK     "PopupRightClickAction"
#define SETTING_PUOTHER              "PopupShowOther"
#define SETTING_PUCONNLOST           "PopupShowConnLost"
#define SETTING_PUCONNRETRY          "PopupShowConnRetry"
#define SETTING_PURESULT             "PopupShowResult"
#define SETTING_PUSHOWEXTRA          "PopupShowExtra"
#define POPUP_ACT_NOTHING            0
#define POPUP_ACT_CANCEL             1
#define POPUP_ACT_CLOSEPOPUP         2
#define POPUP_DELAYFROMPU            0
#define POPUP_DELAYCUSTOM            1
#define POPUP_DELAYPERMANENT         2
#define LOGINERR_NOTHING             0
#define LOGINERR_CANCEL              1
#define LOGINERR_SETDELAY            2

#define IDT_PROCESSACK               0x01
#define IDT_CHECKCONN                0x02
#define IDT_AFTERCHECK               0x04
#define IDT_CHECKCONTIN              0x08
#define IDT_CHECKCONNECTING          0x10

#define KS_ISENABLED                 WM_APP + 10
#define KS_ENABLEITEMS               WM_APP + 11

// action
#define SETTING_ENABLECHECKING       "EnableChecking"
// trigger
#define TRIGGERNAME                  "KeepStatus: Connection state change"
#define TRIGGER_CONNLOST             0x01
#define TRIGGER_LOGINERROR           0x02
#define TRIGGER_OTHERLOC             0x04
#define TRIGGER_CONNRETRY            0x08
#define TRIGGER_CONNSUCCESS          0x10
#define TRIGGER_CONNGIVEUP           0x20
#define SETTING_TRIGGERON            "TriggerOn"

struct TConnectionSettings : public PROTOCOLSETTINGEX
{
	TConnectionSettings( PROTOACCOUNT *pa );
	~TConnectionSettings();

	int lastStatusAckTime; // the time the last status ack was received
};

/* old; replaced by PROTOCOLSETTINGEX see m_statusplugins.h */
typedef struct {
	char *szName;	// pointer to protocol modulename
	WORD status;	// the status
	WORD lastStatus;// last status
} PROTOCOLSETTING;

/* old; replaced by ME_CS_STATUSCHANGE see m_statusplugins.h */
// wParam = PROTOCOLSETTING**
// lParam = 0
#define ME_CS_STATUSCHANGE				"CommonStatus/StatusChange"

/////////////////////////////////////////////////////////////////////////////////////////
// main.cpp

extern HINSTANCE hInst;
extern HANDLE hMainThread;
extern unsigned long mainThreadId;

/////////////////////////////////////////////////////////////////////////////////////////
// keepstatus.cpp

int LoadMainOptions();
LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif //__KEEPSTATUS_HEADER
