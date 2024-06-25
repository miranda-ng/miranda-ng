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

#define MODULENAME "ProtoCyrTranslitByIKR"

namespace CyrTranslit
{
	//------------------------------------------------------------------------------

	void TransliterationProtocol::initialize()
	{
		Proto_RegisterModule(PROTOTYPE_TRANSLATION, MODULENAME);

		CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, sendMessage);
	}

	//------------------------------------------------------------------------------

	INT_PTR TransliterationProtocol::sendMessage(WPARAM wParam, LPARAM lParam)
	{
		CCSDATA *ccs = (CCSDATA *)lParam;
		if (!MirandaContact::bIsActive(ccs->hContact))
			return Proto_ChainSend(wParam, ccs);

		LPARAM oldlParam = ccs->lParam;
		
		CMStringA szEncoded(cyrillicToLatin((char *)ccs->lParam));
		ccs->lParam = (LPARAM)szEncoded.c_str();

		int ret = Proto_ChainSend(wParam, ccs);
		ccs->lParam = oldlParam;
		return ret;
	}
}
