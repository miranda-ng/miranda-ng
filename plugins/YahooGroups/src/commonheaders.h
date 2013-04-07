/*
YahooGroups plugin for Miranda IM

Copyright © 2007 Cristian Libotean

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


#ifndef M_YAHOOGROUPS_COMMONHEADERS_H
#define M_YAHOOGROUPS_COMMONHEADERS_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "version.h"
#include "dlg_handlers.h"
#include "hooked_events.h"
#include "services.h"
#include "list.h"
#include "services.h"
#include "utils.h"

#include "resource.h"

#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_message.h>
#include <m_statusplugins.h>

extern char ModuleName[];
extern HINSTANCE hInstance;
extern UINT currentCodePage;

#endif //M_YAHOOGROUPS_COMMONHEADERS_H