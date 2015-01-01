#pragma once

#ifndef _COMMONPROTOTYPES
#define _COMMONPROTOTYPES

#ifndef commonheaders_h__
#error "hdr/modern_commonheaders.h have to be including first"
#endif

#include "modern_commonheaders.h"  //TO DO: Move contents of this file to commonheaders.h
#include "modern_clist.h"
#include "CLUIFrames/cluiframes.h"
#include "modern_row.h"
#include "modern_skinengine.h"
#include "modern_skinselector.h"
#include "modern_statusbar.h"

#define SKIN  "ModernSkin"

extern PLUGININFOEX pluginInfo;
extern CLIST_INTERFACE * pcli;
extern CLIST_INTERFACE corecli;

//Global variables
extern int ON_SETALLEXTRAICON_CYCLE;
extern FRAMEWND *g_pfwFrames;
extern int g_nFramesCount;
extern RECT g_rcEdgeSizingRect;
extern FRAMEWND *wndFrameEventArea;
extern ROWCELL * gl_RowTabAccess[];
extern ROWCELL * gl_RowRoot;
extern HIMAGELIST hAvatarOverlays;
extern int  g_nTitleBarHeight;
extern BOOL g_bTransparentFlag;
extern HIMAGELIST g_himlCListClc;
extern BOOL g_mutex_bSizing;
extern BOOL LOCK_RECALC_SCROLLBAR;
extern HIMAGELIST g_himlCListClc;
extern BYTE nameOrder[];
extern SortedList lContactsCache;
extern BOOL g_flag_bOnModulesLoadedCalled;
extern SKINOBJECTSLIST g_SkinObjectList;
extern CURRWNDIMAGEDATA * g_pCachedWindow;
extern BOOL g_mutex_bLockUpdating;
extern STATUSBARDATA g_StatusBarData;
extern SKINOBJECTSLIST g_SkinObjectList;
extern CURRWNDIMAGEDATA * g_pCachedWindow;
extern char * g_szConnectingProto;
extern BOOL g_mutex_bLockUpdating;
extern int  g_mutex_nCalcRowHeightLock;
extern int  g_mutex_bOnTrayRightClick;
extern BOOL g_flag_bPostWasCanceled;
extern BOOL g_flag_bFullRepaint;
extern BOOL g_bMultiConnectionMode;
extern BYTE g_bCalledFromShowHide;
extern HICON g_hListeningToIcon;
extern HWND g_hCLUIOptionsWnd;
extern BOOL g_bTransparentFlag;
extern HINSTANCE g_hInst;
extern BOOL g_mutex_bChangingMode;
extern UINT g_dwMainThreadID;
extern HANDLE g_hAwayMsgThread, g_hGetTextAsyncThread, g_hSmoothAnimationThread;
extern HWND g_hwndViewModeFrame;
extern BYTE gl_TrimText;

/************************************************************************/
/*                              TYPE DEFS                               */
/************************************************************************/

typedef INT_PTR(*PSYNCCALLBACKPROC)(WPARAM, LPARAM);

/************************************************************************/
/*                              PROTOTYPES                              */
/************************************************************************/


/* CLCItems */
BOOL CLCItems_IsShowOfflineGroup(ClcGroup* group);

/* CListMod */
int CListMod_HideWindow();

/* CLUI */
HANDLE  RegisterIcolibIconHandle(char * szIcoID, char *szSectionName, char * szDescription, TCHAR * tszDefaultFile, int iDefaultIndex, HINSTANCE hDefaultModule, int iDefaultResource);
void    CLUI_UpdateAeroGlass();
void    CLUI_ChangeWindowMode();
BOOL    CLUI_CheckOwnedByClui(HWND hwnd);
INT_PTR CLUI_GetConnectingIconService(WPARAM wParam, LPARAM lParam);
int     CLUI_HideBehindEdge();
int     CLUI_IconsChanged(WPARAM, LPARAM);
int     CLUI_IsInMainWindow(HWND hwnd);
int     CLUI_OnSkinLoad(WPARAM wParam, LPARAM lParam);
int     CLUI_ReloadCLUIOptions();
int     CLUI_ShowFromBehindEdge();
int     CLUI_SizingGetWindowRect(HWND hwnd, RECT *rc);
int     CLUI_SizingOnBorder(POINT, int);
int     CLUI_SmoothAlphaTransition(HWND hwnd, BYTE GoalAlpha, BOOL wParam);
int     CLUI_TestCursorOnBorders();
int     CLUI_UpdateTimer();
void    CLUI_UpdateLayeredMode();
UINT_PTR CLUI_SafeSetTimer(HWND hwnd, int ID, int Timeout, TIMERPROC proc);

/* CLUIServices */
int     CLUIUnreadEmailCountChanged(WPARAM wParam, LPARAM lParam);

