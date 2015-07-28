/*
   Authorization State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2006-2010 by Thief

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

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_database.h>

#include "version.h"

#define MIID_REMOVEPERSONALSETTINGS { 0x5eaec989, 0x8ff, 0x4820, { 0xb8, 0x6c, 0x2b, 0x6e, 0xf0, 0x8e, 0x33, 0x73 } }

#define INI_FILE_NAME  "RemovePersonalSettings.ini"

#define PLUGINDISABLELIST "PluginDisable"

#define METACONTACTS_PROTOCOL_NAME "MetaContacts"

#define NOTICE_TEXT LPGEN("All your personal settings will be erased!\n\
Make sure you are running this from a copy of your profile (and not over the original one).\n\
Running this will erase files/folders under Miranda main folder.\n\n\
Are you sure you want to remove all your personal settings?\n\n\
(You cannot say that I don't told you about the risks :P)")

#define MSGBOX_TITLE LPGEN("Remove Personal Settings")
