/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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

#pragma once

#define VK_APP_ID 3917910

// LongPool servers events
#define VKPOLL_MSG_REMOVED	0
#define VKPOLL_MSG_NEWFLAGS	1
#define VKPOLL_MSG_ADDFLAGS	2
#define VKPOLL_MSG_DELFLAGS	3
#define VKPOLL_MSG_ADDED	4
#define VKPOLL_MSG_EDITED	5
#define VKPOLL_READ_ALL_IN	6
#define VKPOLL_READ_ALL_OUT	7
#define VKPOLL_USR_ONLINE	8
#define VKPOLL_USR_OFFLINE	9
#define VKPOLL_CHAT_CHANGED	51
#define VKPOLL_USR_UTN		61
#define VKPOLL_CHAT_UTN		62
#define VKPOLL_RING			70

// Flags for VKPOLL_MSG_ADDED
#define VKFLAG_MSGUNREAD	1	// сообщение не прочитано
#define VKFLAG_MSGOUTBOX	2	// исходящее сообщение
#define VKFLAG_MSGREPLIED	4	// на сообщение был создан ответ
#define VKFLAG_MSGIMPORTANT	8	// помеченное сообщение
#define VKFLAG_MSGCHAT		16	// сообщение отправлено через чат
#define VKFLAG_MSGFRIENDS	32	// сообщение отправлено другом
#define VKFLAG_MSGSPAM		64	// сообщение помечено как "Спам"
#define VKFLAG_MSGDELETED	128	// сообщение удалено (в корзине)
#define VKFLAG_MSGFIXED		256	// сообщение проверено пользователем на спам
#define VKFLAG_MSGMEDIA		512	// сообщение содержит медиаконтент

// Errors
#define VKERR_NO_JSONNODE						-2	// No JSON Node in server reply
#define VKERR_OFFLINE							-1	// Proto is offline
#define VKERR_NOERRORS							0	// No error
#define VKERR_UNKNOWN							1	// Unknown error occurred
#define VKERR_TOO_MANY_REQ_PER_SEC				6	// Too many requests per second
#define VKERR_AUTHORIZATION_FAILED				5	// User authorization failed
#define VKERR_FLOOD_CONTROL						9	// Flood control
#define VKERR_INTERNAL_SERVER_ERR				10	// Internal server error
#define VKERR_CAPTCHA_NEEDED					14	// Captcha needed
#define VKERR_ACCESS_DENIED						15	// Access denied
#define VKERR_VALIDATION_REQUIRED				17	// Validation Required
#define VKERR_COULD_NOT_SAVE_FILE				105	// Couldn't save file
#define VKERR_INVALID_ALBUM_ID					114	// Invalid album id
#define VKERR_INVALID_SERVER					118	// Invalid server
#define VKERR_INVALID_PARAMETERS				100	// One of the parameters specified was missing or invalid
#define VKERR_INVALID_HASH						121	// Invalid hash
#define VKERR_INVALID_AUDIO						123	// Invalid audio
#define VKERR_HIMSELF_AS_FRIEND					174	// Cannot add user himself as friend
#define VKERR_YOU_ON_BLACKLIST					175	// Cannot add this user to friends as they have put you on their blacklist
#define VKERR_USER_ON_BLACKLIST					176	// Cannot add this user to friends as you put him on blacklist
#define VKERR_ACC_WALL_POST_DENIED				214	// Access to adding post denied
#define VKERR_AUDIO_DEL_COPYRIGHT				270	// The audio file was removed by the copyright holder and cannot be reuploaded.
#define VKERR_INVALID_FILENAME					301	// Invalid filename
#define VKERR_INVALID_FILESIZE					302	// Invalid filesize
#define VKERR_CANT_SEND_USER_ON_BLACKLIST		900	// Can't send messages for users from blacklist
#define VKERR_CANT_SEND_USER_WITHOUT_DIALOGS	901	// Can't send messages for users without dialogs
#define VKERR_CANT_SEND_YOU_ON_BLACKLIST		902	// Can't send messages to this user due to their privacy settings
#define VKERR_MESSAGE_IS_TOO_LONG				914	// Message is too long

// File upload custom error
#define VKERR_FILE_NOT_EXIST					10100	// File does not exist
#define VKERR_FTYPE_NOT_SUPPORTED				10101	// File type not supported
#define VKERR_ERR_OPEN_FILE						10103	// Error open file
#define VKERR_ERR_READ_FILE						10104	// Error read file
#define VKERR_FILE_NOT_UPLOADED					10105	// File upload error
#define VKERR_INVALID_URL						10106	// Upload server returned empty URL
#define VKERR_INVALID_USER						10107	// Invalid or unknown recipient user ID

#define VK_USER_DEACTIVATE_ACTION	9321

#define VK_API_VER "5.131"
#define VER_API CHAR_PARAM("v", VK_API_VER)

#define VK_FEED_USER 2147483647L
#define VK_INVALID_USER 0L
#define VK_CHAT_FLAG 2000000000

#if defined(_DEBUG)
	#define VK_NODUMPHEADERS 0
#else
	#define VK_NODUMPHEADERS NLHRF_NODUMPHEADERS
#endif

struct CVkProto;
extern mir_cs csInstances;
extern bool g_bMessageState;

bool wlstrstr(wchar_t *_s1, wchar_t *_s2);

void InitIcons(void);

char* ExpUrlEncode(const char *szUrl, bool strict = false);

bool IsEmpty(LPCWSTR str);
bool IsEmpty(LPCSTR str);
