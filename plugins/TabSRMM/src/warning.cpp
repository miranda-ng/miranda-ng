/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

using namespace CWarning;

static MWindowList hWindowList;

class CWarningImpl
{
	ptrW     m_szTitle, m_szText;
	UINT     m_uId;
	HFONT    m_hFontCaption = nullptr;
	uint32_t m_dwFlags;
	HWND     m_hwnd = nullptr;
	bool     m_fIsModal;

public:
	CWarningImpl(const wchar_t *tszTitle, const wchar_t *tszText, const UINT uId, const uint32_t dwFlags) :
		m_szTitle(mir_wstrdup(tszTitle)),
		m_szText(mir_wstrdup(tszText))
	{
		m_uId = uId;
		m_dwFlags = dwFlags;
		m_fIsModal = ((m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) ? true : false);
	}

	~CWarningImpl()
	{
		if (m_hFontCaption)
			::DeleteObject(m_hFontCaption);
	}

	// static function to construct and show the dialog, returns the user's choice
	LRESULT ShowDialog() const
	{
		if (!m_fIsModal) {
			::CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WARNING), nullptr, stubDlgProc, LPARAM(this));
			return 0;
		}

		return ::DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WARNING), nullptr, stubDlgProc, LPARAM(this));
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// stub dlg procedure.Just register the object pointer in WM_INITDIALOG

	static INT_PTR CALLBACK stubDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		CWarningImpl *w = reinterpret_cast<CWarningImpl *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (w)
			return(w->dlgProc(hwnd, msg, wParam, lParam));

		switch (msg) {
		case WM_INITDIALOG:
			w = reinterpret_cast<CWarningImpl *>(lParam);
			if (w) {
				::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
				return(w->dlgProc(hwnd, msg, wParam, lParam));
			}
			break;
		}
		return FALSE;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// dialog procedure for the warning dialog box

	INT_PTR CALLBACK dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_INITDIALOG:
			m_hwnd = hwnd;

			::SetWindowTextW(hwnd, TranslateT("TabSRMM warning message"));
			::Window_SetSkinIcon_IcoLib(hwnd, SKINICON_OTHER_MIRANDA);
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_AUTOURLDETECT, TRUE, 0);
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETEVENTMASK, 0, ENM_LINK);

			TranslateDialogDefault(hwnd);
			{
				CMStringW str(FORMAT, RTF_DEFAULT_HEADER, 0, 0, 0, 30 * 15);
				str.Append(m_szText);
				str.Append(L"}");
				str.Replace(L"\n", L"\\line ");
				SETTEXTEX stx = {ST_SELECTION, CP_UTF8};
				::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETTEXTEX, (WPARAM)&stx, T2Utf(str));

				::SetDlgItemTextW(hwnd, IDC_CAPTION, m_szTitle);

				if (m_dwFlags & CWF_NOALLOWHIDE)
					Utils::showDlgControl(hwnd, IDC_DONTSHOWAGAIN, SW_HIDE);
				if (m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) {
					Utils::showDlgControl(hwnd, IDOK, SW_HIDE);
					::SetFocus(::GetDlgItem(hwnd, IDCANCEL));
				}
				else {
					Utils::showDlgControl(hwnd, IDCANCEL, SW_HIDE);
					Utils::showDlgControl(hwnd, IDYES, SW_HIDE);
					Utils::showDlgControl(hwnd, IDNO, SW_HIDE);
					::SetFocus(::GetDlgItem(hwnd, IDOK));
				}

				UINT uResId = 0;
				if ((m_dwFlags & MB_ICONERROR) || (m_dwFlags & MB_ICONHAND))
					uResId = 32513;
				else if ((m_dwFlags & MB_ICONEXCLAMATION) || (m_dwFlags & MB_ICONWARNING))
					uResId = 32515;
				else if ((m_dwFlags & MB_ICONASTERISK) || (m_dwFlags & MB_ICONINFORMATION))
					uResId = 32516;
				else if (m_dwFlags & MB_ICONQUESTION)
					uResId = 32514;

				HICON hIcon;
				if (uResId)
					hIcon = reinterpret_cast<HICON>(::LoadImage(nullptr, MAKEINTRESOURCE(uResId), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
				else
					hIcon = ::Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);
				::SendDlgItemMessageW(hwnd, IDC_WARNICON, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0);

				if (!(m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL))
					::ShowWindow(hwnd, SW_SHOWNORMAL);

				WindowList_Add(hWindowList, hwnd, (UINT_PTR)hwnd);
			}
			return TRUE;

		case WM_CTLCOLORSTATIC:
			{
				HWND hwndChild = reinterpret_cast<HWND>(lParam);
				UINT id = ::GetDlgCtrlID(hwndChild);
				if (nullptr == m_hFontCaption) {
					HFONT hFont = reinterpret_cast<HFONT>(::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_GETFONT, 0, 0));
					LOGFONT lf = {0};

					::GetObject(hFont, sizeof(lf), &lf);
					lf.lfHeight = (int)((double)lf.lfHeight * 1.7f);
					m_hFontCaption = ::CreateFontIndirect(&lf);
					::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
				}

				if (IDC_CAPTION == id) {
					::SetTextColor(reinterpret_cast<HDC>(wParam), ::GetSysColor(COLOR_HIGHLIGHT));
					::SendMessage(hwndChild, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
				}

				if (IDC_WARNGROUP != id && IDC_DONTSHOWAGAIN != id) {
					::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
					return reinterpret_cast<INT_PTR>(::GetSysColorBrush(COLOR_WINDOW));
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:
			case IDYES:
			case IDNO:
				if (::IsDlgButtonChecked(hwnd, IDC_DONTSHOWAGAIN)) {
					uint32_t newVal = M.GetDword("cWarningsL", 0) | ((uint32_t)1L << m_uId);
					db_set_dw(0, SRMSGMOD_T, "cWarningsL", newVal);

					if (LOWORD(wParam) != IDNO) {
						newVal = M.GetDword("cWarningsV", 0) | ((uint32_t)1L << m_uId);
						db_set_dw(0, SRMSGMOD_T, "cWarningsV", newVal);
					}
				}
				__fallthrough;

			case IDCANCEL:
				if (!m_fIsModal && (IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam))) // modeless dialogs can receive a IDCANCEL from destroyAll()
					::DestroyWindow(hwnd);
				else
					::EndDialog(hwnd, LOWORD(wParam));
				break;
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR *)lParam)->code) {
			case EN_LINK:
				switch (((ENLINK *)lParam)->msg) {
				case WM_LBUTTONUP:
					ENLINK *e = reinterpret_cast<ENLINK *>(lParam);

					const wchar_t *wszUrl = Utils::extractURLFromRichEdit(e, ::GetDlgItem(hwnd, IDC_WARNTEXT));
					if (wszUrl) {
						Utils_OpenUrlW(wszUrl);
						mir_free(const_cast<wchar_t *>(wszUrl));
					}
				}
			}
			break;

		case WM_DESTROY:
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			delete this;

			WindowList_Remove(hWindowList, hwnd);
			Window_FreeIcon_IcoLib(hwnd);
			break;
		}

		return FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// implementation of the CWarningImpl class
