/*
   Authorization State plugin for Miranda-IM (www.miranda-im.org)
   (c) 2006 by Thief
   Icons by Faith Healer

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

   File name      : $URL: http://svn.mirandaim.ru/mainrepo/authstate/trunk/commonheaders.h $
   Revision       : $Rev: 222 $
   Last change on : $Date: 2006-09-26 05:54:03 +0700 (Вт, 26 сен 2006) $
   Last change by : $Author: Thief $

*/

#ifndef COMMHEADERS_H
#define COMMHEADERS_H

#define _CRT_SECURE_NO_WARNINGS

//needed by m_utils.h
#include <stdio.h>

// needed by m_icolib.h > r9256
#include <stddef.h>

//Miranda API headers
#include <windows.h>
#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
//#include <m_protocols.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include "resource.h"

//external SDK headers
#include <m_cluiframes.h>
#include <m_extraicons.h>

static const int DefaultSlot = EXTRA_ICON_PROTO;
#define MODULENAME "Rate"

/* 0e2866a8-6f4c-4132-98ae-6afdb8766c48 */
#define MIID_CONTACTSRATE {0x0e2866a8, 0x6f4c, 0x4132, {0x98, 0xae, 0x6a, 0xfd, 0xb8, 0x76, 0x6c, 0x48}}

static struct
{
	char*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIconLibItem;
} iconList[] =
{
  { LPGEN( "Rate high" ), "rate_high", IDI_RATEHI },
  { LPGEN( "Rate medium" ), "rate_medium", IDI_RATEME },
  { LPGEN( "Rate low" ), "rate_low", IDI_RATELO },
};



#endif //COMMHEADERS_H
