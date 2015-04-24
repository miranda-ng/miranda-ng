#if !defined(HISTORYSTATS_GUARD_ICONLIB_H)
#define HISTORYSTATS_GUARD_ICONLIB_H

#include "stdafx.h"


#include <set>

class IconLib
	: private pattern::NotInstantiable<IconLib>
{
public:
	enum IconIndex {
		iiMenuCreateStatistics =  0,
		iiMenuShowStatistics   =  1,
		iiMenuConfigure        =  2,
		iiContactMenu          =  3,
		iiExcludeNo            =  4,
		iiExcludeYes           =  5,
		iiTreeCheck1           =  6,
		iiTreeCheck2           =  7,
		iiTreeCheck3           =  8,
		iiTreeCheck4           =  9,
		iiTreeRadio1           = 10,
		iiTreeRadio2           = 11,
		iiTreeRadio3           = 12,
		iiTreeRadio4           = 13,
		iiTreeEdit1            = 14,
		iiTreeEdit2            = 15,
		iiTreeCombo1           = 16,
		iiTreeCombo2           = 17,
		iiTreeFolder1          = 18,
		iiTreeFolder2          = 19,
		iiTreeButton1          = 20,
		iiTreeButton2          = 21,
		iiTreeDateTime1        = 22,
		iiTreeDateTime2        = 23,
	};

	typedef void (*CallbackProc)(LPARAM lParam);

private:
	struct IconInfo
	{
		WORD wID;
		char* szIconName;
		TCHAR* szSection;
		TCHAR* szDescription;
		HICON hIcon;
	};

	typedef std::pair<CallbackProc, LPARAM> CallbackPair;
	typedef std::set<CallbackPair> CallbackSet;

private:
	static bool m_bIcoLibAvailable;
	static HANDLE m_hHookSkin2IconsChanged;
	static IconInfo m_IconInfo[];
	static ext::string m_Section;
	static ext::a::string m_IconName;
	static CallbackSet m_Callbacks;

private:
	static int handleCallbacks(WPARAM wParam, LPARAM lParam);

public:
	static void init();
	static void uninit();
	static void registerCallback(CallbackProc callback, LPARAM lParam);
	static void unregisterCallback(CallbackProc callback, LPARAM lParam);
	static HICON getIcon(IconIndex index);
};

#endif // HISTORYSTATS_GUARD_ICONLIB_H