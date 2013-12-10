#ifdef UNICODE
  #ifndef _UNICODE
    #define _UNICODE
  #endif
#endif

//#ifdef VCL_VC60
#include "AggressiveOptimize.h"
//#endif

//#include <math.h>

#ifdef _DEBUG
#ifndef UNICODE
  #define AALOG
  #define AALOG1
#endif
#endif

#define _WIN32_WINNT 0x0700
#ifndef SAA_PLUGIN
 #include "../../src/core/commonheaders.h"
#else 
  #include <windows.h>
  #include <commctrl.h>
  #include <uxtheme.h>
  #include <win2k.h> 
  #include <stdio.h>
  #include "newpluginapi.h"
  #include <m_system.h>
  #include <m_langpack.h>
  #include <m_database.h>
  #include <m_clist.h>
  #include <m_options.h>
  #include <m_awaymsg.h>
  #include <m_protosvc.h>
  #include "m_idle.h"
  #ifndef time
    #include <time.h>
  #endif
  #define offsetof(s,m)   (size_t)&(((s *)0)->m)
  extern BOOL (WINAPI *enableThemeDialogTexture)(HANDLE, DWORD);
#ifdef AALOG 
	#include <m_netlib.h>
#endif
#endif
#include <m_hotkeys.h>


//#ifdef SAA_PLUGIN
//	extern struct MM_INTERFACE memoryManagerInterface;
//	#define mir_alloc(n) memoryManagerInterface.mmi_malloc(n)
//	#define mir_free(ptr) memoryManagerInterface.mmi_free(ptr)
//	#define mir_realloc(ptr,size) memoryManagerInterface.mmi_realloc(ptr,size)
//#endif

#define SECTIONNAME "Smart Auto Away"

  #include "resource.h"
//#define IDF_ISIDLE		0x1 // idle has become active (if not set, inactive)
#define IDF_SHORT		0x2 // short idle mode
#define IDF_LONG		0x4 // long idle mode
//#define IDF_PRIVACY		0x8 // if set, the information provided shouldn't be given to third parties.
#define IDF_ONFORCE	   0x10 // screensaver/onstationlocked,etc cause instant idle

extern int FirstTick;

#ifdef UNICODE
extern bool *protoHasAwayMessageW;
extern bool HasAwayMessageW;
#endif

extern HOTKEYDESC hotkeydescs[];

//typedef struct {
//	int cbSize;			     // sizeof()
//	unsigned int idleTime;	 // idle in mins, if zero then disabled
//	int privacy;		     // user doesnt want other people seeing anything more than they are idle
//  int aaStatus; // status to go to when user is auto away
//} MIRANDA_IDLE_INFO;
//typedef struct {
//	int cbSize;			// sizeof()	
//	int enabled;		// monitoring is going on
//	int idleShortTime;	// short idle in mins, if zero then disabled
//	int idleLongTime;	// long idle in mins, if zero then disabled
//	int privacy;		// user doesnt want other people seeing anything more than they are idle
//} MIRANDA_IDLE_INFO;

//typedef struct{
//	int cbSize;	//sizeof	//5*sizeof(int) or 4*sizeof(int)
//	unsigned int fieldTime; //enabled or idleTime in minutes
//	int field1;				//idleShortTime or privacy
//	int field2;				//idleLongTime or aaStatus
//	// the "new structure ends here"
//	int field3;				//privacy or none
//} MYIDLEINFO;
//#define MS_IDLE_GETIDLEINFO "Miranda/Idle/GetInfo"
//#define ME_IDLE_CHANGED "Miranda/Idle/Changed"
extern int isCurrentlyIdle;

