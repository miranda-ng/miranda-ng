//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#pragma once
#include "resources.h"

#define LAST_MAIL_TIME_FROM_JID  SHORT_PLUGIN_NAME ".LastMailTimeFromJid"
#define LAST_THREAD_ID_FROM_JID  SHORT_PLUGIN_NAME ".LastThreadIdFromJid"

LPTSTR ReadJidSetting(LPCSTR name, LPCTSTR jid);
void WriteJidSetting(LPCSTR name, LPCTSTR jid, LPCTSTR setting);
