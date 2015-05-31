/**
 * CyrTranslit: the Cyrillic transliteration plug-in for Miranda IM.
 * Copyright 2005 Ivan Krechetov. 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "stdafx.h"

namespace CyrTranslit
{

char *TransliterationProtocol::MODULE_NAME = "ProtoCyrTranslitByIKR";

//------------------------------------------------------------------------------

void TransliterationProtocol::initialize()
{
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE_NAME;
	pd.type = PROTOTYPE_TRANSLATION;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	CreateProtoServiceFunction(MODULE_NAME, PSS_MESSAGE, sendMessage);
}

//------------------------------------------------------------------------------
void TransliterationProtocol::TranslateMessageUTF(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = reinterpret_cast<CCSDATA*>(lParam);

	wchar_t* txtWdecoded = mir_utf8decodeW(reinterpret_cast<const char*>(ccs->lParam));
	std::wstring txtW = txtWdecoded;
	mir_free(txtWdecoded);

	txtW = TransliterationMap::getInstance().cyrillicToLatin(txtW);

	char* txtUTFencoded = mir_utf8encodeW(txtW.c_str());
	std::string txtUTF = txtUTFencoded;
	mir_free(txtUTFencoded);

	ccs->lParam = reinterpret_cast<LPARAM>(mir_alloc(txtUTF.length()));
	mir_strcpy(reinterpret_cast<char*>(ccs->lParam), txtUTF.c_str());
}

//------------------------------------------------------------------------------

INT_PTR TransliterationProtocol::sendMessage(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = reinterpret_cast<CCSDATA*>(lParam);
	if ( !MirandaContact::bIsActive(ccs->hContact))
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);

	LPARAM oldlParam = ccs->lParam;
	bool msgProcessed = true;

	TranslateMessageUTF(wParam, lParam);

	int ret = CallService(MS_PROTO_CHAINSEND, wParam, lParam);

	if (msgProcessed) {
		mir_free(reinterpret_cast<void*>(ccs->lParam));
		ccs->lParam = oldlParam;
	}

	return ret;
}

}
