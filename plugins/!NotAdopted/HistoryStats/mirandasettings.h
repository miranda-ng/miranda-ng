#if !defined(HISTORYSTATS_GUARD_MIRANDASETTINGS_H)
#define HISTORYSTATS_GUARD_MIRANDASETTINGS_H

#include "_globals.h"
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
	typedef int (*fnGetSettingStr)(HANDLE, const mu_ansi*, const mu_ansi*, DBVARIANT*);

private:
	static fnGetSettingStr m_pGetSettingStr;

private:
	HANDLE m_hContact;
	ext::a::string m_strModule;

private:
	static int enumSettingsProc(const mu_ansi* szSetting, LPARAM lParam);

public:
	// constructor
	explicit MirandaSettings();

	// contact/module management
	HANDLE getContact() const { return m_hContact; }
	void setContact(HANDLE hContact) { m_hContact = hContact; }
	const ext::a::string& getModule() const { return m_strModule; }
	void setModule(const mu_ansi* module) { m_strModule = module; }

	// reading
	bool readBool(const mu_ansi* szSetting, bool bDefault) const;
	int readByte(const mu_ansi* szSetting, int bDefault) const;
	int readWord(const mu_ansi* szSetting, int wDefault) const;
	int readDWord(const mu_ansi* szSetting, int dwDeault) const;
	ext::string readStr(const mu_ansi* szSetting, const mu_text* szDefault) const;
	ext::string readStrDirect(const mu_ansi* szSetting, const mu_text* szDefault) const;
	void readTree(const mu_ansi* szSetting, const mu_text* szDefault, SettingsTree& value) const;

	// writing
	void writeBool(const mu_ansi* szSetting, bool bValue) const;
	void writeByte(const mu_ansi* szSetting, int bValue) const;
	void writeWord(const mu_ansi* szSetting, int wValue) const;
	void writeDWord(const mu_ansi* szSetting, int dwValue) const;
	void writeStr(const mu_ansi* szSetting, const mu_text* szValue) const;
	void writeStrDirect(const mu_ansi* szSetting, const mu_text* szValue) const;
	void writeTree(const mu_ansi* szSetting, const SettingsTree& value) const;

	// misc functions
	bool settingExists(const mu_ansi* szSetting) const;
	bool delSetting(const mu_ansi* szSetting);

	// enumeration
	void enumSettings(SetInserter& insertIterator);
};

#endif // HISTORYSTATS_GUARD_MIRANDASETTINGS_H
