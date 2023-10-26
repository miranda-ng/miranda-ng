/*
Miranda SmileyAdd Plugin
Copyright (C) 2008 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SMILEYADD_CUSTOMSMILEY_H_
#define SMILEYADD_CUSTOMSMILEY_H_

class SmileyCType : public SmileyType
{
public:
	SmileyCType(const CMStringW &fullpath, const wchar_t *filepath);

	bool CreateTriggerText(char *text);
};

class SmileyPackCType
{
	SMOBJLIST<SmileyCType> m_SmileyList;
	OBJLIST<SmileyLookup> m_SmileyLookup;

	ptrA m_szModule;

	void AddTriggersToSmileyLookup(void);

public:
	SmileyPackCType(const char *pszModule) :
		m_szModule(mir_strdup(pszModule)),
		m_SmileyLookup(100, PtrKeySortT)
	{}

	auto& GetSmileyList(void) { return m_SmileyList; }
	auto& GetSmileyLookup(void) { return m_SmileyLookup; }

	int SmileyCount(void) const { return m_SmileyList.getCount(); }

	SmileyCType* GetSmiley(unsigned index) { return &m_SmileyList[index]; }

	const char* GetId(void) { return m_szModule; }

	bool LoadSmiley(const wchar_t *path);
	bool LoadSmileyDir(const wchar_t *dir);
};


class SmileyPackCListType 
{
	typedef SMOBJLIST<SmileyPackCType> SmileyPackVectorType;
	SmileyPackVectorType m_SmileyPacks;

public:
	int NumberOfSmileyPacks(void) { return m_SmileyPacks.getCount(); }

	bool AddSmiley(const char *pszModule, const wchar_t *path);
	bool AddSmileyPack(const char *pszModule, const wchar_t *dir);

	void ClearAndFreeAll(void) { m_SmileyPacks.destroy(); }

	SmileyPackCType* GetSmileyPack(const char *pszModule);
};

extern SmileyPackCListType g_SmileyPackCStore;

#endif
