#pragma once


// Windows headers
#include <windows.h>
#include <commctrl.h>
#include <assert.h>

// Miranda headers
//LotusNotify.h

#include <m_core.h>
#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_protosvc.h>
#include <m_system.h>
#include <m_netlib.h>

// Notesapi headers
#define W32
#if defined(_WIN64)
#define _AMD64_
#endif
#include "cnotesapi/include/global.h"
#include "cnotesapi/include/osmisc.h"
#include "cnotesapi/include/nsfdb.h"
#include "cnotesapi/include/nsfsearc.h"
#include "cnotesapi/include/names.h"
#include "cnotesapi/include/osenv.h"
#include "cnotesapi/include/extmgr.h"
#include "cnotesapi/include/bsafeerr.h"
#include "cnotesapi/include/nsferr.h"

BOOL checkNotesIniFile(BOOL bInfo);
void decodeServer(char *tmp);
void deleteElements();
void LoadSettings();
void ErMsgByLotusCode(STATUS erno);

#define MAX_SETTING_STR 512
#define MAX_FIELD 256
#define STATUS_COUNT 9

#define MODULENAME "LotusNotify"

extern char settingServer[MAX_SETTING_STR], settingServerSec[MAX_SETTING_STR], settingDatabase[MAX_SETTING_STR];
extern char settingCommand[MAX_SETTING_STR], settingParameters[MAX_SETTING_STR], settingPassword[MAX_SETTING_STR];
extern wchar_t settingFilterSubject[MAX_SETTING_STR], settingFilterSender[MAX_SETTING_STR], settingFilterTo[MAX_SETTING_STR];

extern COLORREF settingBgColor, settingFgColor;
extern int settingInterval, settingInterval1;
extern uint32_t settingNewestID;
extern uint8_t settingSetColours, settingShowError, settingIniAnswer, settingIniCheck;
extern uint8_t settingOnceOnly, settingNonClickedOnly, settingNewest, settingEvenNonClicked, settingKeepConnection;
extern BOOL settingStatus[STATUS_COUNT];
