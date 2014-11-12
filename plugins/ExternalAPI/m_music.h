#ifndef M_MUSIC
#define M_MUSIC

#define MIID_WATRACK {0xfc6c81f4, 0x837e, 0x4430, {0x96, 0x01, 0xa0, 0xaa, 0x43, 0x17, 0x7a, 0xe3}}

typedef struct tSongInfoA {
  CHAR*   artist;
  CHAR*   title;
  CHAR*   album;
  CHAR*   genre;
  CHAR*   comment;
  CHAR*   year;
  CHAR*   mfile;     // media file
  DWORD   kbps;
  DWORD   khz;
  DWORD   channels;
  DWORD   track;
  DWORD   total;     // music length
  DWORD   time;      // elapsed time
  CHAR*   wndtext;   // window title
  CHAR*   player;    // player name
  DWORD   plyver;    // player version
  HICON   icon;      // player icon
  DWORD   fsize;     // media file size
  DWORD   vbr;
  int     status;    // WAT_MES_* const
  HWND    plwnd;     // player window
  // video part
  DWORD   codec;
  DWORD   width;
  DWORD   height;
  DWORD   fps;
  __int64 date;
  CHAR*   txtver;
  CHAR*   lyric;
  CHAR*   cover;
  DWORD   volume;
  CHAR*   url;       // player homepage
  HWND    winampwnd; // player window
} SONGINFOA, *LPSONGINFOA;

typedef struct tSongInfo {
  WCHAR*  artist;
  WCHAR*  title;
  WCHAR*  album;
  WCHAR*  genre;
  WCHAR*  comment;
  WCHAR*  year;
  WCHAR*  mfile;     // media file
  DWORD	  kbps;
  DWORD	  khz;
  DWORD	  channels;
  DWORD	  track;
  DWORD	  total;     // music length
  DWORD	  time;      // elapsed time
  WCHAR*  wndtext;   // window title
  WCHAR*  player;    // player name
  DWORD   plyver;    // player version
  HICON   icon;      // player icon
  DWORD   fsize;     // media file size
  DWORD   vbr;
  int     status;    // WAT_MES_* const
  HWND    plwnd;     // player window
  // video part
  DWORD   codec;
  DWORD   width;
  DWORD   height;
  DWORD   fps;
  __int64 date;
  WCHAR*  txtver;
  WCHAR*  lyric;
  WCHAR*  cover;
  DWORD   volume;
  WCHAR*  url;       // player homepage
  HWND    winampwnd; // player window
} SONGINFO, *LPSONGINFO;

#if defined(_UNICODE)
  #define WAT_INF_TCHAR WAT_INF_UNICODE
  #define SongInfoT tSongInfo
#else
  #define WAT_INF_TCHAR WAT_INF_ANSI
  #define SongInfoT tSongInfoA
#endif 

  // result codes
#define WAT_RES_UNKNOWN     -2
#define WAT_RES_NOTFOUND    -1
#define WAT_RES_ERROR       WAT_RES_NOTFOUND
#define WAT_RES_OK          0
#define WAT_RES_ENABLED     WAT_RES_OK
#define WAT_RES_DISABLED    1
  // internal
#define WAT_RES_NEWFILE     3
#define WAT_RES_NEWPLAYER   4

#define WAT_INF_UNICODE     0
#define WAT_INF_ANSI        1
#define WAT_INF_UTF8        2
#define WAT_INF_CHANGES     0x100

/*
  wParam : WAT_INF_* constant
  lParam : pointer to LPSONGINGO (Unicode) or LPSONGINFOA (ANSI/UTF8)
  Affects: Fill structure by currently played music info
  returns: WAT_RES_* constant
  note: pointer will be point to global SONGINFO structure of plugin
  warning: Non-Unicode data filled only by request
  if lParam=0 only internal SongInfo structure will be filled
  Example:
    LPSONGINFO p;
    PluginLink->CallService(MS_WAT_GETMUSICINFO,0,(LPARAM)&p);
*/
#define MS_WAT_GETMUSICINFO  "WATrack/GetMusicInfo"

/*
  wParam:0
  lParam : pointer to pSongInfo (Unicode)
  Affects: Fill structure by info from file named in SongInfo.mfile
  returns: 0, if success
  note: fields, which values can't be obtained, leaves old values.
    you must free given strings by miranda mmi.free
*/
#define MS_WAT_GETFILEINFO "WATrack/GetFileInfo"

/*
  wParam: encoding (WAT_INF_* consts, 0 = WAT_INF_UNICODE)
  lParam: codepage (0 = ANSI)
  Returns Global unicode SongInfo pointer or tranlated to Ansi/UTF8 structure
*/
#define MS_WAT_RETURNGLOBAL "WATrack/GetMainStructure"

//!! DON'T CHANGE THESE VALUES!
#define WAT_CTRL_FIRST 1

#define WAT_CTRL_PREV  1
#define WAT_CTRL_PLAY  2
#define WAT_CTRL_PAUSE 3
#define WAT_CTRL_STOP  4
#define WAT_CTRL_NEXT  5
#define WAT_CTRL_VOLDN 6
#define WAT_CTRL_VOLUP 7
#define WAT_CTRL_SEEK  8 // lParam is new position (sec)

