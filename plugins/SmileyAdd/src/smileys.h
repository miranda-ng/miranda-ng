/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2005-11 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003-04 Rein-Peter de Boer

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

#define MS_SMILEYADD_CUSTOMCATMENU  "SmileyAdd/CustomCatMenu"

const unsigned HiddenSmiley = 1;
const unsigned RegExSmiley = 2;
const unsigned ServiceSmiley = 4;
const unsigned TextSmiley = 8;
const unsigned SoundSmiley = 16;

class SmileyType
{
	LIST<ISmileyBase> m_arSmileys;

protected:
	unsigned m_flags;
	int m_index;

	SIZE m_size;

	HICON m_SmileyIcon;
	ImageBase *m_xepimg;

	CMStringW m_filepath;

	void SetFlagsBit(unsigned flag, bool set)
	{
		if (set) m_flags |= flag; else m_flags &= ~flag;
	}

public:

	CMStringW m_TriggerText;
	CMStringW m_ToolText;
	CMStringW m_InsertText;

	SmileyType();
	~SmileyType();

	const CMStringW& GetTriggerText(void) const { return m_TriggerText; }
	const CMStringW& GetToolText(void) const { return m_ToolText; }
	const CMStringW& GetInsertText(void) const { return m_InsertText; }
	const CMStringW& GetFilePath(void) const { return m_filepath; }

	bool IsHidden(void) const { return (m_flags & HiddenSmiley) != 0; }
	bool IsRegEx(void) const { return (m_flags & RegExSmiley) != 0; }
	bool IsService(void) const { return (m_flags & ServiceSmiley) != 0; }
	bool IsSound(void) const { return (m_flags & SoundSmiley) != 0; }
	bool IsText(void) const { return (m_flags & TextSmiley) != 0; }

	bool IsFileFound(void) { return _waccess(m_filepath.c_str(), 0) == 0; }
	bool IsValid(void) { return m_size.cx != 0; }

	ImageBase* CreateCachedImage(void);

	void GetSize(SIZE &size);

	HICON GetIcon(void);
	HICON GetIconDup(void);
	HBITMAP GetBitmap(COLORREF bkgClr, int sizeX, int sizeY);

	void SetFrameDelay(void);
	void MoveToNextFrame(void);

	void AddObject(ISmileyBase *pObject);
	void RemoveObject(ISmileyBase *pObject);

	void SetHidden(bool hid) { SetFlagsBit(HiddenSmiley, hid); }
	void SetRegEx(bool regex) { SetFlagsBit(RegExSmiley, regex); }
	void SetService(bool service) { SetFlagsBit(ServiceSmiley, service); }
	void SetSound(bool sound) { SetFlagsBit(SoundSmiley, sound); }
	void SetText(bool text) { SetFlagsBit(TextSmiley, text); }

	void SetImList(HIMAGELIST hImLst, long i);

	bool LoadFromResource(const CMStringW &file, const int index);
	bool LoadFromImage(IStream *pStream);
};

template<class T> struct SMOBJLIST : public OBJLIST<T>
{
	SMOBJLIST() : OBJLIST<T>(5) {};

	SMOBJLIST<T>& operator=(const SMOBJLIST<T>& lst)
	{
		OBJLIST<T>::destroy();
		return operator+=(lst);
	}

	SMOBJLIST<T>& operator+=(const SMOBJLIST<T>& lst)
	{
		for (auto &it : lst)
			insert(new T(*it));
		return *this;
	}

	void splice(SMOBJLIST<T> &lst)
	{
		for (auto &it : lst)
			insert(it);
		lst.LIST<T>::destroy();
	}
};

class SmileyLookup
{
	MRegexp16 m_pattern;
	int m_ind;
	CMStringW m_text;
	bool m_valid;

public:
	struct SmileyLocType
	{
		size_t pos, len;
		SmileyLocType(size_t p, size_t l) : pos(p), len(l) {}
	};
	typedef SMOBJLIST<SmileyLocType> SmileyLocVecType;

	SmileyLookup() : m_pattern(L"") { m_ind = 0; m_valid = false; m_pattern = nullptr; };
	SmileyLookup(const CMStringW &str, const bool regexs, const int ind, const CMStringW &smpt);
	~SmileyLookup();

	void Find(const CMStringW &str, SmileyLocVecType &smlcur, bool firstOnly);
	int GetIndex(void) const { return m_ind; }
	bool IsValid(void) const { return m_valid; }
};

class SmileyPackType
{
	CMStringW m_Filename;  //used as identification
	CMStringW m_Name;
	CMStringW m_Author;
	CMStringW m_Date;
	CMStringW m_Version;
	CMStringW m_ButtonSmiley;

	HIMAGELIST m_hSmList;

	int m_VisibleCount;

	typedef SMOBJLIST<SmileyType> SmileyVectorType;
	SmileyVectorType m_SmileyList;

	typedef SMOBJLIST<SmileyLookup> SmileyLookupType;
	SmileyLookupType m_SmileyLookup;

	bool errorFound;

	void AddTriggersToSmileyLookup(void);
	void ReplaceAllSpecials(const CMStringW &Input, CMStringW &Output);

public:
	SmileyPackType();
	~SmileyPackType();

	POINT selec, win;

