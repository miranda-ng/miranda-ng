/*

Miranda NG: the free IM client for Microsoft* Windows*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_CONTACTS_H__
#define M_CONTACTS_H__ 1

#ifndef M_CORE_H__
#include <m_core.h>
#endif

// Types of information you can retrieve by setting the dwFlag in CONTACTINFO
#define CNF_FIRSTNAME   1  // returns first name (string)
#define CNF_LASTNAME    2  // returns last name (string)
#define CNF_NICK        3  // returns nick name (string)
#define CNF_CUSTOMNICK	4  // returns custom nick name, clist name (string)
#define CNF_EMAIL       5  // returns email (string)
#define CNF_CITY        6  // returns city (string)
#define CNF_STATE       7  // returns state (string)
#define CNF_COUNTRY     8  // returns country (string)
#define CNF_PHONE       9  // returns phone (string)
#define CNF_HOMEPAGE    10 // returns homepage (string)
#define CNF_ABOUT       11 // returns about info (string)
#define CNF_GENDER      12 // returns gender (byte, 'M', 'F' character)
#define CNF_AGE         13 // returns age (byte, 0 == unspecified)
#define CNF_FIRSTLAST   14 // returns first name + last name (string)
#define CNF_UNIQUEID    15 // returns uniqueid, protocol username (must check type for type of return)
#define CNF_FAX         18 // returns fax (string)
#define CNF_CELLULAR    19 // returns cellular (string)
#define CNF_TIMEZONE    20 // returns timezone (string)
#define CNF_MYNOTES     21 // returns user specified notes (string)
#define CNF_BIRTHDAY    22 // returns birthday day of month (byte)
#define CNF_BIRTHMONTH  23 // returns birthday month (byte)
#define CNF_BIRTHYEAR   24 // returns birthday year (word)
#define CNF_STREET      25 // returns street (string)
#define CNF_ZIP         26 // returns zip code (string)
#define CNF_LANGUAGE1   27 // returns language1 (string)
#define CNF_LANGUAGE2   28 // returns language2 (string)
#define CNF_LANGUAGE3   29 // returns language3 (string)
#define CNF_CONAME      30 // returns company name (string)
#define CNF_CODEPT      31 // returns company department (string)
#define CNF_COPOSITION  32 // returns company position (string)
#define CNF_COSTREET    33 // returns company street (string)
#define CNF_COCITY      34 // returns company city (string)
#define CNF_COSTATE     35 // returns company state (string)
#define CNF_COZIP       36 // returns company zip code (string)
#define CNF_COCOUNTRY   37 // returns company country (string)
#define CNF_COHOMEPAGE  38 // returns company homepage (string)
#define CNF_DISPLAYUID  39 // returns uniqueid to display in interface (must check type for type of return)
#define CNF_MAX         40 // maximum number

// Special types
// Return the custom name using the name order setting
#define CNF_DISPLAY		16
// Same as CNF_DISPLAY except the custom handle is not used
#define CNF_DISPLAYNC	17

namespace Contact
{
	// Get contact information
	// Returns NULL on failure to retrieve the info or a string on success.
	// If successful, this string must be freed using mir_free

	MIR_APP_DLL(wchar_t *) GetInfo(
		int type,                    // one of the CNF_* constants
		MCONTACT hContact,           // contact id or NULL for the global data
		const char *szProto = nullptr); // protocol for global data. if skipped, grabbed from hContact

	MIR_APP_DLL(int) GetStatus(MCONTACT hContact);

	/////////////////////////////////////////////////////////////////////////////////////////
	// if contact is a group chat

	MIR_APP_DLL(bool) IsGroupChat(MCONTACT hContact, const char *szProto = nullptr);

	/////////////////////////////////////////////////////////////////////////////////////////
	// gets / sets hidden status for a contact

	MIR_APP_DLL(bool) IsHidden(MCONTACT hContact);
	MIR_APP_DLL(void) Hide(MCONTACT hContact, bool bHidden = true);

	/////////////////////////////////////////////////////////////////////////////////////////
	// checks / adds / removes a contact from list of contacts
	// removed contacts would be physically deleted after restart

	MIR_APP_DLL(bool) OnList(MCONTACT hContact);
	MIR_APP_DLL(void) PutOnList(MCONTACT hContact);
	MIR_APP_DLL(void) RemoveFromList(MCONTACT hContact);

	/////////////////////////////////////////////////////////////////////////////////////////
	// Add contact's dialog

	// passing hWnd == NULL will result in a dialog that is created modeless

	MIR_APP_DLL(void) Add(MCONTACT hContact, MWindow hwndParent = nullptr);
	MIR_APP_DLL(void) AddByEvent(MEVENT hEvent, MWindow hwndParent = nullptr);
	MIR_APP_DLL(void) AddBySearch(const char *szProto, struct PROTOSEARCHRESULT *psr, MWindow hwndParent = nullptr);
};

#endif // M_CONTACTS_H__
