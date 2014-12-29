{$IFNDEF M_RADIO}
{$DEFINE M_RADIO}
{command codes}

// defined in interfaces.inc
//const MIID_MRADIO:MUUID='{EEBC474C-B0AD-470F-99A8-9DD9210CE233}';

const
  MRC_STOP   = 0;
  MRC_PLAY   = 1; // lParam is radio contact handle
  MRC_PAUSE  = 2;
  MRC_PREV   = 3;
  MRC_NEXT   = 4;
  MRC_STATUS = 5; // lParam is RD_STATUS_* value (RD_STATUS_GET only now)
  MRC_SEEK   = 6; // lParam is value in sec; -1 mean obtain current position
  MRC_RECORD = 7; // lParam is 0 - switch; 1 - on; 2 - off
  MRC_MUTE   = 8;

const
  // Plugin status (result of RD_STATUS_GET)
  RD_STATUS_NOSTATION  = 0;   // no active station found
  RD_STATUS_PLAYING    = 1;   // media is playing
  RD_STATUS_PAUSED     = 2;   // media is paused
  RD_STATUS_STOPPED    = 3;   // media is stopped (only for playlists)
  RD_STATUS_CONNECT    = 4;   // plugin try to connect to the station
  RD_STATUS_ABORT      = 5;   // plugin want to abort while try to connect
  // next is for events only
  RD_STATUS_POSITION   = 107; // position was changed
  RD_STATUS_MUTED      = 108; // Mute/Unmute command was sent
  RD_STATUS_RECORD     = 109; // "Record" action called
  RD_STATUS_NEWTRACK   = 110; // new track/station
  RD_STATUS_NEWTAG     = 111; // tag data changed
  RD_STATUS_NEWSTATION = 112; // new station (contact)
  // next command is for users
  RD_STATUS_GET        = 6;   // to get current status

const
{
  Open radio Options, if Main Options window not opened
  wParam: 0
  lParam: 0
}
  MS_RADIO_SETTINGS:PAnsiChar = 'mRadio/Settings';
{
  Switch 'record' mode
  wParam: not used
  lParam: 0 - switch mode; else - get record status
  Return: Current status: 1 - record is ON, 0 - OFF
}
  MS_RADIO_RECORD:PAnsiChar = 'mRadio/REC';

{
  Set current radio volume
  wParam: volume (0-100)
  lParam: must be 0
  Return: previous value
}
  MS_RADIO_SETVOL:PAnsiChar = 'mRadio/SetVol';

{
  Get current radio volume
  wParam: 0
  lParam: 0
  Return: volime value (negative if muted)
}
  MS_RADIO_GETVOL:PAnsiChar = 'mRadio/GetVol';

{
  wParam,lParam = 0
}
  MS_RADIO_MUTE:PAnsiChar = 'mRadio/Mute';

{
  wParam,lParam = 0
}
  MS_RADIO_QUICKOPEN:PAnsiChar = 'mRadio/QuickOpen';

{
  Send command to mRadio
  wParam: command (see MRC_* constant)
  lParam: value (usually 0)
  Return: return value (now for status only)
}
  MS_RADIO_COMMAND:PAnsiChar = 'mRadio/Command';

{
  Starting or stopping radio station
  wParam: Radio contact handle (lParam=0) or Station name
  lParam: 0 - wParam is handle, 1 - ANSI, else - unicode
}
  MS_RADIO_PLAYSTOP:PAnsiChar = 'mRadio/PlayStop';

{
  wParam: station handle (0 - all)
  lParam: nil (through dialog, radio.ini by default) or ansi string with filename
  Return: exported stations amount
}
  MS_RADIO_EXPORT:PAnsiChar = 'mRadio/Export';

{
  wParam: group to import radio or 0
  lParam: nil (through dialog, radio.ini by default) or ansi string with filename
  Return: imported stations amount
}
  MS_RADIO_IMPORT:PAnsiChar = 'mRadio/Import';

{
  wParam: 0 - switch; 1 - switch on; -1 - switch off
  lParam: 0
  Return: last state (0 - was off, 1 - was on)
}
  MS_RADIO_EQONOFF:PAnsiChar = 'mRadio/EqOnOff';

{
  wParam: 0 
  lParam: 0
  Return: 0, if cancelled, 101 - "mute", 102 - "play/pause", 103 - "stop" or station handle
}
  MS_RADIO_TRAYMENU:PAnsiChar = 'mRadio/MakeTrayMenu';

{
  wParam: RD_STATUS_* constants
  lParam: argument
    RD_STATUS_NEWSTATION - contact handle
    RD_STATUS_NEWTRACK   - URL (unicode)
    RD_STATUS_PAUSED     - 1 - pause, 0 - continued
    RD_STATUS_RECORD     -,0 - off,   1 - on
}
  ME_RADIO_STATUS:PAnsiChar = 'mRadio/Status';

{$ENDIF}
