/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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
#ifndef M_EXCHANGE_COMMONHEADERS_H
#define M_EXCHANGE_COMMONHEADERS_H
#if !defined(MIID_EXCHANGE)
	#define MIID_EXCHANGE  {0xcfd79a89, 0x9959, 0x4e65, {0xb0, 0x76, 0x41, 0x3f, 0x98, 0xfe, 0x0d, 0x15}}
#endif

#define EXCHANGE_PORT 25
#define DEFAULT_INTERVAL 60 //in seconds
#define DEFAULT_RECONNECT_INTERVAL 10 //in minutes

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_options.h>
#include <m_clist.h>
#include <m_langpack.h>
#include "m_utils.h"

#include "resource.h"
#include "version.h"
#include "utils.h"
#include "emails.h"
#include "services.h"
#include "hooked_events.h"

extern char ModuleName[];
extern HINSTANCE hInstance;
extern HICON hiMailIcon;
extern HWND hEmailsDlg;



#endif //M_EXCHANGE_COMMONHEADERS_H