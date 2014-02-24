/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
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

#include "m_options.h"

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

/* UserInfo/AddPage			  v0.1.2.0+
Must only be called during an userinfo/initialise hook
Adds a page to the details dialog
wParam = addInfo
lParam = (LPARAM)(OPTIONSDIALOGPAGE*)odp
addInfo must have come straight from the wParam of userinfo/initialise
Pages in the details dialog operate just like pages in property sheets. See the
Microsoft documentation for info on how they operate.
When the pages receive WM_INITDIALOG, lParam = (LPARAM)hContact
Strings in the structure can be released as soon as the service returns, but
icons must be kept around. This is not a problem if you're loading them from a
resource
The 3 'group' elements in the structure are ignored, and will always be ignored
Unlike the options dialog, the details dialog does not resize to fit its
largest page. Details dialog pages should be 222x132 dlus.
The details dialog (currently) has no Cancel button. I'm waiting to see if it's
sensible to have one.
Pages will be sent PSN_INFOCHANGED through WM_NOTIFY (idFrom = 0) when a protocol
ack is broadcast for the correct contact and with type = ACKTYPE_GETINFO.
To help you out, PSN_INFOCHANGED will also be sent to each page just after it's
created.
All PSN_ WM_NOTIFY messages have PSHNOTIFY.lParam = (LPARAM)hContact
*/
#define PSN_INFOCHANGED   1
#define PSN_PARAMCHANGED   2
#define PSM_FORCECHANGED  (WM_USER+100)   //force-send a PSN_INFOCHANGED to all pages

extern int hLangpack;

__forceinline INT_PTR UserInfo_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE* odp)
{	odp->hLangpack = hLangpack;
	return CallService("UserInfo/AddPage", wParam, (LPARAM)odp);
}

#endif // M_USERINFO_H__
