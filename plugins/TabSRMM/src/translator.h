/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
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
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// string handling

#ifndef __STRINGS_H
#define __STRINGS_H

class CTranslator {

public:

	/*
	 * identities for the option trees
	 */

	enum {
		TREE_MODPLUS = 0,
		TREE_NEN = 1,
		TREE_MSG = 2,
		TREE_LOG = 3,
		TREE_TAB = 4,
	};

	CTranslator();
	~CTranslator();

	static TOptionListItem* 	getTree(UINT id);
	static TOptionListGroup* 	getGroupTree(UINT id);
};

#endif
