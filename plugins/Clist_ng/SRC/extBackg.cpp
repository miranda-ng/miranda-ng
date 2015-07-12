/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * parts (C) by pixel, written many years ago for the original
 * clist_nicer plugin.
 * $Id: extBackg.cpp 138 2010-11-01 10:51:15Z silvercircle $
 *
 */

#include <commonheaders.h>
#include "../coolsb/coolscroll.h"

extern HWND g_hwndViewModeFrame;
extern struct CluiTopButton top_buttons[];

void ReloadThemedOptions();


TStatusItem DefaultStatusItems[ID_EXTBK_LAST_D + 1] = {
    {"Offline", ID_EXTBKOFFLINE,
        CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Online", ID_EXTBKONLINE,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Away", ID_EXTBKAWAY,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"DND", ID_EXTBKDND,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"NA", ID_EXTBKNA,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Occupied", ID_EXTBKOCCUPIED,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Free for chat", ID_EXTBKFREECHAT,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Invisible", ID_EXTBKINVISIBLE,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"On the phone", ID_EXTBKONTHEPHONE,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Out to lunch", ID_EXTBKOUTTOLUNCH,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Idle", ID_EXTBKIDLE,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"{-}Expanded Group", ID_EXTBKEXPANDEDGROUP,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Collapsed Group", ID_EXTBKCOLLAPSEDDGROUP,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"Empty Group", ID_EXTBKEMPTYGROUPS,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"{-}First contact of a group", ID_EXTBKFIRSTITEM,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"Single item in group", ID_EXTBKSINGLEITEM,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"Last contact of a group", ID_EXTBKLASTITEM,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"{-}First contact of NON-group", ID_EXTBKFIRSTITEM_NG,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"Single item in NON-group", ID_EXTBKSINGLEITEM_NG,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"Last contact of NON-group", ID_EXTBKLASTITEM_NG,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, 0, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"{-}Even rows", ID_EXTBKEVEN_CNTCTPOS,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"Odd rows", ID_EXTBKODD_CNTCTPOS,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"{-}Selection", ID_EXTBKSELECTION,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"Hottracked", ID_EXTBKHOTTRACK,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"{-}Frame titlebars", ID_EXTBKFRAMETITLE,
        CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 0
    }, {"Event area", ID_EXTBKEVTAREA,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 0
    }, {"Status Bar", ID_EXTBKSTATUSBAR,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1
    }, {"{-}UI Button pressed", ID_EXTBKBUTTONSPRESSED,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"UI Button normal", ID_EXTBKBUTTONSNPRESSED,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
    }, {"UI Button hover", ID_EXTBKBUTTONSMOUSEOVER,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"{-}Status floater", ID_EXTBKSTATUSFLOATER,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
	}, {"{-}Frame border no titlebar", ID_EXTBKOWNEDFRAMEBORDER,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, 1,
        1, 1, 1, CLCDEFAULT_IGNORE, S_ITEM_IMAGE_ONLY
	}, {"Frame border with titlebar", ID_EXTBKOWNEDFRAMEBORDERTB,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, 1,
        CLCDEFAULT_MRGN_TOP, 1, 1, CLCDEFAULT_IGNORE, S_ITEM_IMAGE_ONLY
	}, {"{-}Avatarframe", ID_EXTBKAVATARFRAME,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1, S_ITEM_IMAGE_ONLY
    }, {"Avatarframe offline", ID_EXTBKAVATARFRAMEOFFLINE,
        CLCDEFAULT_GRADIENT,CLCDEFAULT_CORNER,
        CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, CLCDEFAULT_TEXTCOLOR, CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT,
        CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT, CLCDEFAULT_MRGN_BOTTOM, 1, S_ITEM_IMAGE_ONLY
    }
};

void SetButtonToSkinned()
{
    int bSkinned = cfg::dat.bSkinnedButtonMode = cfg::getByte("CLCExt", "bskinned", 0);

    SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BM_SETSKINNED, 0, bSkinned);
    SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BM_SETSKINNED, 0, bSkinned);
    if(bSkinned) {
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN, 0, 0);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN, 0, 0);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN + 10, 0, 0);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN + 10, 0, 0);
    }
    else {
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN, 0, 1);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN, 0, 1);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBGLOBALSTATUS, BUTTONSETASFLATBTN + 10, 0, 1);
        SendDlgItemMessage(pcli->hwndContactList, IDC_TBMENU, BUTTONSETASFLATBTN + 10, 0, 1);
    }
    SendMessage(g_hwndViewModeFrame, WM_USER + 100, 0, 0);
}

