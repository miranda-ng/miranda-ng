/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef M_USERINFO_H__
#define M_USERINFO_H__ 1

#include <m_gui.h>

class MIR_APP_EXPORT CUserInfoPageDlg : public CDlgBase
{
protected:
	MCONTACT m_hContact = INVALID_CONTACT_ID;

public:
	CUserInfoPageDlg(class CMPluginBase &pPlug, int idDialog);

	__forceinline void SetContact(MCONTACT hContact) {
		m_hContact = hContact;
	}

	virtual bool OnRefresh();
};

struct USERINFOPAGE
{
	MAllStrings szTitle, szGroup; // [TRANSLATED-BY-CORE]
	HPLUGIN pPlugin;
	uint32_t flags;               // ODPF_*, look at m_options.h
	CUserInfoPageDlg *pDialog;

	// used in UInfoEx only
	int position;
	INT_PTR dwInitParam;
};

//show the User Details dialog box
//wParam = (MCONTACT)hContact
//lParam = 0
#define MS_USERINFO_SHOWDIALOG       "UserInfo/ShowDialog"

/* UserInfo/Initialise			v0.1.2.0+
The user opened a details dialog. Modules should do whatever initialisation
they need and call userinfo/addpage one or more times if they want pages
displayed in the options dialog
wParam = addInfo
lParam = (LPARAM)hContact
addInfo should be passed straight to the wParam of userinfo/addpage
NB: The built-in userinfo module is loaded after all plugins, so calling
HookEvent() in your plugin's Load(void) function will fail if you specify this
hook. Look up core/m_system.h:me_system_modulesloaded.
*/
#define ME_USERINFO_INITIALISE   "UserInfo/Initialise"

#endif // M_USERINFO_H__
