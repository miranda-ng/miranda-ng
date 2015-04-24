#include "stdafx.h"
#include "dlgconfigure.h"

#include "main.h"
#include "utils.h"
#include "dlgoption.h"
#include "resource.h"

/*
 * DlgConfigure
 */

HWND DlgConfigure::m_hCfgWnd = NULL;
bool DlgConfigure::m_bHookedEvent = false;

INT_PTR CALLBACK DlgConfigure::staticConfigureProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DlgConfigure* pDlg = reinterpret_cast<DlgConfigure*>(GetWindowLongPtr(hDlg, DWLP_USER));

	switch (msg) {
	case WM_INITDIALOG:
		pDlg = new DlgConfigure(hDlg);
		SetWindowLongPtr(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pDlg));
		pDlg->onWMInitDialog();
		return TRUE;

	case WM_DESTROY:
		delete pDlg;
		SetWindowLongPtr(hDlg, DWLP_USER, 0);
		break;

	case PSM_CHANGED:
		EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
		pDlg->m_bChanged = true;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pDlg->onCancel();
			return TRUE;

		case IDOK:
			pDlg->onApply();
			DestroyWindow(hDlg);
			return TRUE;

		case IDC_APPLY:
			pDlg->onApply();
			return TRUE;
		}
		break;

	case WM_WINDOWPOSCHANGED:
		pDlg->rearrangeControls();
		return TRUE;

	case WM_GETMINMAXINFO:
	{
		static POINT sizeMin = { 0, 0 };
		MINMAXINFO* pMMI = reinterpret_cast<MINMAXINFO*>(lParam);

		if (sizeMin.x == 0) {
			RECT rectWin;

			GetWindowRect(hDlg, &rectWin);
			sizeMin.x = rectWin.right - rectWin.left;
			sizeMin.y = rectWin.bottom - rectWin.top;
		}

		pMMI->ptMinTrackSize = sizeMin;
	}
		return TRUE;
	}

	return FALSE;
}

int DlgConfigure::staticEventPreShutdown(WPARAM, LPARAM)
{
	if (IsWindow(m_hCfgWnd)) {
		DestroyWindow(m_hCfgWnd);
	}

	return 0;
}

void DlgConfigure::showModal()
{
	if (g_bConfigureLock) {
		MessageBox(0,
			TranslateT("You can't access the stand-alone configuration dialog of HistoryStats as long as the options dialog of Miranda NG is open. Please close the options dialog and try again.\r\n\r\nNote that the options offered by both dialogs are the same."),
			TranslateT("HistoryStats - Warning"),
			MB_ICONWARNING | MB_OK);

		return;
	}

	if (IsWindow(m_hCfgWnd))
		SetForegroundWindow(m_hCfgWnd);
	else
		CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_CONFIGURE), NULL, staticConfigureProc);
}

void DlgConfigure::onWMInitDialog()
{
	TranslateDialogDefault(m_hWnd);
	utils::centerDialog(m_hWnd);
	SendMessage(m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_HISTORYSTATS))));
	EnableWindow(GetDlgItem(m_hWnd, IDC_APPLY), FALSE);

	m_hOptWnd = CreateDialogA(g_hInst, MAKEINTRESOURCEA(IDD_OPTIONS), m_hWnd, DlgOption::staticDlgProc);

	ShowWindow(m_hOptWnd, SW_SHOW);
	SetFocus(m_hOptWnd);
}

void DlgConfigure::onCancel()
{
	PSHNOTIFY pshn;
	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_RESET;
	pshn.hdr.hwndFrom = m_hOptWnd;
	pshn.lParam = 0;
	SendMessage(m_hOptWnd, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&pshn));

	rearrangeControls();

	DestroyWindow(m_hWnd);
}

void DlgConfigure::onApply()
{
	EnableWindow(GetDlgItem(m_hWnd, IDC_APPLY), FALSE);

	PSHNOTIFY pshn;
	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_KILLACTIVE;
	pshn.hdr.hwndFrom = m_hOptWnd;
	pshn.lParam = 0;
	SendMessage(m_hOptWnd, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&pshn));

	if (m_bChanged) {
		m_bChanged = false;
		pshn.hdr.code = PSN_APPLY;
		SendMessage(m_hOptWnd, WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&pshn));
	}
}

void DlgConfigure::rearrangeControls()
{
	RECT rClient;

	GetClientRect(m_hWnd, &rClient);

	if (m_nPadY == -1) {
		RECT rButton = utils::getWindowRect(m_hWnd, IDOK);
		m_nPadY = rClient.bottom - rButton.bottom;
		m_nOKPadX = rClient.right - rButton.right;
		m_nCancelPadX = rClient.right - utils::getWindowRect(m_hWnd, IDCANCEL).right;
		m_nApplyPadX = rClient.right - utils::getWindowRect(m_hWnd, IDC_APPLY).right;
	}

	RECT rButton;

	rButton = utils::getWindowRect(m_hWnd, IDOK);
	OffsetRect(&rButton, rClient.right - rButton.right - m_nOKPadX, rClient.bottom - rButton.bottom - m_nPadY);
	utils::moveWindow(m_hWnd, IDOK, rButton);

	rButton = utils::getWindowRect(m_hWnd, IDCANCEL);
	OffsetRect(&rButton, rClient.right - rButton.right - m_nCancelPadX, rClient.bottom - rButton.bottom - m_nPadY);
	utils::moveWindow(m_hWnd, IDCANCEL, rButton);

	rButton = utils::getWindowRect(m_hWnd, IDC_APPLY);
	OffsetRect(&rButton, rClient.right - rButton.right - m_nApplyPadX, rClient.bottom - rButton.bottom - m_nPadY);
	utils::moveWindow(m_hWnd, IDC_APPLY, rButton);

	RECT rPage = utils::getWindowRect(m_hWnd, m_hOptWnd);

	rPage.top = m_nPadY;
	rPage.left = m_nApplyPadX;
	rPage.right = rClient.right - rClient.left - m_nApplyPadX;
	rPage.bottom = rButton.top - m_nPadY;

	utils::moveWindow(m_hOptWnd, rPage);

	InvalidateRect(m_hWnd, NULL, TRUE);
}

DlgConfigure::DlgConfigure(HWND hWnd) :
	m_hWnd(hWnd), m_bChanged(false), m_hOptWnd(NULL),
	m_nPadY(-1), m_nOKPadX(0), m_nCancelPadX(0), m_nApplyPadX(0)
{
	if (!m_hCfgWnd) {
		m_hCfgWnd = hWnd;

		if (!m_bHookedEvent) {
			HookEvent(ME_SYSTEM_PRESHUTDOWN, staticEventPreShutdown);
			m_bHookedEvent = true;
		}
	}
}

DlgConfigure::~DlgConfigure()
{
	DestroyWindow(m_hOptWnd);

	if (m_hWnd == m_hCfgWnd)
		m_hCfgWnd = NULL;
}
