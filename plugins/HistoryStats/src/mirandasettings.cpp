#include "_globals.h"
#include "mirandasettings.h"


MirandaSettings::MirandaSettings() :
	m_hContact(0)
{
}

bool MirandaSettings::readBool(const char* szSetting, bool bDefault) const
{
	return (readByte(szSetting, bDefault ? 1 : 0) != 0);
}

int MirandaSettings::readByte(const char* szSetting, int bDefault) const
{
	return db_get_b(m_hContact, m_strModule.c_str(), szSetting, bDefault);
}

int MirandaSettings::readWord(const char* szSetting, int wDefault) const
{
	return db_get_w(m_hContact, m_strModule.c_str(), szSetting, wDefault);
}

int MirandaSettings::readDWord(const char* szSetting, int dwDefault) const
{
	return db_get_dw(m_hContact, m_strModule.c_str(), szSetting, dwDefault);
}

ext::string MirandaSettings::readStr(const char* szSetting, const TCHAR* szDefault) const
{
	DBVARIANT dbv;
	if (db_get_s(m_hContact, m_strModule.c_str(), szSetting, &dbv))
		return szDefault;

	ext::string str = (dbv.type != DBVT_ASCIIZ) ? szDefault : utils::fromUTF8(dbv.pszVal);
	db_free(&dbv);
	return str;
}

ext::string MirandaSettings::readStrDirect(const char* szSetting, const TCHAR* szDefault) const
{
	DBVARIANT dbv;

	memset(&dbv, 0, sizeof(dbv));

	dbv.type = DBVT_WCHAR;

	if (db_get_s(m_hContact, m_strModule.c_str(), szSetting, &dbv, 0))
		return szDefault;

	ext::string str;

	switch (dbv.type) {
	case DBVT_ASCIIZ:
		str = utils::fromA(dbv.pszVal);
		break;

	case DBVT_WCHAR:
		str = utils::fromW(dbv.pwszVal);
		break;

	case DBVT_UTF8:
		str = utils::fromUTF8(dbv.pszVal);
		break;

	default:
		str = szDefault;
		break;
	}

	db_free(&dbv);
	return str;
}

void MirandaSettings::readTree(const char* szSetting, const TCHAR* szDefault, SettingsTree& value) const
{
	value.fromString(readStr(szSetting, szDefault));
}

void MirandaSettings::writeBool(const char* szSetting, bool bValue) const
{
	writeByte(szSetting, bValue ? 1 : 0);
}

void MirandaSettings::writeByte(const char* szSetting, int bValue) const
{
	db_set_b(m_hContact, m_strModule.c_str(), szSetting, bValue);
}

void MirandaSettings::writeWord(const char* szSetting, int wValue) const
{
	db_set_w(m_hContact, m_strModule.c_str(), szSetting, wValue);
}

void MirandaSettings::writeDWord(const char* szSetting, int dwValue) const
{
	db_set_dw(m_hContact, m_strModule.c_str(), szSetting, dwValue);
}

void MirandaSettings::writeStr(const char* szSetting, const TCHAR* szValue) const
{
	db_set_ts(m_hContact, m_strModule.c_str(), szSetting, szValue);
}

void MirandaSettings::writeStrDirect(const char* szSetting, const TCHAR* szValue) const
{
	db_set_ts(m_hContact, m_strModule.c_str(), szSetting, szValue);
}

void MirandaSettings::writeTree(const char* szSetting, const SettingsTree& value) const
{
	writeStr(szSetting, value.toString().c_str());
}

bool MirandaSettings::settingExists(const char* szSetting) const
{
	DBVARIANT dbv;
	if (db_get_s(m_hContact, m_strModule.c_str(), szSetting, &dbv, 0))
		return false;

	db_free(&dbv);
	return true;
}

bool MirandaSettings::delSetting(const char* szSetting)
{
	return db_unset(m_hContact, m_strModule.c_str(), szSetting) == 0;
}

int MirandaSettings::enumSettingsProc(const char* szSetting, LPARAM lParam)
{
	SetInserter* pInserter = reinterpret_cast<SetInserter*>(lParam);

	*pInserter = szSetting;

	return 0;
}

void MirandaSettings::enumSettings(SetInserter& insertIterator)
{
	mu::db_contact::enumSettings(m_hContact, m_strModule.c_str(), enumSettingsProc, reinterpret_cast<LPARAM>(&insertIterator));
}
