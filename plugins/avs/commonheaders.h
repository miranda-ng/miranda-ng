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

#define MIRANDA_VER  0x0A00
#define _WIN32_WINNT 0x0501

#include "m_stdhdr.h"

#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include <math.h>
#include <win2k.h>

#include <newpluginapi.h>
#include <m_system_cpp.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_plugins.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_file.h>
#include <m_addcontact.h>
#include <m_png.h>
#include <m_userinfo.h>
#include "m_folders.h"

#include <m_avatars.h>
#include <m_popup.h>
#include <m_imgsrvc.h>
#include "m_metacontacts.h"
#include "m_avatarhistory.h"

#include "resource.h"
#include "m_updater.h"
#include "m_flash.h"
#include "image_utils.h"
#include "mir_thread.h"
#include "poll.h"
#include "m_acc.h"
#include "acc.h"


// shared vars
//extern HINSTANCE g_hInst;

/* most free()'s are invalid when the code is executed from a dll, so this changes
 all the bad free()'s to good ones, however it's still incorrect code. The reasons for not
 changing them include:

  * DBFreeVariant has a CallService() lookup
  * free() is executed in some large loops to do with clist creation of group data
  * easy search and replace

*/

// The same fields as avatarCacheEntry + proto name
struct protoPicCacheEntry : public avatarCacheEntry
{
	__inline void* operator new( size_t size ) { return mir_alloc( size ); }
	__inline void operator delete( void* p ) { mir_free( p ); }

    protoPicCacheEntry() { memset(this, 0, sizeof(*this)); };
	~protoPicCacheEntry();

	void clear();

	char*  szProtoname;
	TCHAR* tszAccName;
};

extern OBJLIST<protoPicCacheEntry> g_ProtoPictures, g_MyAvatars;


int SetAvatarAttribute(HANDLE hContact, DWORD attrib, int mode);

#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define GAIR_FAILED 1000

#define AVS_IGNORENOTIFY 0x1000

#define AVS_DEFAULT "Global avatar"
