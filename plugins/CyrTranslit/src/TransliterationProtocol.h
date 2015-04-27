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

#pragma once

#include "stdafx.h"

namespace CyrTranslit
{

/**
 * A simnple TRANSLATION-type Miranda protocol which performs the
 * transliteration of Cyrillic text for selected contacts.
 */
class TransliterationProtocol
{
public:
	/**
	* Initializes this protocol on Miranda start-up.
	*/
	static void initialize();

	/**
	* A call-back function called when a new outgoing message must be
	* transferred by this protocol.
	*
	* @param wParam flags
	* @param lParam (LPARAM)(const char*)szMessage
	* @return a hProcess corresponding to the one in the ack event.
	*/
	static INT_PTR sendMessage(WPARAM wParam, LPARAM lParam);

	static void TranslateMessageA(WPARAM wParam, LPARAM lParam);
	static void TranslateMessageW(WPARAM wParam, LPARAM lParam);
	static void TranslateMessageUTF(WPARAM wParam, LPARAM lParam);

private:
	static char *MODULE_NAME;

	TransliterationProtocol();
	~TransliterationProtocol();
};

}