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

File contains implementation of skinpp_OBJECT class HEADER

\**************************************************************************/

#ifndef skinpp_object_h__
#define skinpp_object_h__

class skinpp_SKINOBJECT
{
public:
    //constructor and destructor
    skinpp_SKINOBJECT();
    ~skinpp_SKINOBJECT();

    virtual int GetObjectDataFromXMLNode(XMLNode * lpNode);	// gets data from node
    virtual int PutObjectDataToXMLNode(XMLNode * lpNode);	// put data to node
    
    virtual int Draw(HDC hDC, int nX, int nY, int nWidth, int nHeight, RECT * rcClipRect=NULL);  //paint itself
    int ApplyObjectData(void * lpData);     //modify data at lpData

    string m_strObjectName;
};


#endif // skinpp_object_h__
