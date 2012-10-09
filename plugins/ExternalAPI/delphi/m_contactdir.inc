{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project, 
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
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_CONTACTDIR}
{$DEFINE M_CONTACTDIR}

{ Contactdir module was created on 2005/05/17, 0.4.0.1 

   -- How you use this module as a protocol --
   
   On Load() Register your protocol with the setting name that stores unique IDs, example:
   
 if ( ContactDir_SupportExists() ) g_Atom=ContactDir_Register("ICQ", "UIN");
 
 This will register your protocol and walk the database looking for all contacts on PROTOCOL_NAME which have
 a "UIN" setting and store it in memory (converting to a string as needed) You of course have to 
 provide fallback if the services don't exist, it's an idea to keep existing code for that.
 
 -
 
 When you add a new contact via MS_DB_CONTACT_ADD, you must register it with your protocol atom too, via 
 ContactDir_AddContact(atom, "UIN #", hContact) and when it is deleted ContactDir_RemoveContact(atom, "UIN #")
 
 -
 
 To find a contact, use ContactDir_Lookup(atom, "ICQ #") which will return the hContact.
}

type
  PCONTACTDIRECTORYDESCRIPTOR = ^TCONTACTDIRECTORYDESCRIPTOR;
  TCONTACTDIRECTORYDESCRIPTOR = record
    cbSize   :int;
    szProto  :PAnsiChar;
    szSetting:PAnsiChar;
    atom     :THANDLE; // out arg
  end;

{
 wParam: 0
 lParam: (LPARAM) &CONTACTDIRECTORYDESCRIPTOR;
 Affect: Register a given protocol and it's setting name which contains the unique key entry. e.g. ("ICQ", "UIN")
  and return a HANDLE for use with other lookup services.
 Returns: 0 on success, non zero on failure -- a returned handle is in .atom
 Note: The directory will convert dword values into string representations but will not do this for bytes or words
  used as IDs -- the protocol has to convert the IDs itself (:
 Note: See ContactDir_Register() for a quicker way.
 *** WARNING ***: This service does not expect the given module name to have registered as a protocol module, it
  completely bypasses this information.
 Version: 0.4.0.1 (2005/05/17+)
}
const
  MS_CONTACTDIR_REGISTER = 'ContactDir/Register';

type
  PCONTACTDIRECTORYLOOKUP = ^TCONTACTDIRECTORYLOOKUP;
  TCONTACTDIRECTORYLOOKUP = record
    cbSize  :int;
    atom    :THANDLE; // Atom handle from MS_CONTACTDIR_REGISTER
    szID    :PAnsiChar;   // in: value you wish to find (makes its own copy if needed)
    hContact:THANDLE; // out: hContact associated with szID, if any.
  end;

{
 wParam: 0
 lParam: (LPARAM) &CONTACTDIRECTORYLOOKUP;
 Affect: Given an atom and string ID, will find the associated DB hContact value
 Returns: 0 on success, non zero on failure
 Version: 0.4.0.1 (2005/05/17+)
 Note: ContactDir_Lookup() helper macro might be of use.
}
const
  MS_CONTACTDIR_LOOKUP = 'ContactDir/Lookup';

{
 wParam: 0
 lParam: (LPARAM)&CONTACTDIRECTORYLOOKUP;
 Affect: Add a contact to a protocol atom association.
 Returns: 0 on success, non zero on failure
 Version: 0.4.0.1 (2005/05/17+)
 Note: You must call this when you create a contact with MS_DB_CONTACT_ADD, see  ContactDir_AddContact()
}
  MS_CONTACTDIR_ADDCONTACT = 'ContactDir/AddContact';

{
 wParam: 0
 lParam: (LPARAM)&CONTACTDIRECTORYLOOKUP;
 Affect: Remove a contact to a protocol atom association.
 Returns: 0 on success, non zero on failure
 Version: 0.4.0.1 (2005/05/17+)
 Note: see ContactDir_RemoveContact()
}
  MS_CONTACTDIR_REMOVECONTACT = 'ContactDir/RemoveContact';

(*
/* -- Helper functions -- */

static int ContactDir_SupportExists(void)
{
 return ServiceExists(MS_CONTACTDIR_REGISTER);
}

// Only take as valid if ContactDir_SupportExists() returns true.
static HANDLE ContactDir_Register(AnsiChar * szProto, AnsiChar * szSetting)
{
 CONTACTDIRECTORYDESCRIPTOR cd;
 cd.cbSize=sizeof(CONTACTDIRECTORYDESCRIPTOR);
 cd.szProto=szProto;
 cd.szSetting=szSetting;
 cd.atom=NULL;
 CallService(MS_CONTACTDIR_REGISTER, 0, (LPARAM)&cd);
 return cd.atom;
}

static __inline HANDLE ContactDir_Lookup(HANDLE atom, AnsiChar * szID) 
{
 CONTACTDIRECTORYLOOKUP f;
 f.cbSize=sizeof(f);
 f.atom=atom;
 f.szID=szID;
 f.hContact=NULL;
 CallService(MS_CONTACTDIR_LOOKUP, 0, (LPARAM)&f);
 return f.hContact;
}

static __inline void ContactDir_AddContact(HANDLE atom, AnsiChar * szID, HANDLE hContact)
{
 CONTACTDIRECTORYLOOKUP c = {0};
 c.cbSize=sizeof(CONTACTDIRECTORYLOOKUP);  
 c.atom=atom;
 c.szID=szID;
 c.hContact=hContact;
 CallService(MS_CONTACTDIR_ADDCONTACT, 0, (LPARAM)&c);
}

static __inline void ContactDir_RemoveContact(HANDLE atom, AnsiChar * szID)
{
 CONTACTDIRECTORYLOOKUP c = {0};
 c.cbSize=sizeof(CONTACTDIRECTORYLOOKUP);  
 c.atom=atom;
 c.szID=szID;
 c.hContact=NULL;
 CallService(MS_CONTACTDIR_REMOVECONTACT, 0, (LPARAM)&c);
}
*)
{$ENDIF}