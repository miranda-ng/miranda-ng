/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project,
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
#include <m_flash.h>

#include "resource.h"
#include "version.h"
#include "image_utils.h"
#include "poll.h"
#include "acc.h"


// shared vars
//extern HINSTANCE g_hInst;

/* most free()'s are invalid when the code is executed from a dll, so this changes
 all the bad free()'s to good ones, however it's still incorrect code. The reasons for not
 changing them include:

  * db_unset has a CallService() lookup
  * free() is executed in some large loops to do with clist creation of group data
  * easy search and replace

*/

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

extern FI_INTERFACE *fei;

int SetAvatarAttribute(HANDLE hContact, DWORD attrib, int mode);

#define GAIR_FAILED 1000

#define AVS_IGNORENOTIFY 0x1000

#define AVS_DEFAULT "Global avatar"

void mir_sleep(int time);
extern bool g_shutDown;