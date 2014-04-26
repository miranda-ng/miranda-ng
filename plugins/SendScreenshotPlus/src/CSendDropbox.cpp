/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

//---------------------------------------------------------------------------
#include "global.h"

//---------------------------------------------------------------------------
CSendDropbox::CSendDropbox(HWND Owner, MCONTACT hContact, bool bAsync)
: CSend(Owner, hContact, bAsync){
/// @todo : re-enable SS_DLG_DELETEAFTERSSEND with full implemention of Dropbox upload with progress, msg and sounds
	m_EnableItem = SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND/* | SS_DLG_DELETEAFTERSSEND*/;
	m_pszSendTyp = LPGENT("Dropbox transfer");
}

CSendDropbox::~CSendDropbox(){
}

//---------------------------------------------------------------------------
int CSendDropbox::Send() {
	if(!m_bAsync){
		SendThread();
		return 1;
	}
	mir_forkthread(&CSendDropbox::SendThreadWrapper, this);
	return 0;
}

//---------------------------------------------------------------------------


void CSendDropbox::SendThread() {
/// @todo : SS_DLG_DESCRIPTION and SS_DLG_DELETEAFTERSSEND are of no use as of now since we don't track upload progress
	INT_PTR ret=0;
	if(!m_hContact)
		SetContact(db_find_first("Dropbox"));
	if(m_hContact)
		ret = CallService(MS_DROPBOX_SEND_FILE, (WPARAM)m_hContact, (LPARAM)m_pszFile);
	if(!ret) {
		Error(LPGENT("%s (%i):\nCould not add a share to the Dropbox plugin."),TranslateTS(m_pszSendTyp),ret);
		Exit(ACKRESULT_FAILED); return;
	}
	m_bSilent=true;
	Exit(ACKRESULT_SUCCESS);
}

void CSendDropbox::SendThreadWrapper(void * Obj) {
	reinterpret_cast<CSendDropbox*>(Obj)->SendThread();
}
