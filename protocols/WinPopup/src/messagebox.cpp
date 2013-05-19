/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2011 Nikolay Raspopov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

class messagebox
{
public:
	messagebox () :
		m_hwndOwner (NULL),
		m_hwndMessageBox (NULL),
		m_Timeout (0),
		m_MonitorTerm (NULL)
	{
	}

	int WINAPI DoModal (const LPMSGBOXPARAMS lpMsgBoxParams, DWORD dwTimeout)
	{
		int ret = 0;
		m_hwndOwner = lpMsgBoxParams->hwndOwner;
		m_Timeout = dwTimeout;
		m_hwndMessageBox = NULL;
		m_MonitorTerm = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (m_MonitorTerm) {
			HANDLE hThread = (HANDLE)mir_forkthread( MsgBox, this );
			if (hThread) {

				ret = MessageBoxIndirect (lpMsgBoxParams);	

				// Ожидание завершения
				SetEvent (m_MonitorTerm);
				WaitForSingleObject (hThread, INFINITE);
			}
			CloseHandle (m_MonitorTerm);
		}
		return ret;
	}

protected:
	static BOOL CALLBACK EnumWindowsProc (HWND hWnd, LPARAM lParam)
	{
		messagebox* self = reinterpret_cast <messagebox*> (lParam);

		DWORD dwProcessId;
		GetWindowThreadProcessId (hWnd, &dwProcessId);
		const LONG req_style = WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU;
		HWND hButton;
		TCHAR ClassName[ 8 ];
		if ( GetCurrentProcessId () != dwProcessId ||
			( GetWindowLongPtr(hWnd, GWL_STYLE) & req_style ) != req_style ||
			GetParent( hWnd ) != NULL ||
			GetWindow( hWnd, GW_OWNER ) != self->m_hwndOwner ||
			( hButton = GetWindow( hWnd, GW_CHILD ) ) == NULL ||
			GetClassName( hButton, ClassName, _countof( ClassName ) ) == 0 ||
			lstrcmpi( _T("button"), ClassName ) != 0 )
			return TRUE;

		self->m_hwndMessageBox = hWnd;

		return FALSE;
	}

	static void MsgBox (LPVOID param)
	{
		messagebox* self = reinterpret_cast <messagebox*> (param);

		// Ловим диалог 10 секунд
		DWORD i = 0;
		while (WaitForSingleObject (self->m_MonitorTerm, 250) == WAIT_TIMEOUT &&
			EnumWindows (messagebox::EnumWindowsProc, reinterpret_cast <LPARAM> (self)) &&
			i++ < 10 * 4);
		if (!self->m_hwndMessageBox)
			// Так и не поймали
			return;
		
		HWND hButton = GetWindow (self->m_hwndMessageBox, GW_CHILD);

		// Отсчёт времени
		while (self->m_Timeout-- &&
			WaitForSingleObject (self->m_MonitorTerm, 1000) == WAIT_TIMEOUT && 
			IsWindow (self->m_hwndMessageBox) && IsWindow (hButton)) {
			CString buf, msg;
			int buf_size = GetWindowTextLength (hButton);
			if (buf_size) {
				GetWindowText (hButton, buf.GetBuffer (buf_size + 1), buf_size + 1);
				buf.ReleaseBuffer ();
			}
			int n = buf.ReverseFind (_T('='));
			msg.Format (_T(" = %u"), self->m_Timeout);
			SetWindowText (hButton, ((n < 1) ? buf : buf.Left (n - 1)) + msg);
		}

		// Закрытие окна
		if (IsWindow (self->m_hwndMessageBox) && IsWindow (hButton)) {
			DWORD_PTR res;
			SendMessageTimeout (self->m_hwndMessageBox, WM_COMMAND,
				(WPARAM) GetDlgCtrlID (hButton),
				(LPARAM) hButton, SMTO_ABORTIFHUNG | SMTO_NORMAL, 10000, &res);
		}

		return;
	}

