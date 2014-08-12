#ifndef M_CKL_COMMONHEADERS_H
#define M_CKL_COMMONHEADERS_H
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_system.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <win2k.h>

#include <m_ieview.h>
#include <m_changekeyboardlayout.h>

#include "resource.h"
#include "options.h"
#include "hook_events.h"
#include "text_operations.h"
#include "version.h"

#define MaxTextSize 64000
#define ModuleName LPGEN("ChangeKeyboardLayout")

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

extern HINSTANCE hInst;

extern HANDLE hChangeLayout, hGetLayoutOfText, hChangeTextLayout;

extern HICON hPopupIcon, hCopyIcon;

extern LPCTSTR ptszKeybEng;
extern HKL hklEng;

extern BYTE bLayNum;
extern HKL hklLayouts[20];
extern LPTSTR ptszLayStrings[20];

extern LPCTSTR ptszSeparators;

extern HHOOK kbHook_All;

extern MainOptions moOptions;
extern PopupOptions poOptions, poOptionsTemp;

LRESULT CALLBACK Keyboard_Hook(int code, WPARAM wParam, LPARAM lParam);
int OnIconsChanged(WPARAM wParam, LPARAM lParam);

#endif