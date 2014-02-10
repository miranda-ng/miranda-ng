// Copyright © 2010-2012 sss
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
extern bool bAppendTags, bPresenceSigning, bStripTags, gpg_valid, gpg_keyexist, tabsrmm_used, bSameAction, bFileTransfers, bDebugLog;
extern TCHAR *inopentag, *inclosetag, *outopentag, *outclosetag;
extern logtofile debuglog;

extern map<int, MCONTACT> user_data;
extern int item_num;

extern bool _terminate;
extern wstring new_key;
extern MCONTACT new_key_hcnt;
extern boost::mutex new_key_hcnt_mutex;
#endif
