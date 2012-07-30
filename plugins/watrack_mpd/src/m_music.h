#ifndef M_MUSIC
#define M_MUSIC

#define MIID_WATRACK {0xfc6c81f4, 0x837e, 0x4430, {0x96, 0x01, 0xa0, 0xaa, 0x43, 0x17, 0x7a, 0xe3}}

typedef struct tSongInfoA {
    char *artist;
    char *title;
    char *album;
    char *genre;
    char *comment;
    char *year;
    char *mfile;    // media file
    int kbps;
    int khz;
    int channels;
    int track;
    int total;      // music length
    int time;       // elapsed time
    char *wndtext;  // window title
    char *player;   // player name
    int plyver;     // player version
    HANDLE icon;    // player icon
    int fsize;      // media file size
    int vbr;
    int status;     // player status: 0 - stopped; 1 - playing; 2 - paused
    HWND plwnd;     // player window
    // video part
    int codec;
    int width;
    int height;
    int fps;
    __int64 date;
    char *txtver;
    char *lyric;
    char *cover;
    int volume;
    char *url;
} SONGINFOA, *LPSONGINFOA;

typedef struct tSongInfo {
    wchar_t *artist;
    wchar_t *title;
    wchar_t *album;
    wchar_t *genre;
    wchar_t *comment;
    wchar_t *year;
    wchar_t *mfile;   // media file
    int kbps;
    int khz;
    int channels;
    int track;
    int total;      // music length
    int time;       // elapsed time
    wchar_t *wndtext; // window title
    wchar_t *player;  // player name
    int plyver;     // player version
    HANDLE icon;    // player icon
    int fsize;      // media file size
    int vbr;
    int status;     // player status: 0 - stopped; 1 - playing; 2 - paused
    HWND plwnd;     // player window
    // video part
    int codec;
    int width;
    int height;
    int fps;
    __int64 date;
    wchar_t *txtver;
    // not implemented yet
    wchar_t *lyric;
    wchar_t *cover;
    int volume;
    wchar_t *url;
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

#define WAT_PLS_NORMAL   WAT_RES_OK
#define WAT_PLS_NOMUSIC  WAT_RES_DISABLED
#define WAT_PLS_NOTFOUND WAT_RES_NOTFOUND

#define WAT_INF_UNICODE 0
#define WAT_INF_ANSI    1
#define WAT_INF_UTF8    2
#define WAT_INF_CHANGES 0x100

/*
  wParam : WAT_INF_* constant
  lParam : pointer to LPSONGINGO (Unicode) or LPSONGINFOA (ANSI/UTF8)
  Affects: Fill structure by currently played music info
  returns: WAT_PLS_* constant
  note: pointer will be point to global SONGINFO structure of plugin
  warning: Non-Unicode data filled only by request
  if lParam=0 only internal SongInfo structure will be filled
  Example:
    LPSONGINFO p;
    PluginLink->CallService(MS_WAT_GETMUSICINFO,0,(DWORD)&p);
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

#define WAT_CTRL_PREV   1
#define WAT_CTRL_PLAY   2
#define WAT_CTRL_PAUSE  3
#define WAT_CTRL_STOP   4
#define WAT_CTRL_NEXT   5
#define WAT_CTRL_VOLDN  6
#define WAT_CTRL_VOLUP  7
#define WAT_CTRL_SEEK   8 // lParam is new position (sec)
/*
  wParam: button code (WAT_CTRL_* const)
  lParam: 0, or value (see WAT_CTRL_* const comments)
  Affects: emulate player button pressing
  returns: 0 if unsuccesful
*/
#define MS_WAT_PRESSBUTTON  "WATrack/PressButton"

/*
  Get user's Music Info
*/
#define MS_WAT_GETCONTACTINFO = "WATrack/GetContactInfo"

// ------------ Plugin/player status ------------

/*
  wParam: 1  - switch off plugin
          0  - switch on plugin
          -1 - switch plugin status
          2  - get plugin version
          other - get plugin status
  lParam: 0
  Affects: Switch plugin status to enabled or disabled
  returns: version, old plugin status, 0, if was enabled
*/

#define MS_WAT_PLUGINSTATUS "WATrack/PluginStatus"

#define ME_WAT_MODULELOADED "WATrack/ModuleLoaded"

#define WAT_EVENT_PLAYERSTATUS    1 // 0-normal; 1-no music (possibly stopped); 2-not found
#define WAT_EVENT_NEWTRACK        2
#define WAT_EVENT_PLUGINSTATUS    3 // 0-enabled; 1-dis.temporary; 2-dis.permanent
#define WAT_EVENT_NEWPLAYER       4 //
#define WAT_EVENT_NEWTEMPLATE     5 // TM_* constant

/*
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

// media file status

#define WAT_MES_STOPPED 0
#define WAT_MES_PLAYING 1
#define WAT_MES_PAUSED  2
#define WAT_MES_UNKNOWN -1

#define WAT_ACT_REGISTER    1
#define WAT_ACT_UNREGISTER  2
#define WAT_ACT_DISABLE     3
#define WAT_ACT_ENABLE      4
#define WAT_ACT_GETSTATUS   5         // not found/enabled/disabled
#define WAT_ACT_SETACTIVE   6
#define WAT_ACT_REPLACE     0x10000   // can be combined with WAT_REGISTERFORMAT

  // flags
#define WAT_OPT_DISABLED    0x00000001 // format registered but disabled
#define WAT_OPT_ONLYONE     0x00000002 // format can't be overwriten
#define WAT_OPT_PLAYERINFO  0x00000004 // song info from player
#define WAT_OPT_WINAMPAPI   0x00000008 // Winamp API support
#define WAT_OPT_CHECKTIME   0x00000010 // check file time for changes
#define WAT_OPT_VIDEO       0x00000020 // only for format registering used
#define WAT_OPT_LAST        0x00000040 // (internal)
#define WAT_OPT_FIRST       0x00000080 // (internal)
#define WAT_OPT_TEMPLATE    0x00000100 // (internal)
#define WAT_OPT_IMPLANTANT  0x00000200 // use process implantation
#define WAT_OPT_HASURL      0x00000400 // (player registration) URL field present
#define WAT_OPT_CHANGES     0x00000800 // obtain only chaged values
                                       // (volume, status, window text, elapsed time)
#define WAT_OPT_APPCOMMAND  0x00001000 // Special (multimedia) key support
#define WAT_OPT_CHECKALL    0x00002000 // Check all players
#define WAT_OPT_KEEPOLD     0x00004000 // Keep Old opened file
#define WAT_OPT_MULTITHREAD 0x00008000 // Use multithread scan
#define WAT_OPT_SINGLEINST  0x00010000 // Single player instance


typedef BOOL (__cdecl *LPREADFORMATPROC)(LPSONGINFO Info);

typedef struct tMusicFormat {
    LPREADFORMATPROC proc;
    char ext[8];
    int flags;
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
*/

#define MS_WAT_WINAMPINFO   "WATrack/WinampInfo"

/*
  wParam: window
  lParam: LoWord - command; HiWord - value
*/

#define MS_WAT_WINAMPCOMMAND  "WATrack/WinampCommand"

int tInitProc();
int tDeInitProc();
int tStatusProc();

typedef int (__cdecl *LPINITPROC)();
typedef int (__cdecl *LPDEINITPROC)();
typedef int (__cdecl *LPSTATUSPROC)(HWND wnd);
typedef wchar_t (__cdecl *LPNAMEPROC)(HWND wnd, int flags);
typedef HWND (__cdecl *LPCHECKPROC)(HWND wnd, int flags);
typedef int (__cdecl *LPGETSTATUSPROC) (HWND wnd);
typedef int (__cdecl *LPINFOPROC)(LPSONGINFO Info, int flags);
typedef int (__cdecl *LPCOMMANDPROC)(HWND wnd, int command, int value);

typedef struct tPlayerCell {
    char *Desc;
    int flags;
    HICON Icon;                // can be 0. for registration only
    LPINITPROC      Init;
    LPDEINITPROC    DeInit;
    LPCHECKPROC     Check;     // check player 
    LPGETSTATUSPROC GetStatus;
    LPNAMEPROC      GetName;   // can be NULL. get media filename
    LPINFOPROC      GetInfo;   // can be NULL. get info from player
    LPCOMMANDPROC   Command;   // can be NULL. send command to player
    char *URL;                 // only if WAT_OPT_HASURL flag present
    wchar_t *Notes;
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

// --------- Templates ----------

/*
  wParam: not used
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
