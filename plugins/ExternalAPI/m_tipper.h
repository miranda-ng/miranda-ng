/*
Copyright (C) 2006-07 Scott Ellis
Copyright (C) 2007-09 Jan Holub

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

// Tipper API
# pragma once

#include <m_clc.h>

// translation function type
// use hContact, module and setting to read your db value(s) and put the resulting string into buff
// return buff if the translation was successful, or return 0 for failure
typedef wchar_t* (TranslateFunc)(MCONTACT hContact, const char *module, const char *setting_or_prefix, wchar_t *buff, int bufflen);

typedef struct {
	TranslateFunc *transFunc;	// address of your translation function (see typedef above)
	const wchar_t *swzName;		// make sure this is unique, and DO NOT translate it
	DWORD id;				// will be overwritten by Tipper - do not use
} DBVTranslation;

// add a translation to tipper
// wParam not used
// lParam = (DBVTranslation *)translation
#define MS_TIPPER_ADDTRANSLATION "Tipper/AddTranslation"

// ansi version of tipper
// wParam - optional (wchar_t *)text for text-only tips
// lParam - (CLCINFOTIP *)infoTip
#define MS_TIPPER_SHOWTIP  "mToolTip/ShowTip"

// unicode extension to the basic functionality
// wParam - optional (wchar_t *)text for text-only tips
// lParam - (CLCINFOTIP *)infoTip
#define MS_TIPPER_SHOWTIPW "mToolTip/ShowTipW"

__forceinline void Tipper_ShowTip(const wchar_t *pwsztext, CLCINFOTIP *ti)
{	CallService(MS_TIPPER_SHOWTIPW, WPARAM(pwsztext), LPARAM(ti));
}

// hides a tooltip
// wParam - 0
// lParam - 0
#define MS_TIPPER_HIDETIP "mToolTip/HideTip"

__forceinline void Tipper_Hide()
{	CallService(MS_TIPPER_HIDETIP, 0, 0);
}
