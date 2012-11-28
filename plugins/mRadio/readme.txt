Description:
------------
This plugin plays and records Internet radio streams. Also local media files can be played.

This is not 'clear' mod of mRadio plugin from Bankrut.
So, some things can be different or even uncomplete.

Notes:
------
1) Supported formats
BASS library support plugins, so format list can be expanded.
BASS.dll give support for MP3 and OGG formats
BASSWMA.dll give WMA/ASX stream support
PLS (ansi), M3U (old and new format, ansi), M3U8 (old and new format utf8) supported.
This is plugin processing and now all strings converted to ansi.

2) Tooltip templates
Tipper template:
Common status message (not only mRadio): %sys:status_msg%
also you can check CList/StatusMsg or StationURL values.

mToolTip templates:
[Station_Codec]
DBSub=mRadio
DBSet=ActiveCodec

[Station_URL]
DBSub=mRadio
DBSet=StationURL

[Station_Bitrate]
DBSub=mRadio
DBSet=Bitrate

[Station_Genre]
DBSub=mRadio
DBSet=Genre

3) Different info
  WMA stations don't recorded, only MP3 and OGG stream record tested.
  Not all metatags supported.
  Manual proxy settings supported.
  BASS Plugins loading ONLY when protocol going Online

Changelog:
----------
0.0.2.1 ()
  64 bit compatibility fix
  new event notifications
0.0.2.0 (19 mar 2011)
  Adapted for FreePascal and 64 bit support
0.0.1.7 ()
  Added internet radio reconnect on track end
  Added button to import all radiostations from file
  Added frame right-click action "Open mradio settings"
  Improved frame coloration
  Added Frame (1st try)
  Added another WMA stream processing
  Added option for several tries to connect radiostation
0.0.1.6 (20 sep 2010)
  Changed streams/files tag process
0.0.1.5 (6 sep 2010)
  Fixed simple form of M3U playlist reading
  Improved compatibility with Watrack plugin
  Restored ANSI URL processing if no Unicode possible (especially for BASSWMA)
  Fixed group choosing for "Add to List" of new stations
  Fixed some memory leaks
  Changed proxy processing to ability to use IE settings (through miranda proxy settings)
  Fixed crash when contact goin offline (Offline as offline setting)
  Code changed to BASS 2.4.6 support (unicode URL support)
0.0.1.4 ()
  Added event ME_RADIO_STATUS
  Added Variables plugin parsing for record filename
  Added ability to choose BASS.dll placement
  Radio stopping now if active contact deleting
  Added services for import/export stations
0.0.1.3 (15 oct 2009)
  Radio record filename must calc from media info
  Radio record fixed
0.0.1.2 (15 oct 2009)
  Added equalizer preset work
  "Record" crash fixed
  Added changing icon in contact menu for start/stop broadcasting
0.0.1.1 (10 oct 2009)
  Added option to export all radio station from list to choosed INI-file
0.0.1.0 ()
  Fixed UTF8 station track info showing
  Dropped ANSI and pre-0.7 Miranda IM version support
0.0.0.15 (14 jan 2008)
  Toolbar button to switch sound on/off affect now on mRadio sound
  Added 'radio_codec' variable
  Added GUID to header files
  Added option to show offline stations as Offline (not invisible)
  Added option to autconnect last played station
0.0.0.14 ( nov 2007)
  Fixed: record directory not created if it not exists
  Fixed: Database hook error
  Station choosing change plugin status from Offline to Online
  Fixed: last played track wrongly restored at start
0.0.0.13 (24 jun 2007)
  Added notification for 'Work Offline' IE mode
  New option for network timeout
0.0.0.12 (20 jun 2007)
  Small fixes
  Added Unicode for file select dialog in Options
0.0.0.11 (18 jun 2007)
  Added test unicode support (can work wrong)
  Added ability to change INI-file for station search
  Changed search interface for Miranda version 0.7+
  Added button to switch equalizer off
  Options interface changed
0.0.0.10 (13 jun 2007)
  Added partial SHOUTcast metatags support
  Added playing station status info
0.0.0.9 (10 jun 2007)
  Added support old and new ANSI and UTF8 M3U (M3U8) playlists
  Added relative pathnames in playlist support
0.0.0.8 (9 jun 2007)
  Added partial updater support
  Small fixes
  Added new option for playlist
0.0.0.7 (7 jun 2007)
  Fixed: equalizer values was reversed
  Added PLS and M3U playlist support
  Code changes
0.0.0.6 (3 jun 2007)
  Fixed: Global volume (not music only) used
  Fixed: Can't break slowly connected stations
0.0.0.5 (2 jun 2007)
  Fixed: Double click contact dialog can't open
  Changed: OGG station stream saving 
0.0.0.4 (1 jun 2007)
  Small Radio station search dialog adaptation for 0.7 and 0.6 Miranda versions
  Added volume control synchronization with SndVol
  Added option to repeat media
  Added global 'Record' menu item to contact menu
  Fixed: exit while online Radio status make crash
  Added: Equalizer
0.0.0.3 (30 may 2007)
  Added UserInfoEx (station editing) compatibility
  Code cleaning and optimization
  Convert to (2 in 1)
0.0.0.2 (30 may 2007)
  Added Variables plugin support
  Added Buffer size changing ability
0.0.0.1 (29 may 2007)
  First release
