#if !defined(HISTORYSTATS_GUARD_SETTINGSTREE_H)
#define HISTORYSTATS_GUARD_SETTINGSTREE_H

#include "_globals.h"
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

	void setKey(const TCHAR* key);
	const ext::string& getKey() const { return m_CurKey; }

	bool readBool(const TCHAR* setting, bool errorValue) const;
	int readInt(const TCHAR* setting, int errorValue) const;
	int readIntRanged(const TCHAR* setting, int errorValue, int minValue, int maxValue) const;
	ext::string readStr(const TCHAR* setting, const TCHAR* errorValue) const;

	void writeBool(const TCHAR* setting, bool value);
	void writeInt(const TCHAR* setting, int value);
	void writeStr(const TCHAR* setting, const TCHAR* value);

	bool hasSetting(const TCHAR* setting) const;
	bool delSetting(const TCHAR* setting);
};

#endif // HISTORYSTATS_GUARD_SETTINGSTREE_H