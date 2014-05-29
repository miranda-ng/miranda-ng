/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
Copyright 2007 Artem Shpynov

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

/////////////////////////////////////////////////////////////////////////
/// This file contains default settings value predefinitions
//////////////////////////////////////////////////////////////////////////

#pragma once

#define SETTING_TOOLWINDOW_DEFAULT              1 //"CList","ToolWindow"
#define SETTING_ONTOP_DEFAULT                   0 //"CList","OnTop"
#define SETTING_MIN2TRAY_DEFAULT                1 //"CList","Min2Tray"
#define SETTING_HIDEOFFLINE_DEFAULT             0 //"CList", "HideOffline"
#define SETTING_HIDEEMPTYGROUPS_DEFAULT         0 //"CList","HideEmptyGroups"
#define SETTING_USEGROUPS_DEFAULT               1 //"CList","UseGroups"
#define SETTING_PLACEOFFLINETOROOT_DEFAULT      0 //"CList","PlaceOfflineToRoot"
#define SETTING_ALWAYSPRIMARY_DEFAULT           0 //! "CList","AlwaysPrimary"
#define SETTING_DISABLETRAYFLASH_DEFAULT        0 //! "CList","DisableTrayFlash"
#define SETTING_ICONFLASHTIME_DEFAULT         550 //! "CList","IconFlashTime"
#define SETTING_THINBORDER_DEFAULT              0 //! "CList","ThinBorder"
#define SETTING_NOBORDER_DEFAULT                0 //! "CList","NoBorder"
#define SETTING_WINDOWSHADOW_DEFAULT            0 //! "CList","WindowShadow"
#define SETTING_ONDESKTOP_DEFAULT               0 //! "CList","OnDesktop"
#define SETTING_DISABLEWORKINGSET_DEFAULT       1 //! "CList","DisableWorkingSet"
#define SETTING_NOICONBLINF_DEFAULT             0 //! "CList","NoIconBlink"
#define SETTING_ALWAYSVISICON_DEFAULT           0 //! "CList","AlwaysShowAlwaysVisIcon"

#define SETTING_SHOWMAINMENU_DEFAULT            1 //"CLUI","ShowMainMenu"
#define SETTING_SHOWCAPTION_DEFAULT             1 //"CLUI","ShowCaption"
#define SETTING_CLIENTDRAG_DEFAULT              1 //"CLUI","ClientAreaDrag"
#define SETTING_SHOWSBAR_DEFAULT                1 //! "CLUI","ShowSBar"
#define SETTING_SBARPERPROTO_DEFAULT            0 //! "CLUI","SBarPerProto"
#define SETTING_USECONNECTINGICON_DEFAULT       1 //! "CLUI","UseConnectingIcon"
#define SETTING_SHOWXSTATUS_DEFAULT             6 //! "CLUI","ShowXStatus"
#define SETTING_SHOWUNREADEMAILS_DEFAULT        0 //! "CLUI","ShowUnreadEmails"
#define SETTING_SBARSHOW_DEFAULT                3 //! "CLUI","SBarShow"
#define SETTING_SBARACCOUNTISCUSTOM_DEFAULT     0 //! "CLUI","AccountIsCustom"
#define SETTING_SBARHIDEACCOUNT_DEFAULT         0 //! "CLUI","HideAccount"
#define SETTING_SBARRIGHTCLK_DEFAULT            0 //! "CLUI","SBarRightClk"
#define SETTING_EQUALSECTIONS_DEFAULT           0 //! "CLUI","EqualSections"
#define SETTING_LEFTOFFSET_DEFAULT              0 //! "CLUI","LeftOffset"
#define SETTING_RIGHTOFFSET_DEFAULT             0 //! "CLUI","RightOffset
#define SETTING_TOPOFFSET_DEFAULT               0 //! "CLUI","TopOffset"
#define SETTING_BOTTOMOFFSET_DEFAULT            0 //! "CLUI","BottomOffset
#define SETTING_SPACEBETWEEN_DEFAULT            0 //! "CLUI","SpaceBetween"
#define SETTING_ALIGN_DEFAULT                   0 //! "CLUI","Align"
#define SETTING_VALIGN_DEFAULT                  1 //! "CLUI","VAlign"
#define SETTING_PADDINGLEFT_DEFAULT				0 //! "CLUI","PaddingLeft_*"
#define SETTING_PADDINGRIGHT_DEFAULT			0 //! "CLUI","PaddingRight_*"
#define SETTING_DRAGTOSCROLL_DEFAULT            0 //! "CLUI","DragToScroll"
#define SETTING_AUTOSIZE_DEFAULT                0 //! "CLUI","AutoSize"
#define SETTING_LOCKSIZE_DEFAULT                0 //! "CLUI","LockSize"
#define SETTING_MINHEIGTH_DEFAULT               0 //! "CLUI","MinHeight"
#define SETTING_MINWIDTH_DEFAULT               18 //! "CLUI","MinWidth"
#define SETTING_MAXSIZEHEIGHT_DEFAULT          75 //! "CLUI","MaxSizeHeight"
#define SETTING_MINSIZEHEIGHT_DEFAULT          10 //! "CLUI","MinSizeHeight"
#define SETTING_AUTOSIZEUPWARD_DEFAULT          0 //! "CLUI","AutoSizeUpward"
#define SETTING_SNAPTOEDGES_DEFAULT             1 //! "CLUI","SnapToEdges"
#define SETTING_DOCKTOSIDES_DEFAULT				1 //! "CLUI","DockToSides",

