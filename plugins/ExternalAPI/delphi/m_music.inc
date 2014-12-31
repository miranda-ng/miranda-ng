{$IFNDEF M_MUSIC}
{$DEFINE M_MUSIC}

// defined in interfaces.inc
//const MIID_WATRACK:MUUID='{FC6C81F4-837E-4430-9601-A0AA43177AE3}';

type
  pSongInfoA = ^tSongInfoA;
  tSongInfoA = record
    artist   :PAnsiChar;
    title    :PAnsiChar;
    album    :PAnsiChar;
    genre    :PAnsiChar;
    comment  :PAnsiChar;
    year     :PAnsiChar;
    mfile    :PAnsiChar; // media file
    kbps     :dword;
    khz      :dword;
    channels :dword;
    track    :dword;
    total    :dword;     // music length
    time     :dword;     // elapsed time
    wndtext  :PAnsiChar; // window title
    player   :PAnsiChar; // player name
    plyver   :dword;     // player version
    icon     :HICON;     // player icon
    fsize    :dword;     // media file size
    vbr      :dword;
    status   :integer;   // WAT_PLS_*
    plwnd    :HWND;      // player window
    // video  part
    codec    :dword;
    width    :dword;
    height   :dword;
    fps      :dword;
    date     :int64;
    txtver   :PAnsiChar;
    lyric    :PAnsiChar;
    cover    :PAnsiChar;
    volume   :dword;
    url      :PAnsiChar; // player homepage
    winampwnd:HWND;
  end;
type
  pSongInfo=^tSongInfo;
  tSongInfo = record
    artist   :PWideChar;
    title    :PWideChar;
    album    :PWideChar;
    genre    :PWideChar;
    comment  :PWideChar;
    year     :PWideChar;
    mfile    :PWideChar; // media file
    kbps     :dword;
    khz      :dword;
    channels :dword;
    track    :dword;
    total    :dword;     // music length
    time     :dword;     // elapsed time
    wndtext  :PWideChar; // window title
    player   :PWideChar; // player name
    plyver   :dword;     // player version
    icon     :HICON;     // player icon
    fsize    :dword;     // media file size
    vbr      :dword;
    status   :integer;   // WAT_PLS_*
    plwnd    :HWND;      // player window
    // video  part
    codec    :dword;
    width    :dword;
    height   :dword;
    fps      :dword;
    date     :int64;
    txtver   :PWideChar;
    lyric    :PWideChar;
    cover    :PWideChar; // cover path
    volume   :dword;
    url      :PWideChar; // player homepage
    winampwnd:HWND;
  end;
  pSongInfoW = pSongInfo;
  tSongInfoW = tSongInfo;

const
  // result codes
  WAT_RES_UNKNOWN   = -2;
  WAT_RES_NOTFOUND  = -1;
  WAT_RES_ERROR     = WAT_RES_NOTFOUND;
  WAT_RES_OK        = 0;
  WAT_RES_ENABLED   = WAT_RES_OK;
  WAT_RES_DISABLED  = 1;
  // internal
  WAT_RES_NEWFILE   = 3;
  WAT_RES_NEWPLAYER = 4;

const
  WAT_INF_UNICODE = 0;
  WAT_INF_ANSI    = 1;
  WAT_INF_UTF8    = 2;
  WAT_INF_CHANGES = $100;

const
  MS_WAT_INSERT:PAnsiChar = 'WATrack/Insert';
  MS_WAT_EXPORT:PAnsiChar = 'WATrack/Export';

const
{
  wParam : WAT_INF_* constant
  lParam : pointer to pSongInfo (Unicode) or pSongInfoA (ANSI/UTF8)
  Affects: Fill structure by currently played music info
  returns: WAT_RES_* constant
  note: pointer will be point to global SongInfo structure of plugin
  warning: Non-Unicode data filled only by request
  if lParam=0 only internal SongInfo structure will be filled
  Example:
    var p:pSongInfo;
    CallService(MS_WAT_GETMUSICINFO,0,lparam(@p));
}
  MS_WAT_GETMUSICINFO:PAnsiChar = 'WATrack/GetMusicInfo';
{
  wParam:0
  lParam : pointer to pSongInfo (Unicode)
  Affects: Fill structure by info from file named in SongInfo.mfile
  returns: 0, if success
  note: fields, which values can't be obtained, leaves old values.
    you must free given strings by miranda mir_free
}
  MS_WAT_GETFILEINFO:PAnsiChar = 'WATrack/GetFileInfo';

