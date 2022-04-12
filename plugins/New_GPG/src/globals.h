// Copyright © 2010-22 sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef GLOBALS_H
#define GLOBALS_H

struct contact_data
{
	list<string>  msgs_to_send;// msgs_to_pass;
	string key_in_prescense;
};

struct globals_s
{
	bool bDecryptFiles = false;
	CMStringW wszInopentag, wszInclosetag, wszOutopentag, wszOutclosetag, wszPassword;
	wchar_t key_id_global[17] = { 0 };
	list <JabberAccount*> Accounts;
	HFONT bold_font = nullptr;
	logtofile debuglog;
	bool gpg_valid = false, gpg_keyexist = false;
	std::map<MCONTACT, contact_data> hcontact_data;
	bool _terminate;
};

extern globals_s globals;


#endif
