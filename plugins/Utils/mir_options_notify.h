/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __MIR_OPTIONS_NOTIFY_H__
# define __MIR_OPTIONS_NOTIFY_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" 
{
#endif



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dialog to save options
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CONTROL_CHECKBOX 0		// Stored as BYTE
#define CONTROL_SPIN 1			// Stored as WORD
#define CONTROL_COLOR 2			// Stored as DWORD
#define CONTROL_RADIO 3			// Stored as WORD
#define CONTROL_TEXT 4			// Stored as char *
#define CONTROL_TEMPLATE 5		// Stored as Template

struct OptPageControl {
	int type;
	int nID;
	char *setting;
	union {
		DWORD dwDefValue;
		const TCHAR *szDefVale;
	};
	union {
		int nIDSpin;
		int value;
	};
	WORD min;
	WORD max;
};

BOOL CALLBACK SaveOptsDlgProc(OptPageControl *controls, int controlsSize, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);




#ifdef __cplusplus
}
#endif

#endif // __MIR_OPTIONS_NOTIFY_H__
