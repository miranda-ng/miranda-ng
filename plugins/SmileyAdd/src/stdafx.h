/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GENERAL_
#define _GENERAL_

#define _SCL_SECURE_NO_WARNINGS

#define _HAS_EXCEPTIONS 0
#define _SECURE_SCL 0
#define _SECURE_SCL_THROWS 0
#define _STRALIGN_USE_SECURE_CRT 0
#define _NO_EXCEPTIONS
#define _CRTDBG_MAP_ALLOC

#include <windows.h>
#include <richedit.h>
#include <richole.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <tom.h>
#include <msapi/comptr.h>

#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <time.h>
#include <delayimp.h>

#include <vector>
#include <map>
#include <algorithm>

#define NETLIB_NOLOGGING

#include <newpluginapi.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_folders.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>
#include <m_genmenu.h>
#include <m_gui.h>
#include <m_icolib.h>
#include <m_imgsrvc.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_metacontacts.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_regexp.h>
#include <m_smileyadd.h>
#include <m_xml.h>

#include "resource.h"
#include "version.h"
#include "imagecache.h"
#include "SmileyBase.h"
#include "smileys.h"
#include "customsmiley.h"
#include "services.h"
#include "options.h"
#include "download.h"
#include "anim.h"
#include "smileyroutines.h"
#include "smltool.h"

extern HNETLIBUSER hNetlibUser;
extern HANDLE g_hevOptionsChanged;
extern HGENMENU hContactMenuItem;
extern SmileyCategoryListType g_SmileyCategories;
extern SmileyPackListType g_SmileyPacks;
extern LIST<void> menuHandleArray;

#define MODULENAME "SmileyAdd"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;

	wchar_t wszDefaultPath[MAX_PATH];
};

#define WM_REMAKERICH (WM_USER + 0x3457)

/////////////////////////////////////////////////////

#define MAX_SMILEY_LENGTH    96

// init functions
void InstallDialogBoxHook(void);
void RemoveDialogBoxHook(void);

int  UpdateSrmmDlg(WPARAM, LPARAM);
int  SmileyButtonCreate(WPARAM, LPARAM);
int  SmileyButtonPressed(WPARAM, LPARAM);

// functions for general use (defined in general.cpp)
int CalculateTextHeight(HDC hdc, CHARFORMAT2 *chf);

MCONTACT DecodeMetaContact(MCONTACT hContact);
bool IsSmileyProto(char *proto);

HICON ImageList_GetIconFixed (HIMAGELIST himl, INT i, UINT fStyle);

bool InitGdiPlus(void);
void DestroyGdiPlus(void);

void ReportError(const wchar_t *errmsg);
HICON GetDefaultIcon(bool copy = true);

void CloseRichCallback(HWND hwnd);
void CloseRichOwnerCallback(HWND hwnd);
bool SetRichCallback(HWND hwnd, MCONTACT hContact, bool subany, bool subnew);
void SetRichOwnerCallback(HWND hwnd, HWND hwndInput, HWND hwndLog);
void ProcessAllInputAreas(bool restoreText);
void RichEditData_Destroy(void);

void CloseSmileys(void);
int CheckForTip(int x, int y, HWND hwnd, wchar_t **smltxt);

#endif