#define WAT_CTRL_LAST  8

/*
  wParam: button code (WAT_CTRL_* const)
  lParam: 0, or value (see WAT_CTRL_* const comments)
  Affects: emulate player button pressing
  returns: 0 if unsuccesful
*/
#define MS_WAT_PRESSBUTTON "WATrack/PressButton"

/*
  Get user's Music Info
*/
#define MS_WAT_GETCONTACTINFO "WATrack/GetContactInfo"

// ------------ Plugin/player status ------------

/*
  wParam: 1  - switch off plugin
          0  - switch on plugin
          -1 - switch plugin status
          2  - get plugin version
          other - get plugin status
  lParam: 0
  Affects: Switch plugin status to enabled or disabled
  returns: old plugin status, 0, if was enabled
*/
#define MS_WAT_PLUGINSTATUS "WATrack/PluginStatus"

// ---------- events ------------

/*ME_WAT_MODULELOADED
  wParam: 0, lParam: 0
*/
#define ME_WAT_MODULELOADED "WATrack/ModuleLoaded"

#define WAT_EVENT_PLAYERSTATUS    1 //lParam: WAT_PLS_* const
#define WAT_EVENT_NEWTRACK        2 //lParam: LPSONGINFO
#define WAT_EVENT_PLUGINSTATUS    3 //lParam: 0-enabled; 1-dis.temporary; 2-dis.permanent
#define WAT_EVENT_NEWPLAYER       4 //
#define WAT_EVENT_NEWTEMPLATE     5 //lParam: TM_* constant

/*ME_WAT_NEWSTATUS
  Plugin or player status changed:
  wParam: type of event (see above)
  lParam: value
*/
#define ME_WAT_NEWSTATUS    "WATrack/NewStatus"

// ---------- Popup module ------------

/*
  wParam: not used
  lParam: not used
  Affects: Show popup or Info window with current music information
  note: Only Info window will be showed if Popup plugin disabled
*/
#define MS_WAT_SHOWMUSICINFO "WATrack/ShowMusicInfo"

// --------- Statistic (report) module -------------

/*
  wParam: pointer to log file name or NULL
  lParam: pointer to report file name or NULL
  Affects: Create report from log and run it (if option is set)
  returns: 0 if unsuccesful
  note: if wParam or lParam is a NULL then file names from options are used
*/
#define MS_WAT_MAKEREPORT   "WATrack/MakeReport"

/*
  wParam, lParam - not used
  Affects: pack statistic file
*/
#define MS_WAT_PACKLOG = "WATrack/PackLog"

/*
  wParam: not used
  lParam: pointer to SongInfo
*/
#define MS_WAT_ADDTOLOG = "WATrack/AddToLog"

// ----------- Formats and players -----------

// player status

#define WAT_PLS_NOTFOUUND   0
#define WAT_PLS_PLAYING     1
#define WAT_PLS_PAUSED      2
#define WAT_PLS_STOPPED     3
#define WAT_PLS_UNKNOWN     4

#define WAT_ACT_REGISTER    1
#define WAT_ACT_UNREGISTER  2
#define WAT_ACT_DISABLE     3
#define WAT_ACT_ENABLE      4
#define WAT_ACT_GETSTATUS   5       // not found/enabled/disabled
#define WAT_ACT_SETACTIVE   6
#define WAT_ACT_REPLACE     0x10000 // can be combined with WAT_REGISTERFORMAT

  // flags
#define WAT_OPT_DISABLED    0x00001 // format registered but disabled
#define WAT_OPT_ONLYONE     0x00002 // format can't be overwriten
#define WAT_OPT_PLAYERINFO  0x00004 // song info from player
#define WAT_OPT_WINAMPAPI   0x00008 // Winamp API support
#define WAT_OPT_CHECKTIME   0x00010 // check file time for changes
#define WAT_OPT_VIDEO       0x00020 // only for format registering used
#define WAT_OPT_LAST        0x00040 // (internal)
#define WAT_OPT_FIRST       0x00080 // (internal)
#define WAT_OPT_TEMPLATE    0x00100 // (internal)
#define WAT_OPT_IMPLANTANT  0x00200 // use process implantation
#define WAT_OPT_HASURL      0x00400 // (player registration) URL field present
#define WAT_OPT_CHANGES     0x00800 // obtain only chaged values
                                    // (volume, status, window text, elapsed time)
#define WAT_OPT_APPCOMMAND  0x01000 // Special (multimedia) key support
#define WAT_OPT_CHECKALL    0x02000 // Check all players
#define WAT_OPT_KEEPOLD     0x04000 // Keep Old opened file
//#define WAT_OPT_MULTITHREAD 0x08000 // Use multithread scan
#define WAT_OPT_SINGLEINST  0x10000 // Single player instance
#define WAT_OPT_PLAYERDATA  0x20000 // (internal) to obtain player data
#define WAT_OPT_CONTAINER   0x40000 // [formats] format is container (need to check full)
#define WAT_OPT_UNKNOWNFMT  0x80000 // [formats] check unknown (not disabled) formats (info from player)


