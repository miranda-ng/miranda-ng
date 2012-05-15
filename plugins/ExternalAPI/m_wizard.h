/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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

#ifndef M_WIZARD_H__
#define M_WIZARD_H__ 1

#include "m_options.h"

#define MWF_UNICODE		0x01
#define MWF_ICOLIB		0x02
#ifdef _UNICODE
	#define MWF_TCHAR	MWF_UNICODE
#else
	#define MWF_TCHAR	0
#endif

typedef struct
{
	DWORD dwFlags;
	HANDLE hContact;
	char *pszWizardName;
	union
	{
		HICON hIcon;
		HANDLE hIcolibItem;
	};
	union
	{
		char *pszTitle;
		WCHAR *pwszTitle;
		TCHAR *ptszTitle;
	};
} WIZARDINFO;

//show the User Details dialog box
//wParam=0
//lParam=(WPARAM)(WIZARDINFO *)wizardInfo
#define MS_WIZARD_SHOW             "Wizard/Show"

// Predefined wizards
#define MW_MIRANDA_STARTUP         "Miranda/Startup"
#define MW_MIRANDA_CONFIG          "Miranda/Config"

/* Wizard/Initialise
The user opened a details dialog. Modules should do whatever initialisation
they need and call wizard/addpage one or more times if they want pages
displayed in the options dialog
wParam=addInfo
lParam=(WIZARDINFO *)wizardInfo
addInfo should be passed straight to the wParam of wizard/addpage
*/
#define ME_WIZARD_INITIALISE   "Wizard/Initialise"

/* Wizard/AddPage
Must only be called during an userinfo/initialise hook
Adds a page to the details dialog
wParam=addInfo
lParam=(LPARAM)(OPTIONSDIALOGPAGE*)odp
addInfo must have come straight from the wParam of userinfo/initialise
Pages in the details dialog operate just like pages in property sheets. See the
Microsoft documentation for info on how they operate.
When the pages receive WM_INITDIALOG, lParam=(LPARAM)(WIZARDINFO *)wizardInfo
Strings in the structure can be released as soon as the service returns, but
icons must be kept around. This is not a problem if you're loading them from a
resource
*/
#define MS_WIZARD_ADDPAGE      "Wizard/AddPage"

#endif // M_WIZARD_H__

