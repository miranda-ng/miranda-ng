/*
Copyright (C) 2026 Miranda NG team

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

#include <windows.h>
#include <time.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_utils.h>

#include "resource.h"
#include "version.h"
#include "oscar.h"
#include "proto.h"

extern PLUGININFOEX pluginInfoEx;

typedef CProtoDlgBase<CKicqProto> CKicqDlgBase;
