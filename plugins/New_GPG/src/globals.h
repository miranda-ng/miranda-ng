// Copyright © 2010-19 sss
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

struct globals_s
{
	bool bAppendTags = false, bDebugLog = false, bJabberAPI = false, bPresenceSigning = false, bFileTransfers = false, bSameAction = false, bAutoExchange = false, bStripTags = false, tabsrmm_used = false, bDecryptFiles = false;;
	wchar_t *inopentag = nullptr, *inclosetag = nullptr, *outopentag = nullptr, *outclosetag = nullptr, *password = nullptr;
	wchar_t key_id_global[17] = { 0 };
	list <JabberAccount*> Accounts;
	HFONT bold_font = nullptr;
	HANDLE hLoadPubKey = nullptr, g_hCLIcon = nullptr, hExportGpgKeys = nullptr, hImportGpgKeys = nullptr;
	HGENMENU hSendKey = nullptr, hToggleEncryption = nullptr;
	RECT key_from_keyserver_rect = { 0 }, firstrun_rect = { 0 }, new_key_rect = { 0 }, key_gen_rect = { 0 }, load_key_rect = { 0 }, import_key_rect = { 0 }, key_password_rect = { 0 }, load_existing_key_rect = { 0 };
	logtofile debuglog;
	bool gpg_valid = false, gpg_keyexist = false;
	std::map<MCONTACT, contact_data> hcontact_data;
	map<int, MCONTACT> user_data;
	bool _terminate;

	int item_num; //TODO: get rid of this

};

extern globals_s globals;


#endif