/* GDIPlus */
BOOL    GDIPlus_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION * blendFunction);
HBITMAP GDIPlus_LoadGlyphImage(const TCHAR *szFileName);

/* EventArea */
void    EventArea_ConfigureEventArea();

/* ModernSkinButton */
int     ModernSkinButton_AddButton(HWND parent, char * ID, char * CommandService, char * StateDefService, char * HandeService, int Left, int Top, int Right, int Bottom, DWORD AlignedTo, TCHAR * Hint, char * DBkey, char * TypeDef, int MinWidth, int MinHeight);
int     ModernSkinButtonLoadModule();
int     ModernSkinButton_ReposButtons(HWND parent, BYTE draw, RECT *r);
int     ModernSkinButtonUnloadModule(WPARAM, LPARAM);

/* RowHeight */
int     RowHeight_CalcRowHeight(ClcData *dat, HWND hwnd, ClcContact *contact, int item);

/* SkinEngine */
BOOL    ske_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction);
void    ske_ApplyTranslucency(void);
int     ske_BltBackImage(HWND destHWND, HDC destDC, RECT *BltClientRect);
HBITMAP ske_CreateDIB32(int cx, int cy);
HBITMAP ske_CreateDIB32Point(int cx, int cy, void ** bits);
HRGN    ske_CreateOpaqueRgn(BYTE Level, bool Opaque);
HICON   ske_CreateJoinedIcon(HICON hBottom, HICON hTop, BYTE alpha);
int     ske_DrawImageAt(HDC hdc, RECT *rc);
BOOL    ske_DrawIconEx(HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags);
int     ske_DrawNonFramedObjects(BOOL Erase, RECT *r);
BOOL    ske_DrawText(HDC hdc, LPCTSTR lpString, int nCount, RECT *lpRect, UINT format);
LPSKINOBJECTDESCRIPTOR   ske_FindObjectByName(const char * szName, BYTE objType, SKINOBJECTSLIST* Skin);
HBITMAP ske_GetCurrentWindowImage();
int     ske_GetFullFilename(TCHAR *buf, const TCHAR *file, TCHAR *skinfolder, BOOL madeAbsolute);
int     ske_GetSkinFolder(TCHAR *szFileName, char * t2);
BOOL    ske_ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
HICON   ske_ImageList_GetIcon(HIMAGELIST himl, int i);
int     ske_JustUpdateWindowImageRect(RECT *rty);
HBITMAP ske_LoadGlyphImage(const TCHAR *szFileName);
HRESULT SkinEngineLoadModule();
void    ske_LoadSkinFromDB(void);
int     ske_LoadSkinFromIniFile(TCHAR*, BOOL);
TCHAR*  ske_ParseText(TCHAR *stzText);
int     ske_PrepareImageButDontUpdateIt(RECT *r);
int     ske_ReCreateBackImage(BOOL Erase, RECT *w);
int     ske_RedrawCompleteWindow();
bool    ske_ResetTextEffect(HDC);
bool    ske_SelectTextEffect(HDC hdc, BYTE EffectID, DWORD FirstColor, DWORD SecondColor);
INT_PTR ske_Service_DrawGlyph(WPARAM wParam, LPARAM lParam);
BOOL    ske_SetRectOpaque(HDC memdc, RECT *fr, BOOL force = FALSE);
BOOL    ske_SetRgnOpaque(HDC memdc, HRGN hrgn, BOOL force = FALSE);
BOOL    ske_TextOut(HDC hdc, int x, int y, LPCTSTR lpString, int nCount);
int     ske_UnloadGlyphImage(HBITMAP hbmp);
int     SkinEngineUnloadModule();
int     ske_UpdateWindowImage();
int     ske_UpdateWindowImageRect(RECT *lpRect);
int     ske_ValidateFrameImageProc(RECT *r);

__forceinline BOOL ske_DrawTextA(HDC hdc, char *lpString, int nCount, RECT *lpRect, UINT format)
{
	return ske_DrawText(hdc, _A2T(lpString), nCount, lpRect, format);
}

/* CLUIFrames.c PROXIED */

int CLUIFrames_ActivateSubContainers(BOOL wParam);
int CLUIFrames_OnClistResize_mod(WPARAM wParam, LPARAM lParam);
int CLUIFrames_OnMoving(HWND, RECT *);
int CLUIFrames_OnShowHide(int mode);
int CLUIFrames_SetLayeredMode(BOOL fLayeredMode, HWND hwnd);
int CLUIFrames_SetParentForContainers(HWND parent);
int CLUIFramesOnClistResize(WPARAM wParam, LPARAM lParam);

FRAMEWND * FindFrameByItsHWND(HWND FrameHwnd);                  //cluiframes.c

//int callProxied_DrawTitleBar(HDC hdcMem2,RECT *rect,int Frameid);
int DrawTitleBar(HDC hdcMem2, RECT *rect, int Frameid);

