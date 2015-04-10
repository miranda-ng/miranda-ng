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

#include "common.h"

LIST<CSkypeProto> CSkypeProto::Accounts(1, CSkypeProto::CompareAccounts);

int CSkypeProto::CompareAccounts(const CSkypeProto *p1, const CSkypeProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

CSkypeProto* CSkypeProto::InitAccount(const char *protoName, const wchar_t *userName)
{
	CSkypeProto *proto = new CSkypeProto(protoName, userName);
	Accounts.insert(proto);
	return proto;
}

int CSkypeProto::UninitAccount(CSkypeProto *proto)
{
	Accounts.remove(proto);
	delete proto;
	return 0;
}

CSkypeProto* CSkypeProto::GetContactAccount(MCONTACT hContact)
{
	for (int i = 0; i < Accounts.getCount(); i++)
		if (mir_strcmpi(GetContactProto(hContact), Accounts[i]->m_szModuleName) == 0)
			return Accounts[i];
	return NULL;
}

int CSkypeProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CSkypeProto::OnPreCreateMessage);

	return 0;
}

INT_PTR CSkypeProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)(CSkypeOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}