<?php

// SongInfo offsets

define("wato_artist"  ,0);
define("wato_title"   ,wato_artist  +4);
define("wato_album"   ,wato_title   +4);
define("wato_genre"   ,wato_album   +4);
define("wato_comment" ,wato_genre   +4);
define("wato_year"    ,wato_comment +4);
define("wato_mfile"   ,wato_year    +4);
define("wato_kbps"    ,wato_mfile   +4);
define("wato_khz"     ,wato_kbps    +4);
define("wato_channels",wato_khz     +4);
define("wato_track"   ,wato_channels+4);
define("wato_total"   ,wato_track   +4);
define("wato_time"    ,wato_total   +4);
define("wato_wndtext" ,wato_time    +4);
define("wato_player"  ,wato_wndtext +4);
define("wato_plyver"  ,wato_player  +4);
define("wato_icon"    ,wato_plyver  +4);
define("wato_fsize"   ,wato_icon    +4);
define("wato_vbr"     ,wato_fsize   +4);
define("wato_status"  ,wato_vbr     +4);
define("wato_plwnd"   ,wato_status  +4);
define("wato_codec"   ,wato_plwnd   +4);
define("wato_width"   ,wato_codec   +4);
define("wato_height"  ,wato_width   +4);
define("wato_fps"     ,wato_height  +4);
define("wato_date"    ,wato_fps     +4);
define("wato_txtver"  ,wato_date    +4);
define("wato_lyric"   ,wato_txtver  +4);
define("wato_cover"   ,wato_lyric   +4);
define("wato_volume"  ,wato_cover   +4);
define("wato_url"     ,wato_volume  +4);

// player control commands

define("WAT_CTRL_PREV" ,1);
define("WAT_CTRL_PLAY" ,2);
define("WAT_CTRL_PAUSE",3);
define("WAT_CTRL_STOP" ,4);
define("WAT_CTRL_NEXT" ,5);
define("WAT_CTRL_VOLDN",6);
define("WAT_CTRL_VOLUP",7);
define("WAT_CTRL_SEEK" ,8);

// hook service actions

define("WAT_ACT_REGISTER"  ,1);
define("WAT_ACT_UNREGISTER",2);
define("WAT_ACT_DISABLE"   ,3);
define("WAT_ACT_ENABLE"    ,4);
define("WAT_ACT_GETSTATUS" ,5); // not found/enabled/disabled
define("WAT_ACT_SETACTIVE" ,6);
define("WAT_ACT_REPLACE"   ,0x10000); // can be combined with WAT_REGISTERFORMAT

// result codes

define("WAT_RES_UNKNOWN" ,-2);
define("WAT_RES_NOTFOUND",-1);
define("WAT_RES_ERROR"   ,WAT_RES_NOTFOUND);
define("WAT_RES_OK"      ,0);
define("WAT_RES_DISABLED",1);
define("WAT_RES_ENABLED" ,WAT_RES_OK);
define("WAT_RES_NEWFILE" ,3);

// hook service options

define("WAT_OPT_DISABLED"   ,0x00000001); // registered but disabled
define("WAT_OPT_ONLYONE"    ,0x00000002); // can't be overwriten
define("WAT_OPT_PLAYERINFO" ,0x00000004); // song info from player
define("WAT_OPT_WINAMPAPI"  ,0x00000008); // Winamp API support
define("WAT_OPT_CHECKTIME"  ,0x00000010); // check file time for changes
define("WAT_OPT_VIDEO"      ,0x00000020); // only for format registering used
define("WAT_OPT_LAST"       ,0x00000040); // (internal)
define("WAT_OPT_FIRST"      ,0x00000080); // (internal)
define("WAT_OPT_TEMPLATE"   ,0x00000100); // (internal)
define("WAT_OPT_IMPLANTANT" ,0x00000200); // use process implantation
define("WAT_OPT_HASURL"     ,0x00000400); // (player registration) URL field present
define("WAT_OPT_CHANGES"    ,0x00000800); // obtain only chaged values
                                          // (volume, status, window text, elapsed time)

define("WAT_OPT_APPCOMMAND" ,0x00001000); // Special (multimedia) key support
define("WAT_OPT_CHECKALL"   ,0x00002000); // Check all players
define("WAT_OPT_KEEPOLD"    ,0x00004000); // Keep Old opened file
define("WAT_OPT_MULTITHREAD",0x00008000); // Use multithread scan
define("WAT_OPT_SINGLEINST" ,0x00010000); // Single player instance
// services

define("MS_WAT_GETMUSICINFO" ,"WATrack/GetMusicInfo");
define("MS_WAT_GETFILEINFO"  ,"WATrack/GetFileInfo");
define("MS_WAT_SHOWMUSICINFO","WATrack/ShowMusicInfo");
define("MS_WAT_MAKEREPORT"   ,"WATrack/MakeReport");
define("MS_WAT_PACKLOG"      ,"WATrack/PackLog");
define("MS_WAT_ADDTOLOG"     ,"WATrack/AddToLog");
define("MS_WAT_PLUGINSTATUS" ,"WATrack/PluginStatus");
define("MS_WAT_PRESSBUTTON"  ,"WATrack/PressButton");
define("MS_WAT_REPLACETEXT"  ,"WATrack/ReplaceText");

// events

define("ME_WAT_MODULELOADED" ,"WATrack/ModuleLoaded");
define("ME_WAT_NEWSTATUS"    ,"WATrack/NewStatus");

// hook services

define("MS_WAT_FORMAT"       ,"WATrack/Format");
define("MS_WAT_WINAMPINFO"   ,"WATrack/WinampInfo");
define("MS_WAT_WINAMPCOMMAND","WATrack/WinampCommand");
define("MS_WAT_PLAYER"       ,"WATrack/Player");

// GetMuscInfo flags
define("WAT_INF_UNICODE",0);
define("WAT_INF_ANSI"   ,1);
define("WAT_INF_UTF8"   ,2);
define("WAT_INF_CHANGES",0x100);

// player status

define("WAT_PLS_NORMAL"  ,WAT_RES_OK);
define("WAT_PLS_NOMUSIC" ,WAT_RES_DISABLED);
define("WAT_PLS_NOTFOUND",WAT_RES_NOTFOUND);

// media status

define("WAT_MES_STOPPED",0);
define("WAT_MES_PLAYING",1);
define("WAT_MES_PAUSED" ,2);
define("WAT_MES_UNKNOWN",-1);

// event types for History

define("EVENTTYPE_WAT_REQUEST",9601);
define("EVENTTYPE_WAT_ANSWER" ,9602);
define("EVENTTYPE_WAT_ERROR"  ,9603);
define("EVENTTYPE_WAT_MESSAGE",9604);

// Status events
define("WAT_EVENT_PLAYERSTATUS",1);
define("WAT_EVENT_NEWTRACK"    ,2);
define("WAT_EVENT_PLUGINSTATUS",3);
define("WAT_EVENT_NEWPLAYER"   ,4);
define("WAT_EVENT_NEWTEMPLATE" ,5);

define("TM_MESSAGE"   ,0); // privat message
define("TM_CHANNEL"   ,1); // chat
define("TM_STAT_TITLE",2); // xstatus title
define("TM_STAT_TEXT" ,3); // [x]status text
define("TM_POPTITLE"  ,4); // popup title
define("TM_POPTEXT"   ,5); // popup text
define("TM_EXPORT"    ,6); // other app
define("TM_FRAMEINFO" ,7); // frame

define("TM_SETTEXT"   ,0x100); // only for service
define("TM_GETTEXT"   ,0);     // only for service
?>
