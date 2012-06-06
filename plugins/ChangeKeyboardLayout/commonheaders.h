#ifndef M_CKL_COMMONHEADERS_H
#define M_CKL_COMMONHEADERS_H
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#define MIRANDA_VER    0x0A00 

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_system.h>
#include <m_ieview.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <m_updater.h>


#include "resource.h"
#include "m_changekeyboardlayout.h"
#include "options.h"
#include "hook_events.h"
#include "text_operations.h"


#define VERSION PLUGIN_MAKE_VERSION(0,0,1,5)
#define MaxTextSize 64000
#define ModuleName "ChangeKeyboardLayout"


// History++ API
#define MS_HPP_EG_WINDOW			"History++/ExtGrid/NewWindow"
#define MS_HPP_EG_EVENT				"History++/ExtGrid/Event"
#define MS_HPP_EG_NAVIGATE			"History++/ExtGrid/Navigate"
#define MS_HPP_EG_OPTIONSCHANGED	"History++/ExtGrid/OptionsChanged"

// Типы окон
#define WTYPE_Unknown		0
#define WTYPE_HistoryPP		1
#define WTYPE_IEView		2
#define WTYPE_RichEdit		3
#define WTYPE_Edit			4

//Тип операции над текстом
#define TOT_Layout 0
#define TOT_Case 1

// Имена звуков
#define SND_ChangeLayout "ChangingLayout"
#define SND_ChangeCase   "ChangingCase"

//Цвета попапов
#define PPC_POPUP		0
#define PPC_WINDOWS		1
#define PPC_CUSTOM		2

//Таймаут попапов
#define PPT_POPUP			0
#define PPT_PERMANENT		1
#define PPT_CUSTOM			2


typedef struct  
{
	DWORD dwHotkey_Layout;
	DWORD dwHotkey_Layout2;
	DWORD dwHotkey_Case;
	BOOL CurrentWordLayout;
	BOOL CurrentWordLayout2;
	BOOL CurrentWordCase;
	BOOL TwoWay;
	BOOL ChangeSystemLayout;
	BOOL CopyToClipboard;
	BOOL ShowPopup;
	BYTE bCaseOperations;
} MainOptions;

typedef struct  
{
	BYTE bColourType;
	COLORREF crTextColour;
	COLORREF crBackColour;
	BYTE bTimeoutType;
	BYTE bTimeout;
	BYTE bLeftClick;
	BYTE bRightClick;
	POPUPACTION paActions[1];
} PopupOptions;

PLUGININFOEX pluginInfoEx;
PLUGININFO pluginInfo;

HINSTANCE hInst;

HANDLE hChangeLayout,hGetLayoutOfText,hChangeTextLayout;
HANDLE hIcoLibIconsChanged;

HICON hPopupIcon, hCopyIcon;

LPCTSTR ptszKeybEng;
HKL hklEng;

BYTE bLayNum;
HKL hklLayouts[20];
LPTSTR ptszLayStrings[20];

LPCTSTR ptszSeparators;

HHOOK kbHook_All;

MainOptions moOptions;
PopupOptions poOptions,poOptionsTemp;
DWORD dwMirandaVersion;

LRESULT CALLBACK Keyboard_Hook(int code, WPARAM wParam, LPARAM lParam);

#endif