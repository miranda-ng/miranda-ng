/* 
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#define POPUPS 3

//=== Objects =====
//This one is used to easily tie status id, icon, text...
typedef struct
{
	int ID;
	COLORREF colorBack;
	COLORREF colorText;
} aPopups;

#define DEFAULT_POPUP_LCLICK				1
#define DEFAULT_POPUP_RCLICK				0
#define DEFAULT_POPUP_ENABLED				1
#define DEFAULT_MESSAGE_ENABLED				0
#define DEFAULT_TIMEOUT_VALUE				0

#define COLOR_BG_FIRSTDEFAULT	RGB(173,206,247)
#define COLOR_BG_SECONDDEFAULT	RGB(255,189,189)
#define COLOR_TX_DEFAULT		RGB(0,0,0)

#define byCOLOR_OWN	0x1
#define byCOLOR_WINDOWS	0x2
#define byCOLOR_POPUP	0x3
#define DEFAULT_COLORS	byCOLOR_POPUP

#define POPUP_TYPE_MSG  0
#define POPUP_TYPE_ERROR 1
#define POPUP_TYPE_INFO 2

// Actions on popup click
#define PCA_CLOSEPOPUP		0	// close popup
#define PCA_DONOTHING		1   // do nothing

//===== Options flags
typedef struct tagMYOPTIONS {
	BYTE DefColors;
	BYTE LeftClickAction;
	BYTE RightClickAction;
	int Timeout;
} POPUP_OPTIONS;

static struct {
	TCHAR *Text;
	int Action;
} PopupActions[] = {
	LPGENT("Close popup"), PCA_CLOSEPOPUP,
	LPGENT("Do nothing"), PCA_DONOTHING
};