extern  unsigned long idleOptsPerm;		 // CMHG SL S SM P  L S --LONG-- --SHORT-
extern unsigned long idleOpts;			 // C  G  L S tM P  L S --LONG-- --SHORT-
#define idleOptsDefault		0x95851E0A	 // 1001 0101 1000 0101 00011110 00001010
#define IdleBitsTimeFirst	0x000000FF	 // ---- ---- ---- ---- -------- 11111111
#define IdleBitsTimeSecond	0x0000FF00	 // ---- ---- ---- ---- 11111111 --------
#define IdleBitsCheck		0x80000000	 // 1000 0000 0000 0000 00000000 00000000
#define IdleBitsMethod		0x00400000	 // 0000 0000 0100 0000 00000000 00000000
#define IdleBitsFirstOn		0x00010000	 // 0000 0000 0000 0001 00000000 00000000
#define IdleBitsSecondOn	0x00040000	 // 0000 0000 0000 0100 00000000 00000000
#define IdleBitsOnSaver		0x01000000	 // 0000 0001 0000 0000 00000000 00000000
#define IdleBitsOnLock		0x04000000   // 0000 0100 0000 0000 00000000 00000000
#define IdleBitsGLI			0x10000000   // 0001 0000 0000 0000 00000000 00000000
#define IdleBitsPrivate		0x00100000	 // 0000 0000 0001 0000 00000000 00000000
#define IdleSuppressIdleMenu 0x40000000	 // 0100 0000 0000 0000 00000000 00000000
#define IdleSuppressAutoHere 0x20000000	 // 0010 0000 0000 0000 00000000 00000000
#define KeepMainStatusOffline 0x08000000 // 0000 1000 0000 0000 00000000 00000000
#define IdleHereOnStatusChange 0x00800000// 0000 0000 1000 0000 00000000 00000000




#ifdef SAA_PLUGIN
  int __cdecl LoadAutoAwayModule(void);
  extern HINSTANCE g_hInst;
#endif


#define MS_SYSTEM_GETIDLE "Miranda/GetIdle"
#ifdef AALOG
	extern HANDLE hNetlib;
#endif
static int InitVariables( WPARAM wParam, LPARAM lParam );

extern HANDLE hIdleEvent;
extern UINT_PTR hIdleTimer;

extern INT_PTR CALLBACK DlgProcAutoAwayOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern INT_PTR CALLBACK DlgProcOptsIdle(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


int AutoAwayOptInitialise(WPARAM wParam,LPARAM lParam);
//extern int AutoAwayShutdown(WPARAM wParam,LPARAM lParam);

#define IDLEMODULE "Idle"
// live settings
//extern int idleCheckPerm;
//extern int idleMethodPerm;
//extern int idleTimeFirstPerm; // this is in mins
//extern int idleTimeSecondPerm; // 
//extern int idleTimeFirstOnPerm;
//extern int idleTimeSecondOnPerm;
//extern int idleOnSaverPerm;
//extern int idleOnLockPerm;
//extern int idleGLIPerm;
//extern int idlePrivatePerm;

//We will not try to import previous Idle settings anymore
// db settings keys
//#define IDL_IDLECHECK "IdleCheck"
//#define IDL_IDLEMETHOD "IdleMethod"
//#define IDL_IDLEGLI		"IdleGLI"
//#define IDL_IDLETIME1ST "IdleTime1st"
//#define IDL_IDLETIME2ND "IdleTime2nd"
//#define IDL_IDLEONSAVER "IdleOnSaver"
//#define IDL_IDLEONLOCK "IdleOnLock"
//#define IDL_IDLETIME1STON "IdleTime1stOn"
//#define IDL_IDLETIME2NDON "IdleTime2ndOn"
//#define IDL_IDLEPRIVATE "IdlePrivate"

extern BOOL (WINAPI * MyGetLastInputInfo)(PLASTINPUTINFO);
extern INT_PTR IdleGetInfo(WPARAM wParam, LPARAM lParam);

VOID CALLBACK IdleTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);
BOOL InitDialogIdlePart(HWND hwndDlg);


//AutoAway Module
#define AA_IDLEOPTIONS "IdleOptions"
#define AA_LASTIDLESTATUS "IdleLastStatus"
#define AA_LASTIDLETIME "IdleLastTime"
#define AA_OLDMODULE "AutoAway"
#define AA_USESHORT "IsShortEnabled"
#define AA_USELONG "IsLongEnabled"
#define AA_SHORTSTATUS "ShortStatus"
#define AA_LONGSTATUS "LongStatus"

#define AA_MODULE "SmartAutoAway"

#define AA_PROTOSELECTION "ProtoSelection"

#define AA_AWAYSTATUSES "AwayStatuses_%s" 
#define AA_BITSONLYIF "BitsOnlyIf_%s"
#define AA_LASTREQUESTEDSTATUS "LastRequestedStatus_%s"

