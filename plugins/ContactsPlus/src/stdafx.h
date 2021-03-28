// ---------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
// 
// Copyright © 2002 Dominus Procellarum 
// Copyright © 2004-2008 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ---------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include "newpluginapi.h"
#include "m_protosvc.h"
#include "m_database.h"
#include "m_langpack.h"
#include "m_skin.h"
#include "m_clist.h"
#include "m_clistint.h"
#include "m_clc.h"
#include "m_contacts.h"
#include "m_history.h"
#include "m_userinfo.h"
#include "m_button.h"
#include "m_message.h"

#include "resource.h"
#include "version.h"
#include "utils.h"
#include "send.h"
#include "receive.h"

#define MODULENAME "SendReceiveContacts"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#define MS_CONTACTS_SEND "ContactsTransfer/SendContacts"
#define MS_CONTACTS_RECEIVE "ContactsTransfer/ReceiveContacts"