{
  wParam: encoding (WAT_INF_* consts, 0 = WAT_INF_UNICODE)
  lParam: codepage (0 = ANSI)
  Returns Global unicode SongInfo pointer or tranlated to Ansi/UTF8 structure
}
  MS_WAT_RETURNGLOBAL:PAnsiChar = 'WATrack/GetMainStructure';

//!! DON'T CHANGE THESE VALUES!
const
  WAT_CTRL_FIRST = 1;

  WAT_CTRL_PREV  = 1;
  WAT_CTRL_PLAY  = 2;
  WAT_CTRL_PAUSE = 3;
  WAT_CTRL_STOP  = 4;
  WAT_CTRL_NEXT  = 5;
  WAT_CTRL_VOLDN = 6;
  WAT_CTRL_VOLUP = 7;
  WAT_CTRL_SEEK  = 8; // lParam is new position (sec)

  WAT_CTRL_LAST  = 8;

{
  wParam: button code (WAT_CTRL_* const)
  lParam: 0, or value (see WAT_CTRL_* const comments)
  Affects: emulate player button pressing
  returns: 0 if unsuccesful
}
  MS_WAT_PRESSBUTTON:PAnsiChar = 'WATrack/PressButton';

{
  Get user's Music Info
}
  MS_WAT_GETCONTACTINFO:PAnsiChar = 'WATrack/GetContactInfo';

// ------------ Plugin/player status ------------

{
  wParam: 1  - switch off plugin
          0  - switch on plugin
          -1 - switch plugin status
          2  - get plugin version
          other - get plugin status
  lParam: 0
  Affects: Switch plugin status to enabled or disabled
  returns: version, old plugin status, 0, if was enabled
}
  MS_WAT_PLUGINSTATUS:PAnsiChar = 'WATrack/PluginStatus';

// ---------- events ------------

{
  ME_WAT_MODULELOADED
  wParam: 0, lParam: 0
}
  ME_WAT_MODULELOADED:PAnsiChar = 'WATrack/ModuleLoaded';

const
  WAT_EVENT_PLAYERSTATUS = 1; // WAT_PLS_*
  WAT_EVENT_NEWTRACK     = 2; // SongInfo ptr
  WAT_EVENT_PLUGINSTATUS = 3; // 0-enabled; 1-dis.temporary; 2-dis.permanent
  WAT_EVENT_NEWPLAYER    = 4; //
  WAT_EVENT_NEWTEMPLATE  = 5; // TM_* constant

{
  Plugin or player status changed:
  wParam: type of event (see above)
  lParam: value
}
  ME_WAT_NEWSTATUS:PAnsiChar = 'WATrack/NewStatus';

// ---------- Popup module ------------

{
  wParam: not used
  lParam: not used
  Affects: Show popup or Info window with current music information
  note: Only Info window will be showed if Popup plugin disabled
}
  MS_WAT_SHOWMUSICINFO:PAnsiChar = 'WATrack/ShowMusicInfo';

// --------- Statistic (report) module -------------

{
  wParam: pointer to log file name or NIL
  lParam: pointer to report file name or NIL
  Affects: Create report from log and run it (if option is set)
  returns: 0 if unsuccesful
  note: if wParam or lParam is a NIL then file names from options are used
}
  MS_WAT_MAKEREPORT :PAnsiChar = 'WATrack/MakeReport';
//  MS_WAT_MAKEREPORTW:PAnsiChar = 'WATrack/MakeReportW';

{
  wParam, lParam - not used
  Affects: pack statistic file
}
  MS_WAT_PACKLOG:PAnsiChar = 'WATrack/PackLog';

