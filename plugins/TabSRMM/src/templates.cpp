/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// Simple editor for the message log templates

#include "stdafx.h"

/*
* hardcoded default set of templates for both LTR and RTL.
* cannot be changed and may be used at any time to "revert" to a working layout
*/

char* TemplateNames[TMPL_MAX] =
{
	LPGEN("Message In"),
	LPGEN("Message Out"),
	LPGEN("Group In (Start)"),
	LPGEN("Group Out (Start)"),
	LPGEN("Group In (Inner)"),
	LPGEN("Group Out (Inner)"),
	LPGEN("Status change"),
	LPGEN("Error message")
};

wchar_t* TemplateNamesW[TMPL_MAX] =
{
	LPGENW("Message In"),
	LPGENW("Message Out"),
	LPGENW("Group In (Start)"),
	LPGENW("Group Out (Start)"),
	LPGENW("Group In (Inner)"),
	LPGENW("Group Out (Inner)"),
	LPGENW("Status change"),
	LPGENW("Error message")
};

TTemplateSet LTR_Default = 
{
	TRUE,
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%I %N  %?&r%\\&E%\\!, %\\T%\\!: %?n%?S %?T%?|%M",
	L"%S %T%|%M",
	L"%S %T%|%M",
	L"%I %S %&r, %&T, %N %M%! ",
	L"%I%S %r, %T, %e%l%M",
	"Default LTR"
};

TTemplateSet RTL_Default =
{
	TRUE,
	L"%I %N  %r%n%S %T%|%M",
	L"%I %N  %r%n%S %T%|%M",
	L"%I %N  %r%n%S %T%|%M",
	L"%I %N  %r%n%S %T%|%M",
	L"%S %T%|%M",
	L"%S %T%|%M",
	L"%I%S %r, %T, %N %M%! ",
	L"%I%S %r, %T, %e%l%M",
	"Default RTL"
};

TTemplateSet LTR_Active, RTL_Active;

/*
* loads template set overrides from hContact into the given set of already existing
* templates
*/

static void LoadTemplatesFrom(TTemplateSet *tSet, MCONTACT hContact, int rtl)
{
	for (int i = 0; i < TMPL_MAX; i++) {
		DBVARIANT dbv = { 0 };
		if (db_get_ws(hContact, rtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[i], &dbv))
			continue;
		if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR)
			wcsncpy_s(tSet->szTemplates[i], dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}
}

void LoadDefaultTemplates()
{
	LTR_Active = LTR_Default;
	RTL_Active = RTL_Default;

	if (db_get_b(0, RTLTEMPLATES_MODULE, "setup", 0) < 2) {
		for (int i = 0; i < TMPL_MAX; i++)
			db_set_ws(0, RTLTEMPLATES_MODULE, TemplateNames[i], RTL_Default.szTemplates[i]);
		db_set_b(0, RTLTEMPLATES_MODULE, "setup", 2);
	}
	if (db_get_b(0, TEMPLATES_MODULE, "setup", 0) < 2) {
		for (int i = 0; i < TMPL_MAX; i++)
			db_set_ws(0, TEMPLATES_MODULE, TemplateNames[i], LTR_Default.szTemplates[i]);
		db_set_b(0, TEMPLATES_MODULE, "setup", 2);
	}
	LoadTemplatesFrom(&LTR_Active, 0, 0);
	LoadTemplatesFrom(&RTL_Active, 0, 1);
}
