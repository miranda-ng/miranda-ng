/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

namespace Modern
{
	extern CMOption<bool> bDisableEngine, bEnableLayering, bAeroGlass;
	extern CMOption<bool> bInternalAwayMsgDiscovery, bRemoveAwayMessageForOffline;

	extern CMOption<uint8_t> iHideBehind, iBehindEdge;
	extern CMOption<uint16_t> iShowDelay, iHideDelay, iHideBehindBorderSize;
}

#define SETTING_MIN2TRAY_DEFAULT                  1 
#define SETTING_DISABLEWORKINGSET_DEFAULT         1 
														    
#define SETTING_SHOWSBAR_DEFAULT                  1 
#define SETTING_USECONNECTINGICON_DEFAULT         1 
#define SETTING_SHOWXSTATUS_DEFAULT               6 
#define SETTING_SHOWUNREADEMAILS_DEFAULT          1 
#define SETTING_SBARSHOW_DEFAULT                  3 
#define SETTING_VALIGN_DEFAULT                    1 
#define SETTING_MINWIDTH_DEFAULT                 18 
#define SETTING_MAXSIZEHEIGHT_DEFAULT            75 
#define SETTING_MINSIZEHEIGHT_DEFAULT            10 
#define SETTING_SNAPTOEDGES_DEFAULT               1 
#define SETTING_DOCKTOSIDES_DEFAULT               1 
														    
#define SETTING_TEXTEFFECTID_DEFAULT           0xFF 
#define SETTING_HIDETOOLTIPTIME_DEFAULT        5000 
														    
#define SETTING_EXTRACOLUMNSPACE_DEFAULT         18 
														    
#define SETTING_GAPFRAMES_DEFAULT                 1 
#define SETTING_GAPTITLEBAR_DEFAULT               1 
#define SETTING_KEYCOLOR_DEFAULT                  RGB(255,0,255) 

#define SETTING_METAAVOIDDBLCLICK_DEFAULT         1 
#define SETTING_METAIGNOREEMPTYEXTRA_DEFAULT      1 
#define SETTING_METAEXPANDING_DEFAULT             1 
#define SETTING_USEMETAICON_DEFAULT               1 
#define SETTING_DRAWOVERLAYEDSTATUS_DEFAULT       3 
															    
															    
#define SETTING_SORTBY1_DEFAULT                   SORTBY_RATE
#define SETTING_SORTBY2_DEFAULT                   SORTBY_NAME_LOCALE
#define SETTING_SORTBY3_DEFAULT                   SORTBY_PROTO
															    
#define SETTING_OFFLINEBOTTOM_DEFAULT             1
#define SETTING_HILIGHTMODE_DEFAULT               1
															    
#define SETTING_EVENTAREAMODE_DEFAULT             1
#define SETTING_SHOWEVENTAREAFRAME_DEFAULT        1
															    
#define SETTING_TRAYOPTION_DEFAULT               15

//////////////////////////////////////////////////////////////////////////
// ROW SETTINGS
#define SETTING_ROWBORDER_DEFAULT                 1 
#define SETTING_ROW_ROWBORDER_DEFAULT             1 
#define SETTING_VARIABLEROWHEIGHT_DEFAULT         1 
#define SETTING_ALIGNLEFTTOLEFT_DEFAULT           1 
#define SETTING_ALIGNRIGHTORIGHT_DEFAULT          1 

#define SETTINGS_AVATARCORNERSIZE_DEFAULT         4 
#define SETTINGS_AVATAROVERLAYTYPE_DEFAULT        SETTING_AVATAR_OVERLAY_TYPE_NORMAL
#define SETTING_AVATARHEIGHT_DEFAULT             24 
#define SETTING_AVATARWIDTH_DEFAULT              24 

#define SETTING_SHOWTIMEIFDIFF_DEFAULT            1 

#define SETTING_TEXT_SMILEY_DEFAULT               1 
#define SETTING_TEXT_RESIZESMILEY_DEFAULT         1 
#define SETTING_TEXT_PROTOSMILEY_DEFAULT          1 

#define SETTING_FIRSTLINE_TRIMTEXT_DEFAULT        1 

#define SETTING_SECONDLINE_TOPSPACE_DEFAULT       2 
#define SETTING_SECONDLINE_SMILEYS_DEFAULT        1 
#define SETTING_SECONDLINE_TYPE_DEFAULT           TEXT_STATUS_MESSAGE 
#define SETTING_SECONDLINE_XSTATUS_DEFAULT        1 
#define SETTING_SECONDLINE_STATUSIFNOAWAY_DEFAULT 1 
#define SETTING_SECONDLINE_LISTENINGIFNOAWAY_DEFAULT 1

#define SETTING_THIRDLINE_TOPSPACE_DEFAULT        2 
#define SETTING_THIRDLINE_TYPE_DEFAULT            TEXT_STATUS_MESSAGE 
#define SETTING_THIRDLINE_XSTATUS_DEFAULT         1 

#define CLCDEFAULT_MODERN_SELTEXTCOLOUR           (g_CluiData.fDisableSkinEngine?GetSysColor(COLOR_HIGHLIGHTTEXT):RGB(0,0,128))
#define CLCDEFAULT_MODERN_HOTTEXTCOLOUR           (g_CluiData.fDisableSkinEngine ? CLCDEFAULT_MODERN_SELTEXTCOLOUR : RGB(0,0,255))
#define CLCDEFAULT_MODERN_QUICKSEARCHCOLOUR       RGB(255,255,0)
#define CLCDEFAULT_INFOTIPTIME                  750 

#define SETTINGS_BARBTNWIDTH_DEFAULT             22 
#define SETTINGS_BARBTNHEIGHT_DEFAULT            22 
#define SETTINGS_BARAUTOSIZE_DEFAULT              1 
#define SETTINGS_BARMULTILINE_DEFAULT             1 

#define SETTING_ENABLESOUNDS_DEFAULT              1 