#define SETTING_PROTOSPERLINE_DEFAULT           0 //! "CLUI","StatusBarProtosPerLine"
#define SETTING_TEXTEFFECTID_DEFAULT         0xFF //! "StatusBar","TextEffectID"
#define SETTING_TEXTEFFECTCOLOR1_DEFAULT        0 //! "StatusBar","TextEffectColor1"
#define SETTING_TEXTEFFECTCOLOR2_DEFAULT        0 //! "StatusBar","TextEffectColor2"
#define SETTING_SBHILIGHTMODE_DEFAULT           0 //! "StatusBar","HiLightMode"
#define SETTING_HIDETOOLTIPTIME_DEFAULT      5000 //! "CLUIFrames","HideToolTipTime"

#define SETTING_EXTRA_ICON_EMAIL_DEFAULT        0 //!
#define SETTING_EXTRA_ICON_PROTO_DEFAULT        0 //!
#define SETTING_EXTRA_ICON_SMS_DEFAULT          0 //!
#define SETTING_EXTRA_ICON_ADV1_DEFAULT         1 //!
#define SETTING_EXTRA_ICON_ADV2_DEFAULT         1 //!
#define SETTING_EXTRA_ICON_WEB_DEFAULT          0 //!
#define SETTING_EXTRA_ICON_CLIENT_DEFAULT       1 //!
#define SETTING_EXTRA_ICON_VISMODE_DEFAULT      1 //!
#define SETTING_EXTRA_ICON_ADV3_DEFAULT         1 //!
#define SETTING_EXTRA_ICON_ADV4_DEFAULT         1 //!

#define SETTING_EXTRACOLUMNSPACE_DEFAULT       18 //! "CLUI","ExtraColumnSpace"

#define SETTING_HIDEBEHIND_DEFAULT              0 //! "ModernData","HideBehind" //(0-none, 1-leftedge, 2-rightedge)
#define SETTING_BEHINDEDGE_DEFAULT              0 //! "ModernData", "BehindEdge"
#define SETTING_SHOWDELAY_DEFAULT               3 //! "ModernData","ShowDelay"
#define SETTING_HIDEDELAY_DEFAULT               3 //! "ModernData","HideDelay"
#define SETTING_HIDEBEHINDBORDERSIZE_DEFAULT    0 //! "ModernData","HideBehindBorderSize"
#define SETTING_AEROGLASS_DEFAULT               1 //! "ModernData","AeroGlass"

#define SETTING_ROUNDCORNERS_DEFAULT            0 //! "CLC","RoundCorners"
#define SETTING_GAPFRAMES_DEFAULT               1 //! "CLUIFrames","GapBetweenFrames"
#define SETTING_GAPTITLEBAR_DEFAULT             1 //! "CLUIFrames","GapBetweenTitleBar"
#define SETTING_LEFTCLIENTMARIGN_DEFAULT        0 //! "CLUI","LeftClientMargin"
#define SETTING_RIGHTCLIENTMARIGN_DEFAULT       0 //! "CLUI","RightClientMargin"
#define SETTING_TOPCLIENTMARIGN_DEFAULT         0 //! "CLUI","TopClientMargin"
#define SETTING_BOTTOMCLIENTMARIGN_DEFAULT      0 //! "CLUI","BottomClientMargin"
#define SETTING_KEYCOLOR_DEFAULT   RGB(255,0,255) //! "ModernSettings","KeyColor"
#define SETTING_LINEUNDERMENU_DEFAULT           0 //! "CLUI","LineUnderMenu"
#define SETTING_SHOWONSTART_DEFAULT             0 //! "CList","ShowOnStart"