TStatusItem *GetProtocolStatusItem(const char *szProto)
{
    int i;

    if(szProto == NULL)
        return NULL;

    for(i = ID_EXTBK_LAST_D + 1; i <= Skin::ID_EXTBK_LAST; i++) {
        if(!strcmp(Skin::statusItems[i].szName[0] == '{' ? &Skin::statusItems[i].szName[3] : Skin::statusItems[i].szName, szProto))
            return &Skin::statusItems[i];
    }
    return NULL;
}

// Save Non-StatusItems Settings
void SaveNonStatusItemsSettings(HWND hwndDlg)
{
    BOOL translated;

    cfg::dat.bApplyIndentToBg = IsDlgButtonChecked(hwndDlg, IDC_APPLYINDENTBG) ? 1 : 0;
    cfg::dat.bUsePerProto = IsDlgButtonChecked(hwndDlg, IDC_USEPERPROTO) ? 1 : 0;
    cfg::dat.bOverridePerStatusColors = IsDlgButtonChecked(hwndDlg, IDC_OVERRIDEPERSTATUSCOLOR) ? 1 : 0;
    cfg::dat.titleBarHeight = (BYTE)GetDlgItemInt(hwndDlg, IDC_LASTITEMPADDING, &translated, FALSE);
    cfg::dat.group_padding = GetDlgItemInt(hwndDlg, IDC_GRPTOPPADDING, &translated, FALSE);

    cfg::writeByte("CLCExt", "applyindentbg", (BYTE)cfg::dat.bApplyIndentToBg);
    cfg::writeByte("CLCExt", "useperproto", (BYTE)cfg::dat.bUsePerProto);
    cfg::writeByte("CLCExt", "override_status", (BYTE)cfg::dat.bOverridePerStatusColors);
    cfg::writeByte("CLCExt", "bskinned", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SETALLBUTTONSKINNED) ? 1 : 0));
    cfg::writeByte("CLC", "IgnoreSelforGroups", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IGNORESELFORGROUPS));

    cfg::writeDword("CLCExt", "grp_padding", cfg::dat.group_padding);
    cfg::writeByte("CLCExt", "frame_height", cfg::dat.titleBarHeight);

    SetButtonToSkinned();
}