//#define AA_SIMULATEIDLESERVICE "SAA/SimulateIdle"
#define AA_IDLE_BENOTIDLESERVICE "SAA/IdleBeNotIdle"
MIRANDASERVICE idleServiceNotIdle(WPARAM w, LPARAM l);
#define AA_IDLE_BESHORTIDLESERVICE "SAA/IdleBeShortIdle"
MIRANDASERVICE idleServiceShortIdle(WPARAM w, LPARAM l);
#define AA_IDLE_BELONGIDLESERVICE "SAA/IdleBeLongIdle"
MIRANDASERVICE idleServiceLongIdle(WPARAM w, LPARAM l);
#define AA_IDLE_RECONNECTSERVICE "SAA/ForceReconnect"
MIRANDASERVICE reconnectService(WPARAM w, LPARAM l);

//#define aa_AwayStatusesDefault ((StatusToProtoIndex(ID_STATUS_AWAY,allprotomodes)<<8)|StatusToProtoIndex(ID_STATUS_NA,allprotomodes)) | 0x8000
						//K--- --21 LLLL SSSS
#define maskIsShortAwayEnabled 0x0100
#define maskIsLongAwayEnabled 0x0200
#define maskShortAwayStatus 0x000F
#define maskLongAwayStatus 0x00F0

#define allprotomodes			0x03FF		// 0000 0011 1111 1111;
#define aa_OnlyIfBitsDefault	0x01BB01B8	//Short 0001 1011 1000;
											//Long	0001 1011 1011


extern HANDLE hStatusModeChangeEvent;

extern int aa_Status[];
extern int numStatuses;

extern int *reqStatus;
extern int *courStatus;
extern int *courAwayStatus; //to be used in reconnect part

extern int *protoModes;

extern short int *awayStatusesPerm;
extern long *onlyIfBitsPerm;
extern short int *awayStatuses;
extern long *onlyIfBits;


extern HWND theDialog;
extern HWND theDialogReconnect;
extern HWND theDialogIdle;
extern HWND theDialogAA;
extern HWND theDialogIdleMessages;


extern PROTOACCOUNT **accounts;
extern int protoCount;
extern int courProtocolSelection;

unsigned long GetBits (unsigned long source,unsigned long mask);
unsigned long SetBits (unsigned long target,unsigned long mask,unsigned long value);
BOOL IsStatusBitSet(int status, int bits);
int StatusByProtoIndex(int modes, int index);
int StatusToProtoIndex(int status, int modes);

int OldIndexToStatus(int oldIndex);
int StatusToOldIndex(int status);

LRESULT SetDlgItems(HWND hwndDlg, int coursel);
LRESULT SetDlgItemsReconnect(HWND hwndDlg, int coursel);
LRESULT SetDlgItemsAA(HWND hwndDlg, int coursel);
LRESULT SetDlgItemsIdleMessages(HWND hwndDlg, int coursel);

int StatusToProtoIndex(int status, int modes);
int GetCourSelProtocol(int sel);
BOOL StrikeOut(HWND hWnd, int item, BOOL striked);

int AutoAwayShutdown(WPARAM wParam,LPARAM lParam);
int AutoAwayEvent(WPARAM wParam, LPARAM lParam);
//extern int SimulateIdle(HWND theDialog, int button);
int StatusModeChangeEvent(WPARAM wParam,LPARAM lParam);
int ProtoAckEvent(WPARAM wParam,LPARAM lParam);
int SimulateIdle(int idlstatus);


