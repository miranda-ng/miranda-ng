/*
Plugin for Miranda NG for communicating with users of the MSN Messenger protocol.
Copyright (c) 2013 Miranda NG Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

void CMsnProto::AvatarQueue_Init()
{
	::InitializeCriticalSection(&csAvatarQueue);
}

void CMsnProto::AvatarQueue_Uninit()
{
	::DeleteCriticalSection(&csAvatarQueue);
}

void CMsnProto::pushAvatarRequest(HANDLE hContact, LPCSTR pszUrl)
{
	SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);

	if (pszUrl != NULL && *pszUrl != 0) {
		mir_cslock lck(csAvatarQueue);

		for (int i=0; i < lsAvatarQueue.getCount(); i++)
			if (lsAvatarQueue[i].hContact == hContact)
				return;

		lsAvatarQueue.insert(new AvatarQueueEntry(hContact, pszUrl));
	}
}
