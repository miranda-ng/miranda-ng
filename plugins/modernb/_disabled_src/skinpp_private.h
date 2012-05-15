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

File contains class definitions for skinpp system

\**************************************************************************/

#ifndef skinpp_private_h__
#define skinpp_private_h__

#ifdef UNICODE
#define tstring wstring
#else 
#define tstring string
#endif

using namespace std;
template<class string>
struct OBJLIST_myless : public binary_function <string, string, bool> 
{
	bool operator()(
		const string& _Left, 
		const string& _Right
		) const
	{
		return (bool) (stricmp(_Left.c_str(),_Right.c_str())<0);
	}
};

typedef map<string, class skinpp_SKINOBJECT*, OBJLIST_myless<string> > OBJLIST;
typedef OBJLIST::const_iterator OBJLIST_ITER;

typedef list<class skinpp_SELECTOR* > SELECTORLIST;
typedef SELECTORLIST::const_iterator SELECTORLIST_ITER;



class skinpp_SKIN
{
public:
	//constructor and destructor
	skinpp_SKIN();
	~skinpp_SKIN();
	HRESULT LoadSkin(const char * szFileName);
	HRESULT LoadSkinFromMemory(const char * szBuffer);
	HRESULT SaveSkin(const char * szFileName);

	class skinpp_SKINOBJECT * FindObjectByName(char * szName);
	class skinpp_SKINOBJECT * FindObjectByRule(char * szSelector);
	class skinpp_SKINOBJECT * FindObjectBySelector(class skinpp_SELECTOR * lpSelector);
private:
	HRESULT GetDataFromXMLDocument();		// convert skin data from xmldocument nodes
	HRESULT PutDataToXMLDocument();			// convert skin data to xmldocument nodes
	void ClearSkin();						// remove all data from skin	

	XMLDocument* m_lpXMLDocument;		// XML document loaded from
	OBJLIST		 m_ObjectList;			// map of objects
	SELECTORLIST m_SelectorList;		// list of selectors
	// list of buttons ???
	// map of skinable db keys

	string	m_strSkinName;
	string	m_strSkinAuthor;
	string	m_strSkinAbout;
	string  m_strAuthorMail;
	string  m_strSkinWeb;

};

#include "skinpp_object.h"

class skinpp_SOLID: public skinpp_SKINOBJECT
{
public:
	//constructor and destructor
	skinpp_SOLID();
	~skinpp_SOLID();

	virtual int GetObjectDataFromXMLNode(XMLNode * lpNode);	// gets data from node
	virtual int PutObjectDataToXMLNode(XMLNode * lpNode);	// put data to node

    // data of solid brush object
    BYTE    m_bOpacity;     // Common brush opacity
    DWORD   m_dwColor;      // Color of solid brush
};

class skinpp_GRADIENT: public skinpp_SKINOBJECT
{
public:
	//constructor and destructor
	skinpp_GRADIENT();
	~skinpp_GRADIENT();

	virtual int GetObjectDataFromXMLNode(XMLNode * lpNode);	// gets data from node
	virtual int PutObjectDataToXMLNode(XMLNode * lpNode);	// put data to node

    // data of gradient brush object
    BYTE    m_bStartOpacity;     // start brush opacity
    DWORD   m_dwStartColor;      // start color of gradient brush
    BYTE    m_bEndOpacity;       // end brush opacity
    DWORD   m_dwEndColor;        // end color of gradient brush
    BYTE    m_bDirection;       
};

class skinpp_EMPTY: public skinpp_SKINOBJECT
{
public:
	//constructor and destructor
	skinpp_EMPTY();
	~skinpp_EMPTY();

	virtual int GetObjectDataFromXMLNode(XMLNode * lpNode);	// gets data from node
	virtual int PutObjectDataToXMLNode(XMLNode * lpNode);	// put data to node
};

class skinpp_SELECTOR
{
public:
	//constructor and destructor
};

#endif // skinpp_private_h__