#include "_globals.h"
#include "mirandasettings.h"

MirandaSettings::fnGetSettingStr MirandaSettings::m_pGetSettingStr = NULL;

MirandaSettings::MirandaSettings()
	: m_hContact(0)
{
	// determine correct string read function
	if (!m_pGetSettingStr)
	{
		m_pGetSettingStr = mu::_link::serviceExists(MS_DB_CONTACT_GETSETTING_STR) ? mu::db_contact::getSettingStr : mu::db_contact::getSetting;
	}
}

bool MirandaSettings::readBool(const mu_ansi* szSetting, bool bDefault) const
{
	return (readByte(szSetting, bDefault ? 1 : 0) != 0);
}

int MirandaSettings::readByte(const mu_ansi* szSetting, int bDefault) const
{
	DBVARIANT dbv;
	
	return mu::db_contact::getSetting(m_hContact, m_strModule.c_str(), szSetting, &dbv) ? bDefault : dbv.bVal;
}

int MirandaSettings::readWord(const mu_ansi* szSetting, int wDefault) const
{
	DBVARIANT dbv;
	
	return mu::db_contact::getSetting(m_hContact, m_strModule.c_str(), szSetting, &dbv) ? wDefault : dbv.wVal;
}

int MirandaSettings::readDWord(const mu_ansi* szSetting, int dwDefault) const
{
	DBVARIANT dbv;

	return mu::db_contact::getSetting(m_hContact, m_strModule.c_str(), szSetting, &dbv) ? dwDefault : dbv.dVal;
}

ext::string MirandaSettings::readStr(const mu_ansi* szSetting, const mu_text* szDefault) const
{
	DBVARIANT dbv;

	ZeroMemory(&dbv, sizeof(dbv));
	
	if (mu::db_contact::getSetting(m_hContact, m_strModule.c_str(), szSetting, &dbv))
	{
		return szDefault;
	}
	else
	{
		ext::string str = (dbv.type != DBVT_ASCIIZ) ? szDefault : utils::fromUTF8(dbv.pszVal);

		mu::db_contact::freeVariant(&dbv);

		return str;
	}
}

ext::string MirandaSettings::readStrDirect(const mu_ansi* szSetting, const mu_text* szDefault) const
{
	DBVARIANT dbv;

	ZeroMemory(&dbv, sizeof(dbv));

	dbv.type = MU_DO_BOTH(DBVT_ASCIIZ, DBVT_WCHAR);

	if (m_pGetSettingStr(m_hContact, m_strModule.c_str(), szSetting, &dbv))
	{
		return szDefault;
	}
	else
	{
		ext::string str;

		switch (dbv.type)
		{
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

		mu::db_contact::freeVariant(&dbv);

		return str;
	}
}

void MirandaSettings::readTree(const mu_ansi* szSetting, const mu_text* szDefault, SettingsTree& value) const
{
	value.fromString(readStr(szSetting, szDefault));
}

void MirandaSettings::writeBool(const mu_ansi* szSetting, bool bValue) const
{
	writeByte(szSetting, bValue ? 1 : 0);
}

void MirandaSettings::writeByte(const mu_ansi* szSetting, int bValue) const
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = m_strModule.c_str();
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BYTE;
	cws.value.bVal = bValue;

	mu::db_contact::writeSetting(m_hContact, &cws);
}

void MirandaSettings::writeWord(const mu_ansi* szSetting, int wValue) const
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = m_strModule.c_str();
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WORD;
	cws.value.wVal = wValue;

	mu::db_contact::writeSetting(m_hContact, &cws);
}

void MirandaSettings::writeDWord(const mu_ansi* szSetting, int dwValue) const
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = m_strModule.c_str();
	cws.szSetting = szSetting;
	cws.value.type = DBVT_DWORD;
	cws.value.dVal = dwValue;

	mu::db_contact::writeSetting(m_hContact, &cws);
}

void MirandaSettings::writeStr(const mu_ansi* szSetting, const mu_text* szValue) const
{
	ext::a::string valueANSI = utils::toUTF8(szValue);
	DBCONTACTWRITESETTING cws;

	cws.szModule = m_strModule.c_str();
	cws.szSetting = szSetting;
	cws.value.type = DBVT_ASCIIZ;
	cws.value.pszVal = const_cast<mu_ansi*>(valueANSI.c_str());

	mu::db_contact::writeSetting(m_hContact, &cws);
}

void MirandaSettings::writeStrDirect(const mu_ansi* szSetting, const mu_text* szValue) const
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = m_strModule.c_str();
	cws.szSetting = szSetting;
	cws.value.type = MU_DO_BOTH(DBVT_ASCIIZ, DBVT_WCHAR);
	cws.value.ptszVal = const_cast<mu_text*>(szValue);

	mu::db_contact::writeSetting(m_hContact, &cws);
}

void MirandaSettings::writeTree(const mu_ansi* szSetting, const SettingsTree& value) const
{
	writeStr(szSetting, value.toString().c_str());
}

bool MirandaSettings::settingExists(const mu_ansi* szSetting) const
{
	DBVARIANT dbv;

	if (mu::db_contact::getSetting(m_hContact, m_strModule.c_str(), szSetting, &dbv))
	{
		return false;
	}
	else
	{
		mu::db_contact::freeVariant(&dbv);

		return true;
	}
}

bool MirandaSettings::delSetting(const mu_ansi* szSetting)
{
	return (mu::db_contact::deleteSetting(m_hContact, m_strModule.c_str(), szSetting) == 0);
}

int MirandaSettings::enumSettingsProc(const mu_ansi* szSetting, LPARAM lParam)
{
	SetInserter* pInserter = reinterpret_cast<SetInserter*>(lParam);

	*pInserter = szSetting;

	return 0;
}

void MirandaSettings::enumSettings(SetInserter& insertIterator)
{
	mu::db_contact::enumSettings(m_hContact, m_strModule.c_str(), enumSettingsProc, reinterpret_cast<LPARAM>(&insertIterator));
}
