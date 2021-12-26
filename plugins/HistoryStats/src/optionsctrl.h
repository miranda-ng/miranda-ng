#if !defined(HISTORYSTATS_GUARD_OPTIONSCTRL_H)
#define HISTORYSTATS_GUARD_OPTIONSCTRL_H

#include "stdafx.h"
#include "optionsctrldefs.h"

/*
 * OptionsCtrl
 */

class OptionsCtrl
	: public OptionsCtrlDefs
	, private pattern::NotCopyable<OptionsCtrl>
{
public:
	typedef HANDLE Item;
	typedef HANDLE Group;
	typedef HANDLE Check;
	typedef HANDLE Radio;
	typedef HANDLE Edit;
	typedef HANDLE Combo;
	typedef HANDLE Button;
	typedef HANDLE DateTime;
	typedef HANDLE Color;

private:
	HWND m_hOptWnd;

private:
	explicit OptionsCtrl(const OptionsCtrl& other); // no implementation
	const OptionsCtrl& operator =(const OptionsCtrl& other); // no implementation

public:
	explicit OptionsCtrl(HWND hOptWnd = nullptr) : m_hOptWnd(hOptWnd) { }
	~OptionsCtrl() { }

public:
	const OptionsCtrl& operator <<(HWND hOptWnd) { m_hOptWnd = hOptWnd; return *this; }
	operator HWND() { return m_hOptWnd; }

public:
	HANDLE insertGroup(HANDLE hParent, const wchar_t* szLabel, uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertCheck(HANDLE hParent, const wchar_t* szLabel, uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertRadio(HANDLE hParent, HANDLE hSibling, const wchar_t* szLabel, uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertEdit(HANDLE hParent, const wchar_t* szLabel, const wchar_t* szEdit = L"", uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertCombo(HANDLE hParent, const wchar_t* szLabel, uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertButton(HANDLE hParent, const wchar_t* szLabel, const wchar_t* szButton, uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertDateTime(HANDLE hParent, const wchar_t* szLabel, uint32_t dwDateTime, const wchar_t* szFormat = L"%Y-%m-%d", uint32_t dwFlags = 0, INT_PTR dwData = 0);
	HANDLE insertColor(HANDLE hParent, const wchar_t* szLabel, COLORREF crColor = 0, uint32_t dwFlags = 0, INT_PTR dwData = 0);
	const wchar_t* getItemLabel(HANDLE hItem);
	void setItemLabel(HANDLE hItem, const wchar_t* szLabel);
	bool isItemEnabled(HANDLE hItem);
	void enableItem(HANDLE hItem, bool bEnable);
    uint32_t getItemData(HANDLE hItem);
	void setItemData(HANDLE hItem, INT_PTR dwData);
	bool isItemChecked(HANDLE hItem);
	void checkItem(HANDLE hItem, bool bCheck);
	int getRadioChecked(HANDLE hRadio);
	void setRadioChecked(HANDLE hRadio, int nCheck);
    int getEditNumber(HANDLE hEdit);
	void setEditNumber(HANDLE hEdit, int nNumber);
	const wchar_t* getEditString(HANDLE hEdit);
	void setEditString(HANDLE hEdit, const wchar_t* szString);
    void addComboItem(HANDLE hCombo, const wchar_t* szItem);
	int getComboSelected(HANDLE hCombo);
	void setComboSelected(HANDLE hCombo, int nSelect);
	void ensureVisible(HANDLE hItem);
	void deleteAllItems();
	HANDLE getSelection();
	void selectItem(HANDLE hItem);
	HANDLE getFirstItem();
	HANDLE getNextItem(HANDLE hItem);
	HANDLE getPrevItem(HANDLE hItem);
	void setRedraw(bool bRedraw);
	void deleteItem(HANDLE hItem);
	void moveItem(HANDLE& hItem, HANDLE hInsertAfter);
	int getScrollPos(int nBar);
	void setScrollPos(int nBar, int nPos);
	bool isDateTimeNone(HANDLE hDateTime);
	void setDateTimeNone(HANDLE hDateTime);
	uint32_t getDateTime(HANDLE hDateTime, bool* pbNone = nullptr);
	void setDateTime(HANDLE hDateTime, uint32_t dwDateTime);
	ext::string getDateTimeStr(HANDLE hDateTime);
	void setDateTimeStr(HANDLE hDateTime, const ext::string& strDateTime);
	COLORREF getColor(HANDLE hColor);
	void setColor(HANDLE hColor, COLORREF crColor);
};

#endif // HISTORYSTATS_GUARD_OPTIONSCTRL_H
