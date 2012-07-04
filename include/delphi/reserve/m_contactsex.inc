{
  Miranda IM: the free IM client for Microsoft  Windows

  Copyright 2000-2003 Miranda ICQ/IM project,
  all portions of this codebase are copyrighted to the people
  listed in contributors.txt.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  aLONG with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_CONTACTSEX}
{$DEFINE M_CONTACTSEX}

const
  USERINFO = 'UserInfo';

// new flags that come with userinfoex
const
  CNF_TITLE              = 18; //returns title (string)
  CNF_SECONDNAME         = 19; // returns second name (string)
  CNF_PREFIX             = 20; // returns name prefix (string)
  CNF_MYNOTES            = 21; // returns notes (string)
  CNF_LANGUAGE1          = 22; // returns first language (string)
  CNF_LANGUAGE2          = 23; // returns second language (string)
  CNF_LANGUAGE3          = 24; // returns third language (string)
  CNF_TIMEZONE           = 25; // returns timezone (WORD)
  CNF_MARITAL            = 26; // returns marital status (string)
  CNF_PARTNER            = 27; // returns partner (string)
  CNF_ORIGIN_CITY        = 28; // returns origin city (string)
  CNF_ORIGIN_STATE       = 29; // returns origin state (string)
  CNF_ORIGIN_COUNTRY     = 30; // returns origin country (string)
  CNF_STREET             = 31; // returns street (string)
  CNF_POSTAL             = 32; // returns postal code (string)
  CNF_FAX                = 34; // returns fax (string)
  CNF_CELLULAR           = 35; // returns cellular (string)
  CNF_EMAIL2             = 36; // returns second email adress (string)
  CNF_EMAIL3             = 37; // returns third email adress (string)
  CNF_COMPANY            = 38;
  CNF_COMPANY_POSITION   = 39;
  CNF_COMPANY_OCCUPATION = 40; // returns occupation (string)
  CNF_COMPANY_SUPERIOR   = 41;
  CNF_COMPANY_ASSISTENT  = 42;
  CNF_COMPANY_DEPARTMENT = 43;
  CNF_COMPANY_STREET     = 44;
  CNF_COMPANY_POSTAL     = 45;
  CNF_COMPANY_CITY       = 46;
  CNF_COMPANY_STATE      = 47;
  CNF_COMPANY_COUNTRY    = 48;
  CNF_COMPANY_PHONE      = 49;
  CNF_COMPANY_FAX        = 50;
  CNF_COMPANY_CELLULAR   = 51;
  CNF_COMPANY_EMAIL      = 52;
  CNF_COMPANY_EMAIL2     = 53;
  CNF_COMPANY_EMAIL3     = 54;
  CNF_COMPANY_HOMEPAGE   = 55;

  { UserInfo/GetContactInfo    v0.1.0.4+
  This is more or less an extended version of MS_UTILS_GETCONTACTINFO. Same syntax and parameters
  except the dwFlag member of CONTACTINFO. It can also have the values above. CNF_DISPLAY and CNF_DISPLAYNC
  are ignored.
  }
  MS_UINFOEX_GETCONTACTINFO = 'UserInfo/GetContactInfo';

  { UserInfo/FreeContactInfo    v0.1.0.4+
  Strings returned by MS_UINFOEX_GETCONTACTINFO (and btw. MS_UTILS_GETCONTACTINFO too) are pointers to
  dynamically allocated portions of memory and therefor should be freed after use by miranda's own
  version of free that can be accessed via MS_SYSTEM_GET_MMI. This function is an alternitive. It does
  all the mmi stuff for you and cleanly frees the memory. An helper macro (DBFreeContactInfoString) frees
  an string got by MS_UINFOEX_GETCONTACTINFO or MS_UTILS_GETCONTACTINFO service explicitly.
  wParam=(LPSTR)pszVal - pointer to a string got by DBGetCustomContactInfoString to delete directly
  lParam=(CONTACTINFO )ci - pointer to a CONTACTINFO whose string value is to be freed
  }
  MS_UINFOEX_FREECONTACTINFO = 'UserInfo/FreeContactInfo';
{$ENDIF}
