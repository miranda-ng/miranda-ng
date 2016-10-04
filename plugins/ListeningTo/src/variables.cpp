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

#include "stdafx.h"

wchar_t* VariablesParseInfo(ARGUMENTSINFO *ai)
{
	if (ai->cbSize < sizeof(ARGUMENTSINFO))
		return NULL;

	LISTENINGTOINFO *lti = GetListeningInfo();
	if (lti == NULL) {
		ai->flags = AIF_FALSE;
		return mir_wstrdup(L"");
	}

	wchar_t *fr[] = {
		L"artist", UNKNOWN(lti->ptszArtist),
		L"album", UNKNOWN(lti->ptszAlbum),
		L"title", UNKNOWN(lti->ptszTitle),
		L"track", UNKNOWN(lti->ptszTrack),
		L"year", UNKNOWN(lti->ptszYear),
		L"genre", UNKNOWN(lti->ptszGenre),
		L"length", UNKNOWN(lti->ptszLength),
		L"player", UNKNOWN(lti->ptszPlayer),
		L"type", UNKNOWN(lti->ptszType)
	};

	Buffer<wchar_t> ret;
	ReplaceTemplate(&ret, NULL, opts.templ, fr, _countof(fr));
	return ret.detach();
}

#define VARIABLES_PARSE_BODY(__field__) \
	if (ai == NULL || ai->cbSize < sizeof(ARGUMENTSINFO)) \
		return NULL; \
	\
	LISTENINGTOINFO *lti = GetListeningInfo(); \
	if (lti == NULL) \
			{ \
		ai->flags = AIF_FALSE; \
		return mir_wstrdup(L""); \
			} \
				else if (IsEmpty(lti->__field__))  \
	{ \
		ai->flags = AIF_FALSE; \
		return mir_wstrdup(opts.unknown); \
	} \
				else \
	{ \
		ai->flags = AIF_DONTPARSE; \
		wchar_t *ret = mir_wstrdup(lti->__field__); \
		return ret; \
	}


wchar_t* VariablesParseType(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszType);
}

wchar_t* VariablesParseArtist(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszArtist);
}

wchar_t* VariablesParseAlbum(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszAlbum);
}

wchar_t* VariablesParseTitle(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszTitle);
}

wchar_t* VariablesParseTrack(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszTrack);
}

wchar_t* VariablesParseYear(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszYear);
}

wchar_t* VariablesParseGenre(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszGenre);
}

wchar_t* VariablesParseLength(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszLength);
}

wchar_t* VariablesParsePlayer(ARGUMENTSINFO *ai)
{
	VARIABLES_PARSE_BODY(ptszPlayer);
}