{
  wParam: not used
  lParam: pointer to SongInfo
}
  MS_WAT_ADDTOLOG:PAnsiChar = 'WATrack/AddToLog';

// ----------- Formats and players -----------

const // player status
  WAT_PLS_NOTFOUND = 0; // player not found
  WAT_PLS_PLAYING  = 1;
  WAT_PLS_PAUSED   = 2;
  WAT_PLS_STOPPED  = 3; // player stopped, no music even searched
  WAT_PLS_UNKNOWN  = 4; // player found with unknown state

const
  WAT_ACT_REGISTER   = 1;
  WAT_ACT_UNREGISTER = 2;
  WAT_ACT_DISABLE    = 3;
  WAT_ACT_ENABLE     = 4;
  WAT_ACT_GETSTATUS  = 5; // not found/enabled/disabled
  WAT_ACT_SETACTIVE  = 6;
  WAT_ACT_REPLACE    = $10000; // can be combined with WAT_REGISTERFORMAT

const
  // flags
  WAT_OPT_DISABLED    = $00000001; // [formats,players,options] registered but disabled
  WAT_OPT_ONLYONE     = $00000002; // [formats,players] code can't be overwriten
  WAT_OPT_PLAYERINFO  = $00000004; // [players] song info from player
  WAT_OPT_WINAMPAPI   = $00000008; // [players] Winamp API support
  WAT_OPT_CHECKTIME   = $00000010; // [options] check file time for changes
  WAT_OPT_VIDEO       = $00000020; // [formats,options] format is video
  WAT_OPT_LAST        = $00000040; // (internal-Winamp Clone) put to the end of queue
  WAT_OPT_FIRST       = $00000080; // (internal)
  WAT_OPT_TEMPLATE    = $00000100; // (internal)
  WAT_OPT_IMPLANTANT  = $00000200; // [options] use process implantation
  WAT_OPT_HASURL      = $00000400; // [players] URL field present
  WAT_OPT_CHANGES     = $00000800; // [subplugin] obtain only chaged values
                                   // (volume, status, window text, elapsed time)
  WAT_OPT_APPCOMMAND  = $00001000; // [options] Special (multimedia) key support
  WAT_OPT_CHECKALL    = $00002000; // [options] Check all players
  WAT_OPT_KEEPOLD     = $00004000; // [options] Keep Old opened file
//  WAT_OPT_MULTITHREAD = $00008000; // [options] Use multithread scan
  WAT_OPT_SINGLEINST  = $00010000; // [players] Single player instance
  WAT_OPT_PLAYERDATA  = $00020000; // [subplugin] to obtain player data
  WAT_OPT_CONTAINER   = $00040000; // [formats] format is container (need to check full)
  WAT_OPT_UNKNOWNFMT  = $00080000; // [formats] check unknown (not disabled) formats (info from player)
  WAT_OPT_INTERNAL    = $80000000; // (internal) for memory manager choosing

type
  tReadFormatProc = function(var Info:tSongInfo):boolean; cdecl;
  pMusicFormat = ^tMusicFormat;
  tMusicFormat = record
    proc :tReadFormatProc;
    ext  :array [0..7] of AnsiChar;
    flags:cardinal;
  end;

const
{
  wParam: action
  lParam: pointer to tMusicFormat if wParam = WAT_ACT_REGISTER,
          else - pointer to extension string (ANSI)
  returns: see result codes
}
  MS_WAT_FORMAT:PAnsiChar = 'WATrack/Format';

{
  wParam: pointer to SongInfo structure (plwind field must be initialized)
  lParam: flags
  Affects: trying to fill SongInfo using Winamp API
}
  MS_WAT_WINAMPINFO:PAnsiChar = 'WATrack/WinampInfo';

{
  wParam: window
  lParam: LoWord - command; HiWord - value
}
  MS_WAT_WINAMPCOMMAND:PAnsiChar = 'WATrack/WinampCommand';

