/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 S�rgio Vieira Rolanski (portet from Borland C++)

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

#ifndef _CSEND_HTTP_SERVER_H
#define _CSEND_HTTP_SERVER_H

//---------------------------------------------------------------------------
class CSendHTTPServer : public CSend {
	public:
		// Deklaration Standardkonstruktor/Standarddestructor
		CSendHTTPServer(HWND Owner, MCONTACT hContact, bool bAsync);
		~CSendHTTPServer();

		int Send();

	protected:
		char*  m_pszFileName;
		char* m_fsi_pszSrvPath;
		TCHAR* m_fsi_pszRealPath;

		STFileShareInfo m_fsi;

		void SendThread();
		static void SendThreadWrapper(void * Obj);

		typedef std::map<HANDLE, CSendHTTPServer *> CContactMapping;
		static CContactMapping _CContactMapping;

//		static INT_PTR			MyCallService(const char *name, WPARAM wParam, LPARAM lParam);
};

//---------------------------------------------------------------------------

#endif
