/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
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

#include "commonheaders.h"

/* command line support */

struct CmdLineParam
{
	__inline CmdLineParam(LPCTSTR _name, LPCTSTR _value) :
		name(_name), value(_value)
		{}

	LPCTSTR name, value;
};

static int CompareParams(const CmdLineParam *p1, const CmdLineParam *p2)
{
	return _tcscmp(p1->name, p2->name);
}

static OBJLIST<CmdLineParam> arParams(5, CompareParams);

MIR_CORE_DLL(void) CmdLine_Parse(LPTSTR ptszCmdLine)
{
	bool bPrevSpace = true;
	for (LPTSTR p = ptszCmdLine; *p; p++) {
		if ( *p == ' ' || *p == '\t') {
			*p = 0;
			bPrevSpace = true;
			continue;
		}

		// new word beginning
		if (bPrevSpace) {
			bPrevSpace = false;
			if (*p != '/' && *p != '-')  // not an option - skip it
				continue;
		}
		else continue;  // skip a text that isn't an option

		TCHAR *pOptionName = p+1;
		if ((p = _tcspbrk(pOptionName, _T(" \t=:"))) == NULL) { // no more text in string
			arParams.insert(new CmdLineParam(pOptionName, _T("")));
			break;
		}
		
		if (*p == ' ' || *p == '\t') {
			arParams.insert(new CmdLineParam(pOptionName, _T("")));
			p--; // the cycle will wipe this space automatically
			continue;
		}

		// parameter with value
		*p = 0;
		arParams.insert(new CmdLineParam(pOptionName, ++p));
		if ((p = _tcspbrk(p, _T(" \t"))) == NULL) // no more text in string
			break;

		p--; // the cycle will wipe this space automatically
	}
}

MIR_CORE_DLL(LPCTSTR) CmdLine_GetOption(const TCHAR* ptszParameter)
{
	CmdLineParam tmp(ptszParameter, 0);
	int idx = arParams.getIndex(&tmp);
	return (idx == -1) ? NULL : arParams[idx].value;
}
