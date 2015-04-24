#if !defined(HISTORYSTATS_GUARD_MIRANDASETTINGS_H)
#define HISTORYSTATS_GUARD_MIRANDASETTINGS_H

#include "stdafx.h"
#include "_consts.h"

#include "utils.h"
#include "settingstree.h"

#include <set>
#include <iterator>

class MirandaSettings
	: private pattern::NotCopyable<MirandaSettings>
{
public:
	typedef std::set<ext::a::string> SettingsSet;
	typedef std::insert_iterator<SettingsSet> SetInserter;

private:
	MCONTACT m_hContact;
	ext::a::string m_strModule;

private:
	static int enumSettingsProc(const char* szSetting, LPARAM lParam);

public:
	// constructor
	explicit MirandaSettings();

	// contact/module management
	MCONTACT getContact() const { return m_hContact; }
	void setContact(MCONTACT hContact) { m_hContact = hContact; }
	const ext::a::string& getModule() const { return m_strModule; }
	void setModule(const char* module) { m_strModule = module; }

	// reading
	bool readBool(const char* szSetting, bool bDefault) const;
	int readByte(const char* szSetting, int bDefault) const;
	int readWord(const char* szSetting, int wDefault) const;
	int readDWord(const char* szSetting, int dwDeault) const;
	ext::string readStr(const char* szSetting, const TCHAR* szDefault) const;
	ext::string readStrDirect(const char* szSetting, const TCHAR* szDefault) const;
	void readTree(const char* szSetting, const TCHAR* szDefault, SettingsTree& value) const;

	// writing
	void writeBool(const char* szSetting, bool bValue) const;
	void writeByte(const char* szSetting, int bValue) const;
	void writeWord(const char* szSetting, int wValue) const;
	void writeDWord(const char* szSetting, int dwValue) const;
	void writeStr(const char* szSetting, const TCHAR* szValue) const;
	void writeStrDirect(const char* szSetting, const TCHAR* szValue) const;
	void writeTree(const char* szSetting, const SettingsTree& value) const;

	// misc functions
	bool settingExists(const char* szSetting) const;
	bool delSetting(const char* szSetting);

	// enumeration
	void enumSettings(SetInserter& insertIterator);
};

#endif // HISTORYSTATS_GUARD_MIRANDASETTINGS_H
