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

Created: Nov 9, 2006

Author:  Artem Shpynov aka FYR:  ashpynov@gmail.com

****************************************************************************

File contains implementation of new version of skining engine based on cpp,
xml load from file etc

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


//////////////////////////////////////////////////////////////////////////
// Implementation of external "interface" procedures
//
extern "C"
{
	//////////////////////////////////////////////////////////////////////////
	//  Initialize subsystem
	//
	void skinpp_LoadSubSystem()
	{

	};

	//////////////////////////////////////////////////////////////////////////
	//  DeInitialize subsystem
	//
	void skinpp_UnloadSubSystem()
	{

	};

};

//////////////////////////////////////////////////////////////////////////
// Implementation of internal procedures
//
skinpp_SKIN::skinpp_SKIN()
{
	m_lpXMLDocument=NULL;
}

skinpp_SKIN::~skinpp_SKIN()
{
	this->ClearSkin();	
}

//////////////////////////////////////////////////////////////////////////
// remove all data from skin	
//
void  skinpp_SKIN::ClearSkin()
{
	if (m_lpXMLDocument!=NULL) 	delete m_lpXMLDocument;
	m_lpXMLDocument=NULL;

	// delete objects from list
	for (OBJLIST_ITER iter = m_ObjectList.begin(); iter!=m_ObjectList.end(); ++iter)
	{
		delete (iter->second);
	}
	m_ObjectList.clear();

	// delete selectors
	for (SELECTORLIST_ITER iter = m_SelectorList.begin(); iter!=m_SelectorList.end(); ++iter)
	{
		delete (*iter);
	}	
	m_SelectorList.clear();
}

//////////////////////////////////////////////////////////////////////////
//	Load skin data from file
//
HRESULT skinpp_SKIN::LoadSkin(const char * szFileName)
{
	HRESULT result=-1;

	ClearSkin();

	m_lpXMLDocument=new XMLDocument();
	result=(HRESULT)m_lpXMLDocument->ParseFromFile(szFileName);

	if (result==S_OK)
	{
		result=GetDataFromXMLDocument();
		if (result==S_OK)	return S_OK;
	}

	// if we are here thats mean error occurred above
	delete m_lpXMLDocument;
	m_lpXMLDocument=NULL;

	return result;
}

//////////////////////////////////////////////////////////////////////////
//	Load skin data from memory
//
HRESULT skinpp_SKIN::LoadSkinFromMemory(const char * szBuffer)
{
	HRESULT result=-1;

	ClearSkin();

	m_lpXMLDocument=new XMLDocument();
	result=(HRESULT)m_lpXMLDocument->ParseFromBuffer(szBuffer);
	
	if (result==S_OK)
	{
		result=GetDataFromXMLDocument();
		if (result==S_OK)	return S_OK;
	}

	// if we are here thats mean error occurred above
	delete m_lpXMLDocument;
	m_lpXMLDocument=NULL;

	return result;
}

//////////////////////////////////////////////////////////////////////////
//	Save skin data to file
//
HRESULT skinpp_SKIN::SaveSkin(const char * szFileName)
{
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//  Convert skin data from xmldocument nodes
//
HRESULT skinpp_SKIN::GetDataFromXMLDocument()
{
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// convert skin data to xmldocument nodes
//
HRESULT skinpp_SKIN::PutDataToXMLDocument()
{
	return S_OK;
}
/////////////////////// END of skinpp_SKIN \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


