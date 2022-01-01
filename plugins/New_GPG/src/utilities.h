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

#ifndef UTILITIES_H
#define UTILITIES_H

void GetFilePath(wchar_t *WindowTittle, char *szSetting, wchar_t *szExt, wchar_t *szExtDesc);
wchar_t *GetFilePath(wchar_t *WindowTittle, wchar_t *szExt, wchar_t *szExtDesc, bool save_file = false);
void GetFolderPath(wchar_t *WindowTittle);

void setSrmmIcon(MCONTACT);

void send_encrypted_msgs_thread(void*);

int ComboBoxAddStringUtf(HWND hCombo, const wchar_t *szString, uint32_t data);
bool isContactSecured(MCONTACT hContact);
bool isContactHaveKey(MCONTACT hContact);
bool isGPGKeyExist();
bool isGPGValid();

void ExportGpGKeysFunc(int type);
void ImportKey(MCONTACT hContact, std::wstring new_key);

void SendErrorMessage(MCONTACT hContact);

const bool StriStr(const char *str, const char *substr);
string toUTF8(wstring str);
wstring toUTF16(string str);
string get_random(int length);

void HistoryLog(MCONTACT, const char *msg, uint32_t _time = 0, uint32_t _flags = 0);
void fix_line_term(std::string &s);
void strip_line_term(std::wstring &s);
void strip_line_term(std::string &s);
void strip_tags(std::string &s);
void clean_temp_dir();
bool gpg_validate_paths(wchar_t *gpg_bin_path, wchar_t *gpg_home_path);
void gpg_save_paths(wchar_t *gpg_bin_path, wchar_t *gpg_home_path);
bool gpg_use_new_random_key(const char *account_name);

#endif
