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

TransliterationMap::Guard TransliterationMap::guard;
const TransliterationMap *TransliterationMap::pInstance = nullptr;

//------------------------------------------------------------------------------

TransliterationMap::Guard::~Guard()
{
    delete TransliterationMap::pInstance;
}

//------------------------------------------------------------------------------

const TransliterationMap& TransliterationMap::getInstance()
{
    if (!pInstance)
    {
        pInstance = new TransliterationMap();
    }
    return *pInstance;
}

//------------------------------------------------------------------------------

TransliterationMap::TransliterationMap()
{
    theMap['а'] = "a";
    theMap['б'] = "b";
    theMap['в'] = "v";
    theMap['г'] = "g";
    theMap['д'] = "d";
    theMap['е'] = "e";
    theMap['ё'] = "e";
    theMap['ж'] = "zh";
    theMap['з'] = "z";
    theMap['и'] = "i";
    theMap['й'] = "i";
    theMap['к'] = "k";
    theMap['л'] = "l";
    theMap['м'] = "m";
    theMap['н'] = "n";
    theMap['о'] = "o";
    theMap['п'] = "p";
    theMap['р'] = "r";
    theMap['с'] = "s";
    theMap['т'] = "t";
    theMap['у'] = "u";
    theMap['ф'] = "f";
    theMap['х'] = "kh";
    theMap['ц'] = "ts";
    theMap['ч'] = "ch";
    theMap['ш'] = "sh";
    theMap['щ'] = "sch";
    theMap['ъ'] = "`";
    theMap['ы'] = "y";
    theMap['ь'] = "'";
    theMap['э'] = "e";
    theMap['ю'] = "yu";
    theMap['я'] = "ya";
    
    theMap['А'] = "A";
    theMap['Б'] = "B";
    theMap['В'] = "V";
    theMap['Г'] = "G";
    theMap['Д'] = "D";
    theMap['Е'] = "E";
    theMap['Ё'] = "E";
    theMap['Ж'] = "ZH";
    theMap['З'] = "Z";
    theMap['И'] = "I";
    theMap['Й'] = "I";
    theMap['К'] = "K";
    theMap['Л'] = "L";
    theMap['М'] = "M";
    theMap['Н'] = "N";
    theMap['О'] = "O";
    theMap['П'] = "P";
    theMap['Р'] = "R";
    theMap['С'] = "S";
    theMap['Т'] = "T";
    theMap['У'] = "U";
    theMap['Ф'] = "F";
    theMap['Х'] = "KH";
    theMap['Ц'] = "TS";
    theMap['Ч'] = "CH";
    theMap['Ш'] = "SH";
    theMap['Щ'] = "SCH";
    theMap['Ъ'] = "`";
    theMap['Ы'] = "Y";
    theMap['Ь'] = "'";
    theMap['Э'] = "E";
    theMap['Ю'] = "YU";
    theMap['Я'] = "YA";




    theMapW[L'а'] = L"a";
    theMapW[L'б'] = L"b";
    theMapW[L'в'] = L"v";
    theMapW[L'г'] = L"g";
    theMapW[L'д'] = L"d";
    theMapW[L'е'] = L"e";
    theMapW[L'ё'] = L"e";
    theMapW[L'ж'] = L"zh";
    theMapW[L'з'] = L"z";
    theMapW[L'и'] = L"i";
    theMapW[L'й'] = L"i";
    theMapW[L'к'] = L"k";
    theMapW[L'л'] = L"l";
    theMapW[L'м'] = L"m";
    theMapW[L'н'] = L"n";
    theMapW[L'о'] = L"o";
    theMapW[L'п'] = L"p";
    theMapW[L'р'] = L"r";
    theMapW[L'с'] = L"s";
    theMapW[L'т'] = L"t";
    theMapW[L'у'] = L"u";
    theMapW[L'ф'] = L"f";
    theMapW[L'х'] = L"kh";
    theMapW[L'ц'] = L"ts";
    theMapW[L'ч'] = L"ch";
    theMapW[L'ш'] = L"sh";
    theMapW[L'щ'] = L"sch";
    theMapW[L'ъ'] = L"`";
    theMapW[L'ы'] = L"y";
    theMapW[L'ь'] = L"'";
    theMapW[L'э'] = L"e";
    theMapW[L'ю'] = L"yu";
    theMapW[L'я'] = L"ya";
    
    theMapW[L'А'] = L"A";
    theMapW[L'Б'] = L"B";
    theMapW[L'В'] = L"V";
    theMapW[L'Г'] = L"G";
    theMapW[L'Д'] = L"D";
    theMapW[L'Е'] = L"E";
    theMapW[L'Ё'] = L"E";
    theMapW[L'Ж'] = L"ZH";
    theMapW[L'З'] = L"Z";
    theMapW[L'И'] = L"I";
    theMapW[L'Й'] = L"I";
    theMapW[L'К'] = L"K";
    theMapW[L'Л'] = L"L";
    theMapW[L'М'] = L"M";
    theMapW[L'Н'] = L"N";
    theMapW[L'О'] = L"O";
    theMapW[L'П'] = L"P";
    theMapW[L'Р'] = L"R";
    theMapW[L'С'] = L"S";
    theMapW[L'Т'] = L"T";
    theMapW[L'У'] = L"U";
    theMapW[L'Ф'] = L"F";
    theMapW[L'Х'] = L"KH";
    theMapW[L'Ц'] = L"TS";
    theMapW[L'Ч'] = L"CH";
    theMapW[L'Ш'] = L"SH";
    theMapW[L'Щ'] = L"SCH";
    theMapW[L'Ъ'] = L"`";
    theMapW[L'Ы'] = L"Y";
    theMapW[L'Ь'] = L"'";
    theMapW[L'Э'] = L"E";
    theMapW[L'Ю'] = L"YU";
    theMapW[L'Я'] = L"YA";
}

//------------------------------------------------------------------------------

std::string TransliterationMap::cyrillicToLatin(char c) const
{
    std::string ret(1, c);

    CharMap::const_iterator it = theMap.find(c);
    if (it != theMap.end())
    {
        ret = (*it).second;
    }
    
    return ret;
}
//------------------------------------------------------------------------------

std::wstring TransliterationMap::cyrillicToLatin(wchar_t c) const
{
    std::wstring ret(1, c);

    WCharMap::const_iterator it = theMapW.find(c);
    if (it != theMapW.end())
    {
        ret = (*it).second;
    }
    
    return ret;
}

//------------------------------------------------------------------------------

std::string TransliterationMap::cyrillicToLatin(const std::string &src) const
{
    std::string ret;
    
    for(std::string::const_iterator i = src.begin(); i != src.end(); ++i)
    {
        ret += cyrillicToLatin(*i);
    }
    
    return ret;
}
//------------------------------------------------------------------------------

std::wstring TransliterationMap::cyrillicToLatin(const std::wstring &src) const
{
    std::wstring ret;
    
    for(std::wstring::const_iterator i = src.begin(); i != src.end(); ++i)
    {
        ret += cyrillicToLatin(*i);
    }
    
    return ret;
}

}