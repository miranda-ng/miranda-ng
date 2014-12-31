/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef UAboutFormH
#define UAboutFormH

//---------------------------------------------------------------------------
class TfrmAbout{

	public:
		// Deklaration Standardkonstruktor/Standarddestructor
		TfrmAbout(HWND Owner);
		~TfrmAbout();

		HWND	m_hWndOwner;

		static void Unload();
		void	Show(){ShowWindow(m_hWnd,SW_SHOW);}
		void	Hide(){ShowWindow(m_hWnd,SW_HIDE);}
		void	Close(){SendMessage(m_hWnd,WM_CLOSE,0,0);}

	private:
		HWND	m_hWnd;

	protected:
		UINT m_Page;
		typedef std::map<HWND, TfrmAbout *> CHandleMapping;
		static CHandleMapping _HandleMapping;
		static INT_PTR CALLBACK DlgTfrmAbout(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		LRESULT wmInitdialog(WPARAM wParam, LPARAM lParam);
		LRESULT wmCommand(WPARAM wParam, LPARAM lParam);
		LRESULT wmClose(WPARAM wParam, LPARAM lParam);

		void btnPageClick();

};

//---------------------------------------------------------------------------
#endif
