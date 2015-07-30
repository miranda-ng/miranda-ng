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