	volatile HWND m_hwndOwner;
	volatile HWND m_hwndMessageBox;
	volatile DWORD m_Timeout;
	HANDLE m_MonitorTerm;
};

static int WINAPI MessageBoxIndirectTimeout (const LPMSGBOXPARAMS lpMsgBoxParams, DWORD dwTimeout)
{
	messagebox mb;
	return mb.DoModal (lpMsgBoxParams, dwTimeout);
}

LPCTSTR const szModules [] = {
	_T("netapi32.dll"),
	_T("netmsg.dll"),
	_T("wininet.dll"),
	_T("ntdll.dll"),
	_T("ntdsbmsg.dll"),
	NULL
};

void GetErrorMessage (DWORD dwLastError, CString &msg)
{
	CString buf;
	if ( HRESULT_FACILITY( dwLastError ) == FACILITY_NETBIOS )
	{
		CString sMessage = (LPCTSTR)CA2T( GetNetbiosError( HRESULT_CODE ( dwLastError ) ) );
		if ( ! sMessage.IsEmpty() )
			msg.Format( _T("%s\r\nNetBIOS %s: %u"), sMessage, T_ERROR, HRESULT_CODE( dwLastError ) );
		else
			msg.Format( _T("NetBIOS %s: %u"), T_ERROR, HRESULT_CODE( dwLastError ) );
	}
	else
	{
		HMODULE hModule = NULL;
		for (int i = 0; szModules [i]; i++)
		{
			hModule = LoadLibraryEx (szModules [i], NULL, LOAD_LIBRARY_AS_DATAFILE);
			LPTSTR MessageBuffer = NULL;
			if (FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_FROM_SYSTEM |
				(hModule ? FORMAT_MESSAGE_FROM_HMODULE : 0u),
				hModule, dwLastError, 0u, (LPTSTR) &MessageBuffer, 0u, NULL))
			{
				buf = MessageBuffer;
				buf.Trim (_T(" \t\r\n"));
				LocalFree (MessageBuffer);
				if (hModule)
					FreeLibrary (hModule);
				break;
			}
			if (hModule)
				FreeLibrary (hModule);
		}
		if ( ! buf.IsEmpty() )
			msg.Format( _T("%s\r\n%s: %u"), (LPCTSTR)buf, T_ERROR, dwLastError );
		else
			msg.Format( _T("%s: %u"), T_ERROR, dwLastError );
	}
}

static void PopupOrMessageBox (LPPOPUPDATAT ppdp)
{
	if (CALLSERVICE_NOTFOUND == PUAddPopupT (ppdp))
	{
		MSGBOXPARAMS mbp = { 0 };
		mbp.cbSize = sizeof (MSGBOXPARAMS);
		mbp.lpszText = ppdp->lptzText;
		mbp.lpszCaption = ppdp->lptzContactName;
		mbp.dwStyle = MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL;
		MessageBoxIndirectTimeout (&mbp, (DWORD)ppdp->iSeconds);
	}
}

void WarningBox (HANDLE hContact /* = NULL */, DWORD dwLastError /* = 0 */, LPCTSTR format, ...)
{
	if (!pluginInstalled)
		return;

	POPUPDATAT pdp = { 0 };
	pdp.lchContact = hContact;
	pdp.lchIcon = (HICON) LoadImage( pluginModule, MAKEINTRESOURCE (IDI_WINPOPUP),
		IMAGE_ICON, 16, 16, LR_SHARED );
	lstrcpy (pdp.lptzContactName, modtitle_t);
	va_list marker;
	va_start (marker, format);
	wvsprintf( pdp.lptzText, format, marker );
	va_end (marker);
	pdp.iSeconds = 10;

	if (dwLastError) {
		CString msg;
		GetErrorMessage (dwLastError, msg);
		int len = lstrlen (pdp.lptzText);
		pdp.lptzText [len] = _T('\r');
		pdp.lptzText [len + 1] = _T('\n');
		lstrcpy (pdp.lptzText + len + 2, msg);
	}

	PopupOrMessageBox (&pdp);
}
