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
 * Maps the Cyrillic letters to the phonetically equivalent latin character
 * sequences. That's what transliteration is.
 */
class TransliterationMap
{
public:
    /**
     * Gives an access to the singleton object of this class.
     *
     * @return One and only instance of TransliterationMap class.
     */
    static const TransliterationMap& getInstance();
    
    /**
     * Performs the transliteration from Cyrillic to Latin.
     *
     * @param src The source string that may contain Cyrillic characters.
     *
     * @return The transliterated source string in which all the occurences of
     * Cyrillic letters are replaced by the phonetically equivalent Latin
     * sequences (1-3 letters for a single Cyrillic leter).
     */
    std::string cyrillicToLatin(const std::string &src) const;
    std::wstring cyrillicToLatin(const std::wstring &src) const;

private:
    class Guard
    {
    public:
        ~Guard();
    };

    TransliterationMap();
    std::string cyrillicToLatin(char c) const;
    std::wstring cyrillicToLatin(wchar_t c) const;
    
    static const TransliterationMap *pInstance;
    static Guard guard;
    
    typedef std::map<char, std::string> CharMap;
    typedef std::map<wchar_t, std::wstring> WCharMap;
    CharMap theMap;
    WCharMap theMapW;
};

}