//Reconnect Part
int GetReconnectDelayFromSlider(int slide);
BOOL GetStringFromDelay(TCHAR * str, int delay);
int GetSliderFromReconnectDelay(int delay);
void ProtocolsProc(int protoIndex, ACKDATA * ack);
void LoginAck(int protoIndex, ACKDATA * ack);
void TimerProc();
extern INT_PTR CALLBACK DlgProcOptsReconnect(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define AA_RECONNECTOPTS "ReconnectOptions_%s"
#define aa_ReconnectOptsDefault	0x003CC1AE	
//-----60 sec-------- 1100 0001 1010 1110
//DDDD DDDD DDDD DDDD Rfxx xxFN ANOD FIPL
#define aa_ReconnectBitsDelay	0xFFFF0000
#define aa_ReconnectBitsEnabled	0x00008000
#define aa_ReconnectNotFatalError	0x00004000

extern INT_PTR CALLBACK DlgProcOptsAA(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern long *reconnectOpts;
extern long *reconnectOptsPerm;
extern unsigned int *protoOfflineTimes;
extern int *protoStatus; //0:OK; :1:couning; 2:fatalError;

//IdleMessage part
extern BOOL starting;
extern int *isWaitingForRestoreStatusMessage;
extern unsigned long secondsAfterLastActivity;


#define idleMsgOptionsTextHere "IdleMsgTextHere_%s"
#define idleMsgOptionsTextShort "IdleMsgTextShort_%s"
#define idleMsgOptionsTextLong "IdleMsgTextLong_%s"
#define messHereDefaultOld _T("I am back")
#define messShortDefaultOld  _T("I am distracted")
#define messLongDefaultOld  _T("I am sleeping")
#define messHereDefault _T("I came back on %H:%M:%S%b%Z (%z)")
#define messShortDefault  _T("I am distracted since %H:%M:%S%b%Z (%z)")
#define messLongDefault  _T("I fell asleep on %H:%M:%S %d %E %Y (%W)%b%Z (%z)")
#define maxMessageLength 2047
extern TCHAR **mesgHere;
extern TCHAR **mesgShort;
extern TCHAR **mesgLong;
extern TCHAR **mesgHerePerm;
extern TCHAR **mesgShortPerm;
extern TCHAR **mesgLongPerm;

#define idleMsgOptionsName "IdleMsgOpts_%s"
#define idleMsgOptionsDefault 0x0111103C
extern int *messCaps;

extern INT_PTR CALLBACK DlgProcOptsIdleMessages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern long *idleMessOptsPerm;				
extern long *idleMessOpts;					
                                                // xxxx xxxL xxxS xxxH xxxE xxx- RSTHERE-
#define IdleMessOptsMaskRstHereDelay 0x000001FF // 0000 0000 0000 0000 0000 0001 11111111
#define IdleMessOptsMaskRstHere      0x00001000 // 0000 0000 0000 0000 0001 0000 00000000
#define IdleMessOptsMaskUseHere      0x00010000 // 0000 0000 0000 0001 0000 0000 00000000
#define IdleMessOptsMaskUseShort     0x00100000 // 0000 0000 0001 0000 0000 0000 00000000
#define IdleMessOptsMaskUseLong      0x01000000 // 0000 0001 0000 0000 0000 0000 00000000

extern int localeID;
extern int codePage;
void my_u2a(const TCHAR* src, char * target, unsigned int max);
void my_a2u(const char* src, TCHAR * target, unsigned int max);
void GetLastActiveLocalTime(SYSTEMTIME * at, unsigned long miliSeconds);
int GetTimeZone(TCHAR * name);
UINT ParseString(TCHAR *szstring,TCHAR *sztemp, unsigned int MaxOutLen);

//borrowed from "Last Seen Plugin"
//#define VARIABLE_LIST "%s\n%%Y: \t %s\n%%y: \t %s\n%%m: \t %s\n%%E: \t %s\n%%e: \t %s\n%%d: \t %s\n%%W: \t %s\n%%w: \t %s\n\n%s\n%%H: \t %s\n%%h: \t %s\n%%p: \t %s\n%%M: \t %s\n%%S: \t %s\n\n%s\n%%z: \t %s\n%%Z: \t %s\n\n%s\n%%t: \t %s\n%%b: \t %s",Translate("-- Date --"),Translate("year (4 digits)"),Translate("year (2 digits)"),Translate("month"),Translate("name of month"),Translate("short name of month"),Translate("day"),Translate("weekday (full)"),Translate("weekday (abbreviated)"),Translate("-- Time --"),Translate("hours (24)"),Translate("hours (12)"),Translate("AM/PM"),Translate("minutes"),Translate("seconds"),Translate("-- Time Zone --"),Translate("total bias"),Translate("time zone name"),Translate("-- Format --"),Translate("tabulator"),Translate("line break")
extern TCHAR *VariableList[][2];

void AddIdleMenu();
extern HANDLE hHookIconsChanged;
void xModifyMenu(int menuInd,long flags);
BOOL isInterestingProto(int j);