//
// IMPORTANT note to translators for translation of the warning dialogs:
// 
//  Make sure to NOT remove the pipe character ( | ) from the strings. This separates the
//  warning title from the actual warning text.
// 
//  Also, do NOT insert multiple | characters in the translated string. Not well-formatted
//  warnings cannot be translated and the plugin will show the untranslated versions.
// 
//  strings marked with a NOT TRANSLATABLE comment cannot be translated at all. This
//  will be used for important and critical error messages only.
// 
//  some strings are empty, this is intentional and used for error messages that share
//  the message with other possible error notifications (popups, tool tips etc.)
// 
//  Entries that do not use the LPGENW() macro are NOT TRANSLATABLE, so don't bother translating them.

static wchar_t *Warnings[] = {
	nullptr,
	LPGENW("Save file|Unable to save temporary file"), // WARN_SAVEFILE 
	LPGENW("Edit user notes|You are editing the user notes. Click the button again or use the hotkey (default: Alt+N) to save the notes and return to normal messaging mode"),  /* WARN_EDITUSERNOTES */
	LPGENW("Missing component|The icon pack is missing. Please install it to the default icons folder.\n\nNo icons will be available"),		/* WARN_ICONPACKMISSING */
	LPGENW("Aero peek warning|You have enabled Aero Peek features and loaded a custom container window skin\n\nThis can result in minor visual anomalies in the live preview feature."),	/* WARN_AEROPEEKSKIN */
	LPGENW("File transfer problem|Sending the image by file transfer failed.\n\nPossible reasons: File transfers not supported, either you or the target contact is offline, or you are invisible and the target contact is not on your visibility list."), /* WARN_IMGSVC_MISSING */
	LPGENW("Settings problem|The option \\b1 History -> Imitate IEView API\\b0  is enabled and the History++ plugin is active. This can cause problems when using IEView as message log viewer.\n\nShould I correct the option (a restart is required)?"), /* WARN_HPP_APICHECK */
	LPGENW("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options -> Message sessions -> Advanced tweaks\\b0. Changing this option requires a restart."), /* WARN_NO_SENDLATER */
	LPGENW("Closing Window|You are about to close a window with multiple tabs open.\n\nProceed?"),		/* WARN_CLOSEWINDOW */
	LPGENW("Closing options dialog|To reflect the changes done by importing a theme in the options dialog, the dialog must be closed after loading a theme \\b1 and unsaved changes might be lost\\b0 .\n\nDo you want to continue?"), /* WARN_OPTION_CLOSE */
	LPGENW("Loading a theme|Loading a color and font theme can overwrite the settings defined by your skin.\n\nDo you want to continue?"), /* WARN_THEME_OVERWRITE */
};

