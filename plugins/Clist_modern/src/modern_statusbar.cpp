#include "stdafx.h"
#include "modern_statusbar.h"
#include "m_skin_eng.h"
#include "modern_clcpaint.h"
#include "modern_sync.h"

BOOL tooltipshoing;
POINT lastpnt;

#define TM_STATUSBAR 23435234
#define TM_STATUSBARHIDE 23435235

HWND hModernStatusBar = nullptr;

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

struct ProtoItemData : public MZeroedObject
{
	~ProtoItemData()
	{}

	HICON  icon;
	HICON  extraIcon;
	int    iconIndex;
	ptrA   szProtoName;
	ptrA   szAccountName;
	int    iProtoStatus;
	ptrW   tszProtoHumanName;
	ptrW   szProtoEMailCount;
	ptrW   tszProtoStatusText;
	ptrW   tszProtoXStatus;
	int    iProtoPos;
	int    fullWidth;
	RECT   protoRect;

	uint8_t   xStatusMode;     // 0-only main, 1-xStatus, 2-main as overlay
	bool   bDoubleIcons;
	bool   bShowProtoIcon;
	bool   bShowProtoName;
	bool   bShowStatusName;
	bool   bConnectingIcon;
	bool   bShowProtoEmails;
	bool   SBarRightClk;
	bool   bIsDimmed;

	int    PaddingLeft;
	int    PaddingRight;
};

static OBJLIST<ProtoItemData> ProtosData(5);

STATUSBARDATA g_StatusBarData = { 0 };

