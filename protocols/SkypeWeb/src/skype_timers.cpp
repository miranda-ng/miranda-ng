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

#include "stdafx.h"

mir_cs CSkypeProto::timerLock;
mir_cs CSkypeProto::accountsLock;

void CSkypeProto::ProcessTimer()
{
	if (IsOnline())
	{
		PushRequest(new GetContactListRequest(m_szTokenSecret), &CSkypeProto::LoadContactList);
		SendPresence(false);
		if (!m_hTrouterThread)
			SendRequest(new CreateTrouterRequest(), &CSkypeProto::OnCreateTrouter);
	}
}

void CALLBACK CSkypeProto::TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslock lck(accountsLock);
	for (int i = 0; i < Accounts.getCount(); i++)
	{
		Accounts[i]->ProcessTimer();
	}
}

void CSkypeProto::SkypeSetTimer(void*)
{
	mir_cslock lck(timerLock);
	if (!m_timer)
		m_timer = SetTimer(NULL, 0, 600000, TimerProc);
}

void CSkypeProto::SkypeUnsetTimer(void*)
{
	mir_cslock lck(timerLock);
	if (m_timer && Accounts.getCount() == 0)
		KillTimer(NULL, m_timer);
	m_timer = 0;
}
