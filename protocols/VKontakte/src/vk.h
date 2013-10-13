/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

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

#define VK_APP_ID  "3917910"

#define VK_API_URL "api.vk.com"
#define VK_REDIRECT_URL "http://" VK_API_URL "/blank.html"

#define VKPOLL_MSG_REMOVED   0
#define VKPOLL_MSG_NEWFLAGS  1
#define VKPOLL_MSG_ADDFLAGS  2
#define VKPOLL_MSG_DELFLAGS  3
#define VKPOLL_MSG_ADDED     4
#define VKPOLL_USR_ONLINE    8
#define VKPOLL_USR_OFFLINE   9
#define VKPOLL_CHAT_CHANGED 51
#define VKPOLL_USR_UTN      61
#define VKPOLL_CHAT_UTN     62
#define VKPOLL_RING         70

struct HttpParam
{
	LPCSTR szName, szValue;
};

extern HINSTANCE hInst;

LPCSTR findHeader(NETLIBHTTPREQUEST *hdr, LPCSTR szField);

void InitIcons(void);
