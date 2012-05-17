/*
   Show Contact Gender plugin for Miranda-IM (www.miranda-im.org)
   (c) 2006-2011 by Thief

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

   File name      : $URL: http://svn.miranda.im/mainrepo/gender/trunk/commonheaders.h $
   Revision       : $Rev: 1687 $
   Last change on : $Date: 2011-01-22 18:44:21 +0200 (Сб, 22 янв 2011) $
   Last change by : $Author: Thief $

*/

#ifndef COMMHEADERS_H
#define COMMHEADERS_H

#include <stdio.h> //needed by m_utils.h
#include <stddef.h> // needed by m_icolib.h > r9256

#include <windows.h>
#include <commctrl.h>

//Miranda API headers
#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_genmenu.h>
#include <m_icolib.h>
#include <m_cluiframes.h>

//Gender SDK
#include "m_gender.h"

//Resources
#include "resource.h"

//external SDK headers
#include "m_updater.h"
#include "m_metacontacts.h"
#include "m_extraicons.h"

static const int DefaultSlot = EXTRA_ICON_ADV2;
#define MODULENAME "Gender"

#endif //COMMHEADERS_H
