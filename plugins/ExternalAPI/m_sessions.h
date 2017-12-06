/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef M_SESSIONS_H__
#define M_SESSIONS_H__

//////////////////////////////////////////////////////////////////////////
// Services
//
//////////////////////////////////////////////////////////////////////////
// Opens session load dialog
//
// wParam  = 0
// lParam  = 0
#define	MS_SESSIONS_OPENMANAGER			"Sessions/Service/OpenManager"

//////////////////////////////////////////////////////////////////////////
// Loads last session
//
// wParam = 0 
// lParam  = 0
#define	MS_SESSIONS_RESTORELASTSESSION		"Sessions/Service/OpenLastSession"

//////////////////////////////////////////////////////////////////////////
// Opens current/user-defined session save dialog 
//
// wParam = 0
// lParam = 0
#define	MS_SESSIONS_SAVEUSERSESSION		"Sessions/Service/SaveUserSession"

//////////////////////////////////////////////////////////////////////////
// Closes current opened session 
//
// wParam = 0
// lParam = 0
#define	MS_SESSIONS_CLOSESESSION			"Sessions/Service/CloseSession"

//////////////////////////////////////////////////////////////////////////
// Builds Favorite Sessions menu 
//
// wParam = 0
// lParam = 0
#define	MS_SESSIONS_SHOWFAVORITESMENU		"Sessions/Service/ShowFavMenu"


#endif  //M_SESSIONS_H__