int FindFrameID(HWND FrameHwnd);
int SetAlpha(BYTE Alpha);


/* others TODO: move above */
int     Docking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
void    DrawBackGround(HWND hwnd, HDC mhdc, HBITMAP hBmpBackground, COLORREF bkColour, DWORD backgroundBmpUse);
HRESULT BackgroundsLoadModule();
int     BackgroundsUnloadModule();
INT_PTR CALLBACK DlgTmplEditorOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);   //RowTemplate.c
BOOL    FindMenuHanleByGlobalID(HMENU hMenu, int globalID, struct _MenuItemHandles * dat);   //GenMenu.c
BOOL    MatchMask(char *name, char *mask);                                    //mod_skin_selector.c
char*   GetContactCachedProtocol(MCONTACT hContact);                                 //clistsettings.c
char*   GetParamN(char *string, char *buf, int buflen, BYTE paramN, char Delim, BOOL SkipSpaces);  //mod_skin_selector.c
WCHAR*  GetParamN(WCHAR *string, WCHAR *buf, int buflen, BYTE paramN, WCHAR Delim, BOOL SkipSpaces);
DWORD   CompareContacts2_getLMTime(MCONTACT u);                                    //contact.c
DWORD   mod_CalcHash(const char * a);                                          //mod_skin_selector.c
HICON   cliGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status);            //clistmod.c
HICON   GetMainStatusOverlay(int STATUS);                                       //clc.c
int     __fastcall CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szStatus, ClcData *dat);  //clcitems.c
int     BgStatusBarChange(WPARAM wParam, LPARAM lParam);                              //clcopts.c
int     ClcDoProtoAck(MCONTACT wParam, ACKDATA *ack);                                 //clc.c
int     ModernSkinButtonDeleteAll();                                                   //modernbutton.c
int     GetContactCachedStatus(MCONTACT hContact);                                 //clistsettings.c
int     GetContactIconC(ClcCacheEntry *cacheEntry);                           //clistmod.c
int     GetContactIndex(ClcGroup *group, ClcContact *contact);               //clcidents.c
int     GetStatusForContact(MCONTACT hContact, char *szProto);                           //clistsettings.c
int     InitCustomMenus(void);                                                //clistmenus.c
int     InitFramesMenus(void);                                                //framesmenus.c
int     LoadMoveToGroup();                                                   //movetogroup.c
int     LoadStatusBarData();                                                //modern_statusbar.c
int     MenuModulesLoaded(WPARAM wParam, LPARAM lParam);                              //clistmenu.c
int     MenuModulesShutdown(WPARAM wParam, LPARAM lParam);                           //clistmenu.c
int     MenuProcessCommand(WPARAM wParam, LPARAM lParam);                           //clistmenu.c
int     OnFrameTitleBarBackgroundChange(WPARAM wParam, LPARAM lParam);                  //cluiframes.c
int     QueueAllFramesUpdating(BYTE);                                          //cluiframes.c
int     RecursiveDeleteMenu(HMENU hMenu);                                       //clistmenus.c
int     ModernSkinButtonRedrawAll();                                                //modern_button.c
int     RegisterButtonByParce(char * ObjectName, char * Params);                     //mod_skin_selector.c
int     RestoreAllContactData(ClcData *dat);                                 //cache_funcs.c

int     SkinSelector_DeleteMask(MODERNMASK *mm);                                 //mod_skin_selector.c
int     StoreAllContactData(ClcData *dat);                                 //cache_func.c
INT_PTR ToggleHideOffline(WPARAM wParam, LPARAM lParam);                              //contact.c
INT_PTR ToggleGroups(WPARAM wParam, LPARAM lParam);                                 //contact.c
INT_PTR SetUseGroups(WPARAM wParam, LPARAM lParam);                                 //contact.c
INT_PTR ToggleSounds(WPARAM wParam, LPARAM lParam);                                 //contact.c
void    ClcOptionsChanged();                                                //clc.c
void    Docking_GetMonitorRectFromWindow(HWND hWnd, RECT *rc);                        //Docking.c
void    DrawAvatarImageWithGDIp(HDC hDestDC, int x, int y, DWORD width, DWORD height, HBITMAP hbmp, int x1, int y1, DWORD width1, DWORD height1, DWORD flag, BYTE alpha);   //gdiplus.cpp
void    FreeRowCell();                                                      //RowHeight
void    InitGdiPlus();                                                      //gdiplus.cpp
void    InvalidateDNCEbyPointer(MCONTACT hContact, ClcCacheEntry *pdnce, int SettingType);  //clistsettings.c
void    ShutdownGdiPlus();                                                   //gdiplus.cpp
void    UninitCustomMenus();                                                //clistmenus.c
void    UnloadAvatarOverlayIcon();                                             //clc.c
void    UnLoadContactListModule();                                             //clistmod.c
void    UpdateAllAvatars(ClcData *dat);                                    //cache_func.c

