/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_MENUS_H_
#define _SKYPE_MENUS_H_

#define CMI_POSITION -201001000

enum
{
	CMI_AUTH_REQUEST,
	CMI_AUTH_GRANT,
	CMI_GETSERVERHISTORY,
	CMI_BLOCK,
	CMI_UNBLOCK,
	CMI_MAX // this item shall be the last one
};

enum ProtoMenuIndexes {
	SMI_CREATECHAT
};

#define SMI_POSITION 200000

#endif //_SKYPE_MENUS_H_