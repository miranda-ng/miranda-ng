#ifndef _CCONFIG_H_
#define _CCONFIG_H_

// Tabs
#define TAB_OPTIONS 0
#define TAB_NOTIFICATIONS 1
#define TAB_CHAT 2
#define TAB_CLIST 3

// Fonts
#define FONT_LOG 0
#define FONT_SESSION 1
#define FONT_CLIST 2
#define FONT_TITLE 3
#define FONT_NOTIFICATION 4

#define FONT_SETTINGS 5

// Bool settings
#define CLIST_HIDEOFFLINE 0
#define NOTIFY_MESSAGES 1
#define NOTIFY_SIGNON 2
#define NOTIFY_SIGNOFF 3
#define NOTIFY_STATUS 4
#define SESSION_LOADDB 5
#define SHOW_LABELS 6
#define SESSION_MARKREAD 7
#define NOTIFY_CONTACTS 8
#define NOTIFY_FILE 9
#define NOTIFY_URL 10
#define SESSION_SENDRETURN 11
#define NOTIFY_PROTO_STATUS 12
#define SESSION_SHOWTYPING 13
#define SESSION_SENDTYPING 14
#define SESSION_SCROLL_MAXIMIZED 15
#define SESSION_REPLY_MAXIMIZED 16
#define MAXIMIZED_TITLE 17
#define MAXIMIZED_LABELS 18
#define CLIST_USEGROUPS 19
#define CLIST_USEIGNORE 20
#define NOTIFY_PROTO_SIGNON 21
#define NOTIFY_PROTO_SIGNOFF 22
#define CLIST_SHOWPROTO 23
#define SESSION_SYMBOLS 24
#define CLIST_COUNTERS 24
#define CLIST_DRAWLINES 25
#define CLIST_POSITION 26
#define CONTROL_BACKLIGHTS 28
#define HOOK_VOLUMEWHEEL 29
#define NOTIFY_SKIP_MESSAGES 30
#define NOTIFY_SKIP_STATUS 31
#define NOTIFY_SKIP_SIGNON 32
#define NOTIFY_SKIP_SIGNOFF 33
#define SESSION_TIMESTAMPS 34
#define NOTIFY_TIMESTAMPS 35
#define TIMESTAMP_SECONDS 36
#define NOTIFY_IRC_MESSAGES 37
#define NOTIFY_IRC_USERS 38
#define NOTIFY_IRC_EMOTES 39
#define NOTIFY_IRC_NOTICES 40
#define NOTIFY_IRC_CHANNEL 41
#define NOTIFY_IRC_STATUS 42
#define CLIST_COLUMNS 43
#define TRANSITIONS 44
#define NOTIFY_NICKCUTOFF 45
#define NOTIFY_SHOWPROTO 46
#define NOTIFY_NO_SKIP_REPLY 47
#define CLIST_SELECTION 48
#define NOTIFY_CHANNELCUTOFF 49
#define SCREENSAVER_LOCK 50
#define SESSION_CLOSE 52
#define SKIP_DRIVER_ERROR 53

#define BOOL_SETTINGS 54

// Int Settings
#define SESSION_LOGSIZE 0
#define NOTIFY_LOGSIZE 1
#define NOTIFY_DURATION 2
#define NOTIFY_TITLE 3
#define SESSION_AUTOSCROLL 4
#define CLIST_GA 5
#define NOTIFY_NICKCUTOFF_OFFSET 6
#define DEVICE 7
#define NOTIFY_CHANNELCUTOFF_OFFSET 8
#define SESSION_CLOSETIMER 9

#define INT_SETTINGS 10

// Enums
#define CLIST_GA_NONE 0
#define CLIST_GA_EXPAND 1
#define CLIST_GA_COLLAPSE 2

#define NOTIFY_TITLE_HIDE 0
#define NOTIFY_TITLE_INFO 1
#define NOTIFY_TITLE_NAME 2

#define SESSION_AUTOSCROLL_NONE 0
#define SESSION_AUTOSCROLL_FIRST 1
#define SESSION_AUTOSCROLL_LAST 2

struct CProtoFilter
{
	tstring strName;
	bool bNotificationFilter;
	bool bContactlistFilter;

	bool bTempNotificationFilter;
	bool bTempContactlistFilter;
};

class CConfig
{
public:
	static void Initialize();
	static void Shutdown();

	static void LoadSettings();
	static void SaveSettings();
	
	// InitDialogHook Callback
	static int InitOptionsDialog(WPARAM wParam, LPARAM lParam);
	// OnConnectionChanged
	static void OnConnectionChanged();

	// Dialog WndProc
	static INT_PTR CALLBACK AppearanceDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ChatDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK NotificationsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ContactlistDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static bool GetBoolSetting(int iSetting);
	static int GetIntSetting(int iSetting);
	static bool GetProtocolNotificationFilter(tstring strProtocol);
	static bool GetProtocolContactlistFilter(tstring strProtocol);

	static LOGFONT &GetFont(int iFont);
	static int GetFontHeight(int iFont);
private:
	static int GetSampleField(int iFont);
	static void ClearTree(HWND hTree);
	static void FillTree(HWND hTree,bool bCList=false);
	static void FillDeviceBox(HWND hBox);
	
	static HWND hDeviceBox;
	static vector<CProtoFilter*> m_ProtoList;
	
	static HANDLE m_hSampleFont[FONT_SETTINGS];
	static TEXTMETRIC m_tmFontInfo[FONT_SETTINGS];
	static LOGFONT m_logfont[FONT_SETTINGS];
	static LOGFONT m_templogfont[FONT_SETTINGS];

	static void LoadFontSettings(int iFont);
	static void SaveFontSettings(int iFont);

	static void UpdateFontSettings(int iFont);

	static bool m_abBoolSettings[BOOL_SETTINGS];
	static int m_aiIntSettings[INT_SETTINGS];
	static bool m_bInitializingDialog;
	static HANDLE m_hOptionsInitHook;
	
};

#endif