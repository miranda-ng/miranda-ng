{
 Copyright (C) 2009 Ricardo Pescuma Domenecci

 This is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this file; see the file license.txt.  If
 not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
}

{$IFNDEF M_EXTRAICONS}
{$DEFINE M_EXTRAICONS}

const
  EXTRAICON_TYPE_CALLBACK = 0; // Similar to old clist callbacks, it fires 2 notifications
  EXTRAICON_TYPE_ICOLIB   = 1; // This extra icon will use only icons registered with icolib.
        // No callbacks needed. Just call MS_EXTRAICON_SET_ICON passing the name of the
        // extraicon to set one.

type
  TEXTRAICON_INFO = record
    cbSize:int;
    _type :int;            // One of EXTRAICON_TYPE_*
    name       :PAnsiChar; // Internal name. More than one plugin can register extra icons
                           // with the same name if both have the same type. In this case,
                           // both will be handled as one. This is usefull for ex for extra
                           // status, where icq and jabber can share the same slot.
                           // If the types are different the second one will be denied.
    description:PAnsiChar; // [Translated by plugin] Description to be used in GUI
    descIcon   :PAnsiChar; // [Optional] Name of an icon registered with icolib to be used in GUI.

    // If type == EXTRAICON_TYPE_CALLBACK this two must be set

    // Callback to add icons to clist, calling MS_CLIST_EXTRA_ADD_ICON
    // wParam=lParam=0
    RebuildIcons:TMIRANDAHOOK;

    // Callback to set the icon to clist, calling MS_CLIST_EXTRA_SET_ICON or MS_EXTRAICON_SET_ICON
    // wParam = HANDLE hContact
    // lParam = int slot
    ApplyIcon:TMIRANDAHOOK;

    // Other optional callbacks

    // [Optional] Callback called when extra icon was clicked
    // wParam = HANDLE hContact
    // lParam = int slot
    // param = onClickParam
    OnClick:TMIRANDAHOOKPARAM;

    onClickParam:LPARAM;
  end;

const
// Register an extra icon
// wParam = (EXTRAICON_INFO *) Extra icon info
// lParam = 0
// Return: (HANDLE) id of extra icon on success, 0 on error
  MS_EXTRAICON_REGISTER:PAnsiChar = 'ExtraIcon/Register';

type
  TEXTRAICON = record
    cbSize    :int;
    hExtraIcon:THANDLE; // Value returned by MS_EXTRAICON_REGISTER
    hContact  :THANDLE; // Contact to set the extra icon
    // The icon to be set. This depends on the type of the extra icon:
	  case boolean of
      false:(hImage :THANDLE);   // Value returned by MS_CLIST_EXTRA_ADD_ICON (if EXTRAICON_TYPE_CALLBACK)
      true :(icoName:PAnsiChar); // Name of the icon registered with icolib (if EXTRAICON_TYPE_ICOLIB)
  end;

const
// Set an extra icon icon
// wParam = (EXTRAICON *) Extra icon
// Return: 0 on success
  MS_EXTRAICON_SET_ICON:PAnsiChar = 'ExtraIcon/SetIcon';

{$ENDIF}
