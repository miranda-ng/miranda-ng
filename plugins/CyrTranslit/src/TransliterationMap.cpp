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
    theMap['à'] = "a";
    theMap['á'] = "b";
    theMap['â'] = "v";
    theMap['ã'] = "g";
    theMap['ä'] = "d";
    theMap['å'] = "e";
    theMap['¸'] = "e";
    theMap['æ'] = "zh";
    theMap['ç'] = "z";
    theMap['è'] = "i";
    theMap['é'] = "i";
    theMap['ê'] = "k";
    theMap['ë'] = "l";
    theMap['ì'] = "m";
    theMap['í'] = "n";
    theMap['î'] = "o";
    theMap['ï'] = "p";
    theMap['ð'] = "r";
    theMap['ñ'] = "s";
    theMap['ò'] = "t";
    theMap['ó'] = "u";
    theMap['ô'] = "f";
    theMap['õ'] = "kh";
    theMap['ö'] = "ts";
    theMap['÷'] = "ch";
    theMap['ø'] = "sh";
    theMap['ù'] = "sch";
    theMap['ú'] = "`";
    theMap['û'] = "y";
    theMap['ü'] = "'";
    theMap['ý'] = "e";
    theMap['þ'] = "yu";
    theMap['ÿ'] = "ya";
    
    theMap['À'] = "A";
    theMap['Á'] = "B";
    theMap['Â'] = "V";
    theMap['Ã'] = "G";
    theMap['Ä'] = "D";
    theMap['Å'] = "E";
    theMap['¨'] = "E";
    theMap['Æ'] = "ZH";
    theMap['Ç'] = "Z";
    theMap['È'] = "I";
    theMap['É'] = "I";
    theMap['Ê'] = "K";
    theMap['Ë'] = "L";
    theMap['Ì'] = "M";
    theMap['Í'] = "N";
    theMap['Î'] = "O";
    theMap['Ï'] = "P";
    theMap['Ð'] = "R";
    theMap['Ñ'] = "S";
    theMap['Ò'] = "T";
    theMap['Ó'] = "U";
    theMap['Ô'] = "F";
    theMap['Õ'] = "KH";
    theMap['Ö'] = "TS";
    theMap['×'] = "CH";
    theMap['Ø'] = "SH";
    theMap['Ù'] = "SCH";
    theMap['Ú'] = "`";
    theMap['Û'] = "Y";
    theMap['Ü'] = "'";
    theMap['Ý'] = "E";
    theMap['Þ'] = "YU";
    theMap['ß'] = "YA";




    theMapW[L'à'] = L"a";
    theMapW[L'á'] = L"b";
    theMapW[L'â'] = L"v";
    theMapW[L'ã'] = L"g";
    theMapW[L'ä'] = L"d";
    theMapW[L'å'] = L"e";
    theMapW[L'¸'] = L"e";
    theMapW[L'æ'] = L"zh";
    theMapW[L'ç'] = L"z";
    theMapW[L'è'] = L"i";
    theMapW[L'é'] = L"i";
    theMapW[L'ê'] = L"k";
    theMapW[L'ë'] = L"l";
    theMapW[L'ì'] = L"m";
    theMapW[L'í'] = L"n";
    theMapW[L'î'] = L"o";
    theMapW[L'ï'] = L"p";
    theMapW[L'ð'] = L"r";
    theMapW[L'ñ'] = L"s";
    theMapW[L'ò'] = L"t";
    theMapW[L'ó'] = L"u";
    theMapW[L'ô'] = L"f";
    theMapW[L'õ'] = L"kh";
    theMapW[L'ö'] = L"ts";
    theMapW[L'÷'] = L"ch";
    theMapW[L'ø'] = L"sh";
    theMapW[L'ù'] = L"sch";
    theMapW[L'ú'] = L"`";
    theMapW[L'û'] = L"y";
    theMapW[L'ü'] = L"'";
    theMapW[L'ý'] = L"e";
    theMapW[L'þ'] = L"yu";
    theMapW[L'ÿ'] = L"ya";
    
    theMapW[L'À'] = L"A";
    theMapW[L'Á'] = L"B";
    theMapW[L'Â'] = L"V";
    theMapW[L'Ã'] = L"G";
    theMapW[L'Ä'] = L"D";
    theMapW[L'Å'] = L"E";
    theMapW[L'¨'] = L"E";
    theMapW[L'Æ'] = L"ZH";
    theMapW[L'Ç'] = L"Z";
    theMapW[L'È'] = L"I";
    theMapW[L'É'] = L"I";
    theMapW[L'Ê'] = L"K";
    theMapW[L'Ë'] = L"L";
    theMapW[L'Ì'] = L"M";
    theMapW[L'Í'] = L"N";
    theMapW[L'Î'] = L"O";
    theMapW[L'Ï'] = L"P";
    theMapW[L'Ð'] = L"R";
    theMapW[L'Ñ'] = L"S";
    theMapW[L'Ò'] = L"T";
    theMapW[L'Ó'] = L"U";
    theMapW[L'Ô'] = L"F";
    theMapW[L'Õ'] = L"KH";
    theMapW[L'Ö'] = L"TS";
    theMapW[L'×'] = L"CH";
    theMapW[L'Ø'] = L"SH";
    theMapW[L'Ù'] = L"SCH";
    theMapW[L'Ú'] = L"`";
    theMapW[L'Û'] = L"Y";
    theMapW[L'Ü'] = L"'";
    theMapW[L'Ý'] = L"E";
    theMapW[L'Þ'] = L"YU";
    theMapW[L'ß'] = L"YA";
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