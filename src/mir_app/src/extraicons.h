/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-22 Miranda NG team

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef __EXTRAICONS_H__
# define __EXTRAICONS_H__

#define EI_MODULE_NAME "ExtraIcons"

// Global Variables

#define ICON_SIZE 16

#define EXTRAICON_TYPE_GROUP -1

class ExtraIconGroup;

/////////////////////////////////////////////////////////////////////////////////////////
// ExtraIcon - base class for all extra icons

class ExtraIcon
{
public:
	ExtraIcon(const char *name);
	virtual ~ExtraIcon();

	__forceinline int getID() const { return m_id; }
	__forceinline void setID(int _id) { m_id = _id; }

	__forceinline ExtraIconGroup* getParent() const { return m_pParent; }
	__forceinline void setParent(ExtraIconGroup *p) { m_pParent = p; }

	void doApply(MCONTACT hContact);
	void applyIcons();

	virtual void rebuildIcons() = 0;
	virtual void applyIcon(MCONTACT hContact) = 0;
	virtual void onClick(MCONTACT hContact) = 0;

	virtual int  setIcon(MCONTACT hContact, HANDLE icon) = 0;
	virtual int  setIconByName(MCONTACT hContact, const char* icon) = 0;
	virtual void storeIcon(MCONTACT, void*) {};

	virtual const char *getName() const;
	virtual const wchar_t *getDescription() const = 0;
	virtual HANDLE getDescIcon() const = 0;
	virtual int getType() const = 0;

	virtual int getSlot() const;
	virtual void setSlot(int slot);

	virtual int getPosition() const;
	virtual void setPosition(int position);

	virtual bool isEnabled() const;

	virtual int ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage) = 0;

	HPLUGIN m_pPlugin = nullptr;

protected:
	ptrA m_szName;

	int m_id = 0;
	int m_slot = -1;
	int m_position = 1000;

	ExtraIconGroup *m_pParent = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////
// BaseExtraIcon - basic class for all 'real' extra icons

class BaseExtraIcon : public ExtraIcon
{
public:
	BaseExtraIcon(const char *name, const wchar_t *description, HANDLE descIcon, MIRANDAHOOKPARAM OnClick, LPARAM param);
	virtual ~BaseExtraIcon();

	virtual const wchar_t* getDescription() const;
	virtual void setDescription(const wchar_t *desc) {
		m_tszDescription = mir_wstrdup(desc);
	}

	virtual HANDLE getDescIcon() const;
	void setDescIcon(HANDLE icon) {
		m_hDescIcon = icon;
	}

	virtual int getType() const = 0;

	virtual void onClick(MCONTACT hContact);
	virtual void setOnClick(MIRANDAHOOKPARAM OnClick, LPARAM param);

	virtual int ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage);

protected:
	ptrW m_tszDescription;
	HANDLE m_hDescIcon;
	MIRANDAHOOKPARAM m_OnClick;
	LPARAM m_onClickParam;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CallbackExtraIcon - extra icon, implemented using callback functions

class CallbackExtraIcon : public BaseExtraIcon
{
public:
	CallbackExtraIcon(const char *name, const wchar_t *description, HANDLE descIcon,
			MIRANDAHOOK RebuildIcons, MIRANDAHOOK ApplyIcon, MIRANDAHOOKPARAM OnClick, LPARAM param);
	virtual ~CallbackExtraIcon();

	virtual int getType() const;

	virtual void rebuildIcons();
	virtual void applyIcon(MCONTACT hContact);

	virtual int  setIcon(MCONTACT hContact, HANDLE icon);
	virtual int  setIconByName(MCONTACT hContact, const char* icon);

private:
	int (*m_pfnRebuildIcons)(WPARAM wParam, LPARAM lParam);
	int (*m_pfnApplyIcon)(WPARAM wParam, LPARAM lParam);

	bool m_needToRebuild;
};

/////////////////////////////////////////////////////////////////////////////////////////
// IcolibExtraIcon - extra icon, implemented using icolib

class IcolibExtraIcon : public BaseExtraIcon
{
public:
	IcolibExtraIcon(const char *name, const wchar_t *description, HANDLE descIcon, MIRANDAHOOKPARAM OnClick, LPARAM param);
	virtual ~IcolibExtraIcon();

	virtual int getType() const;

	virtual void rebuildIcons();
	virtual void applyIcon(MCONTACT hContact);

	virtual int  setIcon(MCONTACT hContact, HANDLE icon);
	virtual int  setIconByName(MCONTACT hContact, const char* icon);
	virtual void storeIcon(MCONTACT hContact, void *icon);
};

/////////////////////////////////////////////////////////////////////////////////////////
// ExtraIconGroup - joins some slots into one

class ExtraIconGroup : public ExtraIcon
{
public:
	ExtraIconGroup(const char *name);
	virtual ~ExtraIconGroup();

	virtual void addExtraIcon(BaseExtraIcon *extra);

	virtual void rebuildIcons();
	virtual void applyIcon(MCONTACT hContact);
	virtual void onClick(MCONTACT hContact);

	virtual int  setIcon(MCONTACT hContact, HANDLE icon);
	virtual int  setIconByName(MCONTACT hContact, const char *icon);

	virtual const wchar_t* getDescription() const;
	virtual HANDLE getDescIcon() const;
	virtual int getType() const;

	virtual int getPosition() const;
	virtual void setSlot(int slot);

	LIST<BaseExtraIcon> m_items;

	virtual int ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage);

	int internalSetIcon(ExtraIcon *pChild, MCONTACT hContact, HANDLE icon, bool bByName);

protected:
	ptrW m_tszDescription;
	bool m_setValidExtraIcon = false;
	bool m_insideApply = false;
	
	ExtraIcon *m_pCurrentItem = nullptr;
};

/////////////////////////////////////////////////////////////////////////////////////////

extern LIST<BaseExtraIcon> registeredExtraIcons;
void RebuildListsBasedOnGroups(LIST<ExtraIconGroup> &groups);
extern LIST<ExtraIcon> extraIconsBySlot;

void eiOptionsRefresh(void);

int ConvertToClistSlot(int slot);

int Clist_SetExtraIcon(MCONTACT hContact, int slot, HANDLE hImage);

void DefaultExtraIcons_Load();

static inline BOOL IsEmpty(const char *str)
{
	return str == nullptr || str[0] == 0;
}

#endif // __EXTRAICONS_H__
