/*
Copyright (c) 2014-17 Robert Pösel, 2017-25 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <Wincrypt.h>

#include <stdio.h>
#include <malloc.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_system.h>

#include <m_avatars.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_extraicons.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_genmenu.h>
#include <m_history.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_imgsrvc.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_timezones.h>
#include <m_toptoolbar.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_proto_listeningto.h>
#include <m_folders.h>
#include <m_version.h>

#include "resource.h"
#include "dummy.h"
#include "dummy_proto.h"

void FillTemplateCombo(HWND hwndDlg, int iCtrlId);
void InitIcons();
