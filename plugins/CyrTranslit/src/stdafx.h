/**
 * Plugin.H: Main common header of MirandaIM plugin
 * Copyright 2009 Valeriy V. Vyshnyak. 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <windows.h>
#include <map>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_clist.h>
#include <m_langpack.h>

#include "version.h"
#include "MirandaContact.h"
#include "TransliterationMap.h"
#include "TransliterationProtocol.h"
