/*

Miranda IM: the free IM client for Microsoft* Windows*
Copyright 2000-2009 Miranda ICQ/IM project, 

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

File name      : $HeadURL: http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/SendSSPlus/CSendHTTPServer.h $
Revision       : $Revision: 13 $
Last change on : $Date: 2010-04-02 02:54:30 +0400 (Пт, 02 апр 2010) $
Last change by : $Author: ing.u.horn $

*/

#ifndef _CSEND_HTTP_SERVER_H
#define _CSEND_HTTP_SERVER_H

//---------------------------------------------------------------------------
#include "global.h"
#include "CSend.h"

//---------------------------------------------------------------------------
class CSendHTTPServer : public CSend {
	public:
		// Deklaration Standardkonstruktor/Standarddestructor
		CSendHTTPServer(HWND Owner, HANDLE hContact, bool bFreeOnExit);
		~CSendHTTPServer();

		void					Send();

	protected:
		LPSTR					m_pszFileName;
		LPSTR					m_URL;
		STFileShareInfo			m_fsi;
		LPSTR					m_fsi_pszSrvPath;
		LPSTR					m_fsi_pszRealPath;
		void					SendThread();
		static void				SendThreadWrapper(void * Obj);

		typedef std::map<HANDLE, CSendHTTPServer *> CContactMapping;
		static CContactMapping _CContactMapping;

		static INT_PTR			MyCallService(const char *name, WPARAM wParam, LPARAM lParam);

};

//---------------------------------------------------------------------------

#endif