type
  tInitProc    = function():integer;cdecl;
  tDeInitProc  = function():integer;cdecl;
  tStatusProc  = function(wnd:HWND):integer;cdecl;
  tNameProc    = function(wnd:HWND;flags:integer):PWideChar;cdecl;
  tCheckProc   = function(wnd:HWND;flags:integer):HWND;cdecl;
  tInfoProc    = function(var SongInfo:tSongInfo;flags:integer):integer;cdecl;
  tCommandProc = function(wnd:HWND;command:integer;value:integer):integer;cdecl;

  pPlayerCell = ^tPlayerCell;
  tPlayerCell = record
    Desc     :PAnsiChar; // Short player name
    flags    :dword;
    Icon     :HICON;     // can be 0. for registration only
    Init     :pointer;   // tInitProc;    can be NIL. initialize any data
    DeInit   :pointer;   // tDeInitProc;  can be NIL. finalize player processing
    Check    :pointer;   // tCheckProc;   check player
    GetStatus:pointer;   // tStatusProc;  can be NIL. get player status
    GetName  :pointer;   // tNameProc;    can be NIL. get media filename
    GetInfo  :pointer;   // tInfoProc;    can be NIL. get info from player
    Command  :pointer;   // tCommandProc; can be NIL. send command to player
    URL      :PAnsiChar; // only if WAT_OPT_HASURL flag present
    Notes    :PWideChar; // any tips, notes etc for this player
  end;

const
{
  wParam: action
  lParam: pointer to tPlayerCell if wParam = WAT_ACT_REGISTER,
          else - pointer to player description string (ANSI)
  returns: player window handle or value>0 if found
  note: If you use GetName or GetInfo field, please, do not return empty
        filename even when mediafile is remote!
}
  MS_WAT_PLAYER:PAnsiChar = 'WATrack/Player';

// --------- MyShows.ru ---------

{
  Toggle MyShows scrobbling status
  wParam,lParam=0
  Returns: previous state
}
const
  MS_WAT_MYSHOWS:PAnsiChar = 'WATrack/MyShows';


const
  MS_WAT_MYSHOWSINFO:PAnsiChar = 'WATrack/MyShowsInfo';

// --------- Last FM  ---------

{
  Toggle LastFM scrobbling status
  wParam,lParam=0
  Returns: previous state
}
const
  MS_WAT_LASTFM:PAnsiChar = 'WATrack/LastFM';

{
  Get Info based on currently played song
  wParam: pLastFMInfo
  lParam: int language (first 2 bytes - 2-letters language code)
}
type
  pLastFMInfo = ^tLastFMInfo;
  tLastFMInfo = record
    request:cardinal;   // 0 - artist, 1 - album, 2 - track
    artist :PWideChar; // artist
    album  :PWideChar; // album or similar artists for Artist info request
    title  :PWideChar; // track title 
    tags   :PWideChar; // tags
    info   :PWideChar; // artist bio or wiki article
    image  :PAnsiChar; // photo/cover link
    similar:PWideChar;
    release:PWideChar;
    trknum :cardinal;
  end;
const
  MS_WAT_LASTFMINFO:PAnsiChar = 'WATrack/LastFMInfo';

// --------- Templates ----------

const
{
  wParam: 0 (standard Info) or pSongInfo
  lParam: Unicode template
  returns: New Unicode (replaced) string
}
  MS_WAT_REPLACETEXT:PAnsiChar = 'WATrack/ReplaceText';

{
  event types for History
  Blob structure for EVENTTYPE_WAT_ANSWER:
   Uniciode artist#0title#0album#0answer
}
const
  EVENTTYPE_WAT_REQUEST = 9601;
  EVENTTYPE_WAT_ANSWER  = 9602;
  EVENTTYPE_WAT_ERROR   = 9603;
  EVENTTYPE_WAT_MESSAGE = 9604;

const
{
  wParam: 0 or parent window
  lParam: 0
  note:   Shows Macro help window with edit aliases ability
}
  MS_WAT_MACROHELP:PAnsiChar = 'WATrack/MacroHelp';

{$ENDIF M_MUSIC}