typedef BOOL (__cdecl *LPREADFORMATPROC)(LPSONGINFO Info);

typedef struct tMusicFormat {
    LPREADFORMATPROC proc;
    CHAR ext[8];
    UINT flags;
} MUSICFORMAT, *LPMUSICFORMAT;

/*
  wParam: action
  lParam: pointer to MUSICFORMAT if wParam = WAT_ACT_REGISTER,
          else - pointer to extension string (ANSI)
  returns: see result codes
*/
#define MS_WAT_FORMAT  "WATrack/Format"

/*
  wParam - pointer to SONGINFO structure (plwind field must be initialized)
  lParam - flags
  Affects: trying to fill SongInfo using Winamp API
*/
#define MS_WAT_WINAMPINFO   "WATrack/WinampInfo"

/*
  wParam: window
  lParam: LoWord - command; HiWord - value
*/
#define MS_WAT_WINAMPCOMMAND  "WATrack/WinampCommand"

typedef int   (__cdecl *LPINITPROC)   ();
typedef int   (__cdecl *LPDEINITPROC) ();
typedef int   (__cdecl *LPSTATUSPROC) (HWND wnd);
typedef WCHAR*(__cdecl *LPNAMEPROC)   (HWND wnd, int flags);
typedef HWND  (__cdecl *LPCHECKPROC)  (HWND wnd,int flags);
typedef int   (__cdecl *LPINFOPROC)   (LPSONGINFO Info, int flags);
typedef int   (__cdecl *LPCOMMANDPROC)(HWND wnd, int command, int value);

typedef struct tPlayerCell {
  CHAR*         Desc;      // Short player name
  UINT          flags;
  HICON         Icon;      // can be 0. for registration only
  LPINITPROC    Init;      // LPINITPROC;    can be NULL. initialize any data
  LPDEINITPROC  DeInit;    // LPDEINITPROC;   can be NULL. finalize player processing
  LPCHECKPROC   Check;     // check player 
  LPSTATUSPROC  GetStatus; // tStatusProc;  can be NULL. get player status
  LPNAMEPROC    GetName;   // can be NULL. get media filename
  LPINFOPROC    GetInfo;   // can be NULL. get info from player
  LPCOMMANDPROC Command;   // can be NULL. send command to player
  CHAR*         URL;       // only if WAT_OPT_HASURL flag present
  WCHAR*        Notes;     // any tips, notes etc for this player
} PLAYERCELL, *LPPLAYERCELL;

/*
  wParam: action
  lParam: pointer to PLAYERCELL if wParam = WAT_ACT_REGISTER,
          else - pointer to player description string (ANSI)
  returns: player window handle or value>0 if found
  note: If you use GetName or GetInfo field, please, do not return empty
        filename even when mediafile is remote!
*/
#define MS_WAT_PLAYER   "WATrack/Player"

// --------- MyShows.ru ---------

/*
  Toggle MyShows scrobbling status
  wParam,lParam=0
  Returns: previous state
*/
#define MS_WAT_MYSHOWS "WATrack/MyShows"

#define MS_WAT_MYSHOWSINFO "WATrack/MyShowsInfo"

// --------- Last FM  ---------

/*
  Toggle LastFM scrobbling status
  wParam,lParam=0
  Returns: previous state
*/
#define MS_WAT_LASTFM "WATrack/LastFM"

/*
  Get Info based on currently played song
  wParam: pLastFMInfo
  lParam: int language (first 2 bytes - 2-letters language code)
*/
typedef struct tLastFMInfo {
  UINT   request; // 0 - artist, 1 - album, 2 - track
  WCHAR* artist;  // artist
  WCHAR* album;   // album or similar artists for Artist info request
  WCHAR* title;   // track title 
  WCHAR* tags;    // tags
  WCHAR* info;    // artist bio or wiki article
  WCHAR* image;   // photo/cover link
  WCHAR* similar;
  WCHAR* release;
  UINT   trknum;
}PLASTFMINFO, *LPLASTFMINFO;

#define MS_WAT_LASTFMINFO "WATrack/LastFMInfo"

// --------- Templates ----------

/*
  wParam: 0 (standard Info) or pSongInfo
  lParam: Unicode template
  returns: New Unicode (replaced) string
*/
#define MS_WAT_REPLACETEXT "WATrack/ReplaceText"

/*
  event types for History
  Blob structure for EVENTTYPE_WAT_ANSWER:
   Uniciode artist#0title#0album#0answer
*/
#define EVENTTYPE_WAT_REQUEST 9601
#define EVENTTYPE_WAT_ANSWER  9602
#define EVENTTYPE_WAT_ERROR   9603
#define EVENTTYPE_WAT_MESSAGE 9604

/*
  wParam: 0 or parent window
  lParam: 0
  note:   Shows Macro help window with edit aliases ability
*/
#define MS_WAT_MACROHELP "WATrack/MacroHelp"

#endif
