/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-04 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_userinfo.h>
#include <m_acc.h>
#include <m_imgsrvc.h>

#include <m_folders.h>
#include <m_metacontacts.h>
#include <m_avatarhistory.h>
#include <m_clistint.h>

#include "resource.h"
#include "version.h"
#include "image_utils.h"
#include "poll.h"
#include "acc.h"

#ifndef SHVIEW_THUMBNAIL
#define SHVIEW_THUMBNAIL 0x702D
#endif

/* most free()'s are invalid when the code is executed from a dll, so this changes
 all the bad free()'s to good ones, however it's still incorrect code. The reasons for not
 changing them include:

  * db_unset has a CallService() lookup
  * free() is executed in some large loops to do with clist creation of group data
  * easy search and replace

*/

struct CacheNode : public avatarCacheEntry, public MZeroedObject
{
	CacheNode();
	~CacheNode();

	BOOL   loaded;
	DWORD  dwFlags;
	int    pa_format;

	void   wipeInfo();
};

// The same fields as avatarCacheEntry + proto name
struct protoPicCacheEntry : public avatarCacheEntry, public MZeroedObject
{
	protoPicCacheEntry() { memset(this, 0, sizeof(*this)); };
	~protoPicCacheEntry();

	void clear();

	char*  szProtoname;
	TCHAR* tszAccName;
};

extern OBJLIST<protoPicCacheEntry> g_ProtoPictures, g_MyAvatars;

struct SetMyAvatarHookData
{
	char *protocol;
	BOOL square;
	BOOL grow;

	BOOL thumbnail;
};

extern FI_INTERFACE *fei;

#define GAIR_FAILED 1000

#define AVS_IGNORENOTIFY 0x1000

#define AVS_DEFAULT "Global avatar"

void mir_sleep(int time);
extern bool  g_shutDown;
extern TCHAR g_szDataPath[];		// user datae path (read at startup only)
extern BOOL  g_AvatarHistoryAvail;
extern HWND  hwndSetMyAvatar;

extern HINSTANCE g_hInst;

extern HANDLE hMyAvatarsFolder;
extern HANDLE hGlobalAvatarFolder;
extern HANDLE hLoaderEvent, hShutdownEvent;
extern HANDLE hEventChanged, hEventContactAvatarChanged, hMyAvatarChanged;

int   GetFileHash(TCHAR* filename);
DWORD GetFileSize(TCHAR *szFilename);
void  MakePathRelative(MCONTACT hContact);
void  MakePathRelative(MCONTACT hContact, TCHAR *dest);

HBITMAP LoadPNG(struct avatarCacheEntry *ace, char *szFilename);

void UnloadCache(void);
int  CreateAvatarInCache(MCONTACT hContact, avatarCacheEntry *ace, char *szProto);
void DeleteAvatarFromCache(MCONTACT hContact, bool bForever);
void PicLoader(LPVOID param);

void InternalDrawAvatar(AVATARDRAWREQUEST *r, HBITMAP hbm, LONG bmWidth, LONG bmHeight, DWORD dwFlags);

int ChangeAvatar(MCONTACT hContact, bool fLoad, bool fNotifyHist = false, int pa_format = 0);
void DeleteGlobalUserAvatar();
int  FetchAvatarFor(MCONTACT hContact, char *szProto = NULL);
CacheNode* FindAvatarInCache(MCONTACT hContact, bool add, bool findAny = false);
void PushAvatarRequest(CacheNode *cc);
int  SetAvatarAttribute(MCONTACT hContact, DWORD attrib, int mode);
void SetIgnoreNotify(char *protocol, BOOL ignore);

INT_PTR DrawAvatarPicture(WPARAM wParam, LPARAM lParam);
INT_PTR GetAvatarBitmap(WPARAM wParam, LPARAM lParam);
INT_PTR GetMyAvatar(WPARAM wParam, LPARAM lParam);
INT_PTR ProtectAvatar(WPARAM wParam, LPARAM lParam);
INT_PTR ReportMyAvatarChanged(WPARAM wParam, LPARAM lParam);

MCONTACT GetContactThatHaveTheAvatar(MCONTACT hContact, int locked = -1);

void ProcessAvatarInfo(MCONTACT hContact, int type, PROTO_AVATAR_INFORMATIONT *pai, const char *szProto);

int  Proto_GetDelayAfterFail(const char *proto);
BOOL Proto_NeedDelaysForAvatars(const char *proto);
BOOL Proto_IsAvatarsEnabled(const char *proto);
BOOL Proto_IsAvatarFormatSupported(const char *proto, int format);
int  Proto_AvatarImageProportion(const char *proto);
void Proto_GetAvatarMaxSize(const char *proto, int *width, int *height);
int  Proto_GetAvatarMaxFileSize(const char *proto);

protoPicCacheEntry* GetProtoDefaultAvatar(MCONTACT hContact);
