#ifndef M_CKL_COMMONHEADERS_H
#define M_CKL_COMMONHEADERS_H

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

#include <m_ieview.h>
#include <m_changekeyboardlayout.h>

#include "resource.h"
#include "options.h"
#include "hook_events.h"
#include "text_operations.h"
#include "version.h"

#define MaxTextSize 64000
#define MODULENAME  LPGEN("ChangeKeyboardLayout")
#define ModuleNameW LPGENW("ChangeKeyboardLayout")

struct CMPlugin : public PLUGIN<CMPlugin>
{
	HKL hklEng = (HKL)0x04090409;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

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
	uint32_t dwHotkey_Layout;
	uint32_t dwHotkey_Layout2;
	uint32_t dwHotkey_Case;
	BOOL CurrentWordLayout;
	BOOL CurrentWordLayout2;
	BOOL CurrentWordCase;
	BOOL TwoWay;
	BOOL ChangeSystemLayout;
	BOOL CopyToClipboard;
	BOOL ShowPopup;
	uint8_t bCaseOperations;
} MainOptions;

typedef struct  
{
	uint8_t bColourType;
	COLORREF crTextColour;
	COLORREF crBackColour;
	uint8_t bTimeoutType;
	uint8_t bTimeout;
	uint8_t bLeftClick;
	uint8_t bRightClick;
	POPUPACTION paActions[1];
} PopupOptions;

extern HICON hPopupIcon, hCopyIcon;

extern int bLayNum;
extern HKL hklLayouts[20];
extern wchar_t *ptszLayStrings[20];

extern LPCTSTR ptszSeparators;

extern HHOOK kbHook_All;

extern MainOptions moOptions;
extern PopupOptions poOptions;

LRESULT CALLBACK Keyboard_Hook(int code, WPARAM wParam, LPARAM lParam);
int OnIconsChanged(WPARAM wParam, LPARAM lParam);

#endif