	SmileyVectorType& GetSmileyList(void) { return m_SmileyList; }
	SmileyLookupType* GetSmileyLookup(void) { return &m_SmileyLookup; }

	const CMStringW& GetFilename(void) const { return m_Filename; }
	const CMStringW& GetName(void) const { return m_Name; }
	const CMStringW& GetAuthor(void) const { return m_Author; }
	const CMStringW& GetDate(void) const { return m_Date; }
	const CMStringW& GetVersion(void) const { return m_Version; }

	int SmileyCount(void) const { return m_SmileyList.getCount(); }
	int VisibleSmileyCount(void) const { return m_VisibleCount; }

	SmileyType* GetSmiley(unsigned index);

	const wchar_t* GetButtonSmiley(void) const { return m_ButtonSmiley.c_str(); }

	bool LoadSmileyFile(const CMStringW &filename, const CMStringW &packname, bool onlyInfo, bool noerr = false);

	bool LoadSmileyFileMSL(const CMStringW &filename, bool bOnlyInfo, CMStringW &modPath);
	bool LoadSmileyFileXEP(const CMStringW &filename, bool bOnlyInfo);

	void Clear(void);
};


class SmileyPackListType
{
	typedef SMOBJLIST<SmileyPackType> SmileyPackVectorType;
	SmileyPackVectorType m_SmileyPacks;

public:
	int NumberOfSmileyPacks(void) { return m_SmileyPacks.getCount(); }

	bool AddSmileyPack(CMStringW &filename, CMStringW &packname);
	void ClearAndFreeAll(void);
	SmileyPackType* GetSmileyPack(CMStringW &filename);
};


typedef enum
{
	smcNone,
	smcStd,
	smcProto,
	smcVirtualProto,
	smcTransportProto,
	smcPhysProto,
	smcCustom,
	smcExt
} SmcType;


class SmileyCategoryType
{
	CMStringW m_Name;
	CMStringW m_DisplayName;
	CMStringW m_Filename;     //functions as identification 

	SmcType type;

	SmileyPackListType* m_pSmileyPackStore;

	bool visible;

public:
	SmileyCategoryType() { type = smcNone; m_pSmileyPackStore = nullptr; visible = true; };
	SmileyCategoryType(SmileyPackListType *pSPS, const CMStringW &name, const CMStringW &displayName, const CMStringW &defaultFilename, SmcType typ);

	const CMStringW& GetDisplayName(void) const { return m_DisplayName; }
	const CMStringW& GetName(void) const { return m_Name; }
	const CMStringW& GetFilename(void) const { return m_Filename; }

	bool IsCustom(void) { return type == smcCustom; }
	bool IsProto(void) { return type == smcProto || type == smcPhysProto || type == smcTransportProto || type == smcVirtualProto; }
	bool IsAcc(void) { return type == smcProto; }
	bool IsVirtual(void) { return type == smcVirtualProto; }
	bool IsPhysProto(void) { return type == smcPhysProto; }
	bool IsTransportProto(void) { return type == smcTransportProto; }
	bool IsExt(void) { return type == smcExt; }
	bool IsVisible(void) { return visible; }

	SmcType GetType(void) { return type; }

	SmileyPackType* GetSmileyPack(void);

	void SetFilename(const CMStringW& name) { m_Filename = name; }
	void SetDisplayName(const CMStringW& name) { m_DisplayName = name; }
	void SetVisible(bool vis) { visible = vis; }

	void ClearFilename(void) { m_Filename.Empty(); }
	void SaveSettings(void);

	void Load(void);
};


class SmileyCategoryListType
{
	typedef SMOBJLIST<SmileyCategoryType> SmileyCategoryVectorType;

	SmileyCategoryVectorType m_SmileyCategories;
	SmileyPackListType *m_pSmileyPackStore;

public:
	void SetSmileyPackStore(SmileyPackListType *pSPS) { m_pSmileyPackStore = pSPS; }

	SmileyCategoryType* GetSmileyCategory(const CMStringW &name);
	SmileyCategoryType* GetSmileyCategory(unsigned index);
	SmileyPackType* GetSmileyPack(const CMStringW &name);
	SmileyCategoryVectorType* GetSmileyCategoryList(void) { return &m_SmileyCategories; };

	int NumberOfSmileyCategories(void) { return m_SmileyCategories.getCount(); }

	void AddCategory(const CMStringW &name, const CMStringW &displayName, SmcType typ, const CMStringW &defaultFilename = L"");
	void AddAndLoad(const CMStringW &name, const CMStringW &displayName);
	void AddAllProtocolsAsCategory(void);
	void AddAccountAsCategory(PROTOACCOUNT *acc, const CMStringW &defaultFile);
	void AddProtoAsCategory(char *acc, const CMStringW &defaultFile);
	void AddContactTransportAsCategory(MCONTACT hContact, const CMStringW &defaultFile);

	void ClearAndLoadAll(void);
	void ClearAll(void)
	{
		m_pSmileyPackStore->ClearAndFreeAll(); m_SmileyCategories.destroy();
	}

	bool DeleteCustomCategory(int index);
	void DeleteAccountAsCategory(PROTOACCOUNT *acc);

	void SaveSettings(void);
};

extern SmileyPackListType g_SmileyPacks;
extern SmileyCategoryListType g_SmileyCategories;

#endif //SMILEYADD_SMILEYS_H_
