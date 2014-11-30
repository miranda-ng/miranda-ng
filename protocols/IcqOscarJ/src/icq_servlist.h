// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#ifndef __ICQ_SERVLIST_H
#define __ICQ_SERVLIST_H

// actions:
#define SSA_CHECK_ROSTER      0     // request serv-list
#define SSA_VISIBILITY        1     // update visibility
#define SSA_CONTACT_UPDATE    2     // update contact's details
#define SSA_GROUP_RENAME      5     // rename group
#define SSA_PRIVACY_ADD       0xA   // add privacy item
#define SSA_PRIVACY_REMOVE    0xB   // remove privacy item
#define SSA_CONTACT_ADD       0x10  // add contact w/o auth
#define SSA_CONTACT_SET_GROUP 0x12  // move to group
#define SSA_CONTACT_REMOVE    0x13  // delete contact
#define SSA_CONTACT_FIX_AUTH  0x40  // reuploading contact for auth re-request
#define SSA_GROUP_ADD         0x15  // create group
#define SSA_GROUP_REMOVE      0x16  // delete group
#define SSA_GROUP_UPDATE      0x17  // update group
#define SSA_SERVLIST_ACK      0x20  // send proto ack only (UploadUI)
#define SSA_SETAVATAR         0x30
#define SSA_REMOVEAVATAR      0x31
#define SSA_IMPORT            7
#define SSA_ACTION_GROUP      0x80  // grouped action

struct CIcqProto;
// callback prototypes for pending operation mechanism:
typedef int (__cdecl CIcqProto::*PENDING_GROUP_CALLBACK)(const char* pszGroup, WORD wGroupId, LPARAM lParam, int nResult);
typedef int (__cdecl CIcqProto::*PENDING_CONTACT_CALLBACK)(MCONTACT hContact, WORD wContactId, WORD wGroupId, LPARAM lParam, int nResult);

// cookie struct for SSI actions
struct cookie_servlist_action
{
	MCONTACT hContact;
	char *szGroup;
	WORD wContactId;
	WORD wGroupId;
	char *szGroupName;
	WORD wNewContactId;
	WORD wNewGroupId;
	int dwAction;
	LPARAM lParam;
	int dwGroupCount;
	cookie_servlist_action **pGroupItems;
};

// server id type groups
#define SSIT_ITEM             0x00000000
#define SSIT_GROUP            0x00010000

// server id flags
#define SSIF_UNHANDLED        0x01000000


// pending operations
#define PENDING_RESULT_SUCCESS  0x00
#define PENDING_RESULT_INLINE   0x01
#define PENDING_RESULT_FAILED   0x0F
#define PENDING_RESULT_PURGE    0x10

// serv-list update board
#define SSOG_SINGLE           0x00010000
#define SSOG_DOUBLE           0x00020000

#define SSOF_CONTACT          0x00800000
#define SSOF_BEGIN_OPERATION  0x00100000
#define SSOF_END_OPERATION    0x00200000
#define SSOF_IMPORT_OPERATION 0x00400000

#define SSOP_ITEM_ACTION      0x01000000 | SSOG_SINGLE
// SSA_PRIVACY_ADD
// SSA_CONTACT_ADD
// SSA_CONTACT_UPDATE
// SSA_VISIBILITY
// SSA_PRIVACY_REMOVE
// SSA_CONTACT_REMOVE
// SSA_SETAVATAR
// SSA_REMOVEAVATAR
#define SSOP_GROUP_ACTION     0x02000000 | SSOG_SINGLE
// SSA_GROUP_ADD
// SSA_GROUP_RENAME
// SSA_GROUP_UPDATE
// SSA_GROUP_REMOVE
#define SSO_CONTACT_SETGROUP  0x04000000 | SSOG_DOUBLE
// SSA_CONTACT_SET_GROUP
#define SSO_CONTACT_FIXAUTH   0x06000000 | SSOG_DOUBLE
// SSA_CONTACT_FIX_AUTH

#define SSO_BEGIN_OPERATION   0x80000000
#define SSO_END_OPERATION     0x40000000

#define SSOF_SEND_DIRECTLY    0x10000000

#define SSOF_ACTIONMASK       0x0000FFFF
#define SSOF_GROUPINGMASK     0x0F0FFFFF


#define MAX_SERVLIST_PACKET_ITEMS 200

// server-list request handler item
struct servlistgroupitem
{ // generic parent
  DWORD dwOperation;
  cookie_servlist_action* cookie;
  icq_packet packet;
  // perhaps add some dummy bytes
};

struct servlistgroupitemdouble: public servlistgroupitem
{
  icq_packet packet2;
  WORD wAction2;
};

struct ssiqueueditems
{
  time_t tAdded;
  int dwTimeout;
  int nItems;
  servlistgroupitem* pItems[MAX_SERVLIST_PACKET_ITEMS];
};


// cookie structs for pending records
struct servlistpendingoperation
{
  DWORD flags;
  PENDING_GROUP_CALLBACK callback;
  LPARAM param;
};

struct servlistpendingitem
{
  int nType;
  MCONTACT hContact;
  char* szGroup;
  WORD wContactID;
  WORD wGroupID;

  servlistpendingoperation* operations;
  int operationsCount;
};


#endif /* __ICQ_SERVLIST_H */
