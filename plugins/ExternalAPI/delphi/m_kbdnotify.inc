{
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

{$IFNDEF M_KBDNOTIFY}
{$DEFINE M_KBDNOTIFY}

const
//Enables all notifications (for use by BossKey)
//wParam=0
//lParam=0
//returns 0
  MS_KBDNOTIFY_ENABLE:PAnsiChar = 'KeyboardNotify/Enable';

//Disables all notifications (for use by BossKey)
//wParam=0
//lParam=0
//returns 0
  MS_KBDNOTIFY_DISABLE:PAnsiChar = 'KeyboardNotify/Disable';

//Makes the flashing begin
//wParam=(unsigned int)eventCount
//lParam=(AnsiChar *)szFlashingSequence or NULL if you want the plugin to use current settings
//returns 0
  MS_KBDNOTIFY_STARTBLINK:PAnsiChar = 'KeyboardNotify/StartBlinking';

//Receives the number of events that were opened (usuful for the 'until events opened' setting)
//wParam=(unsigned int)eventCount
//lParam=0
//returns 0
  MS_KBDNOTIFY_EVENTSOPENED:PAnsiChar = 'KeyboardNotify/EventsWereOpened';


//Informs if the flashing is active
//wParam=0
//lParam=0
//returns 0 if the flashing is inactive or a pointer to the string representing the sequence being used 
  MS_KBDNOTIFY_FLASHINGACTIVE:PAnsiChar = 'KeyboardNotify/IsFlashingActive';


//Normalizes the flashing sequence informed
//wParam=0
//lParam=(AnsiChar *)szFlashingSequence <- it is rewritten
//returns a pointer to the string representing the sequence normalized (which is in fact lParam)
  MS_KBDNOTIFY_NORMALSEQUENCE:PAnsiChar = 'KeyboardNotify/NormalizeSequence';

{$ENDIF}
