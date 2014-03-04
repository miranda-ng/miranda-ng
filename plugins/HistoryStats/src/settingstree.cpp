#include "_globals.h"
#include "settingstree.h"

#include "utils.h"

void SettingsTree::makeKeyValid()
{
	if (!m_pCurValues)
		m_pCurValues = &m_Keys[m_CurKey];
}

SettingsTree::SettingsTree()
	: m_pCurValues(NULL)
{
}

SettingsTree::SettingsTree(const ext::string& config)
	: m_pCurValues(NULL)
{
	fromString(config);
}

void SettingsTree::clear()
{
	m_Keys.clear();
	setKey(muT(""));
}

void SettingsTree::fromString(const ext::string& config)
{
	m_Keys.clear();

	ext::string::size_type i = 0;
	ext::string curKey;

	while (i < config.length())
	{
		if (config[i] == muC('{'))
		{
			++i;

			ValueMap& vals = m_Keys[curKey];

			ext::string curSetting;
			ext::string curVal;

			while (i < config.length() && config[i] != muC('}'))
			{
				if (config[i] == muC(':'))
				{
					curSetting = curVal;
					curVal = muT("");
				}
				else if (config[i] == muC(';'))
				{
					vals[curSetting] = curVal;

					curSetting = muT("");
					curVal = muT("");
				}
				else if (config[i] == muC('\\'))
				{
					++i;
					curVal += config[i];
				}
				else
				{
					curVal += config[i];
				}

				++i;
			}

			curKey = muT("");
		}
		else
		{
			curKey += config[i];
		}

		++i;
	}

	setKey(muT(""));
}

ext::string SettingsTree::toString() const
{
	static const mu_text* replaces[5][2] = {
		{ muT("\\"), muT("\\\\") },
		{ muT("{") , muT("\\{")  },
		{ muT("}") , muT("\\}")  },
		{ muT(":") , muT("\\:")  },
		{ muT(";") , muT("\\;")  }
	};

	ext::string data;

	citer_each_(KeyMap, i, m_Keys)
	{
		const ValueMap& vals = i->second;

		data += i->first;
		data += muT("{");

		citer_each_(ValueMap, j, vals)
		{
			data += j->first;
			data += muT(":");

			ext::string tempSecond = j->second; 

			array_each_(k, replaces)
			{
				utils::replaceAllInPlace(tempSecond, replaces[k][0], replaces[k][1]);
			}

			data += tempSecond;
			data += muT(";");
		}

		data += muT("}");
	}

	return data;
}

void SettingsTree::setKey(const mu_text* key)
{
	m_CurKey = key;

	KeyMap::iterator i = m_Keys.find(key);

	if (i != m_Keys.end())
	{
		m_pCurValues = &i->second;
	}
	else
	{
		m_pCurValues = NULL;
	}
}

bool SettingsTree::readBool(const mu_text* setting, bool errorValue) const
{
	if (m_pCurValues)
	{
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
		{
			return (i->second == muT("y"));
		}
	}

	return errorValue;
}

int SettingsTree::readInt(const mu_text* setting, int errorValue) const
{
	if (m_pCurValues)
	{
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
		{
			return _ttoi(i->second.c_str());
		}
	}

	return errorValue;
}

int SettingsTree::readIntRanged(const mu_text* setting, int errorValue, int minValue, int maxValue) const
{
	int value = readInt(setting, errorValue);

	if (minValue <= value && value <= maxValue)
	{
		return value;
	}
	else
	{
		return errorValue;
	}
}

ext::string SettingsTree::readStr(const mu_text* setting, const mu_text* errorValue) const
{
	if (m_pCurValues)
	{
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
		{
			return i->second;
		}
	}

	return errorValue;
}

void SettingsTree::writeBool(const mu_text* setting, bool value)
{
	makeKeyValid();

	(*m_pCurValues)[setting] = value ? muT("y") : muT("n");
}

void SettingsTree::writeInt(const mu_text* setting, int value)
{
	makeKeyValid();

	(*m_pCurValues)[setting] = utils::intToString(value);
}

void SettingsTree::writeStr(const mu_text* setting, const mu_text* value)
{
	makeKeyValid();

	(*m_pCurValues)[setting] = value;
}

bool SettingsTree::hasSetting(const mu_text* setting) const
{
	if (m_pCurValues)
	{
		ValueMap::const_iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
		{
			return true;
		}
	}

	return false;
}

bool SettingsTree::delSetting(const mu_text* setting)
{
	if (m_pCurValues)
	{
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
		{
			m_pCurValues->erase(i);
			
			if (m_pCurValues->size() == 0)
			{
				m_Keys.erase(m_CurKey);
				m_pCurValues = NULL;
			}

			return true;
		}
	}

	return false;
}
