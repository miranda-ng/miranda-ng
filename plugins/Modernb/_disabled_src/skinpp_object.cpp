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

File contains implementation of skinpp_OBJECT class 

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
#include "skinpp_object.h"

skinpp_SKINOBJECT::skinpp_SKINOBJECT()
{
    m_strObjectName="";
}

skinpp_SKINOBJECT::~skinpp_SKINOBJECT()
{

}
int skinpp_SKINOBJECT::GetObjectDataFromXMLNode(XMLNode * lpNode)
{
    return 0;
}

int skinpp_SKINOBJECT::PutObjectDataToXMLNode(XMLNode * lpNode)
{
    return 0;
}

int skinpp_SKINOBJECT::Draw(HDC hDC, int nX, int nY, int nWidth, int nHeight, RECT * rcClipRect)
{
    return 0;
}

int skinpp_SKINOBJECT::ApplyObjectData(void * lpData)     //modify data at lpData
{
    return 0;
}