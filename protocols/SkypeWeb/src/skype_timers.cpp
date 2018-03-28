/*
Copyright (c) 2015-18 Miranda NG team (https://miranda-ng.org)

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

mir_cs CSkypeProto::timerLock;
mir_cs CSkypeProto::accountsLock;

void CSkypeProto::ProcessTimer()
{
	if (IsOnline()) {
		PushRequest(new GetContactListRequest(li, nullptr), &CSkypeProto::LoadContactList);
		SendPresence(false);
	}
}

void CALLBACK CSkypeProto::TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslock lck(accountsLock);
	for (auto &it : CMPlugin::g_arInstances)
		it->ProcessTimer();
}

void CSkypeProto::SkypeSetTimer()
{
	mir_cslock lck(timerLock);
	if (!m_timer)
		m_timer = SetTimer(nullptr, 0, 600000, TimerProc);
}

void CSkypeProto::SkypeUnsetTimer()
{
	mir_cslock lck(timerLock);
	if (m_timer && CMPlugin::g_arInstances.getCount() == 0)
		KillTimer(nullptr, m_timer);
	m_timer = 0;
}