void extbk_import(char *file, HWND hwndDlg)
{
    int n, i;
    char buffer[255];
    char szKey[255], szSection[255];
    DWORD data, version = 0;
    int oldexIconScale = cfg::dat.exIconScale;

    data = 0;
    GetPrivateProfileStructA("Global", "Version", &version, 4, file);
    if(version >= 2) {
        for(n = 0; n <= FONTID_LAST; n++) {
            mir_snprintf(szSection, 255, "Font%d", n);

            mir_snprintf(szKey, 255, "Font%dName", n);
            GetPrivateProfileStringA(szSection, "Name", "Arial", buffer, sizeof(buffer), file);
            cfg::writeString(NULL, "CLC", szKey, buffer);

            mir_snprintf(szKey, 255, "Font%dSize", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Size", &data, 1, file);
            cfg::writeByte("CLC", szKey, (BYTE)data);

            mir_snprintf(szKey, 255, "Font%dSty", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Style", &data, 1, file);
            cfg::writeByte("CLC", szKey, (BYTE)data);

            mir_snprintf(szKey, 255, "Font%dSet", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Set", &data, 1, file);
            cfg::writeByte("CLC", szKey, (BYTE)data);

            mir_snprintf(szKey, 255, "Font%dCol", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Color", &data, 4, file);
            cfg::writeDword("CLC", szKey, data);

            mir_snprintf(szKey, 255, "Font%dFlags", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "Flags", &data, 4, file);
            cfg::writeDword("CLC", szKey, (WORD)data);

            mir_snprintf(szKey, 255, "Font%dAs", n);
            data = 0;
            GetPrivateProfileStructA(szSection, "SameAs", &data, 2, file);
            cfg::writeDword("CLC", szKey, (WORD)data);
        }
    }
    i = 0;

    ReloadThemedOptions();
    // refresh
    if(hwndDlg && ServiceExists(MS_CLNSE_FILLBYCURRENTSEL))
        CallService(MS_CLNSE_FILLBYCURRENTSEL, (WPARAM)hwndDlg, 0);
    pcli->pfnClcOptionsChanged();
    CLUI::configureGeometry(1);
    SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
	CLUI::Redraw();
    if(oldexIconScale != cfg::dat.exIconScale) {
        ImageList_SetIconSize(CLUI::hExtraImages, cfg::dat.exIconScale, cfg::dat.exIconScale);
        IcoLibReloadIcons();
        pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
    }
}

static void ApplyCLUISkin()
{
    DBVARIANT   dbv = {0};
    wchar_t       tszFinalName[MAX_PATH];
    char        szFinalName[MAX_PATH];
    if(!cfg::getTString(NULL, "CLC", "AdvancedSkin", &dbv)) {
        Utils::pathToAbsolute(dbv.ptszVal, tszFinalName);
        WideCharToMultiByte(CP_ACP, 0, tszFinalName, MAX_PATH, szFinalName, MAX_PATH, 0, 0);
        if(cfg::getByte("CLUI", "skin_changed", 0)) {
            extbk_import(szFinalName, 0);
            //SaveCompleteStructToDB();
            cfg::writeByte("CLUI", "skin_changed", 0);
        }
        //IMG_LoadItems();
        ShowWindow(pcli->hwndContactList, SW_SHOWNORMAL);
        SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
        RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE);
        db_free(&dbv);
    }
}

/**
 * scan a single skin directory and find the .TSK file. Fill the combobox and set the
 * relative path name as item extra data.
 *
 * If available, read the Name property from the [Global] section and use it in the
 * combo box. If such property is not found, the base filename (without .tsk extension)
 * will be used as the name of the skin.
 *
 */
static int ScanSkinDir(const wchar_t* tszFolder, HWND hwndCombobox)
{
	WIN32_FIND_DATA			fd = {0};
	bool					fValid = false;
	wchar_t					tszMask[MAX_PATH];

	mir_sntprintf(tszMask, MAX_PATH, L"%s*.*", tszFolder);

	HANDLE h = FindFirstFile(tszMask, &fd);

	while(h != INVALID_HANDLE_VALUE) {
		if(lstrlenW(fd.cFileName) >= 5 && !wcsncmp(fd.cFileName + lstrlenW(fd.cFileName) - 4, L".cng", 4)) {
			fValid = true;
			break;
		}
	    if(FindNextFileW(h, &fd) == 0)
	    	break;
	}
	if(h != INVALID_HANDLE_VALUE)
		FindClose(h);

	if(fValid) {
		TCHAR	tszFinalName[MAX_PATH], tszRel[MAX_PATH];
		LRESULT lr;
		TCHAR	szBuf[255];

		mir_sntprintf(tszFinalName, MAX_PATH, _T("%s%s"), tszFolder, fd.cFileName);

		GetPrivateProfileStringW(L"SkinInfo", L"Name", L"None", szBuf, 500, tszFinalName);
		if(!wcscmp(szBuf, L"None")) {
			fd.cFileName[lstrlenW(fd.cFileName) - 4] = 0;
			mir_sntprintf(szBuf, 255, L"%s", fd.cFileName);
		}

		Utils::pathToRelative(tszFinalName, tszRel, cfg::szProfileDir);
		if((lr = SendMessageW(hwndCombobox, CB_INSERTSTRING, -1, (LPARAM)szBuf)) != CB_ERR) {
			wchar_t* idata = (wchar_t *)malloc((lstrlenW(tszRel) + 1) * sizeof(wchar_t));

			wcscpy(idata, tszRel);
			SendMessageW(hwndCombobox, CB_SETITEMDATA, lr, (LPARAM)idata);
		}
	}
	return(0);
}

/**
 * scan the skin root folder for subfolder(s). Each folder is supposed to contain a single
 * skin. This function won't dive deeper into the folder structure, so the folder
 * structure for any VALID skin should be:
 * $SKINS_ROOT/skin_folder/skin_name.tsk
 *
 * By default, $SKINS_ROOT is set to %miranda_userdata% or custom folder
 * selected by the folders plugin.
 */
static int RescanSkins(HWND hwndCombobox)
{
	WIN32_FIND_DATA			fd = {0};
	wchar_t					tszSkinRoot[MAX_PATH], tszFindMask[MAX_PATH];
	DBVARIANT 				dbv = {0};


	mir_sntprintf(tszSkinRoot, MAX_PATH, L"%sskin\\clng\\", cfg::szProfileDir);

	SetDlgItemTextW(GetParent(hwndCombobox), IDC_SKINROOTFOLDER, tszSkinRoot);
	mir_sntprintf(tszFindMask, MAX_PATH, L"%s*.*", tszSkinRoot);

	SendMessageW(hwndCombobox, CB_RESETCONTENT, 0, 0);
	SendMessageW(hwndCombobox, CB_INSERTSTRING, -1, (LPARAM)TranslateW(L"<none>"));

	HANDLE h = FindFirstFileW(tszFindMask, &fd);
	while (h != INVALID_HANDLE_VALUE) {
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.') {
			TCHAR	tszSubDir[MAX_PATH];
			mir_sntprintf(tszSubDir, MAX_PATH, _T("%s%s\\"), tszSkinRoot, fd.cFileName);
			ScanSkinDir(tszSubDir, hwndCombobox);
		}
	    if(FindNextFile(h, &fd) == 0)
	    	break;
	}
	if(h != INVALID_HANDLE_VALUE)
		FindClose(h);


	SendMessage(hwndCombobox, CB_SETCURSEL, 0, 0);
	if(0 == cfg::getTString(0, SKIN_DB_MODULE, "gCurrentSkin", &dbv)) {
		LRESULT lr = SendMessage(hwndCombobox, CB_GETCOUNT, 0, 0);
		for(int i = 1; i < lr; i++) {

			wchar_t* idata = (wchar_t *)SendMessage(hwndCombobox, CB_GETITEMDATA, i, 0);
			if(idata && idata != (wchar_t *)CB_ERR) {
				if(!wcsicmp(dbv.ptszVal, idata)) {
					SendMessage(hwndCombobox, CB_SETCURSEL, i, 0);
					break;
				}
			}
		}
		db_free(&dbv);
	}
	return(0);
}

/**
 * free the item extra data (used to store the skin filenames for
 * each entry).
 */
static void TSAPI FreeComboData(HWND hwndCombobox)
{
	LRESULT lr = SendMessage(hwndCombobox, CB_GETCOUNT, 0, 0);

	for(int i = 1; i < lr; i++) {
		void *idata = (void *)SendMessage(hwndCombobox, CB_GETITEMDATA, i, 0);

		if(idata && idata != (void *)CB_ERR)
			free(idata);
	}
}


INT_PTR CALLBACK cfg::DlgProcSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
        {
            TranslateDialogDefault(hwndDlg);
			RescanSkins(GetDlgItem(hwndDlg, IDC_SKINNAME));

            CheckDlgButton(hwndDlg, IDC_SETALLBUTTONSKINNED, cfg::getByte("CLCExt", "bskinned", 0));

            SendDlgItemMessage(hwndDlg, IDC_GRPPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(20, 0));
            SendDlgItemMessage(hwndDlg, IDC_GRPPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.group_padding);

            SendDlgItemMessage(hwndDlg, IDC_LASTITEMPADDINGSPIN, UDM_SETRANGE, 0, MAKELONG(40, 0));
            SendDlgItemMessage(hwndDlg, IDC_LASTITEMPADDINGSPIN, UDM_SETPOS, 0, cfg::dat.titleBarHeight);

            CheckDlgButton(hwndDlg, IDC_APPLYINDENTBG, cfg::dat.bApplyIndentToBg);
            CheckDlgButton(hwndDlg, IDC_USEPERPROTO, cfg::dat.bUsePerProto);
            CheckDlgButton(hwndDlg, IDC_OVERRIDEPERSTATUSCOLOR, cfg::dat.bOverridePerStatusColors);
            CheckDlgButton(hwndDlg, IDC_IGNORESELFORGROUPS, cfg::getByte("CLC", "IgnoreSelforGroups", 0) ? BST_CHECKED : BST_UNCHECKED);
            return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_UNLOAD:
                    break;
                case IDC_RELOADSKIN: {
					DBVARIANT	dbv = {0};
					wchar_t 	wszSkinName[MAX_PATH];

					if(0 == cfg::getTString(0, SKIN_DB_MODULE, "gCurrentSkin", &dbv)) {
						Utils::pathToAbsolute(dbv.ptszVal, wszSkinName, cfg::szProfileDir);
						if(PathFileExistsW(wszSkinName)) {
							ShowWindow(pcli->hwndContactList, SW_HIDE);
							Skin::Unload();
							SkinLoader *s = new SkinLoader(wszSkinName);
							if(s->isValid())
								s->Load();

							delete s;
							Skin::Activate();
							ShowWindow(pcli->hwndContactList, SW_SHOW);
						}
						db_free(&dbv);
					}
                    break;
                }

				case IDC_HELP_GENERAL:
					//CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://blog.miranda.or.at/tabsrmm/skin-selection-changes/");
					break;

				case IDC_RESCANSKIN:
					FreeComboData(GetDlgItem(hwndDlg, IDC_SKINNAME));
					RescanSkins(GetDlgItem(hwndDlg, IDC_SKINNAME));
					break;

				case IDC_SKINNAME: {
					if(HIWORD(wParam) == CBN_SELCHANGE) {
						LRESULT lr = SendDlgItemMessageW(hwndDlg, IDC_SKINNAME, CB_GETCURSEL, 0 ,0);
						if(lr != CB_ERR && lr > 0) {
							wchar_t	*tszRelPath = (TCHAR *)SendDlgItemMessageW(hwndDlg, IDC_SKINNAME, CB_GETITEMDATA, lr, 0);
							if(tszRelPath && tszRelPath != (wchar_t *)CB_ERR)
								cfg::writeTString(0, SKIN_DB_MODULE, "gCurrentSkin", tszRelPath);
							SendMessageW(hwndDlg, WM_COMMAND, IDC_RELOADSKIN, 0);
						}
						else if(lr == 0)		// selected the <no skin> entry
							db_unset(0, SKIN_DB_MODULE, "gCurrentSkin");
						return(0);
					}
					break;
				}
				default:
					break;
            }
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
            break;
        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY:
                            SaveNonStatusItemsSettings(hwndDlg);
                            pcli->pfnClcOptionsChanged();
                            PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
                            return TRUE;
                    }
                    break;
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK cfg::SkinOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static int iInit = TRUE;
   static HWND hwndSkinEdit = 0;

   switch(msg)
   {
      case WM_INITDIALOG:
      {
         TCITEM tci;
         RECT rcClient;
         int oPage = cfg::getByte("CLUI", "s_opage", 0);
         SKINDESCRIPTION sd;

         TranslateDialogDefault(hwnd);
         GetClientRect(hwnd, &rcClient);
         iInit = TRUE;
         tci.mask = TCIF_PARAM|TCIF_TEXT;
         tci.lParam = (LPARAM)CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_OPT_SKIN), hwnd, cfg::DlgProcSkinOpts);
         tci.pszText = TranslateT("Load and apply");
			TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,5,25,rcClient.right-9,rcClient.bottom-65,1);
         ShowWindow((HWND)tci.lParam, oPage == 0 ? SW_SHOW : SW_HIDE);
         if(IS_THEMED)
             Api::pfnEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

		 ZeroMemory(&sd, sizeof(sd));
		 sd.cbSize = sizeof(sd);
		 sd.StatusItems = Skin::statusItems;
		 sd.hWndParent = hwnd;
		 sd.hWndTab = GetDlgItem(hwnd, IDC_OPTIONSTAB);
		 sd.pfnSaveCompleteStruct = 0; //SaveCompleteStructToDB;
		 sd.lastItem = Skin::ID_EXTBK_LAST;
		 sd.firstItem = ID_EXTBKOFFLINE;
		 sd.pfnClcOptionsChanged = pcli->pfnClcOptionsChanged;
		 sd.hwndCLUI = pcli->hwndContactList;
		 hwndSkinEdit = SkinEdit_Invoke(0, (LPARAM)&sd);

         if(hwndSkinEdit) {
             ShowWindow(hwndSkinEdit, oPage == 1 ? SW_SHOW : SW_HIDE);
             ShowWindow(sd.hwndImageEdit, oPage == 2 ? SW_SHOW : SW_HIDE);
             TabCtrl_SetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB), oPage);
             if(IS_THEMED)
                 Api::pfnEnableThemeDialogTexture(hwndSkinEdit, ETDT_ENABLETAB);
         }
         Utils::enableDlgControl(hwnd, IDC_EXPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
         Utils::enableDlgControl(hwnd, IDC_SAVEASMOD, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
         iInit = FALSE;
         return FALSE;
      }

      case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
         if(!iInit)
             SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
         break;
      case WM_COMMAND:
          switch(LOWORD(wParam)) {
              case IDC_EXPORT:
                  {
                      char 			str[MAX_PATH] = "*.cng";
                      char 			szBaseFolder[MAX_PATH];
                      OPENFILENAMEA ofn = {0};

                      if(WarningDlg::show(WarningDlg::WARN_SKIN_OVERWRITE, MB_YESNOCANCEL | MB_ICONWARNING | WarningDlg::CWF_NOALLOWHIDE, 0) != IDYES)
                    	  break;

                      WideCharToMultiByte(CP_ACP, 0, Skin::settings.wszSkinBaseFolder, -1, szBaseFolder, MAX_PATH, 0, 0);

                      ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
                      ofn.hwndOwner = hwnd;
                      ofn.hInstance = NULL;
                      ofn.lpstrFilter = "*.cng";
                      ofn.lpstrFile = str;
                      ofn.Flags = OFN_HIDEREADONLY;
                      ofn.nMaxFile = sizeof(str);
                      ofn.nMaxFileTitle = MAX_PATH;
                      ofn.lpstrDefExt = "cng";

                      ofn.lpstrInitialDir = szBaseFolder;
                      if (!GetSaveFileNameA(&ofn))
                          break;
                      Skin::exportToFile(str);
                      Skin::exportSettingsToFile(str);
                      break;
                  }
              default:
            	  break;
          }
          break;
      case WM_NOTIFY:
         switch(((LPNMHDR)lParam)->idFrom) {
            case 0:
               switch (((LPNMHDR)lParam)->code)
               {
                  case PSN_APPLY:
                     {
                        TCITEM tci;
                        int i,count;
                        tci.mask = TCIF_PARAM;
                        count = TabCtrl_GetItemCount(GetDlgItem(hwnd,IDC_OPTIONSTAB));
                        for (i=0;i<count;i++)
                        {
                           TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),i,&tci);
                           SendMessage((HWND)tci.lParam,WM_NOTIFY,0,lParam);
                        }
                     }
                  break;
               }
            break;
            case IDC_OPTIONSTAB:
               switch (((LPNMHDR)lParam)->code)
               {
                  case TCN_SELCHANGING:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);
                     }
                  break;
                  case TCN_SELCHANGE:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_SHOW);
                        cfg::writeByte("CLUI", "s_opage", (BYTE)TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)));
                        Utils::enableDlgControl(hwnd, IDC_EXPORT, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
                        Utils::enableDlgControl(hwnd, IDC_SAVEASMOD, TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_OPTIONSTAB)) != 0);
                     }
                  break;
               }
            break;

         }
      break;
      case WM_DESTROY:
          hwndSkinEdit = 0;
          break;
   }
   return FALSE;
}

int CoolSB_SetupScrollBar(HWND hwnd)
{
	cfg::dat.bSkinnedScrollbar = TRUE;

	if(cfg::getByte("CLC", "NoVScrollBar", 0)) {
        UninitializeCoolSB(hwnd);
        return 0;
    }
    if(cfg::dat.bSkinnedScrollbar) {
        InitializeCoolSB(hwnd);
        CoolSB_SetStyle(hwnd, SB_VERT, CSBS_HOTTRACKED);
    }
    else
        UninitializeCoolSB(hwnd);
    return 0;
}
