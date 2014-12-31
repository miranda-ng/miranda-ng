/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#define ID_EXTBKIDLE            40081
#define ID_EXTBKEXPANDEDGROUP   40082
#define ID_EXTBKCOLLAPSEDDGROUP 40083
#define ID_EXTBKEMPTYGROUPS     40084
#define ID_EXTBKFIRSTITEM       40085
#define ID_EXTBKSINGLEITEM      40086
#define ID_EXTBKLASTITEM        40087


#define ID_EXTBKFIRSTITEM_NG    40088
#define ID_EXTBKSINGLEITEM_NG   40089
#define ID_EXTBKLASTITEM_NG     40090

#define ID_EXTBKEVEN_CNTCTPOS   40091
#define ID_EXTBKODD_CNTCTPOS    40092

#define ID_EXTBKSELECTION       40093
#define ID_EXTBKHOTTRACK        40094
#define ID_EXTBKFRAMETITLE      40095
#define ID_EXTBKEVTAREA         40096
#define ID_EXTBKSTATUSBAR       40097
#define ID_EXTBKBUTTONBAR       40098
#define ID_EXTBKBUTTONSPRESSED  40099
#define ID_EXTBKBUTTONSNPRESSED 40100
#define ID_EXTBKBUTTONSMOUSEOVER 40101
#define ID_EXTBKTBBUTTONSPRESSED  40102
#define ID_EXTBKTBBUTTONSNPRESSED 40103
#define ID_EXTBKTBBUTTONMOUSEOVER 40104
#define ID_EXTBKSTATUSFLOATER	    40105
#define ID_EXTBKOWNEDFRAMEBORDER    40106
#define ID_EXTBKOWNEDFRAMEBORDERTB  40107
#define ID_EXTBKAVATARFRAME         40108
#define ID_EXTBKAVATARFRAMEOFFLINE  40109
#define ID_EXTBKSCROLLBACK          40110
#define ID_EXTBKSCROLLBACKLOWER     40111
#define ID_EXTBKSCROLLTHUMB         40112
#define ID_EXTBKSCROLLTHUMBHOVER    40113
#define ID_EXTBKSCROLLTHUMBPRESSED  40114
#define ID_EXTBKSCROLLBUTTON        40115
#define ID_EXTBKSCROLLBUTTONHOVER   40116
#define ID_EXTBKSCROLLBUTTONPRESSED 40117
#define ID_EXTBKSCROLLARROWUP       40118
#define ID_EXTBKSCROLLARROWDOWN     40119
#define ID_EXTBK_LAST_D             40119

#define ID_EXTBKSEPARATOR           40200

BOOL CheckItem(int item, HWND hwndDlg);
BOOL isValidItem(void);
void extbk_export(char *file);
void extbk_import(char *file, HWND hwndDlg);

void LoadExtBkSettingsFromDB();
void IMG_LoadItems();
void __fastcall IMG_RenderImageItem(HDC hdc, ImageItem *item, RECT *rc);
void IMG_InitDecoder();
void LoadPerContactSkins(TCHAR *file);

static void SaveCompleteStructToDB();
StatusItems_t *GetProtocolStatusItem(const char *szProto);

void OnListItemsChange(HWND hwndDlg);

void UpdateStatusStructSettingsFromOptDlg(HWND hwndDlg, int index);

void SaveNonStatusItemsSettings(HWND hwndDlg);

void FillItemList(HWND hwndDlg);
void FillOptionDialogByCurrentSel(HWND hwndDlg);
void ReActiveCombo(HWND hwndDlg);
//BOOL __fastcall GetItemByStatus(int status, StatusItems_t *retitem);

void FillOptionDialogByStatusItem(HWND hwndDlg, StatusItems_t *item);
