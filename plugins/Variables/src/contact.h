/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is mir_free software; you can redistribute it and/or modify
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

#define STR_FIRSTNAME   "first"
#define STR_LASTNAME    "last"
#define STR_NICK        "nick"
#define STR_CUSTOMNICK  "cnick"
#define STR_EMAIL       "email"
#define STR_CITY        "city"
#define STR_STATE       "state"
#define STR_COUNTRY     "country"
#define STR_PHONE       "phone"
#define STR_HOMEPAGE    "homepage"
#define STR_ABOUT       "about"
#define STR_GENDER      "gender"
#define STR_AGE         "age"
#define STR_FIRSTLAST   "firstlast"
#define STR_UNIQUEID    "id"
#define STR_DISPLAY     "display"

#define STR_FAX         "fax"
#define STR_CELLULAR    "cellular"
#define STR_TIMEZONE    "timezone"
#define STR_MYNOTES     "mynotes"
#define STR_BIRTHDAY    "bday"
#define STR_BIRTHMONTH  "bmonth"
#define STR_BIRTHYEAR   "byear"
#define STR_STREET      "street"
#define STR_ZIP         "zip"
#define STR_LANGUAGE1   "lang1"
#define STR_LANGUAGE2   "lang2"
#define STR_LANGUAGE3   "lang3"
#define STR_CONAME      "coname"
#define STR_CODEPT      "codept"
#define STR_COPOSITION  "copos"
#define STR_COSTREET    "costreet"
#define STR_COCITY      "cocity"
#define STR_COSTATE     "costate"
#define STR_COZIP       "cozip"
#define STR_COCOUNTRY   "cocountry"
#define STR_COHOMEPAGE  "cohomepage"

#define STR_ACCOUNT     "account"
#define STR_PROTOCOL	   "protocol"
#define STR_STATUS      "status"
#define STR_INTERNALIP  "intip"
#define STR_EXTERNALIP  "extip"
#define STR_GROUP       "group"
#define STR_PROTOID     "protoid"

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
  HANDLE *hContacts;  // (output) Array of contacts found.
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

TCHAR *encodeContactToString(HANDLE hContact);
HANDLE *decodeContactFromString(TCHAR *tszContact);
