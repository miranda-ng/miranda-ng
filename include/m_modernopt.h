/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (ñ) 2000-10 Miranda ICQ/IM project,
Copyright (c) 2007 Artem Shpynov

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

#ifndef M_MODERNOPT_H__
#define M_MODERNOPT_H__ 1

/*
	260x210 - subsection
	368x210 - section
*/

#define MS_MODERNOPT_SHOW			"ModernOpt/Show"

#define ME_MODERNOPT_INITIALIZE		"ModernOpt/Initialize"

#define MODEROPT_FLG_UNICODE		0x01
#define MODEROPT_FLG_NORESIZE		0x02

#ifdef UNICODE
	#define MODEROPT_FLG_TCHAR		MODEROPT_FLG_UNICODE
#else
	#define MODEROPT_FLG_TCHAR		0
#endif

typedef enum {
	MODERNOPT_TYPE_SECTIONPAGE,
	MODERNOPT_TYPE_SUBSECTIONPAGE,
	MODERNOPT_TYPE_IGNOREOBJECT,
	MODERNOPT_TYPE_SELECTORPAGE,
	MODERNOPT_TYPE_COUNT
};

typedef enum {
	MODERNOPT_PAGE_HOME,
	MODERNOPT_PAGE_ACCOUNTS,
	MODERNOPT_PAGE_SKINS,
	MODERNOPT_PAGE_CLIST,
	MODERNOPT_PAGE_MSGS,
	MODERNOPT_PAGE_IGNORE,
	MODERNOPT_PAGE_STATUS,
	MODERNOPT_PAGE_ADVANCED,
	MODERNOPT_PAGE_MODULES,
	MODERNOPT_PAGE_COUNT
};

#define MODERNOPT_CTRL_LAST		0

typedef struct {
	int			cbSize;
	DWORD		dwFlags;
	int			iType;
	int			iSection;

	HICON		hIcon;
	union {
		TCHAR	*lptzSubsection;
		char	*lpzSubsection;
		WCHAR	*lpwzSubsection;
	};

	// Generic dialog page
	HINSTANCE	hInstance;
	char		*lpzTemplate;
	DLGPROC		pfnDlgProc;
	int			*iBoldControls;

	// General page support
	char		*lpzClassicGroup;
	char		*lpzClassicPage;
	char		*lpzClassicTab;
	char		*lpzHelpUrl;

	// Ingore page
	char		*lpzIgnoreModule;
	char		*lpzIgnoreSetting;
	DWORD		dwIgnoreBit;

	// Selector type page
	char		*lpzThemeExtension;
	char		*lpzThemeModuleName;
} MODERNOPTOBJECT;

#define MS_MODERNOPT_ADDOBJECT		"ModernOpt/AddObject"

#define MS_MODERNOPT_SELECTPAGE		"ModernOpt/SelectPage"

#define ME_MODERNINGORE_RELOAD		"ModernOpt/Ignore/Reload"

#define TS_SKIN_ACTIVE				"/Active"

#define TS_SKIN_PREVIEW				"/Preview"

#define TS_SKIN_APPLY				"/Apply"

// used by core options module to switch back to modernopt module
#define MS_MODERNOPT_RESTORE		"ModernOpt/Restore"

#endif // M_MODERNOPT_H__
