/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aLONG with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "version.h"

#ifndef __ICO_VERSION_H__
#define __ICO_VERSION_H__ 1

#define __ICO_MAJOR_VERSION   1
#define __ICO_MINOR_VERSION   2
#define __ICO_RELEASE_NUM     0
#define __ICO_BUILD_NUM       0

#define __ICO_FILEVERSION_STRING		__ICO_MAJOR_VERSION,__ICO_MINOR_VERSION,__ICO_RELEASE_NUM,__ICO_BUILD_NUM
#define __ICO_FILEVERSION_STRING_DOTS	__ICO_MAJOR_VERSION.__ICO_MINOR_VERSION.__ICO_RELEASE_NUM.__ICO_BUILD_NUM

#define __ICO_VERSION_STRING	__STRINGIFY( __FILEVERSION_STRING_DOTS )

#define __ICO_SHORT_DESC	"This is an Iconpack for the UserInfoEx plugin"
#define __ICO_AUTHOR		__AUTHOR
#define __ICO_AUTHOREMAIL	__AUTHOREMAIL
#define __ICO_COPYRIGHT		__COPYRIGHT
#define __ICO_AUTHORWEB     __AUTHORWEB

#endif /* __ICO_VERSION_H__ */
