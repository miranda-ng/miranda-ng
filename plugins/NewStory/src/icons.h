struct MyIconDesriptor
{
	DWORD dwIndex;

	LPSTR szId;
	LPSTR szGroup;
	LPSTR szTitle;

	HICON hIcon;
};

enum
{
	ICO_NEWSTORY,	// 0
	ICO_USERINFO,	// 1
	ICO_USERMENU,	// 2
	ICO_SEARCH,		// 3
	ICO_OPTIONS,	// 4
	ICO_FILTER,		// 5
	ICO_EXPORT,		// 6
	ICO_COPY,		// 7
	ICO_SENDMSG,	// 8
	ICO_CLOSE,		// 9
	ICO_MSGIN,		// 10
	ICO_MSGOUT,		// 11
	ICO_SIGNIN,		// 12
	ICO_FILE,		// 13
	ICO_URL,		// 14
	ICO_UNKNOWN,	// 15
	ICO_FINDPREV,	// 16
	ICO_FINDNEXT,	// 17
	ICO_NOPASSWORD,	// 18
	ICO_PASSWORD,	// 19
	ICO_CALENDAR,	// 20
	ICO_TPLGROUP,	// 21
	ICO_RESET,		// 22
	ICO_PREVIEW,	// 23
	ICO_VARHELP,	// 24
	ICO_SAVEPASS,	// 25
	ICO_COUNT		// 26
};

extern MyIconDesriptor icons[];
extern int iconCount;

void InitIcons(MyIconDesriptor *icons, LPSTR szFilename, int count);
void RefreshIcons(MyIconDesriptor *icons, int count);