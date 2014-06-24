(*
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
*)

type

    PCONTACTINFO = ^TCONTACTINFO;
	TCONTACTINFO = record
		cbSize: int;
		dwFlag: Byte;
		hContact: THandle;
		szProto: PChar;
		type_: Byte;
		retval: record (* in C this is a nameless union *)
			case longint of
            0: (bVal: Byte);
            1: (wVal: WORD);
            2: (dVal: DWORD);
            3: (pszVal: PChar);
            4: (cchVal: Word);
			end;
	end;

const

// CNF_* Types of information you can retrieve by setting the dwFlag in CONTACTINFO

	CNF_FIRSTNAME	= 1;  // returns first name (string)
	CNF_LASTNAME	= 2;  // returns last name (string)
	CNF_NICK		= 3;  // returns nick name (string)
	CNF_CUSTOMNICK	= 4;  // returns custom nick name, clist name (string)
	CNF_EMAIL		= 5;  // returns email (string)
	CNF_CITY		= 6;  // returns city (string)
	CNF_STATE		= 7;  // returns state (string)
	CNF_COUNTRY		= 8;  // returns country (string)
	CNF_PHONE		= 9;  // returns phone (string)
	CNF_HOMEPAGE	= 10; // returns homepage (string)
	CNF_ABOUT		= 11; // returns about info (string)
	CNF_GENDER		= 12; // returns gender (byte,'M','F' character)
	CNF_AGE			= 13; // returns age (byte, 0==unspecified)
	CNF_FIRSTLAST	= 14; // returns first name + last name (string)
	CNF_UNIQUEID	= 15; // returns uniqueid, protocol username (must check type for type of return)

// Special types
// Return the custom name using the name order setting
// IMPORTANT: When using CNF_DISPLAY you MUST free the string returned
// You must **NOT** do this from your version of free() you have to use Miranda's free()
// you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
	CNF_DISPLAY		= 16;
// Same as CNF_DISPLAY except the custom handle is not used
// IMPORTANT: When using CNF_DISPLAYNC you MUST free the string returned
// You must **NOT** do this from your version of free() you have to use Miranda's free()
// you can get a function pointer to Miranda's free() via MS_SYSTEM_GET_MMI, see m_system.h
	CNF_DISPLAYNC	= 17;

// If MS_CONTACT_GETCONTACTINFO returns 0 (valid), then one of the following
// types is setting telling you what type of info you received
	CNFT_BYTE		= 1;
	CNFT_WORD		= 2;
	CNFT_DWORD		= 3;
	CNFT_ASCIIZ		= 4;

	{
		wParam : not used
		lParam : Pointer to an initialised TCONTACTINFO structure
		affects: Get contact information
		returns: Zero on success, non zero on failure.
		notes  : If successful, the type is set and the result is put into the associated member of TCONTACTINFO
	}
	MS_CONTACT_GETCONTACTINFO	= 'Miranda/Contact/GetContactInfo';
