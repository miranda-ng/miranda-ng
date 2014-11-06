/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <mshtml.h>

// Miranda header files
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_database.h>
#include <m_netlib.h>
#include <m_icolib.h>
#include <m_message.h>
#include <win2k.h>
#include <m_protomod.h>
#include <m_xml.h>
#include <m_avatars.h>
#include <m_hotkeys.h>

#include <m_folders.h>
#include <m_toptoolbar.h>
#include <m_string.h>

#include "version.h"
#include "resource.h"

#define MODULE	"NewsAggregator"
#define TAGSDEFAULT _T("#<title>#\r\n#<link>#\r\n#<description>#")
#define DEFAULT_AVATARS_FOLDER "NewsAggregator"
#define DEFAULT_UPDATE_TIME 60

extern HINSTANCE hInst;
extern HWND hAddFeedDlg;
extern HANDLE hChangeFeedDlgList, hNetlibUser;
extern UINT_PTR timerId;
// check if Feeds is currently updating
extern BOOL ThreadRunning;
extern BOOL UpdateListFlag;
extern TCHAR tszRoot[MAX_PATH];
struct ItemInfo
{
	HWND hwndList;
	MCONTACT hContact;
	int SelNumber;
	TCHAR nick[MAX_PATH];
	TCHAR url[MAX_PATH];
};

//============  STRUCT USED TO MAKE AN UPDATE LIST  ============

struct NEWSCONTACTLIST {
	MCONTACT hContact;
	struct NEWSCONTACTLIST *next;
};

typedef struct NEWSCONTACTLIST UPDATELIST;

extern UPDATELIST *UpdateListHead;
extern UPDATELIST *UpdateListTail;

void UpdateListAdd(MCONTACT hContact);
void UpdateThreadProc(LPVOID AvatarCheck);
void DestroyUpdateList(void);

extern HANDLE hUpdateMutex;
extern HGENMENU hService2[7];

int NewsAggrInit(WPARAM wParam,LPARAM lParam);
INT OptInit(WPARAM wParam, LPARAM lParam);
int NewsAggrPreShutdown(WPARAM wParam,LPARAM lParam);
VOID NetlibInit();
VOID NetlibUnInit();
VOID InitMenu();
VOID InitIcons();
HICON LoadIconEx(const char* name, BOOL big);
HANDLE  GetIconHandle(const char* name);
INT_PTR NewsAggrGetName(WPARAM wParam, LPARAM lParam);
INT_PTR NewsAggrGetCaps(WPARAM wp, LPARAM lp);
INT_PTR NewsAggrSetStatus(WPARAM wp, LPARAM /*lp*/);
INT_PTR NewsAggrGetStatus(WPARAM/* wp*/, LPARAM/* lp*/);
INT_PTR NewsAggrLoadIcon(WPARAM wParam, LPARAM lParam);
INT_PTR NewsAggrGetInfo(WPARAM wParam, LPARAM lParam);
INT_PTR NewsAggrGetAvatarInfo(WPARAM wParam, LPARAM lParam);
INT_PTR NewsAggrRecvMessage(WPARAM wParam, LPARAM lParam);

INT_PTR CheckAllFeeds(WPARAM wParam, LPARAM lParam);
INT_PTR AddFeed(WPARAM wParam, LPARAM lParam);
INT_PTR ChangeFeed(WPARAM wParam, LPARAM lParam);
INT_PTR ImportFeeds(WPARAM wParam, LPARAM lParam);
INT_PTR ExportFeeds(WPARAM wParam, LPARAM lParam);
INT_PTR CheckFeed(WPARAM wParam, LPARAM lParam);
INT_PTR EnableDisable(WPARAM wParam, LPARAM lParam);
int OnToolbarLoaded(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcAddFeedOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcChangeFeedOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcChangeFeedMenu(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

BOOL IsMyContact(MCONTACT hContact);
VOID GetNewsData(TCHAR *szUrl, char** szData, MCONTACT hContact, HWND hwndDlg);
VOID CreateList(HWND hwndList);
VOID UpdateList(HWND hwndList);
VOID DeleteAllItems(HWND hwndList);
time_t __stdcall DateToUnixTime(const TCHAR *stamp, BOOL FeedType);
VOID CheckCurrentFeed(MCONTACT hContact);
VOID CheckCurrentFeedAvatar(MCONTACT hContact);
TCHAR* CheckFeed(TCHAR* tszURL, HWND hwndDlg);
void UpdateMenu(BOOL State);
int ImportFeedsDialog();
LPCTSTR ClearText(CMString &value, const TCHAR *message);
BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal);
int StrReplace(TCHAR *lpszOld, const TCHAR *lpszNew, TCHAR *&lpszStr);
void CreateAuthString(char *auth, MCONTACT hContact, HWND hwndDlg);
INT_PTR CALLBACK AuthenticationProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcImportOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcExportOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
MCONTACT GetContactByNick(const TCHAR *nick);
MCONTACT GetContactByURL(const TCHAR *url);

// ===============  NewsAggregator SERVICES  ================
// Check all Feeds info
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_CHECKALLFEEDS	"NewsAggregator/CheckAllFeeds"

// Add new Feed channel
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_ADDFEED	"NewsAggregator/AddNewsFeed"

// Change Feed channel
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_CHANGEFEED	"NewsAggregator/ChangeNewsFeed"

// Import Feed channels from file
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_IMPORTFEEDS	"NewsAggregator/ImportFeeds"

// Export Feed channels to file
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_EXPORTFEEDS	"NewsAggregator/ExportFeeds"

// Check Feed info
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_CHECKFEED	"NewsAggregator/CheckFeed"

// Enable/disable getting feed info
// WPARAM = LPARAM = NULL
#define MS_NEWSAGGREGATOR_ENABLED	"NewsAggregator/Enabled"