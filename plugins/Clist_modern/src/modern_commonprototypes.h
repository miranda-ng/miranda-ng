#pragma once

#ifndef _COMMONPROTOTYPES
#define _COMMONPROTOTYPES

#define SKIN  "ModernSkin"

extern CLIST_INTERFACE corecli;

// Global variables
extern int ON_SETALLEXTRAICON_CYCLE;
extern int g_nFramesCount;
extern int g_nTitleBarHeight;
extern int g_mutex_bLockUpdating;
extern RECT g_rcEdgeSizingRect;
extern bool g_bSizing;
extern bool g_bChangingMode;
extern bool g_bOnModulesLoadedCalled;
extern bool g_bOnTrayRightClick;
extern bool g_bPostWasCanceled;
extern bool g_bTransparentFlag;
extern bool g_bFullRepaint;
extern bool g_bMultiConnectionMode;
extern uint8_t g_bCalledFromShowHide;
extern HWND g_hCLUIOptionsWnd;
extern UINT g_dwMainThreadID;
extern HWND g_hwndViewModeFrame;
extern bool g_bTrimText;
extern char *g_szConnectingProto;
extern HICON g_hListeningToIcon;
extern HANDLE g_hAwayMsgThread, g_hGetTextAsyncThread, g_hSmoothAnimationThread;
extern ROWCELL *gl_RowTabAccess[];
extern ROWCELL *gl_RowRoot;
extern FRAMEWND *g_pfwFrames;
extern HINSTANCE  g_hMirApp;
extern HIMAGELIST hAvatarOverlays;
extern HIMAGELIST g_himlCListClc;
extern STATUSBARDATA g_StatusBarData;
extern SKINOBJECTSLIST g_SkinObjectList;
extern CURRWNDIMAGEDATA *g_pCachedWindow;

/************************************************************************/
/*                              TYPE DEFS                               */
/************************************************************************/

typedef INT_PTR(*PSYNCCALLBACKPROC)(WPARAM, LPARAM);

/************************************************************************/
/*                              PROTOTYPES                              */
/************************************************************************/


/* CLCItems */
bool    CLCItems_IsShowOfflineGroup(ClcGroup *group);

/* CListMod */
int     CListMod_HideWindow();

/* CLUI */
HANDLE  RegisterIcolibIconHandle(char *szIcoID, char *szSectionName, char *szDescription, wchar_t *tszDefaultFile, int iDefaultIndex, HINSTANCE hDefaultModule, int iDefaultResource);
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
int     CLUI_SmoothAlphaTransition(HWND hwnd, uint8_t GoalAlpha, BOOL wParam);
int     CLUI_TestCursorOnBorders();
int     CLUI_UpdateTimer();
void    CLUI_UpdateLayeredMode();
UINT_PTR CLUI_SafeSetTimer(HWND hwnd, int ID, int Timeout, TIMERPROC proc);

/* CLUIServices */
int     CLUIUnreadEmailCountChanged(WPARAM wParam, LPARAM lParam);

/* GDIPlus */
BOOL    GDIPlus_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION *blendFunction);
HBITMAP GDIPlus_LoadGlyphImage(const wchar_t *szFileName);

/* EventArea */
void    EventArea_ConfigureEventArea();

/* ModernSkinButton */
int     ModernSkinButton_AddButton(HWND parent, char *ID, char *CommandService, char *StateDefService, char *HandeService, int Left, int Top, int Right, int Bottom, uint32_t AlignedTo, wchar_t *Hint, char *DBkey, char *TypeDef, int MinWidth, int MinHeight);
int     ModernSkinButtonLoadModule();
int     ModernSkinButton_ReposButtons(HWND parent, uint8_t draw, RECT *r);
int     ModernSkinButtonUnloadModule(WPARAM, LPARAM);

/* RowHeight */
int     RowHeight_CalcRowHeight(ClcData *dat, ClcContact *contact, int item);