/////////////////////////////////////////////////////////////////////////////////////////
// send cancel message to all open warning dialogs so they are destroyed
// before TabSRMM is unloaded.
// 
// called by the OkToExit handler in globals.cpp

void CWarning::destroyAll()
{
	if (hWindowList)
		WindowList_Broadcast(hWindowList, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// show a warning dialog using the id value. Check whether the user has chosen to
// not show this message again. This has room for 64 different warning dialogs, which
// should be enough in the first place. Extending it should not be too hard though.

LRESULT CWarning::show(const int uId, uint32_t dwFlags, const wchar_t *tszTxt)
{
	if (hWindowList == nullptr)
		hWindowList = WindowList_Create();

	// don't open new warnings when shutdown was initiated (modal ones will otherwise
	// block the shutdown)
	if (CMimAPI::m_shutDown)
		return -1;

	wchar_t *_s = nullptr;
	if (tszTxt)
		_s = const_cast<wchar_t *>(tszTxt);
	else {
		if (uId == -1)
			return -1;

		if (dwFlags & CWF_UNTRANSLATED)
			_s = TranslateW(Warnings[uId]);
		else {
			// revert to untranslated warning when the translated message
			// is not well-formatted.
			_s = TranslateW(Warnings[uId]);

			if (mir_wstrlen(_s) < 3 || nullptr == wcschr(_s, '|'))
				_s = TranslateW(Warnings[uId]);
		}
	}

	if (mir_wstrlen(_s) > 3 && wcschr(_s, '|') != nullptr) {
		if (uId >= 0 && !(dwFlags & CWF_NOALLOWHIDE)) {
			uint32_t val = M.GetDword("cWarningsL", 0);
			uint32_t mask = ((__int64)1L) << uId;
			if (mask & val) {
				bool bResult = (M.GetDword("cWarningsV", 0) & mask) != 0;
				if (dwFlags & MB_YESNO || dwFlags & MB_YESNOCANCEL)
					return (bResult) ? IDYES : IDNO;
				return IDOK;
			}
		}

		ptrW s(mir_wstrdup(_s));
		wchar_t *separator_pos = wcschr(s, '|');

		if (separator_pos) {
			*separator_pos = 0;

			CWarningImpl *w = new CWarningImpl(s, separator_pos + 1, uId, dwFlags);
			if (dwFlags & MB_YESNO || dwFlags & MB_YESNOCANCEL)
				return w->ShowDialog();

			w->ShowDialog();
		}
	}
	return -1;
}
