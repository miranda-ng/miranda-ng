/*
Copyright (c) 2015 Miranda NG project (https://miranda-ng.org)

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

LIST<CTelegramProto> CTelegramProto::Accounts(1, CTelegramProto::CompareAccounts);
mir_cs CTelegramProto::accountsLock;

int CTelegramProto::CompareAccounts(const CTelegramProto *p1, const CTelegramProto *p2)
{
	return mir_tstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

CTelegramProto* CTelegramProto::InitAccount(const char *protoName, const wchar_t *userName)
{
	mir_cslock lck(accountsLock);
	CTelegramProto *proto = new CTelegramProto(protoName, userName);
	Accounts.insert(proto);
	return proto;
}

int CTelegramProto::UninitAccount(CTelegramProto *proto)
{
	mir_cslock lck(accountsLock);
	Accounts.remove(proto);
	delete proto;
	return 0;
}

CTelegramProto* CTelegramProto::GetContactAccount(MCONTACT hContact)
{
	mir_cslock lck(accountsLock);
	for (int i = 0; i < Accounts.getCount(); i++)
		if (mir_strcmpi(GetContactProto(hContact), Accounts[i]->m_szModuleName) == 0)
			return Accounts[i];
	return NULL;
}

/*INT_PTR CTelegramProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
//	return (INT_PTR)(CTelegramOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}*/