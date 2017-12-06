#if !defined(HISTORYSTATS_GUARD_DLGCONFIGURE_H)
#define HISTORYSTATS_GUARD_DLGCONFIGURE_H

#include "stdafx.h"
#include "_consts.h"

class DlgConfigure
	: private pattern::NotCopyable<DlgConfigure>
{
private:
	static HWND m_hCfgWnd;
	static bool m_bHookedEvent;

private:
	static INT_PTR CALLBACK staticConfigureProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static int staticEventPreShutdown(WPARAM wParam, LPARAM lParam);

public:
	static void showModal();

private:
	HWND m_hWnd;
	bool m_bChanged;
	HWND m_hOptWnd;
	int m_nPadY;
	int m_nOKPadX;
	int m_nCancelPadX;
	int m_nApplyPadX;

private:
	void onWMInitDialog();
	void onCancel();
	void onApply();
	void rearrangeControls();

private:
	explicit DlgConfigure(HWND hWnd);
	~DlgConfigure();
};

#endif // HISTORYSTATS_GUARD_DLG_CONFIGURE_H