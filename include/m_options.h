/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#ifndef M_OPTIONS_H__
#define M_OPTIONS_H__

extern int hLangpack;

/* Opt/Initialise
The user opened the options dialog. Modules should do whatever initialisation
they need and call opt/addpage one or more times if they want pages displayed
in the options dialog
wParam = addInfo
lParam = 0
addInfo should be passed straight to the wParam of opt/addpage
*/
#define ME_OPT_INITIALISE   "Opt/Initialise"

/* Opt/AddPage
Must only be called during an opt/initialise hook
Adds a page to the options dialog
wParam = addInfo
lParam = (LPARAM)(OPTIONSDIALOGPAGE*)odp
addInfo must have come straight from the wParam of opt/initialise
Pages in the options dialog operate just like pages in property sheets. See the
Microsoft documentation for details on how they operate.
Strings in the structure can be released as soon as the service returns, but
icons must be kept around. This is not a problem if you're loading them from a
resource.
Prior to v0.1.2.1 the options dialog would resize to fit the largest page, but
since then it is fixed in size. The largest page that fits neatly is 314x240
DLUs.
*/

// WARNING: do not use Translate(TS) for pszTitle, pszGroup or pszTab as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
typedef struct {
	int position;        //a position number, lower numbers are topmost
	union {
		char* pszTitle; // [TRANSLATED-BY-CORE]
		TCHAR* ptszTitle;
	};
	DLGPROC pfnDlgProc;
	char *pszTemplate;
	HINSTANCE hInstance;
	HICON hIcon;		 //v0.1.0.1+
	union {
		char* pszGroup;		 //v0.1.0.1+ [TRANSLATED-BY-CORE]
		TCHAR* ptszGroup;		 //v0.1.0.1+
	};
	int groupPosition;	 //v0.1.0.1+
	HICON hGroupIcon;	 //v0.1.0.1+
	DWORD flags;         //v0.1.2.1+

	union {
		char* pszTab;		 //v0.6.0.0+ [TRANSLATED-BY-CORE]
		TCHAR* ptszTab;		 //v0.6.0.0+
	};

	union {
		LPARAM dwInitParam;	 //v0.8.0.0+  a value to pass to lParam of WM_INITDIALOG message
		class CDlgBase *pDialog;
	};
	int hLangpack;
}
	OPTIONSDIALOGPAGE;

#define ODPF_BOLDGROUPS     4   // give group box titles a bold font
#define ODPF_UNICODE        8   // string fields in OPTIONSDIALOGPAGE are WCHAR*
#define ODPF_USERINFOTAB    16  // options page is tabbed
#define ODPF_DONTTRANSLATE  32  // do not translate option page title

#if defined(_UNICODE)
	#define ODPF_TCHAR     ODPF_UNICODE
#else
	#define ODPF_TCHAR     0
#endif

#define PSM_GETBOLDFONT   (WM_USER+102)   //returns HFONT used for group box titles

__forceinline INT_PTR Options_AddPage(WPARAM wParam, OPTIONSDIALOGPAGE* odp)
{	odp->hLangpack = hLangpack;
	return CallService("Opt/AddPage", wParam, (LPARAM)odp);
}

//Opens the options dialog, optionally at the specified page    v0.1.2.1+
//wParam = 0
//lParam = (LPARAM)(OPENOPTIONSDIALOG*)&ood;
//Returns 0 on success, nonzero on failure
//The behaviour if the options dialog is already open is that it will just be
//activated, the page won't be changed. This may change in the future.
typedef struct {
	int cbSize;
	const char *pszGroup;    //set to NULL if it's a root item
	const char *pszPage;     //set to NULL to just open the options at no
	                         //specific page
	const char *pszTab;      //set to NULL to just open the options at no
	                         //specific tab
}
	OPENOPTIONSDIALOG;

__forceinline INT_PTR Options_Open(OPENOPTIONSDIALOG *ood)
{
	return CallService("Opt/OpenOptions", hLangpack, (LPARAM)ood);
}

//Opens the options dialog, with only specified page    v0.8.0.x+

__forceinline HWND Options_OpenPage(OPENOPTIONSDIALOG *ood)
{
	return (HWND)CallService("Opt/OpenOptionsPage", hLangpack, (LPARAM)ood);
}

#endif  //M_OPTIONS_H__
