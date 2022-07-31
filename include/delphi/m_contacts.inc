{
Miranda IM

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
}

{$IFNDEF M_CONTACTS}
{$DEFINE M_CONTACTS}

const

// CNF_* Types of information you can retrieve by setting the dwFlag in CONTACTINFO

  CNF_FIRSTNAME  = 1;  // returns first name (string)
  CNF_LASTNAME   = 2;  // returns last name (string)
  CNF_NICK       = 3;  // returns nick name (string)
  CNF_CUSTOMNICK = 4;  // returns custom nick name, clist name (string)
  CNF_EMAIL      = 5;  // returns email (string)
  CNF_CITY       = 6;  // returns city (string)
  CNF_STATE      = 7;  // returns state (string)
  CNF_COUNTRY    = 8;  // returns country (string)
  CNF_PHONE      = 9;  // returns phone (string)
  CNF_HOMEPAGE   = 10; // returns homepage (string)
  CNF_ABOUT      = 11; // returns about info (string)
  CNF_GENDER     = 12; // returns gender (byte,'M','F' character)
  CNF_AGE        = 13; // returns age (byte, 0==unspecified)
  CNF_FIRSTLAST  = 14; // returns first name + last name (string)
  CNF_UNIQUEID   = 15; // returns uniqueid, protocol username (must check type for type of return)
  CNF_FAX        = 18; // returns fax (string)
  CNF_CELLULAR   = 19; // returns cellular (string)
  CNF_TIMEZONE   = 20; // returns timezone (string)
  CNF_MYNOTES    = 21; // returns user specified notes (string)
  CNF_BIRTHDAY   = 22; // returns birthday day of month (byte)
  CNF_BIRTHMONTH = 23; // returns birthday month (byte)
  CNF_BIRTHYEAR  = 24; // returns birthday year (word)
  CNF_STREET     = 25; // returns street (string)
  CNF_ZIP        = 26; // returns zip code (string)
  CNF_LANGUAGE1  = 27; // returns language1 (string)
  CNF_LANGUAGE2  = 28; // returns language2 (string)
  CNF_LANGUAGE3  = 29; // returns language3 (string)
  CNF_CONAME     = 30; // returns company name (string)
  CNF_CODEPT     = 31; // returns company department (string)
  CNF_COPOSITION = 32; // returns company position (string)
  CNF_COSTREET   = 33; // returns company street (string)
  CNF_COCITY     = 34; // returns company city (string)
  CNF_COSTATE    = 35; // returns company state (string)
  CNF_COZIP      = 36; // returns company zip code (string)
  CNF_COCOUNTRY  = 37; // returns company country (string)
  CNF_COHOMEPAGE = 38; // returns company homepage (string)
  CNF_DISPLAYUID = 39; // returns uniqueid to display in interface (must check type for type of return)

// Special types
// Return the custom name using the name order setting
// IMPORTANT: When using CNF_DISPLAY you MUST free the string returned
// You must **NOT** do this from your version of free() you have to use Miranda's free()
// you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
  CNF_DISPLAY = 16;
// Same as CNF_DISPLAY except the custom handle is not used
// IMPORTANT: When using CNF_DISPLAYNC you MUST free the string returned
// You must **NOT** do this from your version of free() you have to use Miranda's free()
// you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
  CNF_DISPLAYNC = 17;

// result must be freed using mir_free
function Contact_GetInfo(iType:Integer; hContact:TMCONTACT; proto:PAnsiChar=nil) : PWideChar; stdcall; external AppDLL index 261;

{$ENDIF}