//cluiframes.c
void    gtaRenewText(MCONTACT hContact);
int     ExtraImage_ExtraIDToColumnNum(int extra);

int     LoadSkinButtonModule();
void    UninitSkinHotKeys();
void    GetDefaultFontSetting(int i, LOGFONTA *lf, COLORREF *colour);
int     CLUI_OnSkinLoad(WPARAM wParam, LPARAM lParam);
HRESULT CluiLoadModule();
HRESULT PreLoadContactListModule();
HRESULT ClcLoadModule();
HRESULT ToolbarLoadModule();
HRESULT ToolbarButtonLoadModule();
void    ToolbarButtonUnloadModule();

// INTERFACES

void    cliCheckCacheItem(ClcCacheEntry *pdnce);
void    cliFreeCacheItem(ClcCacheEntry *p);
void    cliRebuildEntireList(HWND hwnd, ClcData *dat);
void    cliRecalcScrollBar(HWND hwnd, ClcData *dat);
void    CLUI_cliOnCreateClc(void);
int     cli_AddItemToGroup(ClcGroup *group, int iAboveItem);
int     cli_AddInfoItemToGroup(ClcGroup *group, int flags, const TCHAR *pszText);
int     cliGetGroupContentsCount(ClcGroup *group, int visibleOnly);
int     cliFindRowByText(HWND hwnd, ClcData *dat, const TCHAR *text, int prefixOk);
int     cliGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex);
int     cli_IconFromStatusMode(const char *szProto, int nStatus, MCONTACT hContact);
int     cli_GetContactIcon(MCONTACT hContact);
int     cli_RemoveEvent(MCONTACT hContact, HANDLE hDbEvent);
void    cli_AddContactToTree(HWND hwnd, ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline);
void    cli_DeleteItemFromTree(HWND hwnd, MCONTACT hItem);
void    cli_FreeContact(ClcContact*);
void    cli_FreeGroup(ClcGroup*);
char*   cli_GetGroupCountsText(ClcData *dat, ClcContact *contact);
void    cli_ChangeContactIcon(MCONTACT hContact, int iIcon, int add);
void    cli_SetContactCheckboxes(ClcContact*, int);
LRESULT cli_ProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
struct  CListEvent* cliCreateEvent(void);
struct  CListEvent* cli_AddEvent(CLISTEVENT *cle);
LRESULT CALLBACK cli_ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int     cliShowHide(WPARAM wParam, LPARAM lParam);
BOOL    CLUI__cliInvalidateRect(HWND hWnd, CONST RECT* lpRect, BOOL bErase);
int     cliCompareContacts(const ClcContact *contact1, const ClcContact *contact2);
int     cliFindItem(HWND hwnd, ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible);
int     cliTrayIconPauseAutoHide(WPARAM wParam, LPARAM lParam);
void    cliCluiProtocolStatusChanged(int status, const char * proto);
HMENU   cliBuildGroupPopupMenu(ClcGroup *group);
void    cliInvalidateDisplayNameCacheEntry(MCONTACT hContact);
void    cliCheckCacheItem(ClcCacheEntry *pdnce);
void    cli_SaveStateAndRebuildList(HWND hwnd, ClcData *dat);
void    CLUI_cli_LoadCluiGlobalOpts(void);
INT_PTR cli_TrayIconProcessMessage(WPARAM wParam, LPARAM lParam);
BOOL    CLUI__cliInvalidateRect(HWND hWnd, CONST RECT* lpRect, BOOL bErase);
int		cliTrayIconInit(HWND hwnd);
int		cliTrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status);
void	cliTrayIconUpdateBase(const char *szChangedProto);

ClcContact*    cliCreateClcContact(void);
ClcCacheEntry* cliCreateCacheItem(MCONTACT hContact);
ClcCacheEntry* cliGetCacheEntry(MCONTACT hContact);

#define WM_DWMCOMPOSITIONCHANGED  0x031E

#define DWM_BB_ENABLE               0x00000001
#define DWM_BB_BLURREGION            0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED   0x00000004
struct DWM_BLURBEHIND
{
	DWORD dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
};
extern HRESULT(WINAPI *g_proc_DWMEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

extern tPaintCallbackProc CLCPaint_PaintCallbackProc(HWND hWnd, HDC hDC, RECT *rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData);

/* SkinEngine.c */


BYTE SkinDBGetContactSettingByte(MCONTACT hContact, const char* szSection, const char*szKey, BYTE bDefault);

extern OVERLAYICONINFO g_pAvatarOverlayIcons[ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE + 1];
extern OVERLAYICONINFO g_pStatusOverlayIcons[ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE + 1];


#endif
