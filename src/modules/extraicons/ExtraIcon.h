/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-15 Miranda NG project

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

#ifndef __EXTRAICON_H__
#define __EXTRAICON_H__

#define EXTRAICON_TYPE_GROUP -1

/////////////////////////////////////////////////////////////////////////////////////////
// ExtraIcon - base class for all extra icons

class ExtraIcon
{
public:
	ExtraIcon(const char *name);
	virtual ~ExtraIcon();

	virtual void rebuildIcons() = 0;
	virtual void applyIcons();
	virtual void applyIcon(MCONTACT hContact) =0 ;
	virtual void onClick(MCONTACT hContact) = 0;

	virtual int  setIcon(int id, MCONTACT hContact, HANDLE icon) = 0;
	virtual int  setIconByName(int id, MCONTACT hContact, const char* icon) = 0;
	virtual void storeIcon(MCONTACT hContact, void *icon) {};

	virtual const char *getName() const;
	virtual const TCHAR *getDescription() const = 0;
	virtual const char *getDescIcon() const = 0;
	virtual int getType() const = 0;

	virtual int getSlot() const;
	virtual void setSlot(int slot);

	virtual int getPosition() const;
	virtual void setPosition(int position);

	virtual bool isEnabled() const;

	virtual int ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage) = 0;

	int hLangpack;

protected:
	ptrA szName;

	int slot;
	int position;
};

/////////////////////////////////////////////////////////////////////////////////////////
// BaseExtraIcon - basic class for all 'real' extra icons

class BaseExtraIcon : public ExtraIcon
{
public:
	BaseExtraIcon(int id, const char *name, const TCHAR *description, const char *descIcon, MIRANDAHOOKPARAM OnClick, LPARAM param);
	virtual ~BaseExtraIcon();

	virtual int getID() const;
	virtual const TCHAR *getDescription() const;
	virtual void setDescription(const TCHAR *desc);
	virtual const char *getDescIcon() const;
	virtual void setDescIcon(const char *icon);
	virtual int getType() const =0;

	virtual void onClick(MCONTACT hContact);
	virtual void setOnClick(MIRANDAHOOKPARAM OnClick, LPARAM param);

	virtual int ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage);

protected:
	int id;
	ptrT tszDescription;
	ptrA szDescIcon;
	MIRANDAHOOKPARAM OnClick;
	LPARAM onClickParam;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CallbackExtraIcon - extra icon, implemented using callback functions

class CallbackExtraIcon : public BaseExtraIcon
{
public:
	CallbackExtraIcon(int id, const char *name, const TCHAR *description, const char *descIcon,
			MIRANDAHOOK RebuildIcons, MIRANDAHOOK ApplyIcon, MIRANDAHOOKPARAM OnClick, LPARAM param);
	virtual ~CallbackExtraIcon();

	virtual int getType() const;

	virtual void rebuildIcons();
	virtual void applyIcon(MCONTACT hContact);

	virtual int  setIcon(int id, MCONTACT hContact, HANDLE icon);
	virtual int  setIconByName(int id, MCONTACT hContact, const char* icon);

private:
	int(*RebuildIcons)(WPARAM wParam, LPARAM lParam);
	int(*ApplyIcon)(WPARAM wParam, LPARAM lParam);

	bool needToRebuild;
};

/////////////////////////////////////////////////////////////////////////////////////////
// IcolibExtraIcon - extra icon, implemented using icolib

class IcolibExtraIcon : public BaseExtraIcon
{
public:
	IcolibExtraIcon(int id, const char *name, const TCHAR *description, const char *descIcon, MIRANDAHOOKPARAM OnClick, LPARAM param);
	virtual ~IcolibExtraIcon();

	virtual int getType() const;

	virtual void rebuildIcons();
	virtual void applyIcon(MCONTACT hContact);

	virtual int  setIcon(int id, MCONTACT hContact, HANDLE icon);
	virtual int  setIconByName(int id, MCONTACT hContact, const char* icon);
	virtual void storeIcon(MCONTACT hContact, void *icon);
};

/////////////////////////////////////////////////////////////////////////////////////////
// ExtraIconGroup - joins some slots into one

class ExtraIconGroup : public ExtraIcon
{
	int  internalSetIcon(int id, MCONTACT hContact, HANDLE icon, bool bByName);
public:
	ExtraIconGroup(const char *name);
	virtual ~ExtraIconGroup();

	virtual void addExtraIcon(BaseExtraIcon *extra);

	virtual void rebuildIcons();
	virtual void applyIcon(MCONTACT hContact);
	virtual void onClick(MCONTACT hContact);

	virtual int  setIcon(int id, MCONTACT hContact, HANDLE icon);
	virtual int  setIconByName(int id, MCONTACT hContact, const char *icon);

	virtual const TCHAR* getDescription() const;
	virtual const char* getDescIcon() const;
	virtual int getType() const;

	virtual int getPosition() const;
	virtual void setSlot(int slot);

	LIST<BaseExtraIcon> items;

	virtual int ClistSetExtraIcon(MCONTACT hContact, HANDLE hImage);

protected:
	ptrT tszDescription;
	bool setValidExtraIcon;
	bool insideApply;

	virtual ExtraIcon *getCurrentItem(MCONTACT hContact) const;
};

#endif // __EXTRAICON_H__
