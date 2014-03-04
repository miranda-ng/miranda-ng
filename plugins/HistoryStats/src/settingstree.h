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

	void setKey(const mu_text* key);
	const ext::string& getKey() const { return m_CurKey; }

	bool readBool(const mu_text* setting, bool errorValue) const;
	int readInt(const mu_text* setting, int errorValue) const;
	int readIntRanged(const mu_text* setting, int errorValue, int minValue, int maxValue) const;
	ext::string readStr(const mu_text* setting, const mu_text* errorValue) const;

	void writeBool(const mu_text* setting, bool value);
	void writeInt(const mu_text* setting, int value);
	void writeStr(const mu_text* setting, const mu_text* value);

	bool hasSetting(const mu_text* setting) const;
	bool delSetting(const mu_text* setting);
};

#endif // HISTORYSTATS_GUARD_SETTINGSTREE_H