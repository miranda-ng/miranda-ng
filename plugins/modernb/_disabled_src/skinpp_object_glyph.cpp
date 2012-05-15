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

File contains implementation of skinpp_GLYPH class 

\**************************************************************************/
#include <windows.h>
#include <win2k.h>
#include "newpluginapi.h"	//this is common header for miranda plugin api
#include "m_system.h"
#include "m_database.h"

#include <string>
#include <map>
#include <list>

#include "XMLParser/XMLParser.h"

#include "skinpp.h"
#include "skinpp_private.h"
#include "skinpp_object_glyph.h"

skinpp_GLYPH::skinpp_GLYPH()
{
    m_bOpacity=0;
    m_nX = m_nY = m_nWidth =  m_nHeigh = 0;
    m_hBitmap = NULL;
    m_lpImageFileName = NULL;
    m_nLeft = m_nTop = m_nRight = m_nBottom = 0; 
    m_bFillMode = FM_STRETCH_BOTH;
};

skinpp_GLYPH::~skinpp_GLYPH()
{
    //if (m_hBitmap) skinpp_FreeBitmap(m_hBitmap);
    if (m_lpImageFileName) free(m_lpImageFileName);
}

int skinpp_GLYPH::GetObjectDataFromXMLNode(XMLNode * lpNode)
{ 
    // Early return if current object is empty (should not occur)
    if (!this) return E_FAIL; 
    
    // call parent class to get common object data
    if (FAILED(skinpp_SKINOBJECT::GetObjectDataFromXMLNode(lpNode))) return E_FAIL;
    
    // get opacity, default is '255'
    m_bOpacity=(BYTE)lpNode->GetAttributeInt("opacity",255);
   
    // get image section
    XMLNode * lpImageNode=lpNode->GetNode("image");
    if (lpImageNode)
    {
        // file - get image filename - if empty - use internal image 
        string file=lpImageNode->GetAttributeString("file");
        if (m_lpImageFileName) free(m_lpImageFileName);
        m_lpImageFileName=NULL;
        if (strlen(file.c_str())) strdup(file.c_str());
        // if old bitmap is exists - free it
        //if (m_hBitmap) skinpp_FreeBitmap(m_hBitmap);
        m_hBitmap=NULL;
  
        //x, y
        m_nX=lpImageNode->GetAttributeInt("x");
        m_nY=lpImageNode->GetAttributeInt("y");
        
        //width, height
        m_nWidth=lpImageNode->GetAttributeInt("width");
        m_nHeigh=lpImageNode->GetAttributeInt("height");
    }

    // get resize section
    XMLNode * lpResizeNode=lpNode->GetNode("resize");
    if (lpResizeNode)
    {
        // left, top, right, bottom
        m_nLeft=lpResizeNode->GetAttributeInt("left");
        m_nTop=lpResizeNode->GetAttributeInt("top");
        m_nRight=lpResizeNode->GetAttributeInt("right");
        m_nBottom=lpResizeNode->GetAttributeInt("bottom");

        // mode default is 'stretch'
        string mode=lpResizeNode->GetAttributeString("mode","stretch");        
        char * szMode=(char*)mode.c_str();
        if (strcmpi(szMode,"vtile"))        m_bFillMode=FM_TILE_VERTICAL;
        else if (strcmpi(szMode,"htile"))   m_bFillMode=FM_TILE_HORIZONTAL;
        else if (strcmpi(szMode,"tile"))    m_bFillMode=FM_TILE_BOTH;
        else                                m_bFillMode=FM_STRETCH_BOTH;
    }
    return S_OK;
}

int skinpp_GLYPH::PutObjectDataToXMLNode(XMLNode * lpNode)
{
    return 0;
}

int skinpp_GLYPH::Draw(HDC hDC, int nX, int nY, int nWidth, int nHeight, RECT * rcClipRect)
{
    return 0;
}