#define SETTING_INTERNALAWAYMSGREQUEST_DEFAULT   1 //"ModernData","InternalAwayMsgDiscovery"
#define SETTING_REMOVEAWAYMSGFOROFFLINE_DEFAULT  1 //"ModernData","RemoveAwayMessageForOffline"
#define SETTING_FILTERSEARCH_DEFAULT			 0 //"ModernData","FilterSearch"
#define SETTING_METAAVOIDDBLCLICK_DEFAULT        1 //"CLC","MetaDoubleClick"
#define SETTING_METAIGNOREEMPTYEXTRA_DEFAULT     1 //"CLC","MetaIgnoreEmptyExtra"
#define SETTING_METAHIDEEXTRA_DEFAULT            0 //"CLC","MetaHideExtra"
#define SETTING_METAEXPANDING_DEFAULT            1 //"CLC","MetaExpanding"
#define SETTING_METAHIDEOFFLINESUB_DEFAULT       1 //"CLC","MetaHideOfflineSub"
#define SETTING_USEMETAICON_DEFAULT              0 //"CLC","Meta"
#define SETTING_DRAWOVERLAYEDSTATUS_DEFAULT      3 //todo replace by contstants


#define SETTING_SORTBY1_DEFAULT        SORTBY_RATE //"CList","SortBy1"
#define SETTING_SORTBY2_DEFAULT        SORTBY_NAME //"CList","SortBy2"
#define SETTING_SORTBY3_DEFAULT      SORTBY_STATUS //"CList","SortBy3"

#define SETTING_PLACEOOFLINETOROOT_DEFAULT       0 //"CList","PlaceOfflineToRoot"
#define SETTING_NOOFFLINEBOTTOM_DEFAULT          0 //"CList","NoOfflineBottom"
#define SETTING_HIDEOFFLINEATROOT_DEFAULT        0 //"CLC","HideOfflineRoot"
#define SETTING_HILIGHTMODE_DEFAULT              0 //todo replace by constant //"CLC","HiLightMode"

#define SETTING_DISABLESKIN_DEFAULT              0 //"ModernData","DisableEngine"
#define SETTING_ENABLELAYERING_DEFAULT           1 //! "ModernData","EnableLayering"
#define SETTING_COMPACTMODE_DEFAULT              0 //"CLC","CompactMode"

#define SETTING_EVENTAREAMODE_DEFAULT            1 //autohide todo replace by const //"CLUI","EventArea"
#define SETTING_SHOWEVENTAREAFRAME_DEFAULT       1 //"CLUI","ShowEventArea"

#define SETTING_TRAYOPTION_DEFAULT              15 //show combined icon
#define SETTING_FADEIN_DEFAULT                   0 //"CLUI","FadeInOut"

//////////////////////////////////////////////////////////////////////////
// ROW SETTINGS
#define SETTING_ROWBORDER_DEFAULT                1 //"CList","RowBorder"
#define SETTING_ROW_ADVANCEDLAYOUT_DEFAULT       0 //"ModernData","UseAdvancedRowLayout"
#define SETTING_ROW_ROWBORDER_DEFAULT            1 //"CList","RowBorder"
#define SETTING_VARIABLEROWHEIGHT_DEFAULT        1 //"CList","VariableRowHeight"
#define SETTING_ALIGNLEFTTOLEFT_DEFAULT          0 //"CList","AlignLeftItemsToLeft"
#define SETTING_ALIGNRIGHTORIGHT_DEFAULT         1 //"CList","AlignRightItemsToRight"
#define SETTING_HIDEGROUPSICON_DEFAULT           0 //"CList","HideGroupsIcon"
#define SETTING_ALIGNGROPCAPTION_DEFAULT         0 //left todo replace by const //"CList","AlignGroupCaptions"
#define SETTINS_ROWITEMORDER_DEFAULT              {ITEM_AVATAR, ITEM_ICON, ITEM_TEXT, ITEM_EXTRA_ICONS, ITEM_CONTACT_TIME}

