/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

#ifndef __icons_h__
#define __icons_h__

//ICONS
#define SECT_TOLBAR			"Toolbar"
#define SECT_POPUP			MODULNAME_PLU
#define SECT_POPUP_ACT		"/Actions"
#define SECT_POPUP_OPT		"/Options"
#define SECT_POPUP_MISC		"/Misc"

#define ICO_TB_POPUP_ON		"TBButton_popup_ToogleUp"
#define ICO_TB_POPUP_OFF	"TBButton_popup_ToogleDOWN"
#define ICO_POPUP_ON		MODULNAME"_enabled"
#define ICO_POPUP_OFF		MODULNAME"_disabled"
#define ICO_FAV				MODULNAME"_favourite"
#define ICO_FULLSCREEN		MODULNAME"_fullscreen"
#define ICO_HISTORY			MODULNAME"_history"

#define ICO_MISC_NOTIFY		MODULNAME"_Misc_Notification"
#define ICO_MISC_WARNING	MODULNAME"_Misc_Warning"
#define ICO_MISC_ERROR		MODULNAME"_Misc_Error"

#define ICO_ACT_MESS		MODULNAME"_act_message"
#define ICO_ACT_INFO		MODULNAME"_act_info"
#define ICO_ACT_MENU		MODULNAME"_act_menu"
#define ICO_ACT_ADD			MODULNAME"_act_add"
#define ICO_ACT_REPLY		MODULNAME"_act_reply"
#define ICO_ACT_PIN			MODULNAME"_act_pin"
#define ICO_ACT_PINNED		MODULNAME"_act_pinned"
#define ICO_ACT_CLOSE		MODULNAME"_act_close"
#define ICO_ACT_COPY		MODULNAME"_act_copy"

#define ICO_OPT_RELOAD		MODULNAME"_opt_reload"
#define ICO_OPT_RESIZE		MODULNAME"_opt_resize"
#define ICO_OPT_GROUP		MODULNAME"_opt_group"
#define ICO_OPT_OK			MODULNAME"_opt_ok"
#define ICO_OPT_CANCEL		MODULNAME"_opt_cancel"

#define ICO_OPT_DEF			MODULNAME"_opt_default"
#define ICO_OPT_FAV			MODULNAME"_opt_favorite"
#define ICO_OPT_FULLSCREEN	MODULNAME"_opt_fullscreen"
#define ICO_OPT_BLOCK		MODULNAME"_opt_block"

void	InitIcons();
HICON	IcoLib_GetIcon(LPCSTR pszIcon, bool big = false);

#endif // __icons_h__
