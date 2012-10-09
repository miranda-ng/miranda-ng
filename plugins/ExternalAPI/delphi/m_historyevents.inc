{
Copyright (C) 2006 Ricardo Pescuma Domenecci

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


{$IFNDEF M_HISTORYEVENTS}
{$DEFINE M_HISTORYEVENTS}

const
  HISTORYEVENTS_FORMAT_CHAR      = 1;
  HISTORYEVENTS_FORMAT_WCHAR     = 2;
  HISTORYEVENTS_FORMAT_RICH_TEXT = 4;
  HISTORYEVENTS_FORMAT_HTML      = 8;

const
  HISTORYEVENTS_FLAG_DEFAULT                    = 1 shl 0; // Is a miranda core event type
  HISTORYEVENTS_FLAG_SHOW_IM_SRMM               = 1 shl 1; // If this event has to be shown in srmm dialog
  HISTORYEVENTS_FLAG_USE_SENT_FLAG              = 1 shl 2; // Means that it can be a sent or received and uses DBEF_SENT to mark that
  HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE = 1 shl 3; // Means that who is drawing this should draw the contact name before the text
  HISTORYEVENTS_FLAG_ONLY_LOG_IF_SRMM_OPEN      = 1 shl 4; // If this event will be logged only if the message window is open
  HISTORYEVENTS_FLAG_FLASH_MSG_WINDOW           = 1 shl 5; // If this event will trigger the openning/flashing of the message window

  HISTORYEVENTS_REGISTERED_IN_ICOLIB            = 9 shl 16; // If the icon is a name already registered in icolib

  HISTORYEVENTS_FLAG_KEEP_ONE_YEAR              = 1 shl 8; // By default store in db for 1 year
  HISTORYEVENTS_FLAG_KEEP_SIX_MONTHS            = 2 shl 8; // By default store in db for 6 months
  HISTORYEVENTS_FLAG_KEEP_ONE_MONTH             = 3 shl 8; // By default store in db for 1 month
  HISTORYEVENTS_FLAG_KEEP_ONE_WEEK              = 4 shl 8; // By default store in db for 1 week
  HISTORYEVENTS_FLAG_KEEP_ONE_DAY               = 5 shl 8; // By default store in db for 1 day
  HISTORYEVENTS_FLAG_KEEP_FOR_SRMM              = 6 shl 8; // By default store in db only enought for message log
  HISTORYEVENTS_FLAG_KEEP_MAX_TEN               = 7 shl 8; // By default store in db max 10 entries
  HISTORYEVENTS_FLAG_KEEP_MAX_HUNDRED           = 8 shl 8; // By default store in db for 100 entries
  HISTORYEVENTS_FLAG_KEEP_DONT                  = 9 shl 8; // By default don't store in db (aka ignore it)


// This function must be implemented by subscribers. It must return a pointer or NULL
// to say it can't handle the text
type 
  fGetHistoryEventText = function(hContact:THANDLE; hDbEvent:THANDLE;
                                  dbe:PDBEVENTINFO; format:int):pointer; cdecl;

type
  HISTORY_EVENT_HANDLER = record
    cbSize     :int;
    module     :PAnsiChar;
    name       :PAnsiChar; // Internal event name
    description:PAnsiChar; // Will be translated. When retrieving it is already translated
    eventType  :word;      // The event type it can handle
    defaultIcon:HICON;     // PAnsiChar= icon name if HISTORYEVENTS_REGISTERED_IN_ICOLIB is set.
                           // Always use this one when retrieving
    supports   :int;       // What kind of return is supported - or of HISTORYEVENTS_FORMAT_*
    flags      :int;       // or of HISTORYEVENTS_FLAG_*
    pfGetHistoryEventText:fGetHistoryEventText; // NULL to use default get text (similar to message, without extra format)

 // Aditional data if wants to use add to history services
    templates:^PAnsiChar; // Each entry is: "Name\nDefault\n%var%\tDescription\n%var%\tDescription\n%var%\tDescription"
    numTemplates:int;
  end;
const
{
  Get the number of registered events

  wParam: ignored
  lParam: ignored
  Return: The number of events registered with the plugin
}
  MS_HISTORYEVENTS_GET_COUNT:PAnsiChar = 'HistoryEvents/GetCount';

{
  Get an event by number or by type. 
  To retrieve by number, pass -1 as type. To retrieve by type, pass -1 as number.

  wParam: (int) event number
  lParam: (int) event type
  Return: (const HISTORY_EVENT_HANDLER *) if the event exists, NULL  otherwise. Don't change the
   returned strunc: it is a pointer to the internall struct.
}
  MS_HISTORYEVENTS_GET_EVENT:PAnsiChar = 'HistoryEvents/GetEvent';

{
  Register a plugin that can handle an event type. This must be called during the call to the
  Load function of the plugin. In ModulesLoaded callback all plugins have to be already
  registered, so srmm and history modules can query then.

  wParam: HISTORY_EVENT_HANDLER *
  lParam: ignored
  Return: 0 for success
}
  MS_HISTORYEVENTS_REGISTER:PAnsiChar = 'HistoryEvents/Register';


type
  HISTORY_EVENT_PARAM = record
    cbSize  :int;
    hDbEvent:THANDLE;
    dbe     :PDBEVENTINFO; // Optional
    format  :int;   // one of HISTORYEVENTS_FORMAT_*
  end;

const
{
  Check if an event can be handled by any subscribers

  wParam: WORD - event type
  lParam: ignored
  Return: BOOL 
}
  MS_HISTORYEVENTS_CAN_HANDLE:PAnsiChar = 'HistoryEvents/CanHandle';

{
  Get the icon for a history event type

  wParam: WORD - event type
  lParam: ignored
  Return: HICON - after use free with MS_HISTORYEVENTS_RELEASE_ICON
}
  MS_HISTORYEVENTS_GET_ICON:PAnsiChar = 'HistoryEvents/GetIcon';

{
  Get the flags for a history event type

  wParam: WORD - event type
  lParam: ignored
  Return: int - or of HISTORYEVENTS_FLAG_* or -1 if error
}
  MS_HISTORYEVENTS_GET_FLAGS:PAnsiChar = 'HistoryEvents/GetFlags';

{
  Release the icon for a history event type. This is really just a forward to icolib

  wParam: HICON
  lParam: ignored
}
  MS_HISTORYEVENTS_RELEASE_ICON:PAnsiChar = 'Skin2/Icons/ReleaseIcon';

{
  Get the text for a history event type

  wParam: HISTORY_EVENT_PARAM *
  lParam: ignored
  Return: char * or wchar * depending on sent flags. Free with mir_free or MS_HISTORYEVENTS_RELEASE_TEXT
}
  MS_HISTORYEVENTS_GET_TEXT:PAnsiChar = 'HistoryEvents/GetText';

{
  Release the text for a history event type. Internally is just a call to mir_free

  wParam: char * or wchar * 
  lParam: ignored
}
  MS_HISTORYEVENTS_RELEASE_TEXT:PAnsiChar = 'HistoryEvents/ReleaseText';


type
  HISTORY_EVENT_ADD = record
    cbSize            :int;
    hContact          :THANDLE;
    eventType         :word;
    templateNum       :int;
    variables         :^PTCHAR; // TCHAR **variables
    numVariables      :int;
    additionalData    :pbyte;
    additionalDataSize:int;
    flags             :int;    // Flags for the event type
  end;

const
{
  Add an registered event to the history. This is a helper service

  wParam: HISTORY_EVENT_ADD
  lParam: ignored
  Return: HANDLE to the db event
}
  MS_HISTORYEVENTS_ADD_TO_HISTORY:PAnsiChar = 'HistoryEvents/AddToHistory';

{
  Check if a template is enabled

  wParam: event type
  lParam: template num
  Return: TRUE or FALSE
}
  MS_HISTORYEVENTS_IS_ENABLED_TEMPLATE:PAnsiChar = 'HistoryEvents/IsEnabledTemplate';

{$ENDIF}
