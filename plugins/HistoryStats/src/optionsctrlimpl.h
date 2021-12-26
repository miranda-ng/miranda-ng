#if !defined(HISTORYSTATS_GUARD_OPTIONSCTRLIMPL_H)
#define HISTORYSTATS_GUARD_OPTIONSCTRLIMPL_H

#include "stdafx.h"
#include "optionsctrldefs.h"

#include <vector>

#include "utils.h"

/*
 * OptionsCtrlImpl
 */

class OptionsCtrlImpl
	: public OptionsCtrlDefs
	, private pattern::NotCopyable<OptionsCtrlImpl>
{
private:
	enum ItemType {
		itGroup,
		itCheck,
		itRadio,
		itEdit,
		itCombo,
		itButton,
		itDateTime,
		itColor,
	};

	enum StateIcon {
		// C = checked
		// U = unchecked
		// G = greyed
		siCheckU    =  0,
		siCheckC    =  1,
		siCheckUG   =  2,
		siCheckCG   =  3,
		siRadioU    =  4,
		siRadioC    =  5,
		siRadioUG   =  6,
		siRadioCG   =  7,
		siEdit      =  8,
		siEditG     =  9,
		siCombo     = 10,
		siComboG    = 11,
		siFolder    = 12,
		siFolderG   = 13,
		siButton    = 14,
		siButtonG   = 15,
		siDateTime  = 16,
		siDateTimeG = 17,
		siColor     =  0,
		siColorG    =  2,
	};

	enum ChildControl {
		ccTree     = 100,
		ccEdit     = 101,
		ccCombo    = 102,
		ccButton   = 103,
		ccDateTime = 104,
		ccColor    = 105,
	};

	class Item
	{
	public:
		OptionsCtrlImpl* m_pCtrl;
		HTREEITEM m_hItem;
		int m_nRef;
		ItemType m_ItemType;
		bool m_bEnabled;
		bool m_bDisableChilds;
		ext::string m_strLabel;
		INT_PTR m_dwData;

	protected:
		explicit Item(OptionsCtrlImpl* pCtrl, ItemType ItemType, const wchar_t* szLabel, uint32_t dwFlags, INT_PTR dwData);

		void enableChilds(bool bEnable);

	public:
		virtual ~Item() { }

		virtual void onToggle() { }
		virtual void onSelect() { }
		virtual void onDeselect() { }
		virtual void onActivate() { }

		virtual void setEnabled(bool bEnable) = 0;
		virtual void childAdded(Item* pChild) = 0;

		virtual void setLabel(const wchar_t* szLabel);
		virtual const wchar_t* getLabel() { return m_strLabel.c_str(); }
	};

	class Group
		: public Item
	{
	public:
		bool m_bDrawLine;

	public:
		explicit Group(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, uint32_t dwFlags, INT_PTR dwData);

		virtual void setEnabled(bool bEnable);
		virtual void childAdded(Item* pChild);

	};

	class Check
		: public Item
	{
	public:
		bool m_bChecked;
		bool m_bDisableChildsOnUncheck;

	private:
		int getStateImage();
		bool getChildEnable();
		void updateItem();

	public:
		explicit Check(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, uint32_t dwFlags, INT_PTR dwData);

		virtual void onToggle();

		virtual void setEnabled(bool bEnable) { m_bEnabled = bEnable; updateItem(); }
		virtual void childAdded(Item* pChild);

		bool isChecked() { return m_bChecked; }
		void setChecked(bool bCheck) { m_bChecked = bCheck; updateItem(); }
	};

	class Radio; // forward declaration for RadioSiblings

	class RadioSiblings
	{
		friend class Radio; // for join()/leave()

	public:
		std::vector<Radio*> m_Radios;
		int m_nRadios;
		int m_nChecked;

	private:
		int join(Radio* pRadio);
		bool leave(int nRadio);

	public:
		explicit RadioSiblings() : m_nRadios(0), m_nChecked(-1) { }

		int getChecked() { return m_nChecked; }
		bool isChecked(int nRadio) { return (m_nChecked == nRadio); }
		void setChecked(int nRadio);
	};

	class Radio
		: public Item
	{
		friend class RadioSiblings; // for updateItem()

	public:
		bool m_bDisableChildsOnUncheck;
		int m_nIndex;
		RadioSiblings* m_pSiblings;

	private:
		bool getChildEnable(bool bChecked);
		void updateItem();

	public:
		explicit Radio(OptionsCtrlImpl* pCtrl, Item* pParent, Radio* pSibling, const wchar_t* szLabel, uint32_t dwFlags, INT_PTR dwData);
		virtual ~Radio();

		virtual void onToggle();

		virtual void setEnabled(bool bEnable) { m_bEnabled = bEnable; updateItem(); }
		virtual void childAdded(Item* pChild);

		bool isChecked() { return m_pSiblings->isChecked(m_nIndex); }
		void setChecked() { m_pSiblings->setChecked(m_nIndex); }
	};

	class Edit
		: public Item
	{
	public:
		bool m_bNumber;
		ext::string m_strEdit;
		HWND m_hEditWnd;

	private:
		ext::string getEditText();
		ext::string getCombinedText();

	public:
		explicit Edit(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, const wchar_t* szEdit, uint32_t dwFlags, INT_PTR dwData);

		virtual void onToggle() { onActivate(); }
		virtual void onSelect();
		virtual void onDeselect();
		virtual void onActivate();

		virtual void setEnabled(bool bEnable);
		virtual void childAdded(Item* pChild);

		virtual void setLabel(const wchar_t* szLabel);

		const wchar_t* getString();
		void setString(const wchar_t* szString);
		int getNumber() { return _wtoi(getString()); }
		void setNumber(int nNumber) { setString(utils::intToString(nNumber).c_str()); }
	};

	class Combo
		: public Item
	{
	public:
		bool m_bDisableChildsOnIndex0;
		int m_nSelected;
		std::vector<ext::string> m_Items;
		HWND m_hComboWnd;

	private:
		void enableChildsCombo();
		bool getChildEnable();
		int getComboSel();
		ext::string getCombinedText();

	public:
		explicit Combo(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, uint32_t dwFlags, INT_PTR dwData);

		virtual void onToggle() { onActivate(); }
		virtual void onSelect();
		virtual void onDeselect();
		virtual void onActivate();

		virtual void setEnabled(bool bEnable);
		virtual void childAdded(Item* pChild);

		virtual void setLabel(const wchar_t* szLabel);

		void addItem(const wchar_t* szItem);
		int getSelected();
		void setSelected(int nSelect);
		void onSelChanged();
	};

	class Button
		: public Item
	{
	public:
		ext::string m_strButton;
		HWND m_hButtonWnd;

	public:
		explicit Button(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, const wchar_t* szButton, uint32_t dwFlags, INT_PTR dwData);

		virtual void onToggle() { onActivate(); }
		virtual void onSelect();
		virtual void onDeselect();
		virtual void onActivate();

		virtual void setEnabled(bool bEnable);
		virtual void childAdded(Item* pChild);

		virtual void setLabel(const wchar_t* szLabel);
	};

	class DateTime
		: public Item
	{
	public:
		static ext::string getDTFormatString(const ext::string& strFormat);
		static SYSTEMTIME toSystemTime(time_t timestamp);
		static time_t fromSystemTime(const SYSTEMTIME& st);

	public:
		bool m_bDisableChildsOnNone;
		bool m_bAllowNone;
		ext::string m_strFormat;
		ext::string m_strFormatDT;
		bool m_bNone;
		time_t m_timestamp;
		HWND m_hDateTimeWnd;

	private:
		void enableChildsDateTime();
		bool getChildEnable();
		time_t getTimestampValue();
		bool getTimestampNone();
		ext::string getCombinedText();

	public:
		explicit DateTime(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, const wchar_t* szFormat, time_t timestamp, uint32_t dwFlags, INT_PTR dwData);

		virtual void onToggle() { onActivate(); }
		virtual void onSelect();
		virtual void onDeselect();
		virtual void onActivate();

		virtual void setEnabled(bool bEnable);
		virtual void childAdded(Item* pChild);

		virtual void setLabel(const wchar_t* szLabel);

		bool isNone();
		void setNone();
		time_t getTimestamp();
		void setTimestamp(time_t timestamp);
		void onDateTimeChange();
		bool isMonthCalVisible();
	};

	class Color
		: public Item
	{
	public:
		COLORREF m_crColor;
		HWND m_hColorWnd;

	private:
		COLORREF getColorValue();

	public:
		explicit Color(OptionsCtrlImpl* pCtrl, Item* pParent, const wchar_t* szLabel, COLORREF crColor, uint32_t dwFlags, INT_PTR dwData);

		virtual void onToggle() { onActivate(); }
		virtual void onSelect();
		virtual void onDeselect();
		virtual void onActivate();

		virtual void setEnabled(bool bEnable);
		virtual void childAdded(Item* pChild);

		virtual void setLabel(const wchar_t* szLabel);

		COLORREF getColor();
		void setColor(COLORREF crColor);
		void onColorChange();
	};

private:
	static const wchar_t* m_ClassName;
	static HIMAGELIST m_hStateIcons;
	static int m_nStateIconsRef;

private:
	static LRESULT CALLBACK staticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK staticTreeProc(HWND hTree, UINT msg, WPARAM wParam, LPARAM lParam);
	static void staticInitStateImages();
	static void staticFreeStateImages();
	static void staticUpdateStateImages(LPARAM lParam);

public:
	static bool registerClass();
	static void unregisterClass();

private:
	HWND      m_hWnd;
	UINT_PTR  m_nOwnId;
	HWND      m_hTree;
	WNDPROC   m_pfnOldTreeProc;
	bool      m_bModified;
	HTREEITEM m_hDragItem;

private:
	explicit OptionsCtrlImpl(HWND hWnd, UINT_PTR nOwnId);
	explicit OptionsCtrlImpl(const OptionsCtrlImpl& other); // no implementation
	const OptionsCtrlImpl& operator =(const OptionsCtrlImpl& other); // no implementation
	~OptionsCtrlImpl();

private:
	LRESULT onWMCreate(CREATESTRUCT* pCS);
	void onWMDestroy();
	void onNMClick();
	LRESULT onNMCustomDraw(NMTVCUSTOMDRAW* pNMCustomDraw);
	void onNMKillFocus();
	LRESULT onTVNItemExpanding(NMTREEVIEW* pNMTreeView);
	void onTVNDeleteItem(NMTREEVIEW* pNMTreeView);
	void onTVNSelChanging(NMTREEVIEW* pNMTreeView);
	void onTVNSelChanged(NMTREEVIEW* pNMTreeView);
	void onTVNBeginDrag(NMTREEVIEW* pNMTreeView);
	void onWMMouseMove(const POINT& pt);
	void onWMLButtonUp(const POINT& pt);
	void onENChange();
	void onCBNSelChange();
	void onBNClicked();
	void onDTNDateTimeChange();
	void onCPNColorChanged();
	void onTreeWMLButtonDown(UINT nFlags, POINTS point);
	void onTreeWMChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void onTreeWMKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void onTreeXScroll();
	HTREEITEM onOCMInsertGroup(HTREEITEM hParent, OCGROUP* pGroup);
	HTREEITEM onOCMInsertCheck(HTREEITEM hParent, OCCHECK* pCheck);
	HTREEITEM onOCMInsertRadio(HTREEITEM hParent, OCRADIO* pRadio);
	HTREEITEM onOCMInsertEdit(HTREEITEM hParent, OCEDIT* pEdit);
	HTREEITEM onOCMInsertCombo(HTREEITEM hParent, OCCOMBO* pCombo);
	HTREEITEM onOCMInsertButton(HTREEITEM hParent, OCBUTTON* pButton);
	HTREEITEM onOCMInsertDateTime(HTREEITEM hParent, OCDATETIME* pDateTime);
	HTREEITEM onOCMInsertColor(HTREEITEM hParent, OCCOLOR* pColor);
	const wchar_t* onOCMGetItemLabel(HTREEITEM hItem);
	void onOCMSetItemLabel(HTREEITEM hItem, const wchar_t* szLabel);
	bool onOCMIsItemEnabled(HTREEITEM hItem);
	void onOCMEnableItem(HTREEITEM hItem, bool bEnable);
	uint32_t onOCMGetItemData(HTREEITEM hItem);
	void onOCMSetItemData(HTREEITEM hItem, INT_PTR dwData);
	bool onOCMIsItemChecked(HTREEITEM hItem);
	void onOCMCheckItem(HTREEITEM hItem, bool bCheck);
	int onOCMGetRadioChecked(HTREEITEM hRadio);
	void onOCMSetRadioChecked(HTREEITEM hRadio, int nCheck);
	int onOCMGetEditNumber(HTREEITEM hEdit);
	void onOCMSetEditNumber(HTREEITEM hEdit, int nNumber);
	const wchar_t* onOCMGetEditString(HTREEITEM hEdit);
	void onOCMSetEditString(HTREEITEM hEdit, const wchar_t *szString);
	void onOCMAddComboItem(HTREEITEM hCombo, const wchar_t* szItem);
	int onOCMGetComboSelected(HTREEITEM hCombo);
	void onOCMSetComboSelected(HTREEITEM hCombo, int nSelect);
	void onOCMEnsureVisible(HTREEITEM hItem);
	void onOCMDeleteAllItems();
	HTREEITEM onOCMGetSelection();
	void onOCMSelectItem(HTREEITEM hItem);
	HTREEITEM onOCMGetItem(HTREEITEM hItem, uint32_t dwFlag);
	void onOCMDeleteItem(HTREEITEM hItem);
	void onOCMMoveItem(HTREEITEM& hItem, HTREEITEM hInsertAfter);
	bool onOCMIsDateTimeNone(HTREEITEM hDateTime);
	void onOCMSetDateTimeNone(HTREEITEM hDateTime);
	uint32_t onOCMGetDateTime(HTREEITEM hDateTime, BOOL* pbNone);
	void onOCMSetDateTime(HTREEITEM hDateTime, uint32_t dwTimestamp);
	COLORREF onOCMGetItemColor(HTREEITEM hColor);
	void onOCMSetItemColor(HTREEITEM hColor, COLORREF crColor);

private:
	void insertItem(Item* pParent, Item* pItem, const wchar_t* szNodeText, uint32_t dwFlags, int iImage);
	void setModified(Item* pItem, bool bModified = true);
	bool isItemValid(HTREEITEM hItem);
	Item* getItem(HTREEITEM hItem);
	void setItem(HTREEITEM hItem, Item* pItem);
	void setStateImage(HTREEITEM hItem, int iImage);
	void setNodeText(HTREEITEM hItem, const wchar_t* szNodeText);
	bool getItemFreeRect(HTREEITEM hItem, RECT& outRect);
};

#endif // HISTORYSTATS_GUARD_OPTIONSCTRLIMPL_H
