/*

Miranda NG: the free IM client for Microsoft* Windows*
Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)

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

typedef struct {
	int cbSize;
	BYTE dwFlag;
	MCONTACT hContact;
	char *szProto;
	BYTE type;
	union {
		BYTE bVal;
		WORD wVal;
		DWORD dVal;
		TCHAR *pszVal;
		WORD cchVal;
	};
} CONTACTINFO;

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

// Special types
// Return the custom name using the name order setting
// IMPORTANT: When using CNF_DISPLAY you MUST free the string returned
// You must **NOT** do this from your version of free() you have to use Miranda's free()
// you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
#define CNF_DISPLAY		16
// Same as CNF_DISPLAY except the custom handle is not used
// IMPORTANT: When using CNF_DISPLAYNC you MUST free the string returned
// You must **NOT** do this from your version of free() you have to use Miranda's free()
// you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
#define CNF_DISPLAYNC	17

// Add this flag if you want to get the Unicode info
#define CNF_UNICODE     0x80

#if defined(_UNICODE)
	#define CNF_TCHAR       CNF_UNICODE     // will use TCHAR* instead of char*
#else
	#define CNF_TCHAR       0               // will return char*, as usual
#endif

// If MS_CONTACT_GETCONTACTINFO returns 0 (valid), then one of the following
// types is setting telling you what type of info you received
#define CNFT_BYTE		1
#define CNFT_WORD		2
#define CNFT_DWORD		3
#define CNFT_ASCIIZ		4

// Get contact information
// wParam = not used
// lParam = (CONTACTINFO *)
// Returns 1 on failure to retrieve the info and 0 on success.  If
// sucessful, the type is set and the result is put into the associated
// member of CONTACTINFO
#define MS_CONTACT_GETCONTACTINFO	"Miranda/Contact/GetContactInfo"

#endif // M_CONTACTS_H__
