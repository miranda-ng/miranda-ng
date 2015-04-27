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
const TransliterationMap *TransliterationMap::pInstance = 0;

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
    theMap['�'] = "a";
    theMap['�'] = "b";
    theMap['�'] = "v";
    theMap['�'] = "g";
    theMap['�'] = "d";
    theMap['�'] = "e";
    theMap['�'] = "e";
    theMap['�'] = "zh";
    theMap['�'] = "z";
    theMap['�'] = "i";
    theMap['�'] = "i";
    theMap['�'] = "k";
    theMap['�'] = "l";
    theMap['�'] = "m";
    theMap['�'] = "n";
    theMap['�'] = "o";
    theMap['�'] = "p";
    theMap['�'] = "r";
    theMap['�'] = "s";
    theMap['�'] = "t";
    theMap['�'] = "u";
    theMap['�'] = "f";
    theMap['�'] = "kh";
    theMap['�'] = "ts";
    theMap['�'] = "ch";
    theMap['�'] = "sh";
    theMap['�'] = "sch";
    theMap['�'] = "`";
    theMap['�'] = "y";
    theMap['�'] = "'";
    theMap['�'] = "e";
    theMap['�'] = "yu";
    theMap['�'] = "ya";
    
    theMap['�'] = "A";
    theMap['�'] = "B";
    theMap['�'] = "V";
    theMap['�'] = "G";
    theMap['�'] = "D";
    theMap['�'] = "E";
    theMap['�'] = "E";
    theMap['�'] = "ZH";
    theMap['�'] = "Z";
    theMap['�'] = "I";
    theMap['�'] = "I";
    theMap['�'] = "K";
    theMap['�'] = "L";
    theMap['�'] = "M";
    theMap['�'] = "N";
    theMap['�'] = "O";
    theMap['�'] = "P";
    theMap['�'] = "R";
    theMap['�'] = "S";
    theMap['�'] = "T";
    theMap['�'] = "U";
    theMap['�'] = "F";
    theMap['�'] = "KH";
    theMap['�'] = "TS";
    theMap['�'] = "CH";
    theMap['�'] = "SH";
    theMap['�'] = "SCH";
    theMap['�'] = "`";
    theMap['�'] = "Y";
    theMap['�'] = "'";
    theMap['�'] = "E";
    theMap['�'] = "YU";
    theMap['�'] = "YA";




    theMapW[L'�'] = L"a";
    theMapW[L'�'] = L"b";
    theMapW[L'�'] = L"v";
    theMapW[L'�'] = L"g";
    theMapW[L'�'] = L"d";
    theMapW[L'�'] = L"e";
    theMapW[L'�'] = L"e";
    theMapW[L'�'] = L"zh";
    theMapW[L'�'] = L"z";
    theMapW[L'�'] = L"i";
    theMapW[L'�'] = L"i";
    theMapW[L'�'] = L"k";
    theMapW[L'�'] = L"l";
    theMapW[L'�'] = L"m";
    theMapW[L'�'] = L"n";
    theMapW[L'�'] = L"o";
    theMapW[L'�'] = L"p";
    theMapW[L'�'] = L"r";
    theMapW[L'�'] = L"s";
    theMapW[L'�'] = L"t";
    theMapW[L'�'] = L"u";
    theMapW[L'�'] = L"f";
    theMapW[L'�'] = L"kh";
    theMapW[L'�'] = L"ts";
    theMapW[L'�'] = L"ch";
    theMapW[L'�'] = L"sh";
    theMapW[L'�'] = L"sch";
    theMapW[L'�'] = L"`";
    theMapW[L'�'] = L"y";
    theMapW[L'�'] = L"'";
    theMapW[L'�'] = L"e";
    theMapW[L'�'] = L"yu";
    theMapW[L'�'] = L"ya";
    
    theMapW[L'�'] = L"A";
    theMapW[L'�'] = L"B";
    theMapW[L'�'] = L"V";
    theMapW[L'�'] = L"G";
    theMapW[L'�'] = L"D";
    theMapW[L'�'] = L"E";
    theMapW[L'�'] = L"E";
    theMapW[L'�'] = L"ZH";
    theMapW[L'�'] = L"Z";
    theMapW[L'�'] = L"I";
    theMapW[L'�'] = L"I";
    theMapW[L'�'] = L"K";
    theMapW[L'�'] = L"L";
    theMapW[L'�'] = L"M";
    theMapW[L'�'] = L"N";
    theMapW[L'�'] = L"O";
    theMapW[L'�'] = L"P";
    theMapW[L'�'] = L"R";
    theMapW[L'�'] = L"S";
    theMapW[L'�'] = L"T";
    theMapW[L'�'] = L"U";
    theMapW[L'�'] = L"F";
    theMapW[L'�'] = L"KH";
    theMapW[L'�'] = L"TS";
    theMapW[L'�'] = L"CH";
    theMapW[L'�'] = L"SH";
    theMapW[L'�'] = L"SCH";
    theMapW[L'�'] = L"`";
    theMapW[L'�'] = L"Y";
    theMapW[L'�'] = L"'";
    theMapW[L'�'] = L"E";
    theMapW[L'�'] = L"YU";
    theMapW[L'�'] = L"YA";
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