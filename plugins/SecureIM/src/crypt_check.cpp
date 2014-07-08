#include "commonheaders.h"

int getContactStatus(MCONTACT hContact)
{
	pSupPro ptr = getSupPro(hContact);
	if (ptr)
		return db_get_w(hContact, ptr->name, "Status", ID_STATUS_OFFLINE);

	return -1;
}

bool isSecureProtocol(MCONTACT hContact)
{
	pSupPro ptr = getSupPro(hContact);
	if (!ptr)
		return false;

	return ptr->inspecting != 0;
}

BYTE isContactSecured(MCONTACT hContact)
{
	// нужна проверка на Offline и в этом случае другие статусы
	if (!arClist.getCount()) return 0;

	if (db_mc_isMeta(hContact))
		hContact = db_mc_getMostOnline(hContact); // возьмем тот, через который пойдет сообщение

	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting)
		return false;

	BYTE res = p->mode;
	DBVARIANT dbv;
	switch (p->mode) {
	case MODE_NATIVE:
		if (cpp_keyx(p->cntx) != 0) res |= SECURED;
		break;

	case MODE_PGP:
		if (!db_get_s(hContact, MODULENAME, "pgp", &dbv)) {
			res |= SECURED;
			db_free(&dbv);
		}
		break;

	case MODE_GPG:
		if (!db_get_s(hContact, MODULENAME, "gpg", &dbv)) {
			res |= SECURED;
			db_free(&dbv);
		}
		break;

	case MODE_RSAAES:
		if (exp->rsa_get_state(p->cntx) == 7) res |= SECURED;
		break;

	case MODE_RSA:
		if (p->cntx != 0) res |= SECURED;
		break;
	}
	return res;
}

bool isClientMiranda(pUinKey ptr, BOOL emptyMirverAsMiranda)
{
	if (!bMCD) return true;
	if (!ptr->proto->inspecting) return false;

	bool isMiranda = true;
	LPSTR mirver = db_get_sa(ptr->hContact, ptr->proto->name, "MirVer");
	if (mirver) {
		isMiranda = (emptyMirverAsMiranda && !*mirver) || (strstr(mirver, "Miranda") != NULL);
		mir_free(mirver);
	}
	return isMiranda;
}

bool isClientMiranda(MCONTACT hContact, BOOL emptyMirverAsMiranda)
{
	if (!bMCD) return true;
	if (!arClist.getCount()) return false;

	pUinKey p = findUinKey(hContact);
	return (p) ? isClientMiranda(p, emptyMirverAsMiranda) : false;
}

bool isProtoSmallPackets(MCONTACT hContact)
{
	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting)
		return false;

	return strstr(p->proto->name, "IRC") != NULL || strstr(p->proto->name, "WinPopup") != NULL || strstr(p->proto->name, "VyChat") != NULL;
}

bool isContactInvisible(MCONTACT hContact)
{
	if (db_get_b(hContact, "CList", "Hidden", 0))
		return true;

	pUinKey p = findUinKey(hContact);
	if (!p || p->waitForExchange || !p->proto || !p->proto->inspecting)
		return false;

	switch (db_get_w(hContact, p->proto->name, "ApparentMode", 0)) {
	case 0:
		return CallProtoService(p->proto->name, PS_GETSTATUS, 0, 0) == ID_STATUS_INVISIBLE;
	case ID_STATUS_ONLINE:
		return false;
	case ID_STATUS_OFFLINE:
		return true;
	}
	return false;
}

bool isNotOnList(MCONTACT hContact)
{
	return db_get_b(hContact, "CList", "NotOnList", 0) != 0;
}

bool isContactNewPG(MCONTACT hContact)
{
	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting || !p->cntx)
		return false;

	return (p->features & CPP_FEATURES_NEWPG) != 0;
}

bool isContactPGP(MCONTACT hContact)
{
	if (!bPGPloaded || (!bPGPkeyrings && !bPGPprivkey)) return false;

	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting || p->mode != MODE_PGP)
		return false;

	DBVARIANT dbv;
	if (db_get_s(hContact, MODULENAME, "pgp", &dbv)) return false;
	db_free(&dbv);
	return true;
}

bool isContactGPG(MCONTACT hContact)
{
	if (!bGPGloaded || !bGPGkeyrings) return false;

	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting || p->mode != MODE_GPG)
		return false;

	DBVARIANT dbv;
	if (db_get_s(hContact, MODULENAME, "gpg", &dbv)) return false;
	db_free(&dbv);
	return true;
}

bool isContactRSAAES(MCONTACT hContact)
{
	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting)
		return false;

	return p->mode == MODE_RSAAES;
}

bool isContactRSA(MCONTACT hContact)
{
	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting)
		return false;

	return p->mode == MODE_RSA;
}

bool isChatRoom(MCONTACT hContact)
{
	pUinKey p = findUinKey(hContact);
	if (!p || !p->proto || !p->proto->inspecting)
		return false;

	return db_get_b(hContact, p->proto->name, "ChatRoom", 0) != 0;
}

bool isFileExist(LPCSTR filename)
{
	return (GetFileAttributes(filename) != (UINT)-1);
}

bool isSecureIM(pUinKey ptr, BOOL emptyMirverAsSecureIM)
{
	if (!bAIP) return false;
	if (!ptr->proto->inspecting) return false;

	if (bNOL && db_get_b(ptr->hContact, "CList", "NotOnList", 0))
		return false;

	bool isSecureIM = false;
	LPSTR mirver = db_get_sa(ptr->hContact, ptr->proto->name, "MirVer");
	if (mirver) {
		isSecureIM = (emptyMirverAsSecureIM && !*mirver) || (strstr(mirver, "SecureIM") != NULL) || (strstr(mirver, "secureim") != NULL);
		mir_free(mirver);
	}
	return isSecureIM;
}

bool isSecureIM(MCONTACT hContact, BOOL emptyMirverAsSecureIM)
{
	if (!bAIP) return false;

	pUinKey p = findUinKey(hContact);
	return (p) ? isSecureIM(p, emptyMirverAsSecureIM) : false;
}
