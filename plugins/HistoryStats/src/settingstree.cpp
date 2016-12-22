#include "stdafx.h"
#include "settingstree.h"

#include "utils.h"

void SettingsTree::makeKeyValid()
{
	if (!m_pCurValues)
		m_pCurValues = &m_Keys[m_CurKey];
}

SettingsTree::SettingsTree() :
	m_pCurValues(NULL)
{
}

SettingsTree::SettingsTree(const ext::string& config) :
	m_pCurValues(NULL)
{
	fromString(config);
}

void SettingsTree::clear()
{
	m_Keys.clear();
	setKey(L"");
}

void SettingsTree::fromString(const ext::string& config)
{
	m_Keys.clear();

	size_t i = 0;
	ext::string curKey;

	while (i < config.length()) {
		if (config[i] == '{') {
			++i;

			ValueMap& vals = m_Keys[curKey];

			ext::string curSetting;
			ext::string curVal;

			while (i < config.length() && config[i] != '}') {
				if (config[i] == ':') {
					curSetting = curVal;
					curVal = L"";
				}
				else if (config[i] == ';') {
					vals[curSetting] = curVal;

					curSetting = L"";
					curVal = L"";
				}
				else if (config[i] == '\\') {
					++i;
					curVal += config[i];
				}
				else curVal += config[i];

				++i;
			}

			curKey = L"";
		}
		else curKey += config[i];

		++i;
	}

	setKey(L"");
}

ext::string SettingsTree::toString() const
{
	static const wchar_t* replaces[5][2] = {
		{ L"\\", L"\\\\" },
		{ L"{", L"\\{" },
		{ L"}", L"\\}" },
		{ L":", L"\\:" },
		{ L";", L"\\;" }
	};

	ext::string data;

	citer_each_(KeyMap, i, m_Keys)
	{
		const ValueMap& vals = i->second;

		data += i->first;
		data += L"{";

		citer_each_(ValueMap, j, vals)
		{
			data += j->first;
			data += L":";

			ext::string tempSecond = j->second;

			array_each_(k, replaces)
			{
				utils::replaceAllInPlace(tempSecond, replaces[k][0], replaces[k][1]);
			}

			data += tempSecond;
			data += L";";
		}

		data += L"}";
	}

	return data;
}

void SettingsTree::setKey(const wchar_t* key)
{
	m_CurKey = key;

	KeyMap::iterator i = m_Keys.find(key);

	if (i != m_Keys.end())
		m_pCurValues = &i->second;
	else
		m_pCurValues = NULL;
}

bool SettingsTree::readBool(const wchar_t* setting, bool errorValue) const
{
	if (m_pCurValues) {
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end()) {
			return (i->second == L"y");
		}
	}

	return errorValue;
}

int SettingsTree::readInt(const wchar_t* setting, int errorValue) const
{
	if (m_pCurValues) {
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
			return _wtoi(i->second.c_str());
	}

	return errorValue;
}

int SettingsTree::readIntRanged(const wchar_t* setting, int errorValue, int minValue, int maxValue) const
{
	int value = readInt(setting, errorValue);
	if (minValue <= value && value <= maxValue)
		return value;

	return errorValue;
}

ext::string SettingsTree::readStr(const wchar_t* setting, const wchar_t* errorValue) const
{
	if (m_pCurValues) {
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end())
			return i->second;
	}

	return errorValue;
}

void SettingsTree::writeBool(const wchar_t* setting, bool value)
{
	makeKeyValid();

	(*m_pCurValues)[setting] = value ? L"y" : L"n";
}

void SettingsTree::writeInt(const wchar_t* setting, int value)
{
	makeKeyValid();

	(*m_pCurValues)[setting] = utils::intToString(value);
}

void SettingsTree::writeStr(const wchar_t* setting, const wchar_t* value)
{
	makeKeyValid();

	(*m_pCurValues)[setting] = value;
}

bool SettingsTree::hasSetting(const wchar_t* setting) const
{
	if (m_pCurValues) {
		ValueMap::const_iterator i = m_pCurValues->find(setting);
		if (i != m_pCurValues->end())
			return true;
	}

	return false;
}

bool SettingsTree::delSetting(const wchar_t* setting)
{
	if (m_pCurValues) {
		ValueMap::iterator i = m_pCurValues->find(setting);

		if (i != m_pCurValues->end()) {
			m_pCurValues->erase(i);

			if (m_pCurValues->size() == 0) {
				m_Keys.erase(m_CurKey);
				m_pCurValues = NULL;
			}

			return true;
		}
	}

	return false;
}
