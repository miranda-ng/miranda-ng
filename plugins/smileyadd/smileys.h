/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy
Copyright (C) 2003 - 2004 Rein-Peter de Boer

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

#ifndef SMILEYADD_SMILEYS_H_
#define SMILEYADD_SMILEYS_H_

#include "general.h"
#include "smltool.h"
#include "imagecache.h"

#define MS_SMILEYADD_CUSTOMCATMENU  "SmileyAdd/CustomCatMenu"

const unsigned HiddenSmiley  =  1;
const unsigned RegExSmiley   =  2;
const unsigned ServiceSmiley =  4;
const unsigned TextSmiley    =  8;
const unsigned SoundSmiley   = 16;

class SmileyType
{
protected:
	unsigned m_flags;
	int m_index;

	SIZE m_size;

	HICON m_SmileyIcon;
	ImageBase* m_xepimg;

	bkstring m_filepath;

	void SetFlagsBit(unsigned flag, bool set)
	{ if (set) m_flags |= flag; else m_flags &= ~flag; } 

public:

	bkstring m_TriggerText;
	bkstring m_ToolText;
	bkstring m_InsertText;

	SmileyType(void);
	~SmileyType();

	const bkstring& GetTriggerText(void) const { return m_TriggerText; }
	const bkstring& GetToolText(void) const { return m_ToolText; }
	const bkstring& GetInsertText(void) const { return m_InsertText; }
	const bkstring& GetFilePath(void) const { return m_filepath; }

	bool IsHidden(void) const { return (m_flags & HiddenSmiley) != 0; }
	bool IsRegEx(void) const { return (m_flags & RegExSmiley) != 0; }
	bool IsService(void) const { return (m_flags & ServiceSmiley) != 0; }
	bool IsSound(void) const { return (m_flags & SoundSmiley) != 0; }
	bool IsText(void) const { return (m_flags & TextSmiley) != 0; }

	bool IsFileFound(void) { return _taccess(m_filepath.c_str(), 0) == 0; }
	bool IsValid(void) { return m_size.cx != 0; }

	ImageBase* CreateCachedImage(void);

	void GetSize(SIZE& size);
	int GetStaticFrame(void) const { return m_index; }

	HICON GetIcon(void);
	HICON GetIconDup(void);
	HBITMAP GetBitmap(COLORREF bkgClr, int sizeX, int sizeY);

	void CallSmileyService(HANDLE hContact);

	void SetHidden(bool hid) { SetFlagsBit(HiddenSmiley, hid); }
	void SetRegEx(bool regex) { SetFlagsBit(RegExSmiley, regex); }
	void SetService(bool service) { SetFlagsBit(ServiceSmiley, service); }
	void SetSound(bool sound) { SetFlagsBit(SoundSmiley, sound); }
	void SetText(bool text) { SetFlagsBit(TextSmiley, text); }

	void SetImList(HIMAGELIST hImLst, long i);

	bool LoadFromResource(const bkstring& file, const int index);
	bool LoadFromImage(IStream* pStream);
};


class SmileyLookup
{
private:
	_TPattern* m_pattern;

	int m_ind;
	bkstring m_text;
	bool m_valid;

public:
	struct SmileyLocType
	{ 
		size_t pos, len; 
		SmileyLocType(size_t p, size_t l) : pos(p), len(l) {}
		SmileyLocType() {}
	};
	typedef SMOBJLIST<SmileyLocType> SmileyLocVecType;

	SmileyLookup() { m_ind = 0; m_valid = false; m_pattern = NULL; };
	SmileyLookup(const bkstring& str, const bool regexs, const int ind, const bkstring& smpt);
	~SmileyLookup();

	void find(const bkstring& str, SmileyLocVecType& smlcur, bool firstOnly) const;
	int GetIndex(void) const { return m_ind; }
	bool IsValid(void) const { return m_valid; }
};


class SmileyPackType
{
public:
	typedef SMOBJLIST<SmileyType> SmileyVectorType;
	typedef SMOBJLIST<SmileyLookup> SmileyLookupType;

	POINT selec, win;

private:
	bkstring m_Filename;  //used as identification
	bkstring m_Name;
	bkstring m_Author;
	bkstring m_Date;
	bkstring m_Version;
	bkstring m_ButtonSmiley;

	HIMAGELIST m_hSmList;

	int m_VisibleCount; 

	SmileyVectorType m_SmileyList;
	SmileyLookupType m_SmileyLookup;

	bool errorFound;

	void InsertLookup(SmileyType& sml, bkstring& lk, bool first);
	void AddTriggersToSmileyLookup(void);
	void ReplaceAllSpecials(const bkstring& Input, bkstring& Output);
	bool LoadSmileyFileMSL(bkstring& tbuf, bool onlyInfo, bkstring& modpath);
	bool LoadSmileyFileXEP(bkstring& tbuf, bool onlyInfo, bkstring& modpath);

public:
	SmileyPackType();
	~SmileyPackType();

