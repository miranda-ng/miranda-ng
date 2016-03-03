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
#include <msapi\comptr.h>

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <delayimp.h>

#include <vector>
#include <map>
#include <algorithm>

#define NETLIB_NOLOGGING

#include <win2k.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_button.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_genmenu.h>
#include <m_imgsrvc.h>
#include <m_string.h>
#include <m_fontservice.h>

#include <m_metacontacts.h>
#include <m_smileyadd.h>
#include <m_folders.h>

#include "regexp\wcpattern.h"
#include "regexp\wcmatcher.h"

typedef WCPattern _TPattern;
typedef WCMatcher _TMatcher;
#define createTMatcher createWCMatcher

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

extern FI_INTERFACE *fei;
extern HINSTANCE g_hInst;
extern HANDLE hNetlibUser;
extern HANDLE hEvent1;
extern HGENMENU hContactMenuItem;
extern SmileyCategoryListType g_SmileyCategories;
extern SmileyPackListType g_SmileyPacks;
extern LIST<void> menuHandleArray;

#define IDC_SMLBUTTON        (WM_USER + 33)

///////////////////////////////////////////////////
//
//defines from miranda im sources (undocumented!)
//
#define MI_IDC_LOG           1001  //rich edit
#define MI_IDC_MESSAGE       1002  //edit control
#define MI_IDC_QUOTE         1034  //button control
#define MI_IDC_NAME          1009  //text control
#define MI_IDC_ADD           1070  //Add button.

#define DM_REMAKELOG         (WM_USER + 11)
#define DM_OPTIONSAPPLIED    (WM_USER + 14)
#define DM_APPENDTOLOG       (WM_USER + 17)

#define WM_REMAKERICH        (WM_USER + 0x3457)

/////////////////////////////////////////////////////

#define MAX_SMILEY_LENGTH    96

// init functions
void InstallDialogBoxHook(void);
void RemoveDialogBoxHook(void);
int UpdateSrmmDlg(WPARAM wParam, LPARAM lParam);
bool IsOldSrmm(void);

//functions for general use (defined in general.cpp)
int CalculateTextHeight(HDC hdc, CHARFORMAT2* chf);
const TCHAR* GetImageExt(CMString &fname);

MCONTACT DecodeMetaContact(MCONTACT hContact);
bool IsSmileyProto(char* proto);

HICON ImageList_GetIconFixed (HIMAGELIST himl, INT i, UINT fStyle);

void pathToRelative(const CMString &pSrc, CMString &pOut);
void pathToAbsolute(const CMString &pSrc, CMString &pOut);

bool InitGdiPlus(void);
void DestroyGdiPlus(void);

void ReportError(const TCHAR *errmsg);
HICON GetDefaultIcon(bool copy = true);

void CloseRichCallback(HWND hwnd);
void CloseRichOwnerCallback(HWND hwnd);
bool SetRichCallback(HWND hwnd, MCONTACT hContact, bool subany, bool subnew);
void SetRichOwnerCallback(HWND hwnd, HWND hwndInput, HWND hwndLog);
void ProcessAllInputAreas(bool restoreText);
void RichEditData_Destroy(void);

void CloseSmileys(void);
int CheckForTip(int x, int y, HWND hwnd, TCHAR **smltxt);

void UrlDecode(char* str);

#endif
