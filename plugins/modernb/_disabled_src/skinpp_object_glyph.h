/**************************************************************************\

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

****************************************************************************

Created: Nov 27, 2006

Author:  Artem Shpynov aka FYR:  ashpynov@gmail.com

****************************************************************************

File contains implementation of skinpp_GLYPH class HEADER

\**************************************************************************/

#ifndef skinpp_object_glyph_h__
#define skinpp_object_glyph_h__


#include "skinpp_object.h"

enum // Resizer fill mode Enumerations
{
    FM_STRETCH_BOTH = 0,    
    FM_TILE_HORIZONTAL,
    FM_TILE_VERTICAL,
    FM_TILE_BOTH
};

class skinpp_GLYPH: public skinpp_SKINOBJECT
{
public:
    //constructor and destructor
    skinpp_GLYPH();
    ~skinpp_GLYPH();

    virtual int GetObjectDataFromXMLNode(XMLNode * lpNode);	// gets data from node
    virtual int PutObjectDataToXMLNode(XMLNode * lpNode);	// put data to node
    
    virtual int Draw(HDC hDC, int nX, int nY, int nWidth, int nHeight, RECT * rcClipRect=NULL);

    // data of glyph object
    BYTE    m_bOpacity;     // Common glyph opacity

    //Glyph image fragment
    int     m_nX,
            m_nY,
            m_nWidth,
            m_nHeigh;           // Position of fragment   
    HBITMAP m_hBitmap;          // Glyph image
    char   *m_lpImageFileName;  //TODO: UNICODE file names

    //Resizer
    int     m_nLeft,
            m_nTop,
            m_nRight,
            m_nBottom;      // Resiser   
    BYTE    m_bFillMode;    // one of resizer fill mode value
};


#endif // skinpp_object_glyph_h__