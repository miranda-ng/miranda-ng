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

File contains extern "C" definition for skinppsystem

\**************************************************************************/

#ifndef skinpp_h__
#define skinpp_h__

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

	void skinpp_LoadSubSystem();
	void skinpp_UnloadSubSystem();
	void * skinpp_CreateSkin();		
	void skinpp_LoadSkinFromFile(void * lpSkin, char * szFileName);
	void skinpp_LoadSkinFromMemory(void * lpSkin, char * szBuff);

#ifdef __cplusplus
};
#endif //__cplusplus

#endif // skinpp_h__

