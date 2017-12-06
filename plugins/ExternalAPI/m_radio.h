#ifndef M_RADIO
#define M_RADIO

#ifndef MIID_MRADIO
#define MIID_MRADIO {0xeebc474c, 0xb0ad, 0x470f, {0x99, 0xa8, 0x9d, 0xd9, 0x21, 0x0c, 0xe2, 0x33}}
#endif

// command codes
#define MRC_STOP   0
#define MRC_PLAY   1 // lParam is radio contact handle
#define MRC_PAUSE  2
#define MRC_PREV   3
#define MRC_NEXT   4
#define MRC_STATUS 5 // lParam is RD_STATUS_* value (RD_STATUS_GET only now)
#define MRC_SEEK   6 // lParam is value in sec; -1 mean obtain current position
#define MRC_RECORD 7 // lParam is 0 - switch; 1 - on; 2 - off
#define MRC_MUTE   8

/* RD_STATUS_* constands
   [C]used as command    [E]used as event
   [-]do not use         [+]used as command and event
*/
#define RD_STATUS_NOSTATION  0   // [E] no active station found
#define RD_STATUS_PLAYING    1   // [-] media is playing
#define RD_STATUS_PAUSED     2   // [E] media is paused
#define RD_STATUS_STOPPED    3   // [E] media is stopped (only for playlists)
#define RD_STATUS_CONNECT    4   // [E] plugin try to connect to the station
#define RD_STATUS_ABORT      5   // [E] plugin want to abort while try to connect
#define RD_STATUS_GET        6   // [C] to get current status
// next is for events only +0.0.2.1
#define RD_STATUS_POSITION   107 // [E] position was changed
#define RD_STATUS_MUTED      108 // [E] Mute/Unmute command was sent
#define RD_STATUS_RECORD     109 // [E] "Record" action called
#define RD_STATUS_NEWTRACK   110 // [E] new track/station
#define RD_STATUS_NEWTAG     111 // [E] tag data changed
#define RD_STATUS_NEWSTATION 112 // [E] new station (contact)

/*
  Open radio Options, if Main Options window not opened
  wParam: 0
  lParam: 0
*/
#define MS_RADIO_SETTINGS "mRadio/Settings"
/*
  Switch 'record' mode
  wParam: not used
  lParam: 0 - switch mode; else - get record status
  Return: Current status: 1 - record is ON, 0 - OFF
*/
#define MS_RADIO_RECORD "mRadio/REC"

/*
  Set current radio volume
  wParam: volume (0-100)
  lParam: must be 0
  Return: previous value
*/
#define MS_RADIO_SETVOL "mRadio/SetVol"

/*
  Get current radio volume
  wParam: 0
  lParam: 0
  Return: volime value (negative if muted)
*/
#define MS_RADIO_GETVOL "mRadio/GetVol"

/*
  wParam,lParam = 0
*/
#define MS_RADIO_MUTE "mRadio/Mute"

/*
  wParam,lParam = 0
*/
#define MS_RADIO_QUICKOPEN "mRadio/QuickOpen"

/*
  Send command to mRadio
  wParam: command (see MRC_* constant)
  lParam: value (usually 0)
  Return: return value (now for status only)
*/
#define MS_RADIO_COMMAND "mRadio/Command"

/*
  Starting or stopping radio station
  wParam: Radio contact handle (lParam=0) or Station name
  lParam: 0 - wParam is handle, 1 - ANSI, else - unicode
*/
#define MS_RADIO_PLAYSTOP "mRadio/PlayStop"

/*
  wParam: station handle (0 - all)
  lParam: nil (through dialog, radio.ini by default) or ansi string with filename
  Return: exported stations amount
*/
#define MS_RADIO_EXPORT "mRadio/Export"

/*
  wParam: group to import radio or 0
  lParam: nil (through dialog, radio.ini by default) or ansi string with filename
  Return: imported stations amount
*/
#define MS_RADIO_IMPORT "mRadio/Import"

/*
  wParam: 0 - switch; 1 - switch on; -1 - switch off
  lParam: 0
  Return: last state (0 - was off, 1 - was on)
*/
#define MS_RADIO_EQONOFF "mRadio/EqOnOff"

/*
  wParam: 0 
  lParam: 0
  Return: 0, if cancelled, 101 - "mute", 102 - "play/pause", 103 - "stop" or station handle
*/
#define MS_RADIO_TRAYMENU "mRadio/MakeTrayMenu"

//////event/////

/*
  wParam: RD_STATUS_* (see constants)
  RD_STATUS_NEWSTATION , lParam: contact handle
  RD_STATUS_NEWTRACK   , lParam: URL (unicode)
  RD_STATUS_PAUSED     , lParam: 1 - pause, 0 - continued
  RD_STATUS_RECORD     , lParam: 0 - off,   1 - on
*/
#define ME_RADIO_STATUS "mRadio/Status"

#endif
