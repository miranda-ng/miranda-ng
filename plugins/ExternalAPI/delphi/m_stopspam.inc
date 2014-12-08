{
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2004-009 Roman Miklashevsky, A. Petkevich, Kosh&chka, persei

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

{$IFNDEF M_STOPSPAM}
{$DEFINE M_STOPSPAM}

const
  CS_NOTPASSED = 0;
  CS_PASSED    = 1;

//check is contact pass the stopspam
//wParam=(HANDLE)hContact
//lParam=0
//returns a "Contact State" flag
  MS_STOPSPAM_CONTACTPASSED:PAnsiChar = 'StopSpam/IsContactPassed';

//remove all temporary contacts from db
//wParam=0
//lParam=0
//returns 0
  MS_STOPSPAM_REMTEMPCONTACTS:PAnsiChar = 'StopSpam/RemoveTempContacts';

{$ENDIF}
