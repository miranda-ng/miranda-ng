/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <shlobj.h>            /* for SHChangeNotify() */
#include <Vssym32.h>
#include <malloc.h>
#include <Uxtheme.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_clist.h>
#include "win2k.h"

#define ASSOCMGR_NOHELPERFUNCTIONS

#include "m_assocmgr.h"

#include "utils.h"
#include "reg.h"
#include "assoclist.h"
#include "dde.h"
#include "test.h"
#include "resource.h"
#include "version.h"
