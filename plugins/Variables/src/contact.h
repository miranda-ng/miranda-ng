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

#define STR_FIRSTNAME   _T("first")
#define STR_LASTNAME    _T("last")
#define STR_NICK        _T("nick")
#define STR_CUSTOMNICK  _T("cnick")
#define STR_EMAIL       _T("email")
#define STR_CITY        _T("city")
#define STR_STATE       _T("state")
#define STR_COUNTRY     _T("country")
#define STR_PHONE       _T("phone")
#define STR_HOMEPAGE    _T("homepage")
#define STR_ABOUT       _T("about")
#define STR_GENDER      _T("gender")
#define STR_AGE         _T("age")
#define STR_FIRSTLAST   _T("firstlast")
#define STR_UNIQUEID    _T("id")
#define STR_DISPLAY     _T("display")

#define STR_FAX         _T("fax")
#define STR_CELLULAR    _T("cellular")
#define STR_TIMEZONE    _T("timezone")
#define STR_MYNOTES     _T("mynotes")
#define STR_BIRTHDAY    _T("bday")
#define STR_BIRTHMONTH  _T("bmonth")
#define STR_BIRTHYEAR   _T("byear")
#define STR_STREET      _T("street")
#define STR_ZIP         _T("zip")
#define STR_LANGUAGE1   _T("lang1")
#define STR_LANGUAGE2   _T("lang2")
#define STR_LANGUAGE3   _T("lang3")
#define STR_CONAME      _T("coname")
#define STR_CODEPT      _T("codept")
#define STR_COPOSITION  _T("copos")
#define STR_COSTREET    _T("costreet")
#define STR_COCITY      _T("cocity")
#define STR_COSTATE     _T("costate")
#define STR_COZIP       _T("cozip")
#define STR_COCOUNTRY   _T("cocountry")
#define STR_COHOMEPAGE  _T("cohomepage")

#define STR_ACCOUNT     _T("account")
#define STR_PROTOCOL	_T("protocol")
#define STR_STATUS      _T("status")
#define STR_INTERNALIP  _T("intip")
#define STR_EXTERNALIP  _T("extip")
#define STR_GROUP       _T("group")
#define STR_PROTOID     _T("protoid")

#define CCNF_ACCOUNT    51 // CUSTOM, returns contact's account name (0.8.0+)
#define CCNF_PROTOCOL   50 // CUSTOM, returns the contact's protocol (human-readable)
#define CCNF_STATUS     49 // CUSTOM, returns status mode description
#define CCNF_INTERNALIP 48	// CUSTOM, returns the contact's internal IP
#define CCNF_EXTERNALIP 47	// CUSTOM, returns the contact's external IP
#define CCNF_GROUP      46 // CUSTOM, returns group name
#define CCNF_PROTOID    45 // CUSTOM, returns protocol ID instead of name

#define PROTOID_HANDLE	"_HANDLE_"

// Note: The hContacts array needs to be freed after use using mir_free

typedef struct {
  int cbSize;  // Set this to sizeof(CONTACTSINFO).
  union {
    char *szContact;  // String to search for, e.g. last name (can't be NULL).
    WCHAR *wszContact;
    TCHAR *tszContact;
  };
  MCONTACT *hContacts;  // (output) Array of contacts found.
  DWORD flags;  // Contact details that will be matched with the search
                // string (flags can be combined).
} CONTACTSINFO;

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
#define CI_CNFINFO	    0x40000000  // Searches one of the CNF_* flags (set
                                    // flags to CI_CNFINFO|CNF_X), only one
                                    // CNF_ type possible
#define CI_UNICODE      0x80000000  // tszContact is a unicode string
                                    // (WCHAR*).

#if defined(UNICODE) || defined(_UNICODE)
#define CI_TCHAR    CI_UNICODE  // Strings in structure are TCHAR*.
#else
#define CI_TCHAR    0
#endif

TCHAR *encodeContactToString(MCONTACT hContact);
MCONTACT decodeContactFromString(TCHAR *tszContact);
