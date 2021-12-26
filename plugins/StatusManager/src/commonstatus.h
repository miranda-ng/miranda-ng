/*
	AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
	StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
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

#ifndef COMMONSTATUSHEADER
#define COMMONSTATUSHEADER

#define MODULENAME "StatusManager"

#if defined( _WIN64 )
#define __PLATFORM_NAME  "64"
#else
#define __PLATFORM_NAME  ""
#endif

#include "../helpers/gen_helpers.h"

#define UM_STSMSGDLGCLOSED				WM_APP+1
#define UM_CLOSECONFIRMDLG				WM_APP+2

#define PREFIX_LAST						"last_"
#define PREFIX_LASTMSG					"lastmsg_"
#define DEFAULT_STATUS					ID_STATUS_LAST
#define ID_STATUS_LAST					40081 // doesn't interfere with ID_STATUS_IDLE, since we don't use it. However this *is* a good lesson not to do this again.
#define ID_STATUS_CURRENT				40082
#define ID_STATUS_DISABLED				41083 // this should not be send to setstatus(ex)
#define MAX_STATUS						ID_STATUS_CURRENT
#define MIN_STATUS						ID_STATUS_OFFLINE
#define DEF_CLOSE_TIME					5 //secs

wchar_t *GetDefaultStatusMessage(PROTOCOLSETTINGEX *ps, int status);
int GetActualStatus(PROTOCOLSETTINGEX *protoSetting);
int InitCommonStatus();
bool IsSuitableProto(PROTOACCOUNT *pa);

/////////////////////////////////////////////////////////////////////////////////////////
// external data

#define STATUS_RESET    1
#define STATUS_AUTOAWAY 2
#define STATUS_AUTONA   3

#define FLAG_ONSAVER        0x0001 // db: set lv1 status on screensaver ?
#define FLAG_ONMOUSE        0x0002 // db: set after inactivity ?
#define FLAG_SETNA          0x0004 // db: set NA after xx of away time ?
#define FLAG_CONFIRM        0x0008 // db: show confirm dialog ?
#define FLAG_RESET          0x0010 // db: restore status ?
#define FLAG_LV2ONINACTIVE  0x0020 // db: set lv2 only on inactivity
#define FLAG_MONITORMIRANDA 0x0040 // db: monitor miranda activity only
#define FLAG_ONLOCK         0x0080 // db: on work station lock
#define FLAG_FULLSCREEN     0x0100 // db: on full screen
#define FLAG_ONTS           0x0200 // db: on terminal session disconnect
#define FLAG_ENTERIDLE      0x0400 // db: enter idle with AA mode

#define PF2_OFFLINE			 0x0200

struct StatusModeMap
{
	int iStatus, iFlag;
};

extern StatusModeMap statusModes[MAX_STATUS_COUNT];

enum STATES
{
	ACTIVE, // user is active
	STATUS1_SET, // first status change happened
	STATUS2_SET, // second status change happened
	SET_ORGSTATUS, // user was active again, original status will be restored
	HIDDEN_ACTIVE // user is active, but this is not shown to the outside world
};

struct SMProto : public PROTOCOLSETTINGEX, public MZeroedObject
{
	SMProto(PROTOACCOUNT *pa);
	SMProto(const SMProto&);
	~SMProto();

	// AdvancedAutoAway settings
	int originalStatusMode = ID_STATUS_CURRENT;
	int aaaStatus;
	STATES oldState, curState = ACTIVE;
	bool bStatusChanged; // AAA changed the status, don't update bManualStatus
	bool bManualStatus; // status changed manually or not ?
	int  optionFlags; // db: see above
	int  awayTime; // db: time to wait for inactivity
	int  naTime; // db: time to wait after away is set
	int  statusFlags; // db: set lv1 status if this is original status
	uint16_t lv1Status, lv2Status; // db: statuses to switch protocol to
	unsigned int sts1setTimer;

	// KeepStatus
	int AssignStatus(int status, int lastStatus = 0, wchar_t *szMsg = nullptr);
	int GetStatus() const;

	int lastStatusAckTime; // the time the last status ack was received

	// StartupStatus
	bool ssDisabled;  // prohibits status restoration at startup
};

struct TProtoSettings : public OBJLIST<SMProto>
{
	TProtoSettings();
	TProtoSettings(const TProtoSettings&);
};

extern TProtoSettings protoList;

int CompareProtoSettings(const SMProto *p1, const SMProto *p2);
HWND ShowConfirmDialogEx(TProtoSettings *params, int _timeout);
void ShutdownConfirmDialog(void);

int SetStatusEx(TProtoSettings &ps);

#endif //COMMONSTATUSHEADER
