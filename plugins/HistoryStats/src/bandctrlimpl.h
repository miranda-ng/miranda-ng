#if !defined(HISTORYSTATS_GUARD_BANDCTRLIMPL_H)
#define HISTORYSTATS_GUARD_BANDCTRLIMPL_H

#include "stdafx.h"
#include "bandctrldefs.h"

#include <vector>

/*
 * BandCtrlImpl
 */

class BandCtrlImpl
	: public BandCtrlDefs
	, private pattern::NotCopyable<BandCtrlImpl>
{
private:
	struct ItemData {
		bool bRight;
		int nIcon;
		int nIconD;
		ext::string text;
		ext::string tooltip;
		bool bChecked;
		bool bVisible;
		bool bDropDown;
		INT_PTR dwData;
		RECT rItem;
		UINT uTTId;
		bool bEnabled;
	};

private:
	static const TCHAR* m_ClassName;
	static const int m_PollId;
	static const int m_PollDelay;

private:
	static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	static bool registerClass();
	static void unregisterClass();

private:
	HWND m_hWnd;
	int m_nOwnId;
	HFONT m_hFont;
	HTHEME m_hTheme;
	std::vector<ItemData> m_Items;
	HIMAGELIST m_hImageList;
	HIMAGELIST m_hImageListD;
	SIZE m_IconSize;
	HWND m_hTooltip;
	int m_nCurHot;
	int m_nCurFocused;
	int m_nCurPressed;
	bool m_bCurPressedDD;
	int m_nLayout;
	int m_nDDWidth;
	HICON m_hDDIcon;

private:
	explicit BandCtrlImpl(HWND hWnd, int nOwnId);
	~BandCtrlImpl();

private:
	void onWMPaint();
	void drawButton(HDC hDC, int nItem, int textHeight, bool bBandEnabled);
	void reloadTheme();
	HICON convertToGray(HICON hIcon);
	int onBCMAddButton(BCBUTTON* pButton);
	void onBCMCheckButton(int nItem, bool bCheck);
	void onBCMShowButton(int nItem, bool bShow);
	void onBCMGetButtonRect(int nItem, RECT* pRect);
	void onBCMEnableButton(int nItem, bool bEnable);
	void recalcButtonRects();
	int getNextButton(int nItem);
	int getPrevButton(int nItem);
	void fireEvent(UINT code, int nItem);
	void onWMSetFocus();
	void onWMKeyDown(int nVirtKey);
	void onWMKeyUp(int nVirtKey);
	void onWMMouseLeave();
	void onWMMouseMove(POINTS pts);
	void onWMLButtonDown(POINTS pts);
	void onWMLButtonUp(POINTS pts);
};

#endif // HISTORYSTATS_GUARD_BANDCTRLIMPL_H