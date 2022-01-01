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

#include "stdafx.h"

INT_PTR __cdecl CVkProto::SvcWallPost(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcWallPost");

	WALLPOST_FORM_PARAMS param(db_get_wsa(hContact, m_szModuleName, "Nick"));
	CVkWallPostForm dlg(this, &param);
	if (!dlg.DoModal())
		return 1;

	WallPost((MCONTACT)hContact, param.pwszMsg, param.pwszUrl, param.bFriendsOnly);
	return 0;
}

void CVkProto::WallPost(MCONTACT hContact, wchar_t *pwszMsg, wchar_t *pwszUrl, bool bFriendsOnly)
{
	debugLogA("CVkProto::WallPost");
	if (!IsOnline() || (IsEmpty(pwszMsg) && IsEmpty(pwszUrl)))
		return;

	LONG userID = hContact ? m_myUserId : getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return;

	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/wall.post.json", true, &CVkProto::OnReceiveSmth);
	pReq << INT_PARAM("owner_id", userID) << INT_PARAM("friends_only", bFriendsOnly ? 1 : 0);

	if (!IsEmpty(pwszMsg))
		pReq << WCHAR_PARAM("message", pwszMsg);

	if (!IsEmpty(pwszUrl))
		pReq << WCHAR_PARAM("attachments", pwszUrl);

	Push(pReq);
}