#define SETTINGS_SHOWAVATARS_DEFAULT             1 //"CList","AvatarsShow"
#define SETTINGS_AVATARDRAWBORDER_DEFAULT        0 //"CList","AvatarsDrawBorders"
#define SETTINGS_AVATARBORDERCOLOR_DEFAULT       RGB(0,0,0) //"CList","AvatarsBorderColor"
#define SETTINGS_AVATARROUNDCORNERS_DEFAULT      0 //"CList","AvatarsRoundCorners"
#define SETTINGS_AVATARUSECUTOMCORNERSIZE_DEFAULT   0 //"CList","AvatarsUseCustomCornerSize"
#define SETTINGS_AVATARCORNERSIZE_DEFAULT           4 //"CList","AvatarsCustomCornerSize"
#define SETTINGS_AVATARIGNORESIZEFORROW_DEFAULT  0 //"CList","AvatarsIgnoreSizeForRow"
#define SETTINGS_AVATARDRAWOVERLAY_DEFAULT       0 //"CList","AvatarsDrawOverlay"
#define SETTINGS_AVATAROVERLAYTYPE_DEFAULT       SETTING_AVATAR_OVERLAY_TYPE_NORMAL
#define SETTING_AVATARHEIGHT_DEFAULT            24 //"CList","AvatarsSize"
#define SETTING_AVATARWIDTH_DEFAULT             24 //"CList","AvatarsWidth"
#define SETTINGS_AVATARINSEPARATE_DEFAULT		0 //"CList","AvatarsInSeparateWnd",

#define SETTING_HIDEICONONAVATAR_DEFAULT            0 //"CList","IconHideOnAvatar"
#define SETTING_ICONONAVATARPLACE_DEFAULT               0 //"CList","IconDrawOnAvatarSpace"
#define SETTING_ICONIGNORESIZE_DEFAULT            0 //"CList","IconIgnoreSizeForRownHeight"
#define SETTING_SHOWTIME_DEFAULT                          0 //"CList","ContactTimeShow"
#define SETTING_SHOWTIMEIFDIFF_DEFAULT            1 //"CList","ContactTimeShowOnlyIfDifferent"

#define SETTING_TEXT_RTL_DEFAULT                          0 //"CList","TextRTL"
#define SETTING_TEXT_RIGHTALIGN_DEFAULT               0 //"CList","TextAlignToRight"
#define SETTING_TEXT_SMILEY_DEFAULT                     1 //"CList","TextReplaceSmileys"
#define SETTING_TEXT_RESIZESMILEY_DEFAULT               1 //"CList","TextResizeSmileys"
#define SETTING_TEXT_PROTOSMILEY_DEFAULT              1 //"CList","TextUseProtocolSmileys"
#define SETTING_TEXT_IGNORESIZE_DEFAULT               0 //"CList","TextIgnoreSizeForRownHeight"

#define SETTING_FIRSTLINE_SMILEYS_DEFAULT             1 //"CList","FirstLineDrawSmileys"
#define SETTING_FIRSTLINE_APPENDNICK_DEFAULT          0 //"CList","FirstLineAppendNick"
#define SETTING_FIRSTLINE_TRIMTEXT_DEFAULT        1 //"CList","TrimText"

#define SETTING_SECONDLINE_SHOW_DEFAULT                   1 //"CList","SecondLineShow"
#define SETTING_SECONDLINE_TOPSPACE_DEFAULT           2 //"CList","SecondLineTopSpace"
#define SETTING_SECONDLINE_SMILEYS_DEFAULT              1 //"CList","SecondLineDrawSmileys"
#define SETTING_SECONDLINE_TYPE_DEFAULT                   TEXT_STATUS_MESSAGE //"CList","SecondLineType"
#define SETTING_SECONDLINE_XSTATUS_DEFAULT              1 //"CList","SecondLineXStatusHasPriority"
#define SETTING_SECONDLINE_XSTATUSNAMETEXT_DEFAULT    0 //"CList","SecondLineUseNameAndMessageForXStatus"
#define SETTING_SECONDLINE_STATUSIFNOAWAY_DEFAULT         1 //"CList","SecondLineShowStatusIfNoAway"
#define SETTING_SECONDLINE_LISTENINGIFNOAWAY_DEFAULT    1 //"CList","SecondLineShowListeningIfNoAway"