int LoadStatusBarData()
{
	g_StatusBarData.perProtoConfig = db_get_b(0, "CLUI", "SBarPerProto", SETTING_SBARPERPROTO_DEFAULT);
	g_StatusBarData.bShowProtoIcon = (db_get_b(0, "CLUI", "SBarShow", SETTING_SBARSHOW_DEFAULT) & 1) != 0;
	g_StatusBarData.bShowProtoName = (db_get_b(0, "CLUI", "SBarShow", SETTING_SBARSHOW_DEFAULT) & 2) != 0;
	g_StatusBarData.bShowStatusName = (db_get_b(0, "CLUI", "SBarShow", SETTING_SBARSHOW_DEFAULT) & 4) != 0;
	g_StatusBarData.xStatusMode = db_get_b(0, "CLUI", "ShowXStatus", SETTING_SHOWXSTATUS_DEFAULT);
	g_StatusBarData.bConnectingIcon = db_get_b(0, "CLUI", "UseConnectingIcon", SETTING_USECONNECTINGICON_DEFAULT) != 0;
	g_StatusBarData.bShowProtoEmails = db_get_b(0, "CLUI", "ShowUnreadEmails", SETTING_SHOWUNREADEMAILS_DEFAULT) != 0;
	g_StatusBarData.SBarRightClk = db_get_b(0, "CLUI", "SBarRightClk", SETTING_SBARRIGHTCLK_DEFAULT);

	g_StatusBarData.nProtosPerLine = db_get_b(0, "CLUI", "StatusBarProtosPerLine", SETTING_PROTOSPERLINE_DEFAULT);
	g_StatusBarData.Align = db_get_b(0, "CLUI", "Align", SETTING_ALIGN_DEFAULT);
	g_StatusBarData.VAlign = db_get_b(0, "CLUI", "VAlign", SETTING_VALIGN_DEFAULT);
	g_StatusBarData.sameWidth = db_get_b(0, "CLUI", "EqualSections", SETTING_EQUALSECTIONS_DEFAULT);
	g_StatusBarData.rectBorders.left = db_get_dw(0, "CLUI", "LeftOffset", SETTING_LEFTOFFSET_DEFAULT);
	g_StatusBarData.rectBorders.right = db_get_dw(0, "CLUI", "RightOffset", SETTING_RIGHTOFFSET_DEFAULT);
	g_StatusBarData.rectBorders.top = db_get_dw(0, "CLUI", "TopOffset", SETTING_TOPOFFSET_DEFAULT);
	g_StatusBarData.rectBorders.bottom = db_get_dw(0, "CLUI", "BottomOffset", SETTING_BOTTOMOFFSET_DEFAULT);
	g_StatusBarData.extraspace = (uint8_t)db_get_dw(0, "CLUI", "SpaceBetween", SETTING_SPACEBETWEEN_DEFAULT);

	if (g_StatusBarData.BarFont) {
		DeleteObject(g_StatusBarData.BarFont);
		g_StatusBarData.BarFont = nullptr;
	}

	int vis = db_get_b(0, "CLUI", "ShowSBar", SETTING_SHOWSBAR_DEFAULT);
	int frameID = Sync(FindFrameID, hModernStatusBar);
	int frameopt = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frameID), 0);
	frameopt = frameopt & (~F_VISIBLE);
	if (vis) {
		ShowWindow(hModernStatusBar, SW_SHOW);
		frameopt |= F_VISIBLE;
	}
	else ShowWindow(hModernStatusBar, SW_HIDE);
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, frameID), frameopt);

	g_StatusBarData.TextEffectID = db_get_b(0, "StatusBar", "TextEffectID", SETTING_TEXTEFFECTID_DEFAULT);
	g_StatusBarData.TextEffectColor1 = db_get_dw(0, "StatusBar", "TextEffectColor1", SETTING_TEXTEFFECTCOLOR1_DEFAULT);
	g_StatusBarData.TextEffectColor2 = db_get_dw(0, "StatusBar", "TextEffectColor2", SETTING_TEXTEFFECTCOLOR2_DEFAULT);

	if (g_StatusBarData.hBmpBackground) { DeleteObject(g_StatusBarData.hBmpBackground); g_StatusBarData.hBmpBackground = nullptr; }

	if (g_CluiData.fDisableSkinEngine) {
		g_StatusBarData.bkColour = cliGetColor("StatusBar", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (db_get_b(0, "StatusBar", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmapName(db_get_wsa(0, "StatusBar", "BkBitmap"));
			if (tszBitmapName)
				g_StatusBarData.hBmpBackground = Bitmap_Load(tszBitmapName);
		}
		g_StatusBarData.bkUseWinColors = db_get_b(0, "StatusBar", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		g_StatusBarData.backgroundBmpUse = db_get_w(0, "StatusBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}

	SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
	return 1;
}

int BgStatusBarChange(WPARAM, LPARAM)
{
	if (!MirandaExiting())
		LoadStatusBarData();

	return 0;
}

// ProtocolData;
int NewStatusPaintCallbackProc(HWND hWnd, HDC hDC, RECT *, HRGN, uint32_t, void *)
{
	return ModernDrawStatusBar(hWnd, hDC);
}

int ModernDrawStatusBar(HWND hwnd, HDC hDC)
{
	if (hwnd == (HWND)-1) return 0;
	if (GetParent(hwnd) == g_clistApi.hwndContactList)
		return ModernDrawStatusBarWorker(hwnd, hDC);

	cliInvalidateRect(hwnd, nullptr, FALSE);
	return 0;
}

int ModernDrawStatusBarWorker(HWND hWnd, HDC hDC)
{
	int iconWidth = GetSystemMetrics(SM_CXSMICON);
	int iconHeight = GetSystemMetrics(SM_CYSMICON);

	// Count visible protos
	RECT rc;
	GetClientRect(hWnd, &rc);
	if (g_CluiData.fDisableSkinEngine) {
		if (g_StatusBarData.bkUseWinColors && xpt_IsThemed(g_StatusBarData.hTheme))
			xpt_DrawTheme(g_StatusBarData.hTheme, hWnd, hDC, 0, 0, &rc, &rc);
		else
			DrawBackGround(hWnd, hDC, g_StatusBarData.hBmpBackground, g_StatusBarData.bkColour, g_StatusBarData.backgroundBmpUse);
	}
	else SkinDrawGlyph(hDC, &rc, &rc, "Main,ID=StatusBar");

	g_StatusBarData.nProtosPerLine = db_get_b(0, "CLUI", "StatusBarProtosPerLine", SETTING_PROTOSPERLINE_DEFAULT);
	HFONT hOldFont = g_clcPainter.ChangeToFont(hDC, nullptr, FONTID_STATUSBAR_PROTONAME, nullptr);

	SIZE textSize = { 0 };
	GetTextExtentPoint32A(hDC, " ", 1, &textSize);
	int spaceWidth = textSize.cx;

	ProtosData.destroy();

	auto &accs = Accounts();
	if (accs.getCount() == 0)
		return 0;

	int iProtoInStatusMenu = 0;
	for (int j = 0; j < accs.getCount(); j++) {
		int i = Clist_GetAccountIndex(j);
		if (i == -1)
			continue;

		if (!accs[i]->IsVisible())
			continue;

		char *szProto = accs[i]->szModuleName;
		char buf[256];
		mir_snprintf(buf, "SBarAccountIsCustom_%s", szProto);

		ProtoItemData *p = nullptr;

		if (g_StatusBarData.perProtoConfig && db_get_b(0, "CLUI", buf, SETTING_SBARACCOUNTISCUSTOM_DEFAULT)) {
			mir_snprintf(buf, "HideAccount_%s", szProto);
			if (db_get_b(0, "CLUI", buf, SETTING_SBARHIDEACCOUNT_DEFAULT)) {
				iProtoInStatusMenu++;
				continue;
			}

			mir_snprintf(buf, "SBarShow_%s", szProto);

			uint8_t showOps = db_get_b(0, "CLUI", buf, SETTING_SBARSHOW_DEFAULT);
			p = new ProtoItemData;
			p->bShowProtoIcon = (showOps & 1) != 0;
			p->bShowProtoName = (showOps & 2) != 0;
			p->bShowStatusName = (showOps & 4) != 0;

			mir_snprintf(buf, "ShowXStatus_%s", szProto);
			p->xStatusMode = db_get_b(0, "CLUI", buf, SETTING_SBARSHOW_DEFAULT);

			mir_snprintf(buf, "UseConnectingIcon_%s", szProto);
			p->bConnectingIcon = db_get_b(0, "CLUI", buf, SETTING_USECONNECTINGICON_DEFAULT) != 0;

			mir_snprintf(buf, "ShowUnreadEmails_%s", szProto);
			p->bShowProtoEmails = db_get_b(0, "CLUI", buf, SETTING_SHOWUNREADEMAILS_DEFAULT) != 0;

			mir_snprintf(buf, "SBarRightClk_%s", szProto);
			p->SBarRightClk = db_get_b(0, "CLUI", buf, SETTING_SBARRIGHTCLK_DEFAULT) != 0;

			mir_snprintf(buf, "PaddingLeft_%s", szProto);
			p->PaddingLeft = db_get_dw(0, "CLUI", buf, SETTING_PADDINGLEFT_DEFAULT);

			mir_snprintf(buf, "PaddingRight_%s", szProto);
			p->PaddingRight = db_get_dw(0, "CLUI", buf, SETTING_PADDINGRIGHT_DEFAULT);
		}
		else {
			p = new ProtoItemData;
			p->bShowProtoIcon = g_StatusBarData.bShowProtoIcon;
			p->bShowProtoName = g_StatusBarData.bShowProtoName;
			p->bShowStatusName = g_StatusBarData.bShowStatusName;
			p->xStatusMode = g_StatusBarData.xStatusMode;
			p->bConnectingIcon = g_StatusBarData.bConnectingIcon;
			p->bShowProtoEmails = g_StatusBarData.bShowProtoEmails;
			p->SBarRightClk = 0;
			p->PaddingLeft = 0;
			p->PaddingRight = 0;
		}

		p->iProtoStatus = Proto_GetStatus(szProto);

		if (p->iProtoStatus > ID_STATUS_OFFLINE)
			if (p->bShowProtoEmails == 1 && ProtoServiceExists(szProto, PS_GETUNREADEMAILCOUNT)) {
				int nEmails = (int)CallProtoService(szProto, PS_GETUNREADEMAILCOUNT, 0, 0);
				if (nEmails > 0) {
					wchar_t str[40];
					mir_snwprintf(str, L"[%d]", nEmails);
					p->szProtoEMailCount = mir_wstrdup(str);
				}
			}

		p->tszProtoHumanName = mir_wstrdup(accs[i]->tszAccountName);
		p->szAccountName = mir_strdup(szProto);
		p->szProtoName = mir_strdup(accs[i]->szProtoName);
		p->tszProtoStatusText = mir_wstrdup(Clist_GetStatusModeDescription(p->iProtoStatus, 0));
		p->iProtoPos = iProtoInStatusMenu++;

		p->bIsDimmed = 0;
		if (g_CluiData.bFilterEffective & CLVM_FILTER_PROTOS) {
			char szTemp[2048];
			mir_snprintf(szTemp, "%s|", p->szAccountName.get());
			p->bIsDimmed = strstr(g_CluiData.protoFilter, szTemp) ? 0 : 1;
		}

		ProtosData.insert(p);
	}

	if (ProtosData.getCount() == 0)
		return 0;

	// START MULTILINE HERE 
	int orig_visProtoCount = ProtosData.getCount();
	int protosperline = 0;

	if (g_StatusBarData.nProtosPerLine)
		protosperline = g_StatusBarData.nProtosPerLine;
	else if (orig_visProtoCount)
		protosperline = orig_visProtoCount;
	else if (accs.getCount()) {
		protosperline = accs.getCount();
		orig_visProtoCount = accs.getCount();
	}
	else {
		protosperline = 1;
		orig_visProtoCount = 1;
	}
	protosperline = min(protosperline, orig_visProtoCount);

	int linecount = protosperline ? (orig_visProtoCount + (protosperline - 1)) / protosperline : 1; // divide with rounding to up
	for (int line = 0; line < linecount; line++) {
		int rowheight = 2 + max(textSize.cy, iconHeight);
		int visProtoCount = min(protosperline, (orig_visProtoCount - line*protosperline));
		GetClientRect(hWnd, &rc);

		rc.top += g_StatusBarData.rectBorders.top;
		rc.bottom -= g_StatusBarData.rectBorders.bottom;

		int aligndx = 0, maxwidth = 0, SumWidth = 0;

		int height = (rowheight*linecount);
		if (height > (rc.bottom - rc.top)) {
			rowheight = (rc.bottom - rc.top) / linecount;
			height = (rowheight*linecount);
		}

		int rowsdy = ((rc.bottom - rc.top) - height) / 2;
		if (rowheight*(line)+rowsdy < rc.top - rowheight) continue;
		if (rowheight*(line + 1) + rowsdy > rc.bottom + rowheight)
			break;

		if (g_StatusBarData.VAlign == 0) { // top
			rc.bottom = rc.top + rowheight*(line + 1);
			rc.top = rc.top + rowheight*line + 1;
		}
		else if (g_StatusBarData.VAlign == 1) { // center
			rc.bottom = rc.top + rowsdy + rowheight*(line + 1);
			rc.top = rc.top + rowsdy + rowheight*line + 1;
		}
		else if (g_StatusBarData.VAlign == 2) { // bottom
			rc.top = rc.bottom - (rowheight*(linecount - line));
			rc.bottom = rc.bottom - (rowheight*(linecount - line - 1) + 1);
		}

		int textY = rc.top + (((rc.bottom - rc.top) - textSize.cy) / 2);
		int iconY = rc.top + (((rc.bottom - rc.top) - iconHeight) / 2);

		// Code for each line
		uint32_t sw;
		int rectwidth = rc.right - rc.left - g_StatusBarData.rectBorders.left - g_StatusBarData.rectBorders.right;
		if (visProtoCount > 1)
			sw = (rectwidth - (g_StatusBarData.extraspace*(visProtoCount - 1))) / visProtoCount;
		else
			sw = rectwidth;

		int *ProtoWidth = (int*)_alloca(sizeof(int)*visProtoCount);
		for (int i = 0; i < visProtoCount; i++) {
			ProtoItemData &p = ProtosData[line*protosperline + i];

			uint32_t w = p.PaddingLeft;
			w += p.PaddingRight;

			if (p.bShowProtoIcon) {
				w += iconWidth + 1;

				p.extraIcon = nullptr;
				if ((p.xStatusMode & 8) && p.iProtoStatus > ID_STATUS_OFFLINE) {
					wchar_t str[512];
					CUSTOM_STATUS cs = { sizeof(cs) };
					cs.flags = CSSF_MASK_NAME | CSSF_UNICODE;
					cs.ptszName = str;
					if (CallProtoService(p.szAccountName, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) == 0)
						p.tszProtoXStatus = mir_wstrdup(str);
				}

				if (p.xStatusMode & 3) {
					if (p.iProtoStatus > ID_STATUS_OFFLINE) {
						if (ProtoServiceExists(p.szAccountName, PS_GETCUSTOMSTATUSICON))
							p.extraIcon = (HICON)CallProtoService(p.szAccountName, PS_GETCUSTOMSTATUSICON, 0, 0);
						if (p.extraIcon && (p.xStatusMode & 3) == 3)
							w += iconWidth + 1;
					}
				}
			}

			SIZE txtSize;
			if (p.bShowProtoName) {
				GetTextExtentPoint32(hDC, p.tszProtoHumanName, (int)mir_wstrlen(p.tszProtoHumanName), &txtSize);
				w += txtSize.cx + 3 + spaceWidth;
			}

			if (p.bShowProtoEmails && p.szProtoEMailCount) {
				GetTextExtentPoint32(hDC, p.szProtoEMailCount, (int)mir_wstrlen(p.szProtoEMailCount), &txtSize);
				w += txtSize.cx + 3 + spaceWidth;
			}

			if (p.bShowStatusName) {
				GetTextExtentPoint32(hDC, p.tszProtoStatusText, (int)mir_wstrlen(p.tszProtoStatusText), &txtSize);
				w += txtSize.cx + 3 + spaceWidth;
			}

			if ((p.xStatusMode & 8) && p.tszProtoXStatus) {
				GetTextExtentPoint32(hDC, p.tszProtoXStatus, (int)mir_wstrlen(p.tszProtoXStatus), &txtSize);
				w += txtSize.cx + 3 + spaceWidth;
			}

			if (p.bShowProtoName || (p.bShowProtoEmails && p.szProtoEMailCount) || p.bShowStatusName || ((p.xStatusMode & 8) && p.tszProtoXStatus))
				w -= spaceWidth;

			p.fullWidth = w;
			if (g_StatusBarData.sameWidth) {
				ProtoWidth[i] = sw;
				SumWidth += sw;
			}
			else {
				ProtoWidth[i] = w;
				SumWidth += w;
			}
		}

		// Reposition rects
		for (int i = 0; i < visProtoCount; i++)
			if (ProtoWidth[i] > maxwidth)
				maxwidth = ProtoWidth[i];

		if (g_StatusBarData.sameWidth) {
			for (int i = 0; i < visProtoCount; i++)
				ProtoWidth[i] = maxwidth;
			SumWidth = maxwidth * visProtoCount;
		}
		SumWidth += (visProtoCount - 1) * g_StatusBarData.extraspace;

		if (SumWidth > rectwidth) {
			float f = (float)rectwidth / SumWidth;
			SumWidth = 0;
			for (int i = 0; i < visProtoCount; i++) {
				ProtoWidth[i] = (int)((float)ProtoWidth[i] * f);
				SumWidth += ProtoWidth[i];
			}
			SumWidth += (visProtoCount - 1) * g_StatusBarData.extraspace;
		}

		if (g_StatusBarData.Align == 1) //center
			aligndx = (rectwidth - SumWidth) >> 1;
		else if (g_StatusBarData.Align == 2) //right
			aligndx = (rectwidth - SumWidth);

		// Draw in rects
		RECT r = rc;
		r.left += g_StatusBarData.rectBorders.left + aligndx;
		for (int i = 0; i < visProtoCount; i++) {
			ProtoItemData &p = ProtosData[line*protosperline + i];
			HICON hIcon = nullptr;
			HICON hxIcon = nullptr;
			BOOL bNeedDestroy = false;
			int x = r.left;
			x += p.PaddingLeft;
			r.right = r.left + ProtoWidth[i];

			if (p.bShowProtoIcon) {
				if (p.iProtoStatus > ID_STATUS_OFFLINE && (p.xStatusMode & 3) > 0) {
					if (ProtoServiceExists(p.szAccountName, PS_GETCUSTOMSTATUSICON)) {
						hxIcon = p.extraIcon;
						if (hxIcon) {
							if ((p.xStatusMode & 3) == 2) {
								hIcon = GetMainStatusOverlay(p.iProtoStatus);
								bNeedDestroy = true;
							}
							else if ((p.xStatusMode & 3) == 1) {
								hIcon = hxIcon;
								bNeedDestroy = true;
								hxIcon = nullptr;
							}
						}
					}
				}

				if (hIcon == nullptr && (hxIcon == nullptr || ((p.xStatusMode & 3) == 3))) {
					if ((p.bConnectingIcon == 1) && IsStatusConnecting(p.iProtoStatus)) {
						hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)p.szAccountName, 0);
						if (hIcon)
							bNeedDestroy = true;
						else
							hIcon = Skin_LoadProtoIcon(p.szAccountName, p.iProtoStatus);
					}
					else hIcon = Skin_LoadProtoIcon(p.szAccountName, p.iProtoStatus);
				}

				HRGN rgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);

				if (g_StatusBarData.sameWidth) {
					int fw = p.fullWidth;
					int rw = r.right - r.left;
					if (g_StatusBarData.Align == 1)
						x = r.left + ((rw - fw) / 2);
					else if (g_StatusBarData.Align == 2)
						x = r.left + ((rw - fw));
					else
						x = r.left;
				}

				SelectClipRgn(hDC, rgn);
				p.bDoubleIcons = false;

				uint32_t dim = p.bIsDimmed ? ((64 << 24) | 0x80) : 0;

				if ((p.xStatusMode & 3) == 3) {
					if (hIcon)
						ske_DrawIconEx(hDC, x, iconY, hIcon, iconWidth, iconHeight, 0, nullptr, DI_NORMAL | dim);
					if (hxIcon) {
						ske_DrawIconEx(hDC, x + iconWidth + 1, iconY, hxIcon, iconWidth, iconHeight, 0, nullptr, DI_NORMAL | dim);
						x += iconWidth + 1;
					}
					p.bDoubleIcons = hIcon && hxIcon;
				}
				else {
					if (hxIcon)
						ske_DrawIconEx(hDC, x, iconY, hxIcon, iconWidth, iconHeight, 0, nullptr, DI_NORMAL | dim);
					if (hIcon)
						ske_DrawIconEx(hDC, x, iconY, hIcon, iconWidth, iconHeight, 0, nullptr, DI_NORMAL | ((hxIcon && (p.xStatusMode & 4)) ? (192 << 24) : 0) | dim);
				}

				if (hxIcon || hIcon) { // TODO g_StatusBarData.bDrawLockOverlay  options to draw locked proto
					if (Proto_GetAccount(p.szAccountName)->IsLocked()) {
						HICON hLockOverlay = Skin_LoadIcon(SKINICON_OTHER_STATUS_LOCKED);
						if (hLockOverlay != nullptr) {
							ske_DrawIconEx(hDC, x, iconY, hLockOverlay, iconWidth, iconHeight, 0, nullptr, DI_NORMAL | dim);
							IcoLib_ReleaseIcon(hLockOverlay);
						}
					}
				}
				if (hxIcon) DestroyIcon_protect(hxIcon);
				if (bNeedDestroy)
					DestroyIcon_protect(hIcon);
				else
					IcoLib_ReleaseIcon(hIcon);
				x += iconWidth + 1;
				DeleteObject(rgn);
			}

			if (p.bShowProtoName) {
				int cbLen = (int)mir_wstrlen(p.tszProtoHumanName);
				RECT rt = r;
				rt.left = x + (spaceWidth >> 1);
				rt.top = textY;
				ske_DrawText(hDC, p.tszProtoHumanName, cbLen, &rt, 0);

				if ((p.bShowProtoEmails && p.szProtoEMailCount != nullptr) || p.bShowStatusName || ((p.xStatusMode & 8) && p.tszProtoXStatus)) {
					SIZE txtSize;
					GetTextExtentPoint32(hDC, p.tszProtoHumanName, cbLen, &txtSize);
					x += txtSize.cx + 3;
				}
			}

			if (p.bShowProtoEmails && p.szProtoEMailCount != nullptr) {
				int cbLen = (int)mir_wstrlen(p.szProtoEMailCount);
				RECT rt = r;
				rt.left = x + (spaceWidth >> 1);
				rt.top = textY;
				ske_DrawText(hDC, p.szProtoEMailCount, cbLen, &rt, 0);
				if (p.bShowStatusName || ((p.xStatusMode & 8) && p.tszProtoXStatus)) {
					SIZE txtSize;
					GetTextExtentPoint32(hDC, p.szProtoEMailCount, cbLen, &txtSize);
					x += txtSize.cx + 3;
				}
			}

			if (p.bShowStatusName) {
				int cbLen = (int)mir_wstrlen(p.tszProtoStatusText);
				RECT rt = r;
				rt.left = x + (spaceWidth >> 1);
				rt.top = textY;
				ske_DrawText(hDC, p.tszProtoStatusText, cbLen, &rt, 0);
				if (((p.xStatusMode & 8) && p.tszProtoXStatus)) {
					SIZE txtSize;
					GetTextExtentPoint32(hDC, p.tszProtoStatusText, cbLen, &txtSize);
					x += txtSize.cx + 3;
				}
			}

			if ((p.xStatusMode & 8) && p.tszProtoXStatus) {
				RECT rt = r;
				rt.left = x + (spaceWidth >> 1);
				rt.top = textY;
				ske_DrawText(hDC, p.tszProtoXStatus, (int)mir_wstrlen(p.tszProtoXStatus), &rt, 0);
			}

			p.protoRect = r;

			r.left = r.right + g_StatusBarData.extraspace;
		}
	}

	SelectObject(hDC, hOldFont);
	ske_ResetTextEffect(hDC);
	return 0;
}

#define TOOLTIP_TOLERANCE 5

LRESULT CALLBACK ModernStatusProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static POINT ptToolTipShow = { 0 };
	switch (msg) {
	case WM_CREATE:
		g_StatusBarData.hTheme = xpt_AddThemeHandle(hwnd, L"STATUS");
		break;

	case WM_DESTROY:
		xpt_FreeThemeForWindow(hwnd);
		ProtosData.destroy();
		break;

	case WM_SIZE:
		if (!g_CluiData.fLayered || GetParent(hwnd) != g_clistApi.hwndContactList)
			InvalidateRect(hwnd, nullptr, FALSE);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		if (GetParent(hwnd) == g_clistApi.hwndContactList && g_CluiData.fLayered)
			CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)hwnd, 0);
		else if (GetParent(hwnd) == g_clistApi.hwndContactList && !g_CluiData.fLayered) {
			RECT rc = { 0 };
			GetClientRect(hwnd, &rc);
			rc.right++;
			rc.bottom++;
			HDC hdc = GetDC(hwnd);
			HDC hdc2 = CreateCompatibleDC(hdc);
			HBITMAP hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			HBITMAP hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);
			SetBkMode(hdc2, TRANSPARENT);
			ske_BltBackImage(hwnd, hdc2, &rc);
			ModernDrawStatusBarWorker(hwnd, hdc2);
			BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc2, rc.left, rc.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);

			SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
			ReleaseDC(hwnd, hdc);
			ValidateRect(hwnd, nullptr);
		}
		else {
			RECT rc;
			GetClientRect(hwnd, &rc);

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdc2 = CreateCompatibleDC(hdc);
			HBITMAP hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			HBITMAP hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);

			HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
			FillRect(hdc2, &ps.rcPaint, br);
			ModernDrawStatusBarWorker(hwnd, hdc2);
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
				hdc2, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);
			ps.fErase = FALSE;
			EndPaint(hwnd, &ps);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_GETMINMAXINFO:
		RECT rct;
		GetWindowRect(hwnd, &rct);
		memset((LPMINMAXINFO)lParam, 0, sizeof(MINMAXINFO));
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 16;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = 16;
		((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = 1600;
		((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = 1600;
		return 0;

	case WM_SHOWWINDOW:
		if (tooltipshoing) {
			NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip, 0, 0);
			tooltipshoing = FALSE;
		}
		{
			int ID = Sync(FindFrameID, hwnd);
			if (ID) {
				int res = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, ID), 0);
				if (res >= 0)
					db_set_b(0, "CLUI", "ShowSBar", (uint8_t)(wParam/*(res&F_VISIBLE)*/ ? 1 : 0));
			}
		}
		break;

	case WM_TIMER:
		if (wParam == TM_STATUSBARHIDE) {
			KillTimer(hwnd, TM_STATUSBARHIDE);
			if (tooltipshoing) {
				NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip, 0, 0);
				tooltipshoing = FALSE;
				ReleaseCapture();
			}
		}
		else if (wParam == TM_STATUSBAR) {
			POINT pt;
			KillTimer(hwnd, TM_STATUSBAR);
			GetCursorPos(&pt);
			if (pt.x == lastpnt.x && pt.y == lastpnt.y) {
				ScreenToClient(hwnd, &pt);
				for (auto &it : ProtosData) {
					RECT rc = it->protoRect;
					if (PtInRect(&rc, pt)) {
						NotifyEventHooks(g_CluiData.hEventStatusBarShowToolTip, (WPARAM)it->szAccountName, 0);
						CLUI_SafeSetTimer(hwnd, TM_STATUSBARHIDE, db_get_w(0, "CLUIFrames", "HideToolTipTime", SETTING_HIDETOOLTIPTIME_DEFAULT), nullptr);
						tooltipshoing = TRUE;
						ClientToScreen(hwnd, &pt);
						ptToolTipShow = pt;
						SetCapture(hwnd);
						return 0;
					}
				}
				return 0;
			}
		}
		return 0;

	case WM_MOUSEMOVE:
		if (tooltipshoing) {
			POINT pt;
			GetCursorPos(&pt);
			if (abs(pt.x - ptToolTipShow.x) > TOOLTIP_TOLERANCE || abs(pt.y - ptToolTipShow.y) > TOOLTIP_TOLERANCE) {
				KillTimer(hwnd, TM_STATUSBARHIDE);
				NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip, 0, 0);
				tooltipshoing = FALSE;
				ReleaseCapture();
			}
		}
		break;

	case WM_SETCURSOR:
		if (g_CluiData.bBehindEdgeSettings) CLUI_UpdateTimer();
		{
			POINT pt;
			GetCursorPos(&pt);
			SendMessage(GetParent(hwnd), msg, wParam, lParam);
			if (pt.x == lastpnt.x && pt.y == lastpnt.y)
				return(CLUI_TestCursorOnBorders());

			lastpnt = pt;
			if (tooltipshoing)
				if (abs(pt.x - ptToolTipShow.x) > TOOLTIP_TOLERANCE || abs(pt.y - ptToolTipShow.y) > TOOLTIP_TOLERANCE) {
					KillTimer(hwnd, TM_STATUSBARHIDE);
					NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip, 0, 0);
					tooltipshoing = FALSE;
					ReleaseCapture();
				}
			KillTimer(hwnd, TM_STATUSBAR);
			CLUI_SafeSetTimer(hwnd, TM_STATUSBAR, db_get_w(0, "CLC", "InfoTipHoverTime", CLCDEFAULT_INFOTIPTIME), nullptr);
		}
		return CLUI_TestCursorOnBorders();

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		RECT rc;
		POINT pt = UNPACK_POINT(lParam);
		KillTimer(hwnd, TM_STATUSBARHIDE);
		KillTimer(hwnd, TM_STATUSBAR);

		if (tooltipshoing)
			NotifyEventHooks(g_CluiData.hEventStatusBarHideToolTip, 0, 0);

		tooltipshoing = FALSE;
		for (auto &p : ProtosData) {
			bool isOnExtra = false;

			rc = p->protoRect;
			RECT rc1 = rc;
			rc1.left = rc.left + 16;
			rc1.right = rc1.left + 16;
			if (PtInRect(&rc, pt) && PtInRect(&rc1, pt) && p->bDoubleIcons)
				isOnExtra = true;

			if (PtInRect(&rc, pt)) {
				BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000);
				BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000);

				if (msg == WM_LBUTTONDOWN && bCtrl) {
					char protoF[_countof(g_CluiData.protoFilter)];
					mir_snprintf(protoF, "%s|", p->szAccountName);

					if (g_CluiData.bFilterEffective != CLVM_FILTER_PROTOS || !bShift) {
						ApplyViewMode("");

						// if a user clicks on the same proto again, disable filter
						if (!mir_strcmp(protoF, g_CluiData.protoFilter)) {
							g_CluiData.protoFilter[0] = 0;
							g_CluiData.bFilterEffective = 0;
						}
						else {
							mir_snprintf(g_CluiData.protoFilter, "%s|", p->szAccountName);
							g_CluiData.bFilterEffective = CLVM_FILTER_PROTOS;
						}
					}
					else {
						char *pos = strstri(g_CluiData.protoFilter, protoF);
						if (pos) {
							// remove filter
							size_t len = mir_strlen(protoF);
							memmove(pos, pos + len, mir_strlen(pos + len) + 1);

							if (mir_strlen(g_CluiData.protoFilter) == 0)
								ApplyViewMode("");
							else
								g_CluiData.bFilterEffective = CLVM_FILTER_PROTOS;
						}
						else {
							// add filter
							strncat_s(g_CluiData.protoFilter, protoF, _TRUNCATE);
							g_CluiData.bFilterEffective = CLVM_FILTER_PROTOS;
						}
					}

					if (g_CluiData.bFilterEffective == CLVM_FILTER_PROTOS) {
						CMStringA szFilterName;
						szFilterName.AppendChar(13);

						int protoCount;
						PROTOACCOUNT **accs;
						Proto_EnumAccounts(&protoCount, &accs);

						bool first = true;
						for (int pos = 0; pos < protoCount; pos++) {
							int k = Clist_GetAccountIndex(pos);
							if (k < 0 || k >= protoCount)
								continue;

							mir_snprintf(protoF, "%s|", accs[k]->szModuleName);
							if (strstri(g_CluiData.protoFilter, protoF)) {
								if (!first)
									szFilterName.Append("; ");
								szFilterName.Append(T2Utf(accs[k]->tszAccountName));
								first = false;
							}
						}

						SaveViewMode(szFilterName, L"", g_CluiData.protoFilter, 0, -1, 0, 0, 0, 0);
						ApplyViewMode(szFilterName);
					}
					Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
					cliInvalidateRect(hwnd, nullptr, FALSE);
					SetCapture(nullptr);
					return 0;
				}

				HMENU hMenu = nullptr;
				if (msg == WM_RBUTTONDOWN) {
					BOOL a = ((g_StatusBarData.perProtoConfig && p->SBarRightClk) || g_StatusBarData.SBarRightClk);
					if (a ^ bShift)
						hMenu = Menu_GetMainMenu();
					else
						hMenu = Menu_GetStatusMenu();
				}
				else {
					hMenu = Menu_GetStatusMenu();
					HMENU hSubMenu = GetSubMenu(hMenu, p->iProtoPos);
					if (hSubMenu)
						hMenu = hSubMenu;
				}

				ClientToScreen(hwnd, &pt);

				HWND parent = GetParent(hwnd);
				if (parent != g_clistApi.hwndContactList) parent = GetParent(parent);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, parent, nullptr);
				return 0;
			}
		}

		GetClientRect(hwnd, &rc);
		if (PtInRect(&rc, pt) && msg == WM_LBUTTONDOWN && g_CluiData.bFilterEffective == CLVM_FILTER_PROTOS) {
			ApplyViewMode("");
			cliInvalidateRect(hwnd, nullptr, FALSE);
			SetCapture(nullptr);
			return 0;
		}
		return SendMessage(GetParent(hwnd), msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

wchar_t pluginname[] = L"ModernStatusBar";

HWND StatusBar_Create(HWND parent)
{
	WNDCLASS wndclass = { 0 };
	int h = GetSystemMetrics(SM_CYSMICON) + 2;
	if (GetClassInfo(g_plugin.getInst(), pluginname, &wndclass) == 0) {
		wndclass.lpfnWndProc = ModernStatusProc;
		wndclass.hInstance = g_plugin.getInst();
		wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		wndclass.lpszClassName = pluginname;
		RegisterClass(&wndclass);
	}

	hModernStatusBar = CreateWindow(pluginname, pluginname, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 0, h, parent, nullptr, g_plugin.getInst(), nullptr);

	// register frame
	CLISTFrame Frame = { sizeof(Frame) };
	Frame.hWnd = hModernStatusBar;
	Frame.align = alBottom;
	Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	Frame.Flags = F_LOCKED | F_NOBORDER | F_NO_SUBCONTAINER;
	if (db_get_b(0, "CLUI", "ShowSBar", SETTING_SHOWSBAR_DEFAULT))
		Frame.Flags |= F_VISIBLE;
	Frame.height = h;
	Frame.szName.a = "Status bar";
	Frame.szTBname.a = LPGEN("Status bar");
	g_plugin.addFrame(&Frame);
	
	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)Frame.hWnd, (LPARAM)NewStatusPaintCallbackProc); //$$$$$ register sub for frame

	LoadStatusBarData();
	cliCluiProtocolStatusChanged(0, nullptr);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME, -1, 0);
	return hModernStatusBar;
}
