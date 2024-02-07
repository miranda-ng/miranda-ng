/*
	Variables Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

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

#pragma once

#define STR_FIRSTNAME   L"first"
#define STR_LASTNAME    L"last"
#define STR_NICK        L"nick"
#define STR_CUSTOMNICK  L"cnick"
#define STR_EMAIL       L"email"
#define STR_CITY        L"city"
#define STR_STATE       L"state"
#define STR_COUNTRY     L"country"
#define STR_PHONE       L"phone"
#define STR_HOMEPAGE    L"homepage"
#define STR_ABOUT       L"about"
#define STR_GENDER      L"gender"
#define STR_AGE         L"age"
#define STR_FIRSTLAST   L"firstlast"
#define STR_UNIQUEID    L"id"
#define STR_DISPLAY     L"display"

#define STR_FAX         L"fax"
#define STR_CELLULAR    L"cellular"
#define STR_TIMEZONE    L"timezone"
#define STR_MYNOTES     L"mynotes"
#define STR_BIRTHDAY    L"bday"
#define STR_BIRTHMONTH  L"bmonth"
#define STR_BIRTHYEAR   L"byear"
#define STR_STREET      L"street"
#define STR_ZIP         L"zip"
#define STR_LANGUAGE1   L"lang1"
#define STR_LANGUAGE2   L"lang2"
#define STR_LANGUAGE3   L"lang3"
#define STR_CONAME      L"coname"
#define STR_CODEPT      L"codept"
#define STR_COPOSITION  L"copos"
#define STR_COSTREET    L"costreet"
#define STR_COCITY      L"cocity"
#define STR_COSTATE     L"costate"
#define STR_COZIP       L"cozip"
#define STR_COCOUNTRY   L"cocountry"
#define STR_COHOMEPAGE  L"cohomepage"

#define STR_ACCOUNT     L"account"
#define STR_STATUS      L"status"
#define STR_INTERNALIP  L"intip"
#define STR_EXTERNALIP  L"extip"
#define STR_GROUP       L"group"
#define STR_PROTOID     L"protoid"

#define CCNF_ACCOUNT    50 // CUSTOM, returns contact's account name (0.8.0+)
#define CCNF_STATUS     49 // CUSTOM, returns status mode description
#define CCNF_INTERNALIP 48 // CUSTOM, returns the contact's internal IP
#define CCNF_EXTERNALIP 47 // CUSTOM, returns the contact's external IP
#define CCNF_GROUP      46 // CUSTOM, returns group name
#define CCNF_PROTOID    45 // CUSTOM, returns protocol ID instead of name

#define PROTOID_HANDLE  "_HANDLE_"

// Possible flags:
#define CI_PROTOID      0x00000001  // The contact in the string is encoded
                                    // in the format <PROTOID:UNIQUEID>, e.g.
                                    // <ICQ:12345678>.
#define CI_NICK         0x00000002  // Search nick names.
#define CI_LISTNAME     0x00000004  // Search custom names shown in contact
                                    // list.
#define CI_FIRSTNAME    0x00000008  // Search contact's first names (contact
                                    // details).
#define CI_LASTNAME     0x00000010  // Search contact's last names (contact
                                    // details).
#define CI_EMAIL        0x00000020  // Search contact's email adresses
                                    // (contact details).
#define CI_UNIQUEID     0x00000040  // Search unique ids of the contac, e.g.
                                    // UIN.
#define CI_ALLFLAGS     0x7FFFFFFF  // All possible combinations of previous flags

#define CI_CNFINFO      0x40000000  // Searches one of the CNF_* flags (set
                                    // flags to CI_CNFINFO|CNF_X), only one
                                    // CNF_ type possible
#define CI_NEEDCOUNT    0x80000000  // returns contacts count

wchar_t* encodeContactToString(MCONTACT hContact);