/* SkinEngine */
BOOL    ske_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction);
void    ske_ApplyTranslucency(void);
int     ske_BltBackImage(HWND destHWND, HDC destDC, RECT *BltClientRect);
HBITMAP ske_CreateDIB32(int cx, int cy);
HBITMAP ske_CreateDIB32Point(int cx, int cy, void ** bits);
HRGN    ske_CreateOpaqueRgn(uint8_t Level, bool Opaque);
HICON   ske_CreateJoinedIcon(HICON hBottom, HICON hTop, uint8_t alpha);
int     ske_DrawImageAt(HDC hdc, RECT *rc);
BOOL    ske_DrawIconEx(HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags);
int     ske_DrawNonFramedObjects(BOOL Erase, RECT *r);
BOOL    ske_DrawText(HDC hdc, LPCTSTR lpString, int nCount, RECT *lpRect, UINT format);
SKINOBJECTDESCRIPTOR* ske_FindObjectByName(const char *szName, uint8_t objType, SKINOBJECTSLIST *Skin);
HBITMAP ske_GetCurrentWindowImage();
int     ske_GetFullFilename(wchar_t *buf, const wchar_t *file, wchar_t *skinfolder, BOOL madeAbsolute);
BOOL    ske_ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
HICON   ske_ImageList_GetIcon(HIMAGELIST himl, int i);
int     ske_JustUpdateWindowImageRect(RECT *rty);
HBITMAP ske_LoadGlyphImage(const wchar_t *szFileName);
HRESULT SkinEngineLoadModule();
void    ske_LoadSkinFromDB(void);
int     ske_LoadSkinFromIniFile(wchar_t*, BOOL);
wchar_t* ske_ParseText(wchar_t *stzText);
int     ske_PrepareImageButDontUpdateIt(RECT *r);
int     ske_ReCreateBackImage(BOOL Erase, RECT *w);
int     ske_RedrawCompleteWindow();
bool    ske_ResetTextEffect(HDC);
bool    ske_SelectTextEffect(HDC hdc, uint8_t EffectID, uint32_t FirstColor, uint32_t SecondColor);
INT_PTR ske_Service_DrawGlyph(WPARAM wParam, LPARAM lParam);
BOOL    ske_SetRectOpaque(HDC memdc, RECT *fr, BOOL force = FALSE);
BOOL    ske_SetRgnOpaque(HDC memdc, HRGN hrgn, BOOL force = FALSE);
BOOL    ske_TextOut(HDC hdc, int x, int y, LPCTSTR lpString, int nCount);
int     ske_UnloadGlyphImage(HBITMAP hbmp);
int     SkinEngineUnloadModule();
int     ske_UpdateWindowImage();
int     ske_UpdateWindowImageRect(RECT *lpRect);
int     ske_ValidateFrameImageProc(RECT *r);

/* CLUIFrames.c PROXIED */

int     CLUIFrames_ActivateSubContainers(BOOL wParam);
int     CLUIFrames_OnClistResize_mod(WPARAM wParam, LPARAM lParam);
int     CLUIFrames_OnMoving(HWND, RECT *);
int     CLUIFrames_OnShowHide(int mode);
int     CLUIFrames_SetLayeredMode(BOOL fLayeredMode, HWND hwnd);
int     CLUIFrames_SetParentForContainers(HWND parent);
int     CLUIFramesOnClistResize(WPARAM wParam, LPARAM lParam);

FRAMEWND* FindFrameByItsHWND(HWND FrameHwnd);                  //cluiframes.c

int     DrawTitleBar(HDC hdcMem2, RECT *rect, int Frameid);

int     FindFrameID(HWND FrameHwnd);
int     SetAlpha(uint8_t Alpha);


/* others TODO: move above */
int     Docking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
void    DrawBackGround(HWND hwnd, HDC mhdc, HBITMAP hBmpBackground, COLORREF bkColour, uint32_t backgroundBmpUse);
HRESULT BackgroundsLoadModule();
int     BackgroundsUnloadModule();
char*   GetParamN(char *string, char *buf, int buflen, uint8_t paramN, char Delim, BOOL SkipSpaces);  //mod_skin_selector.c
wchar_t*  GetParamN(wchar_t *string, wchar_t *buf, int buflen, uint8_t paramN, wchar_t Delim, BOOL SkipSpaces);
uint32_t   CompareContacts2_getLMTime(MCONTACT u);                                    //contact.c
uint32_t   mod_CalcHash(const char *a);                                          //mod_skin_selector.c
HICON   cliGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status);            //clistmod.c
HICON   GetMainStatusOverlay(int STATUS);                                       //clc.c
int     CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szStatus, ClcData *dat);  //clcitems.c
int     BgStatusBarChange(WPARAM wParam, LPARAM lParam);                              //clcopts.c
int     ClcDoProtoAck(ACKDATA *ack);                                 //clc.c
int     ModernSkinButtonDeleteAll();                                                   //modernbutton.c
int     GetContactCachedStatus(MCONTACT hContact);                                 //clistsettings.c
int     GetContactIconC(ClcCacheEntry *cacheEntry);                           //clistmod.c
int     GetStatusForContact(MCONTACT hContact, char *szProto);                           //clistsettings.c
int     InitCustomMenus(void);                                                //clistmenus.c
int     LoadStatusBarData();                                                //modern_statusbar.c
int     OnFrameTitleBarBackgroundChange(WPARAM wParam, LPARAM lParam);                  //cluiframes.c
int     QueueAllFramesUpdating(bool);                                          //cluiframes.c
int     RecursiveDeleteMenu(HMENU hMenu);                                       //clistmenus.c
int     ModernSkinButtonRedrawAll();                                                //modern_button.c
int     RegisterButtonByParce(char *ObjectName, char *Params);                     //mod_skin_selector.c
int     SkinSelector_DeleteMask(MODERNMASK *mm);                                 //mod_skin_selector.c
INT_PTR ToggleHideOffline(WPARAM wParam, LPARAM lParam);                              //contact.c
INT_PTR SetUseGroups(WPARAM wParam, LPARAM lParam);                                 //contact.c
INT_PTR ToggleSounds(WPARAM wParam, LPARAM lParam);                                 //contact.c
void    ClcOptionsChanged();                                                //clc.c
void    Docking_GetMonitorRectFromWindow(HWND hWnd, RECT *rc);                        //Docking.c
void    DrawAvatarImageWithGDIp(HDC hDestDC, int x, int y, uint32_t width, uint32_t height, HBITMAP hbmp, int x1, int y1, uint32_t width1, uint32_t height1, uint32_t flag, uint8_t alpha);   //gdiplus.cpp
void    InitGdiPlus();                                                      //gdiplus.cpp
void    ShutdownGdiPlus();                                                   //gdiplus.cpp
void    UnloadAvatarOverlayIcon();                                             //clc.c
void    UnLoadContactListModule();                                             //clistmod.c
void    UpdateAllAvatars(ClcData *dat);                                    //cache_func.c

