/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci
Based on work (C) Heiko Schillinger

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

#define TYPE_GLOBAL 0
#define TYPE_LOCAL 1

typedef struct 
{
	BOOL last_sent_enable;
	int last_sent_msg_type;
	BOOL hide_from_offline_proto;
	BOOL hide_subcontacts;
	BOOL keep_subcontacts_from_offline;
	BOOL group_append;
	BOOL group_column;
	BOOL group_column_left;

	int num_protos;
} Options;

extern Options opts;


// Initializations needed by options
void InitOptions();

// Deinitializations needed by options
void DeInitOptions();


// Loads the options from DB
// It don't need to be called, except in some rare cases
void LoadOptions();




#endif // __OPTIONS_H__
