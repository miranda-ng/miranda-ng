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

struct TransliterationMap
{
	TransliterationMap()
	{
		theMapW["а"] = "a";
		theMapW["б"] = "b";
		theMapW["в"] = "v";
		theMapW["г"] = "g";
		theMapW["д"] = "d";
		theMapW["е"] = "e";
		theMapW["ё"] = "e";
		theMapW["ж"] = "zh";
		theMapW["з"] = "z";
		theMapW["и"] = "i";
		theMapW["й"] = "i";
		theMapW["к"] = "k";
		theMapW["л"] = "l";
		theMapW["м"] = "m";
		theMapW["н"] = "n";
		theMapW["о"] = "o";
		theMapW["п"] = "p";
		theMapW["р"] = "r";
		theMapW["с"] = "s";
		theMapW["т"] = "t";
		theMapW["у"] = "u";
		theMapW["ф"] = "f";
		theMapW["х"] = "kh";
		theMapW["ц"] = "ts";
		theMapW["ч"] = "ch";
		theMapW["ш"] = "sh";
		theMapW["щ"] = "sch";
		theMapW["ъ"] = "`";
		theMapW["ы"] = "y";
		theMapW["ь"] = "'";
		theMapW["э"] = "e";
		theMapW["ю"] = "yu";
		theMapW["я"] = "ya";

		theMapW["А"] = "A";
		theMapW["Б"] = "B";
		theMapW["В"] = "V";
		theMapW["Г"] = "G";
		theMapW["Д"] = "D";
		theMapW["Е"] = "E";
		theMapW["Ё"] = "E";
		theMapW["Ж"] = "ZH";
		theMapW["З"] = "Z";
		theMapW["И"] = "I";
		theMapW["Й"] = "I";
		theMapW["К"] = "K";
		theMapW["Л"] = "L";
		theMapW["М"] = "M";
		theMapW["Н"] = "N";
		theMapW["О"] = "O";
		theMapW["П"] = "P";
		theMapW["Р"] = "R";
		theMapW["С"] = "S";
		theMapW["Т"] = "T";
		theMapW["У"] = "U";
		theMapW["Ф"] = "F";
		theMapW["Х"] = "KH";
		theMapW["Ц"] = "TS";
		theMapW["Ч"] = "CH";
		theMapW["Ш"] = "SH";
		theMapW["Щ"] = "SCH";
		theMapW["Ъ"] = "`";
		theMapW["Ы"] = "Y";
		theMapW["Ь"] = "'";
		theMapW["Э"] = "E";
		theMapW["Ю"] = "YU";
		theMapW["Я"] = "YA";
	}

	std::map<std::string, const char*> theMapW;
}
static m;

//------------------------------------------------------------------------------

CMStringA cyrillicToLatin(const char *src)
{
   CMStringA ret;

   for (auto *p = src; *p; p++) {
		std::string str(p, 2);

      auto f = m.theMapW.find(str);
		if (f != m.theMapW.end()) {
			ret += f->second;
			p++;
		}
      else ret += *p;
   }

	return ret;
}