void    ApplyViewMode(const char *Name);
void    SaveViewMode(const char *name, const wchar_t *szGroupFilter, const char *szProtoFilter, uint32_t statusMask, uint32_t stickyStatusMask, unsigned int options, unsigned int stickies, unsigned int operators, unsigned int lmdat);

// cluiframes.c
int     ExtraImage_ExtraIDToColumnNum(int extra);

int     CLUI_OnSkinLoad(WPARAM wParam, LPARAM lParam);
HRESULT CluiLoadModule();
HRESULT PreLoadContactListModule();
HRESULT ClcLoadModule();
HRESULT ToolbarLoadModule();
HRESULT ToolbarButtonLoadModule();
void    ToolbarButtonUnloadModule();

// INTERFACES

void     cliCheckCacheItem(ClcCacheEntry *pdnce);
void     cliFreeCacheItem(ClcCacheEntry *p);
void     cliRebuildEntireList(HWND hwnd, ClcData *dat);
void     cliRecalcScrollBar(HWND hwnd, ClcData *dat);
int      cliGetGroupContentsCount(ClcGroup *group, int visibleOnly);
int      cliFindRowByText(HWND hwnd, ClcData *dat, const wchar_t *text, int prefixOk);
int      cliGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex);
int      cli_IconFromStatusMode(const char *szProto, int nStatus, MCONTACT hContact);
int      cli_RemoveEvent(MCONTACT hContact, MEVENT hDbEvent);
void     cli_FreeContact(ClcContact*);
void     cli_SetContactCheckboxes(ClcContact*, int);
LRESULT  cli_ProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
int      cliShowHide(bool bAlwaysShow);
BOOL     cliInvalidateRect(HWND hWnd, CONST RECT *lpRect, BOOL bErase);
int      cliCompareContacts(const ClcContact *contact1, const ClcContact *contact2);
int      cliTrayIconPauseAutoHide(WPARAM wParam, LPARAM lParam);
void     cliCluiProtocolStatusChanged(int status, const char *proto);
void     cliInvalidateDisplayNameCacheEntry(MCONTACT hContact);
void     CLUI_cli_LoadCluiGlobalOpts(void);
INT_PTR  cli_TrayIconProcessMessage(WPARAM wParam, LPARAM lParam);
int      cliTrayIconInit(HWND hwnd);
int      cliTrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount);

CListEvent* cli_AddEvent(CLISTEVENT *cle);

LRESULT CALLBACK cli_ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

ClcContact* cliFindItem(uint32_t dwItem, ClcContact *contact);
ClcContact* cliCreateClcContact(void);
ClcCacheEntry* cliCreateCacheItem(MCONTACT hContact);

#define WM_DWMCOMPOSITIONCHANGED  0x031E

#define DWM_BB_ENABLE               0x00000001
#define DWM_BB_BLURREGION            0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED   0x00000004
struct DWM_BLURBEHIND
{
	uint32_t dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
};
extern HRESULT(WINAPI *g_proc_DWMEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

/* SkinEngine.c */

uint8_t SkinDBGetContactSettingByte(MCONTACT hContact, const char* szSection, const char*szKey, uint8_t bDefault);

extern OVERLAYICONINFO g_pAvatarOverlayIcons[MAX_STATUS_COUNT];
extern OVERLAYICONINFO g_pStatusOverlayIcons[MAX_STATUS_COUNT];

#endif
