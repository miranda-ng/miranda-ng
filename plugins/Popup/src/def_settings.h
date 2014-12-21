/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko

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

#ifndef __def_settings_h__
#define __def_settings_h__

//===== Styles =====
enum
{
	STYLE_FC_BEVELED,
	STYLE_FC_FLAT,

	// ranges:
	STYLE_FC_MIN = STYLE_FC_BEVELED,
	STYLE_FC_MAX = STYLE_FC_FLAT
};

//===== Sizes =====
enum
{
	STYLE_SZ_GAP = 4,
	STYLE_SZ_TEXTH = 14,
	STYLE_SZ_NAMEH = 14,
	STYLE_SZ_TIMEW = 35,
	STYLE_SZ_TIMEH = 14,

	//	STYLE_SZ_CLOCKH	= 13,
	//	STYLE_SZ_CLOCKW	= 7,
	//	STYLE_SZ_CLOCK	= 2 + 3 + 4 * STYLE_SZ_CLOCKW + 2,

	// Text and Name are style dependant.
	STYLE_SZ_TEXTW_FLAT = 146,
	STYLE_SZ_NAMEW_FLAT = 101,
	STYLE_SZ_TEXTW_BEVELED = 125,
	STYLE_SZ_NAMEW_BEVELED = 80,

	// border size
	STYLE_SZ_BORDER_BEVELED = 2,
	STYLE_SZ_BORDER_FLAT = 2
};

//===== Popup Positioning =====
enum
{
	POS_UPPERLEFT,
	POS_LOWERLEFT,
	POS_LOWERRIGHT,
	POS_UPPERRIGHT,
	POS_CENTER,

	// ranges
	POS_MINVALUE = POS_UPPERLEFT,
	POS_MAXVALUE = POS_CENTER
};

//===== Spreading =====
enum
{
	SPREADING_HORIZONTAL,
	SPREADING_VERTICAL,

	SPREADING_MINVALUE = SPREADING_HORIZONTAL,
	SPREADING_MAXVALUE = SPREADING_VERTICAL
};

//===== Popup Options flags
enum
{
	SPK_NONE,
	SPK_SMILEYADD,
	SPK_MSL,
	SPK_XEP,
	SPK_NCONVERS
};
enum
{
	TIMER_TIMETOLIVE = 26378, // My Birthday, send me something 8)
	TIMER_TESTCHANGE = 60477, // You know who you are.

	ANIM_TIME = 250,
	FADE_TIME = (ANIM_TIME),
	FADE_STEP = 10,

	MN_MIRANDA = 0x01,
	MN_ACTIVE = 0x02,

	UM_SETDLGITEMINT = 5674
};

// Defaults:
enum
{
	SETTING_BACKCOLOUR_DEFAULT = RGB(173, 206, 247),
	SETTING_TEXTCOLOUR_DEFAULT = RGB(0, 0, 0),

	SETTING_MONITOR_DEFAULT = MN_MIRANDA,
	SETTING_USEMINIMUMWIDTH_DEFAULT = 1,
	SETTING_USEMAXIMUMWIDTH_DEFAULT = 1,
	SETTING_USINGTHREADS_DEFAULT = 1,
	SETTING_MODULEISENABLED_DEFAULT = 1,
	SETTING_MULTILINE_DEFAULT = 1,
	SETTING_FADEINTIME_DEFAULT = (ANIM_TIME),
	SETTING_FADEOUTTIME_DEFAULT = (ANIM_TIME),
	SETTING_FADEOUTTIME_MIN = 0,
	SETTING_FADEINTIME_MIN = 0,
	SETTING_FADEOUTTIME_MAX = 10000,
	SETTING_FADEINTIME_MAX = 10000,

	SETTING_LIFETIME_MIN = 1,
	SETTING_LIFETIME_MAX = 60,
	SETTING_LIFETIME_DEFAULT = 4,
	SETTING_LIFETIME_INFINITE = -1,

	SETTING_ENLARGEMENT_MIN = 100,
	SETTING_ENLARGEMENT_MAX = 200,
	SETTING_ENLARGEMENT_DEFAULT = 120,

	SETTING_HISTORYSIZE_MAX = 100,
	SETTING_HISTORYSIZE_DEFAULT = 20,

	SETTING_AVTSIZE_MIN = 16,
	SETTING_AVTSIZE_MAX = 100,
	SETTING_AVTSIZE_DEFAULT = 32,

	SETTING_MINIMUMWIDTH_MIN = 80,
	SETTING_MINIMUMWIDTH_MAX = 160,
	SETTING_MINIMUMWIDTH_DEFAULT = 110,

	SETTING_MAXIMUMWIDTH_MIN = 160,
	//	SETTING_MAXIMUMWIDTH_MAX	= 640,
	SETTING_MAXIMUMWIDTH_DEFAULT = 310,

	SETTING_SMILEPACKTYPE_DEFAULT = SPK_NONE
};

extern WORD SETTING_MAXIMUMWIDTH_MAX;

#endif