#define SETTING_THIRDLINE_SHOW_DEFAULT                    0 //"CList","ThirdLineShow"
#define SETTING_THIRDLINE_TOPSPACE_DEFAULT            2 //"CList","ThirdLineTopSpace"
#define SETTING_THIRDLINE_SMILEYS_DEFAULT                 0 //"CList","ThirdLineDrawSmileys"
#define SETTING_THIRDLINE_TYPE_DEFAULT                    TEXT_STATUS_MESSAGE //"CList","ThirdLineType"
#define SETTING_THIRDLINE_XSTATUS_DEFAULT                 1 //"ThirdLineXStatusHasPriority"
#define SETTING_THIRDLINE_XSTATUSNAMETEXT_DEFAULT         0 //"ThirdLineUseNameAndMessageForXStatus"
#define SETTING_THIRDLINE_STATUSIFNOAWAY_DEFAULT          0 //"CList","ThirdLineShowStatusIfNoAway"
#define SETTING_THIRDLINE_LISTENINGIFNOAWAY_DEFAULT   0 //"ThirdLineShowListeningIfNoAway"

#define SETTING_TRANSPARENT_DEFAULT               0 //"CList","Transparent"
#define SETTING_AUTOALPHA_DEFAULT                   150 //"CList","AutoAlpha"
#define SETTING_CONFIRMDELETE_DEFAULT             1 //"CList","ConfirmDelete"
#define SETTING_AUTOHIDE_DEFAULT                    0 //"CList","AutoHide"
#define SETTING_HIDETIME_DEFAULT                    30 //"CList","HideTime"
#define SETTING_CYCLETIME_DEFAULT                   4 //"CList","CycleTime"
#define SETTING_TRAYICON_DEFAULT                    SETTING_TRAYICON_SINGLE //"CList","TrayIcon"
#define SETTING_ALWAYSSTATUS_DEFAULT              0 //"CList","AlwaysStatus"
#define SETTING_ALWAYSMULTI_DEFAULT               0 //"CList","AlwaysMulti"

#define SETTING_BLENDINACTIVESTATE_DEFAULT  0 //"CLC","BlendInActiveState"

#define CLCDEFAULT_MODERN_SELTEXTCOLOUR     (g_CluiData.fDisableSkinEngine?GetSysColor(COLOR_HIGHLIGHTTEXT):RGB(0,0,128))
#define CLCDEFAULT_MODERN_HOTTEXTCOLOUR     (g_CluiData.fDisableSkinEngine ? CLCDEFAULT_MODERN_SELTEXTCOLOUR : RGB(0,0,255))
#define CLCDEFAULT_MODERN_QUICKSEARCHCOLOUR RGB(255,255,0)
#define CLCDEFAULT_NOVSCROLL         0 //"CLC","NoVScrollBar"
#define CLCDEFAULT_INFOTIPTIME       750 //"! "CLC","InfoTipHoverTime"
#define CLCDEFAULT_COLLICONTOLEFT    0 //"! "FrameTitleBar","AlignCOLLIconToLeft"

#define SKIN_OFFSET_TOP_DEFAULT      0 //! "ModernSkin","SizeMarginOffset_Top"
#define SKIN_OFFSET_BOTTOM_DEFAULT   0 //! "ModernSkin","SizeMarginOffset_Bottom"
#define SKIN_OFFSET_LEFT_DEFAULT     0 //! "ModernSkin","SizeMarginOffset_Left"
#define SKIN_OFFSET_RIGHT_DEFAULT    0 //! "ModernSkin","SizeMarginOffset_Right"
#define SKIN_SPACEBEFOREGROUP_DEFAULT 0 //! "ModernSkin","SpaceBeforeGroup"

#define SETTINGS_BARBTNWIDTH_DEFAULT	22 //"ModernToolBar", "option_Bar0_BtnWidth"
#define SETTINGS_BARBTNHEIGHT_DEFAULT	22 //"ModernToolBar", "option_Bar0_BtnHeight"
#define SETTINGS_BARBTNSPACE_DEFAULT	0  //"ModernToolBar", "option_Bar0_BtnSpace"
#define SETTINGS_BARAUTOSIZE_DEFAULT	1  //"ModernToolBar", "option_Bar0_Autosize"
#define SETTINGS_BARMULTILINE_DEFAULT	1  //"ModernToolBar", "option_Bar0_Multiline"

#define SETTING_ENABLESOUNDS_DEFAULT 1 // !"Skin", "UseSound",
