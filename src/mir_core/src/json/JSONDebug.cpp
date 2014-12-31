/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "..\commonheaders.h"

#include <m_json.h>

#ifdef JSON_DEBUG

#ifdef JSON_STDERROR
	#include <iostream>  //need std::cerr
#else
	//otherwise, use a callback to tell the end user what happened
	json_error_callback_t ErrorCallback = 0;
	void JSONDebug::register_callback(json_error_callback_t callback){
		ErrorCallback = callback;
	}
#endif

//Something went wrong or an assert failed
void JSONDebug::_JSON_FAIL(const json_string & msg){
	#ifdef JSON_STDERROR  //no callback, just use stderror
		#ifndef JSON_UNICODE
			std::cerr << msg << std::endl;
		#else
			std::cerr << std::string(msg.begin(), msg.end()) << std::endl;
		#endif
	#else
		if (ErrorCallback){  //only do anything if the callback is registered
			#ifdef JSON_LIBRARY
				ErrorCallback(msg.c_str());
			#else
				ErrorCallback(msg);
			#endif
		}
	#endif
}

//asserts that condition is true, more useful than cassert because it lets you keep going
void JSONDebug::_JSON_ASSERT(bool condition, const json_string & msg){
	if (!condition){
		_JSON_FAIL(msg);
	}
}
#endif
