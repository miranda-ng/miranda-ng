/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "interfaces\isettings.h"

/// a typical rule item
/// contains info to reply to specific user
typedef struct _RULE_ITEM : ISettingsStream
{
	LPTSTR	RuleName,		/// item name
			ContactName,	/// contact name
			ReplyText;		/// reply text
			/*ReplyAction;	/// reply action*/
}RULE_ITEM, *PRULE_ITEM;

/// common item... - a rule that is 
/// applied to all users.
typedef struct _COMMON_RULE_ITEM : ISettingsStream
{
	//LPTSTR Header;
	LPTSTR Message;
}COMMON_RULE_ITEM, *P_COMMON_RULE_ITEM;

typedef struct _RULE_METAINFO
{
	LPTSTR ContactName;
}RULE_METAINFO, *PRULE_METAINFO;
