/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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

#define VK_APP_ID  3917910

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

#define VKFLAG_MSGUNREAD     1 // ��������� �� ���������
#define VKFLAG_MSGOUTBOX     2 // ��������� ���������
#define VKFLAG_MSGREPLIED	  4 // �� ��������� ��� ������ �����
#define VKFLAG_MSGIMPORTANT  8 // ���������� ���������
#define VKFLAG_MSGCHAT      16 // ��������� ���������� ����� ���
#define VKFLAG_MSGFRIENDS   32 // ��������� ���������� ������
#define VKFLAG_MSGSPAM      64 // ��������� �������� ��� "����"
#define VKFLAG_MSGDEL�T�D  128 // ��������� ������� (� �������)
#define VKFLAG_MSGFIXED    256 // ��������� ��������� ������������� �� ����
#define VKFLAG_MSGMEDIA    512 // ��������� �������� ������������

#define VK_API_VER "5.24"
#define VER_API CHAR_PARAM("v", VK_API_VER)


#if defined(_DEBUG)
	#define VK_NODUMPHEADERS 0
#else
	#define VK_NODUMPHEADERS NLHRF_NODUMPHEADERS
#endif

struct CVkProto;
extern LIST<CVkProto> vk_Instances;
extern HINSTANCE hInst;

LPCSTR findHeader(NETLIBHTTPREQUEST *hdr, LPCSTR szField);

void InitIcons(void);
HANDLE GetIconHandle(int iCommand);

void MyHtmlDecode(CMStringW &str);

