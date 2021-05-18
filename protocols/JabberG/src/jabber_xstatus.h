/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (C) 2012-21 Miranda NG team

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
	CPepService(CJabberProto *proto, char *name, char *node);
	virtual ~CPepService();

	HGENMENU GetMenu() { return m_hMenuItem; }
	char* GetNode() { return m_node; }
	virtual void ProcessItems(const char *from, const TiXmlElement *items) = 0;

	void Publish();
	void Retract();
	void ResetPublish();

	virtual void InitGui() = 0;
	virtual void RebuildMenu() = 0;
	virtual void ResetExtraIcon(MCONTACT) = 0;
	virtual bool LaunchSetGui() { return false; }
	virtual void UpdateMenuView(void) = 0;

protected:
	CJabberProto *m_proto;
	bool m_wasPublished;
	char *m_name;
	char *m_node;
	HGENMENU m_hMenuItem;

	virtual void CreateData(TiXmlElement*) = 0;
	void ForceRepublishOnLogin();
};

class CPepServiceList: public OBJLIST<CPepService>
{
public:
	CPepServiceList(): OBJLIST<CPepService>(1) {}

	void ProcessEvent(const char *from, const TiXmlElement *eventNode)
	{
		for (auto &it : *this) {
			auto *itemsNode = XmlGetChildByTag(eventNode, "items", "node", it->GetNode());
			if (itemsNode)
				it->ProcessItems(from, itemsNode);
		}
	}

	void InitGui()
	{
		for (auto &it : *this)
			it->InitGui();
	}

	void RebuildMenu()
	{
		for (auto &it : *this)
			it->RebuildMenu();
	}

	void ResetExtraIcon(MCONTACT hContact)
	{
		for (auto &it : *this)
			it->ResetExtraIcon(hContact);
	}

	void PublishAll()
	{
		for (auto &it : *this)
			it->Publish();
	}

	void RetractAll()
	{
		for (auto &it : *this)
			it->Retract();
	}

	void ResetPublishAll()
	{
		for (auto &it : *this)
			it->ResetPublish();
	}

	CPepService* Find(const char *node)
	{
		for (auto &it : *this)
			if (!mir_strcmp(it->GetNode(), node))
				return it;
		return nullptr;
	}
};

class CPepGuiService: public CPepService
{
	typedef CPepService CSuper;
	friend struct CJabberProto;

	HANDLE m_hMenuService;
	HANDLE m_hIcolibItem;
	wchar_t *m_szText;

	bool m_bGuiOpen;
	int __cdecl OnMenuItemClick(WPARAM, LPARAM);

public:
	CPepGuiService(CJabberProto *proto, char *name, char *node);
	~CPepGuiService();
	void InitGui();
	void RebuildMenu();
	bool LaunchSetGui();

protected:
	void UpdateMenuItem(HANDLE hIcolibIcon, wchar_t *text);
	virtual void ShowSetDialog() = 0;
};

class CPepMood: public CPepGuiService
{
	typedef CPepGuiService CSuper;
	friend struct CJabberProto;

	wchar_t *m_text;
	int m_mode;

	void SetExtraIcon(MCONTACT hContact, char *szMood);
	void CreateData(TiXmlElement*);

	void SetMood(MCONTACT hContact, const wchar_t *szMood, const wchar_t *szText);

public:
	CPepMood(CJabberProto *proto);
	~CPepMood();

	void ShowSetDialog() override;
	void UpdateMenuView(void) override;

	void ProcessItems(const char *from, const TiXmlElement *items);
	void ResetExtraIcon(MCONTACT hContact);
};

class CPepActivity: public CPepGuiService
{
	typedef CPepGuiService CSuper;

	wchar_t *m_text;
	int m_mode;

	void CreateData(TiXmlElement*);
	void SetExtraIcon(MCONTACT hContact, char *szActivity);

	void SetActivity(MCONTACT hContact, const char *szFirst, const char *szSecond, const wchar_t *szText);

public:
	CPepActivity(CJabberProto *proto);
	~CPepActivity();

	void ShowSetDialog() override;
	void UpdateMenuView(void) override;

	void ProcessItems(const char *from, const TiXmlElement *items);
	void ResetExtraIcon(MCONTACT hContact);
};

#endif // _JABBER_XSTATUS_H_