	SmileyVectorType& GetSmileyList(void) { return m_SmileyList; }
	SmileyLookupType* GetSmileyLookup(void) { return &m_SmileyLookup; }

	const bkstring& GetFilename(void) const { return m_Filename; }
	const bkstring& GetName(void) const { return m_Name; }
	const bkstring& GetAuthor(void) const { return m_Author; }
	const bkstring& GetDate(void) const { return m_Date; }
	const bkstring& GetVersion(void) const { return m_Version; }

	int SmileyCount(void) const { return m_SmileyList.getCount(); }
	int VisibleSmileyCount(void) const { return m_VisibleCount; }

	SmileyType* GetSmiley(unsigned index);

	const TCHAR* GetButtonSmiley(void) const { return m_ButtonSmiley.c_str(); }

	bool LoadSmileyFile(const bkstring& filename, bool onlyInfo, bool noerr = false);

	void Clear(void);
};


class SmileyPackListType 
{
public:
	typedef SMOBJLIST<SmileyPackType> SmileyPackVectorType;

private:
	SmileyPackVectorType m_SmileyPacks;

public:
	int NumberOfSmileyPacks(void) { return m_SmileyPacks.getCount(); }

	bool AddSmileyPack(bkstring& filename);
	void ClearAndFreeAll(void);
	SmileyPackType* GetSmileyPack(bkstring& filename);
};


typedef enum
{
	smcNone,
	smcStd,
	smcProto,
	smcCustom,
	smcExt
} SmcType;


class SmileyCategoryType
{
private:
	bkstring m_Name;
	bkstring m_DisplayName;
	bkstring m_Filename;     //functions as identification 

	SmcType type;

	SmileyPackListType* m_pSmileyPackStore;

public:
	SmileyCategoryType() { type = smcNone; m_pSmileyPackStore = NULL; };
	SmileyCategoryType(SmileyPackListType* pSPS, const bkstring& name, 
		const bkstring& displayName, const bkstring& defaultFilename, SmcType typ);

	const bkstring& GetDisplayName(void) const { return m_DisplayName; }
	const bkstring& GetName(void) const { return m_Name; }
	const bkstring& GetFilename(void) const { return m_Filename; }

	bool IsCustom(void) { return type == smcCustom; }
	bool IsProto(void)  { return type == smcProto;  }
	bool IsExt(void)    { return type == smcExt;  }

	SmcType GetType(void) { return type; }

	SmileyPackType* GetSmileyPack(void);

	void SetFilename(bkstring& name) { m_Filename = name; }
	void SetDisplayName(bkstring& name) { m_DisplayName = name; }

	void ClearFilename(void) { m_Filename.clear(); }
	void SaveSettings(void);

	void Load(void);
};


class SmileyCategoryListType
{
public:
	typedef SMOBJLIST<SmileyCategoryType> SmileyCategoryVectorType;

private:
	SmileyCategoryVectorType m_SmileyCategories;
	SmileyPackListType* m_pSmileyPackStore;

public:  
	void SetSmileyPackStore(SmileyPackListType* pSPS)  { m_pSmileyPackStore = pSPS; }

	SmileyCategoryType* GetSmileyCategory(const bkstring& name);
	SmileyCategoryType* GetSmileyCategory(unsigned index);
	SmileyPackType* GetSmileyPack(bkstring& name);
	SmileyCategoryVectorType* GetSmileyCategoryList(void) { return &m_SmileyCategories; };

	int NumberOfSmileyCategories(void) { return m_SmileyCategories.getCount(); }

	void AddCategory(const bkstring& name, const bkstring& displayName, SmcType typ,
		const bkstring& defaultFilename = bkstring(_T("Smileys\\nova\\default.msl")));
	void AddAndLoad(const bkstring& name, const bkstring& displayName);
	void AddAllProtocolsAsCategory(void); 
	void AddAccountAsCategory(PROTOACCOUNT *acc, const bkstring& defaultFile);
	void AddContactTransportAsCategory(HANDLE hContact, const bkstring& defaultFile);

	void ClearAndLoadAll(void);
	void ClearAll(void) 
	{ m_pSmileyPackStore->ClearAndFreeAll(); m_SmileyCategories.destroy(); }

	bool DeleteCustomCategory(int index);
	void DeleteAccountAsCategory(PROTOACCOUNT *acc);

	void SaveSettings(void);
};

extern SmileyPackListType g_SmileyPacks;
extern SmileyCategoryListType g_SmileyCategories;

#endif //SMILEYADD_SMILEYS_H_
