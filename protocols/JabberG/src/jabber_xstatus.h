/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (c) 2012-14  Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _JABBER_XSTATUS_H_
#define _JABBER_XSTATUS_H_

struct CJabberProto;

class CPepService
{
public:
	CPepService(CJabberProto *proto, char *name, TCHAR *node);
	virtual ~CPepService();

	HGENMENU GetMenu() { return m_hMenuItem; }
	TCHAR *GetNode() { return m_node; }
	virtual void ProcessItems(const TCHAR *from, HXML items) = 0;

	void Publish();
	void Retract();
	void ResetPublish();

	virtual void InitGui() {}
	virtual void RebuildMenu() {}
	virtual void ResetExtraIcon(MCONTACT) {}
	virtual bool LaunchSetGui() { return false; }

protected:
	CJabberProto *m_proto;
	char *m_name;
	TCHAR *m_node;
	HGENMENU m_hMenuItem;

	int m_wasPublished;

	virtual void CreateData(HXML) = 0;
	void ForceRepublishOnLogin();
};

class CPepServiceList: public OBJLIST<CPepService>
{
public:
	CPepServiceList(): OBJLIST<CPepService>(1) {}

	void ProcessEvent(const TCHAR *from, HXML eventNode)
	{
		for (int i=0; i < getCount(); i++)
		{
			CPepService &pepSvc = (*this)[i];
			HXML itemsNode = xmlGetChildByTag(eventNode, _T("items"), _T("node"), pepSvc.GetNode());
			if (itemsNode)
				pepSvc.ProcessItems(from, itemsNode);
		}
	}

	void InitGui()
	{
		for (int i=0; i < getCount(); i++)
			(*this)[i].InitGui();
	}

	void RebuildMenu()
	{
		for (int i=0; i < getCount(); i++)
			(*this)[i].RebuildMenu();
	}

	void ResetExtraIcon(MCONTACT hContact)
	{
		for (int i=0; i < getCount(); i++)
			(*this)[i].ResetExtraIcon(hContact);
	}

	void PublishAll()
	{
		for (int i=0; i < getCount(); i++)
			(*this)[i].Publish();
	}

	void RetractAll()
	{
		for (int i=0; i < getCount(); i++)
			(*this)[i].Retract();
	}

	void ResetPublishAll()
	{
		for(int i=0; i < getCount(); i++)
			(*this)[i].ResetPublish();
	}

	CPepService *Find(TCHAR *node)
	{
		for (int i=0; i < getCount(); i++)
			if (!mir_tstrcmp((*this)[i].GetNode(), node))
				return &((*this)[i]);
		return NULL;
	}
};

class CPepGuiService: public CPepService
{
	typedef CPepService CSuper;
public:
	CPepGuiService(CJabberProto *proto, char *name, TCHAR *node);
	~CPepGuiService();
	void InitGui();
	void RebuildMenu();
	bool LaunchSetGui(BYTE bQuiet);

protected:
	void UpdateMenuItem(HANDLE hIcolibIcon, TCHAR *text);
	virtual void ShowSetDialog(BYTE bQuiet) = 0;

private:
	HANDLE m_hMenuService;
	HANDLE m_hIcolibItem;
	TCHAR *m_szText;

	bool m_bGuiOpen;

	int __cdecl OnMenuItemClick(WPARAM, LPARAM);
};

class CPepMood: public CPepGuiService
{
	typedef CPepGuiService CSuper;
public:
	CPepMood(CJabberProto *proto);
	~CPepMood();
	void ProcessItems(const TCHAR *from, HXML items);
	void ResetExtraIcon(MCONTACT hContact);

public:
	TCHAR *m_text;
	int m_mode;

protected:
	void CreateData(HXML);
	void ShowSetDialog(BYTE bQuiet);
	void SetExtraIcon(MCONTACT hContact, char *szMood);

	void SetMood(MCONTACT hContact, const TCHAR *szMood, const TCHAR *szText);
};

class CPepActivity: public CPepGuiService
{
	typedef CPepGuiService CSuper;
public:
	CPepActivity(CJabberProto *proto);
	~CPepActivity();
	void ProcessItems(const TCHAR *from, HXML items);
	void ResetExtraIcon(MCONTACT hContact);

protected:
	TCHAR *m_text;
	int m_mode;

	void CreateData(HXML);
	void ShowSetDialog(BYTE bQuiet);
	void SetExtraIcon(MCONTACT hContact, char *szActivity);

	void SetActivity(MCONTACT hContact, LPCTSTR szFirst, LPCTSTR szSecond, LPCTSTR szText);
};

#endif // _JABBER_XSTATUS_H_
