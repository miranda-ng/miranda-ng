#include "stdafx.h"

Account* GetAccountByContact(MCONTACT hContact)
{
	for (auto &it : g_accs)
		if (it->hContact == hContact)
			return it;

	return nullptr;
}
