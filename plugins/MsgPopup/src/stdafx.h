/*

MessagePopup - replacer of MessageBox'es

Copyright 2004 Denis Stanishevskiy

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

#include <windows.h>
#include <tlhelp32.h>
#include <dbghelp.h>

#include <newpluginapi.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <win2k.h>

#include "options.h"
#include "resource.h"
#include "Version.h"

#define SERVICENAME "MessagePopup"

struct MSGBOXOPTIONS
{
	COLORREF FG[4];
	COLORREF BG[4];
	int Timeout[4];
	BOOL Sound;
};

extern MSGBOXOPTIONS options;
extern MSGBOXOPTIONS optionsDefault;

void LoadConfig();
