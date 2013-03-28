/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _UI_PSP_BASE_INCLUDE_
#define _UI_PSP_BASE_INCLUDE_

INT_PTR CALLBACK PSPBaseProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK PSPProcAnniversary(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcCompany(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcContactHome(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcContactProfile(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcContactWork(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcGeneral(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcOrigin(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PSPProcEdit(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, const CHAR* pszSetting);
static FORCEINLINE INT_PTR CALLBACK PSPProcMyNotes(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ return PSPProcEdit(hDlg, uMsg, wParam, lParam, SET_CONTACT_MYNOTES); }
static FORCEINLINE INT_PTR CALLBACK PSPProcAbout(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ return PSPProcEdit(hDlg, uMsg, wParam, lParam, SET_CONTACT_ABOUT); }

void UpDate_CountryIcon(HWND hCtrl, int countryID);

#endif /* _UI_PSP_BASE_INCLUDE_ */
