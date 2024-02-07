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

#pragma once

// Windows Header Files:
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <mshtml.h>

// Miranda header files
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_database.h>
#include <m_netlib.h>
#include <m_icolib.h>
#include <m_message.h>
#include <m_xml.h>
#include <m_avatars.h>
#include <m_hotkeys.h>
#include <m_gui.h>

#include <m_folders.h>
#include <m_toptoolbar.h>

#include "Options.h"
#include "version.h"
#include "resource.h"

#define MODULENAME	"NewsAggregator"
#define TAGSDEFAULT L"#<title>#\r\n#<link>#\r\n#<description>#"
#define DEFAULT_AVATARS_FOLDER "NewsAggregator"
#define DEFAULT_UPDATE_TIME 60

extern HNETLIBUSER hNetlibUser;
extern HANDLE hTBButton;
extern UINT_PTR timerId;

// check if Feeds is currently updating
extern bool ThreadRunning;
extern bool UpdateListFlag;
extern wchar_t tszRoot[MAX_PATH];
extern int g_nStatus;

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
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
void UpdateThreadProc(void*);
void DestroyUpdateList(void);

extern    HANDLE hUpdateMutex;
		    
int       NewsAggrInit(WPARAM wParam,LPARAM lParam);
int       OptInit(WPARAM wParam, LPARAM lParam);
int       NewsAggrPreShutdown(WPARAM wParam,LPARAM lParam);
void      NetlibInit();
void      NetlibUnInit();
void      InitMenu();
void      InitIcons();
		    
INT_PTR   NewsAggrGetCaps(WPARAM wp, LPARAM lp);
INT_PTR   NewsAggrSetStatus(WPARAM wp, LPARAM /*lp*/);
INT_PTR   NewsAggrGetStatus(WPARAM/* wp*/, LPARAM/* lp*/);
INT_PTR   NewsAggrLoadIcon(WPARAM wParam, LPARAM lParam);
INT_PTR   NewsAggrGetInfo(WPARAM wParam, LPARAM lParam);
INT_PTR   NewsAggrGetAvatarInfo(WPARAM wParam, LPARAM lParam);
INT_PTR   NewsAggrRecvMessage(WPARAM wParam, LPARAM lParam);
		    
INT_PTR   CheckAllFeeds(WPARAM wParam, LPARAM lParam);
INT_PTR   AddFeed(WPARAM wParam, LPARAM lParam);
INT_PTR   ChangeFeed(WPARAM wParam, LPARAM lParam);
INT_PTR   ImportFeeds(WPARAM wParam, LPARAM lParam);
INT_PTR   ExportFeeds(WPARAM wParam, LPARAM lParam);
INT_PTR   CheckFeed(WPARAM wParam, LPARAM lParam);
INT_PTR   EnableDisable(WPARAM wParam, LPARAM lParam);
int       OnToolbarLoaded(WPARAM wParam, LPARAM lParam);
CDlgBase* FindFeedEditor(const wchar_t *pwszNick, const wchar_t *pwszUrl);

bool      IsMyContact(MCONTACT hContact);
void      GetNewsData(wchar_t *szUrl, char **szData, MCONTACT hContact, CFeedEditor *pEditDlg);
time_t    DateToUnixTime(const char *stamp, bool FeedType);
void      CheckCurrentFeed(MCONTACT hContact);
void      CheckCurrentFeedAvatar(MCONTACT hContact);
LPCTSTR   CheckFeed(wchar_t* tszURL, CFeedEditor *pEditDlg);
void      UpdateMenu(bool State);
LPCTSTR   ClearText(CMStringW &value, const wchar_t *message);
bool      DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal);
void      CreateAuthString(char *auth, MCONTACT hContact, CFeedEditor *pDlg);
MCONTACT  GetContactByNick(const wchar_t *nick);
MCONTACT  GetContactByURL(const wchar_t *url);

void CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


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
