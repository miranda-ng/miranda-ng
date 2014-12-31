/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

enum TJabberDataFormType
{
	JDFT_BOOLEAN,
	JDFT_FIXED,
	JDFT_HIDDEN,
	JDFT_JID_MULTI,
	JDFT_JID_SINGLE,
	JDFT_LIST_MULTI,
	JDFT_LIST_SINGLE,
	JDFT_TEXT_MULTI,
	JDFT_TEXT_PRIVATE,
	JDFT_TEXT_SINGLE,
};

struct TJabberDataFormRegisry_Field
{
	TCHAR *field;
	TJabberDataFormType type;
	TCHAR *description_en;
	TCHAR *description_tr;
};

struct TJabberDataFormRegisry_Form
{
	TCHAR *form_type;
	TJabberDataFormRegisry_Field *fields;
	int count;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Forwards
class CJabberDlgDataForm;
class CJabberDataField;
class CJabberDataFieldSet;
class CJabberDataForm;
class CJabberDlgDataPage;

/////////////////////////////////////////////////////////////////////////////////////////
// Data form classes
class CJabberDataField
{
public:
	struct TOption
	{
		TCHAR *label;
		TCHAR *value;
	};

	CJabberDataField(CJabberDataForm *form, XmlNode *node);
	~CJabberDataField();

	XmlNode *GetNode() { return m_node; }

	TCHAR *GetTypeName() { return m_typeName; }
	TJabberDataFormType GetType() { return m_type; }

	TCHAR *GetVar() { return m_var; }

	bool IsRequired() { return m_required; }
	TCHAR *GetDescription(int i) { return m_descriptions[i]; }
	TCHAR *GetLabel() { return m_label; }

	int GetValueCount() { return m_values.getCount(); }
	TCHAR *GetValue(int i = 0) { return m_values[i]; }

	int GetOptionCount() { return m_options.getCount(); }
	TOption *GetOption(int i) { return &(m_options[i]); }

private:
	XmlNode *m_node;
	CJabberDataFieldSet *m_fieldset;

	bool m_required;
	TCHAR *m_var;
	TCHAR *m_label;
	TCHAR *m_typeName;
	TJabberDataFormType m_type;

	OBJLIST<TOption> m_options;
	LIST<TCHAR> m_values;
	LIST<TCHAR> m_descriptions;
};

class CJabberDataFieldSet
{
public:
	CJabberDataFieldSet();

	int GetCount() { return m_fields.getCount(); }
	CJabberDataField *GetField(int i) { return &(m_fields[i]); }
	CJabberDataField *GetField(TCHAR *var);

	void AddField(CJabberDataField *field) { m_fields.insert(field, m_fields.getCount()); }

private:
	OBJLIST<CJabberDataField> m_fields;
};

class CJabberDataForm
{
public:
	enum TFormType { TYPE_NONE, TYPE_FORM, TYPE_SUBMIT, TYPE_CANCEL, TYPE_RESULT };

	CJabberDataForm(XmlNode *node);
	~CJabberDataForm();

	TCHAR *GetTypeName() const { return m_typename; }
	TFormType GetType() const { return m_type; }
	TCHAR *GetFormType() const { return m_form_type; }
	TCHAR *GetTitle() const { return m_title; }
	int GetInstructionsCount() const { return m_instructions.getCount(); }
	TCHAR *GetInstructions(int idx=0) const { return m_instructions[idx]; }

	CJabberDataFieldSet *GetFields() { return &m_fields; }

	CJabberDataFieldSet *GetReported() { return &m_reported; }
	int GetItemCount() { return m_items.getCount(); }
	CJabberDataFieldSet *GetItem(int i) { return &(m_items[i]); }

private:
	XmlNode *m_node;

	TCHAR *m_typename;
	TFormType m_type;

	TCHAR *m_form_type;
	TJabberDataFormRegisry_Form *m_form_type_info;

	TCHAR *m_title;
	LIST<TCHAR> m_instructions;

	CJabberDataFieldSet m_fields;
	CJabberDataFieldSet m_reported;
	OBJLIST<CJabberDataFieldSet> m_items;
};

/////////////////////////////////////////////////////////////////////////////////////////
// UI Control
class CCtrlJabberForm: public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	static const TCHAR *ClassName;
	static bool RegisterClass();
	static bool UnregisterClass();

	CCtrlJabberForm(CDlgBase* dlg, int ctrlId);
	~CCtrlJabberForm();

	void OnInit();
	void SetDataForm(CJabberDataForm *pForm);
	XmlNode *FetchData();

protected:
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	static bool ClassRegistered;

	CJabberDataForm *m_pForm;
	CJabberDlgDataPage *m_pDlgPage;

	void SetupForm();
	void Layout();
};
