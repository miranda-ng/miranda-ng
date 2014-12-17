/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#define TOP 0
#define LEFT 1
#define BOTTOM 2
#define RIGHT 3

struct Options
{
	bool cycle_through_protocols;
	int seconds_to_show_protocol;
	bool replace_smileys;
	bool resize_smileys;
	bool use_contact_list_smileys;

	bool draw_text_rtl;
	bool draw_text_align_right;

	bool draw_show_protocol_name;
	bool show_protocol_cycle_button;

	bool global_on_avatar;
	bool global_on_nickname;
	bool global_on_status;
	bool global_on_status_message;

	bool draw_avatar_custom_size;
	bool draw_avatar_allow_to_grow;
	int  draw_avatar_custom_size_pixels;
	bool draw_avatar_border;
	COLORREF draw_avatar_border_color;
	bool draw_avatar_round_corner;
	bool draw_avatar_use_custom_corner_size;
	int draw_avatar_custom_corner_size;

	COLORREF bkg_color;
	int borders[4];

	bool use_avatar_space_to_draw_text;

	bool resize_frame;
	bool use_skinning;

	int refresh_status_message_timer;
};

extern Options opts;


// Initializations needed by options
void InitOptions();

// Loads the options from DB
// It don't need to be called, except in some rare cases
void LoadOptions();


#endif // __OPTIONS_H__
