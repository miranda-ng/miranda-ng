/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common.h"

class LibCurl
{
private:
	typedef CURL					*PCURL;
	typedef struct curl_slist		*PCURL_SLIST;
	typedef curl_version_info_data	*PVERSION_INFO;	

	typedef CURLcode		(__cdecl * _curl_global_init) (long);
	typedef void			(__cdecl * _curl_global_cleanup) (void);

	typedef PCURL			(__cdecl * _curl_easy_init) (void);
	typedef void			(__cdecl * _curl_easy_cleanup) (PCURL);	
	typedef CURLcode 		(__cdecl * _curl_easy_perform) (PCURL);
	typedef CURLcode 		(__cdecl * _curl_easy_pause) (PCURL,int);
	typedef CURLcode 		(__cdecl * _curl_easy_setopt) (PCURL,CURLoption,...);

	typedef PCURL_SLIST		(__cdecl * _curl_slist_append) (PCURL_SLIST,LPCSTR);
	typedef void			(__cdecl * _curl_slist_free_all) (PCURL_SLIST);
	
	typedef PVERSION_INFO	(__cdecl * _curl_version_info) (CURLversion);

	static LibCurl *instance;
	LibCurl() { };
	~LibCurl() { instance = NULL; };

public:
	_curl_global_init		global_init;
	_curl_global_cleanup	global_cleanup;
	_curl_easy_init			easy_init;
	_curl_easy_cleanup		easy_cleanup;
	_curl_easy_perform		easy_perform;
	_curl_easy_pause		easy_pause;
	_curl_easy_setopt		easy_setopt;
	_curl_slist_append		slist_append;
	_curl_slist_free_all	slist_free_all;
	_curl_version_info		version_info;

	static LibCurl &getInstance() 
	{
		if (!instance)
			instance = new LibCurl();
		return *instance;
	};

	BOOL init();
	void deinit();
};