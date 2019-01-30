// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright (c) 2000-01 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright (c) 2001-02 Jon Keating, Richard Hughes
// Copyright (c) 2002-04 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright (c) 2004-10 Joe Kucera
// Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org)
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
//
// -----------------------------------------------------------------------------
// DESCRIPTION:
//
//  Public headers for ICQ protocol plug-in
//
// -----------------------------------------------------------------------------

#ifndef M_ICQ_H__
#define M_ICQ_H__ 1

#include <m_protosvc.h>

//extended search result structure, used for all searches
typedef struct {
  PROTOSEARCHRESULT hdr;
  DWORD uin;
  BYTE auth;
  BYTE gender;
  BYTE age;
  DWORD country;
  BYTE maritalStatus;
} ICQSEARCHRESULT;


// Open ICQ profile
// wParam = (WPARAM)hContact
#define MS_OPEN_PROFILE "/OpenProfile"

// Add contact to server-list
// wParam = (WPARAM)hContact
#define MS_ICQ_ADDSERVCONTACT "/AddServerContact"

// Display XStatus detail (internal use only)
// wParam = (WPARAM)hContact;
#define MS_XSTATUS_SHOWDETAILS "/ShowXStatusDetails"

// Update user details on server
// Permited operation types:
#define CIXT_BASIC      0x0001
#define CIXT_MORE       0x0002
#define CIXT_WORK       0x0004
#define CIXT_CONTACT    0x0008
#define CIXT_LOCATION   0x0010
#define CIXT_BACKGROUND 0x0020
#define CIXT_EDUCATION  0x0040
#define CIXT_EXTRA      0x0080
#define CIXT_FULL       0x00FF

// wParam = operationType
#define PS_CHANGEINFOEX "/ChangeInfoEx"

// miranda/icqoscar/statusmsgreq event
// called when our status message is requested
// wParam = (BYTE)msgType
// lParam = (DWORD)uin
// msgType is one of the ICQ_MSGTYPE_GET###MSG constants in icq_constants.h
// uin is the UIN of the contact requesting our status message
#define ME_ICQ_STATUSMSGREQ      "/StatusMsgReq"

// Request Custom status details (messages) for specified contact
// wParam = hContact  // request custom status details for this contact
// lParam = 0
// return = (int)dwSequence   // if successful it is sequence for ICQACKTYPE_XSTATUS_RESPONSE
                             // 0 failed to request (e.g. auto-request enabled)
                             // -1 delayed (rate control) - sequence unknown
#define PS_ICQ_REQUESTCUSTOMSTATUS "/RequestXStatusDetails"

#define MAX_CAPNAME 64
typedef struct
{
	int cbSize;
	char caps[0x10];
	HANDLE hIcon;
	char name[MAX_CAPNAME];
} ICQ_CUSTOMCAP;

// Add a custom icq capability.
// wParam = 0;
// lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
#define PS_ICQ_ADDCAPABILITY "/IcqAddCapability"

// Check if capability is supportes. Only icqCustomCap.caps does matter.
// wParam = (WPARAM)(HANDLE)hContact;
// lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
// returns non-zero if capability is supported
#define PS_ICQ_CHECKCAPABILITY "/IcqCheckCapability"

#endif // M_ICQ_H__
