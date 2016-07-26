#if !defined(HISTORYSTATS_GUARD_SETTINGSTREE_H)
#define HISTORYSTATS_GUARD_SETTINGSTREE_H

#include "stdafx.h"
#include "_consts.h"

#include <map>

class SettingsTree
	: private pattern::NotCopyable<SettingsTree>
{
private:
	typedef std::map<ext::string, ext::string> ValueMap;
	typedef std::map<ext::string, ValueMap> KeyMap;

private:
	KeyMap m_Keys;
	ext::string m_CurKey;
	ValueMap* m_pCurValues;

private:
	void makeKeyValid();

public:
	explicit SettingsTree();
	explicit SettingsTree(const ext::string& config);

	void clear();

	void fromString(const ext::string& config);
	ext::string toString() const;

	void setKey(const wchar_t* key);
	const ext::string& getKey() const { return m_CurKey; }

	bool readBool(const wchar_t* setting, bool errorValue) const;
	int readInt(const wchar_t* setting, int errorValue) const;
	int readIntRanged(const wchar_t* setting, int errorValue, int minValue, int maxValue) const;
	ext::string readStr(const wchar_t* setting, const wchar_t* errorValue) const;

	void writeBool(const wchar_t* setting, bool value);
	void writeInt(const wchar_t* setting, int value);
	void writeStr(const wchar_t* setting, const wchar_t* value);

	bool hasSetting(const wchar_t* setting) const;
	bool delSetting(const wchar_t* setting);
};

#endif // HISTORYSTATS_GUARD_SETTINGSTREE_H