// Copyright � 2010-2012 SecureIM developers (baloo and others), sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

bool metaIsDefaultSubContact(MCONTACT hContact) 
{
	return db_mc_getDefault(db_mc_getMeta(hContact)) == hContact;
}

MCONTACT metaGetMostOnline(MCONTACT hContact) 
{
	if(db_mc_isMeta(hContact))
		return db_mc_getMostOnline(hContact);
	return NULL;
}
