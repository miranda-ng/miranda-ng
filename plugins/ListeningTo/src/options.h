/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

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


#ifndef __OPTIONS_H__
# define __OPTIONS_H__


#include "stdafx.h"

#include <windows.h>


#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENHISTORY 2

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2

#define SET_XSTATUS 0
#define CHECK_XSTATUS 1
#define CHECK_XSTATUS_MUSIC 2
#define IGNORE_XSTATUS 3


struct Options {
	bool enable_sending;
	bool enable_music;
	bool enable_radio;
	bool enable_video;
	bool enable_others;

	wchar_t templ[1024];
	wchar_t unknown[128];

	bool override_contact_template;
	bool show_adv_icon;
	int adv_icon_slot;

	bool get_info_from_watrack;
	bool enable_other_players;
	bool enable_code_injection;
	int time_to_pool;

	uint16_t xstatus_set;
	wchar_t xstatus_name[1024];
	wchar_t xstatus_message[1024];
	wchar_t nothing[128];
};

extern Options opts;


// Initializations needed by options
void InitOptions();

// Loads the options from DB
// It don't need to be called, except in some rare cases
void LoadOptions();



BOOL IsTypeEnabled(LISTENINGTOINFO *lti);


#endif // __OPTIONS_H__
