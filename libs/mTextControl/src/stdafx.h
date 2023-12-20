/*
Miranda Text Control - Plugin for Miranda IM
Copyright (C) 2005 Victor Pavlychko (nullbie@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __headers_h__
#define __headers_h__

#define _CRT_SECURE_NO_DEPRECATE

#define OEMRESOURCE

#include <windows.h>
#include <richedit.h>
#include <richole.h>
#include <tom.h>
#include <textserv.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_utils.h>

#include <m_text.h>
#include <m_smileyadd.h>

#include "textusers.h"

extern HINSTANCE g_hInst;
extern PCreateTextServices MyCreateTextServices;

void InitRichEdit(ITextServices *ts);
HWND CreateProxyWindow();

#define MODULTITLE "Text Display"
#define MODULENAME "MTextControl"

#endif // __headers_h__
