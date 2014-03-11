#include "_globals.h"
#include "dlgoption.h"

#include "main.h"

/*
 * DlgOption::SubBase
 */

INT_PTR CALLBACK DlgOption::SubBase::staticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SubBase* pDlg = reinterpret_cast<SubBase*>(GetWindowLongPtr(hDlg, DWLP_USER));

	switch (msg) {
	case WM_INITDIALOG:
		pDlg = reinterpret_cast<SubBase*>(lParam);
		SetWindowLongPtr(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pDlg));
		pDlg->m_hWnd = hDlg;
		pDlg->onWMInitDialog();
		pDlg->loadSettings();
		return TRUE;

	case WM_DESTROY:
		pDlg->onWMDestroy();
		delete pDlg;
		SetWindowLongPtr(hDlg, DWLP_USER, 0);
		return TRUE;
	}

	return pDlg ? pDlg->handleMsg(msg, wParam, lParam) : FALSE;
}

DlgOption::SubBase::SubBase() :
	m_pParent(NULL), m_hWnd(NULL)
{
}

DlgOption::SubBase::~SubBase()
{
}

void DlgOption::SubBase::createWindow(DlgOption* pParent, WORD nDlgResource, const RECT& rect)
{
	m_pParent = pParent;

	CreateDialogParam(
		g_hInst,
		MAKEINTRESOURCE(nDlgResource),
		m_pParent->getHWnd(),
		staticDlgProc,
		reinterpret_cast<LPARAM>(this));

	MoveWindow(
		m_hWnd,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top, TRUE);
}

void DlgOption::SubBase::destroyWindow()
{
	if (m_hWnd) {
		DestroyWindow(m_hWnd);

		// exit NOW since we destroyed ourselves
		return;
	}
}

void DlgOption::SubBase::moveWindow(const RECT& rWnd)
{
	utils::moveWindow(m_hWnd, rWnd);
}
