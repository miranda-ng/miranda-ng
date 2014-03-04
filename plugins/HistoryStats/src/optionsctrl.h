#if !defined(HISTORYSTATS_GUARD_OPTIONSCTRL_H)
#define HISTORYSTATS_GUARD_OPTIONSCTRL_H

#include "_globals.h"
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
	explicit OptionsCtrl(HWND hOptWnd = NULL) : m_hOptWnd(hOptWnd) { }
	~OptionsCtrl() { }

public:
	const OptionsCtrl& operator <<(HWND hOptWnd) { m_hOptWnd = hOptWnd; return *this; }
	operator HWND() { return m_hOptWnd; }

public:
	HANDLE insertGroup(HANDLE hParent, const mu_text* szLabel, DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertCheck(HANDLE hParent, const mu_text* szLabel, DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertRadio(HANDLE hParent, HANDLE hSibling, const mu_text* szLabel, DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertEdit(HANDLE hParent, const mu_text* szLabel, const mu_text* szEdit = muT(""), DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertCombo(HANDLE hParent, const mu_text* szLabel, DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertButton(HANDLE hParent, const mu_text* szLabel, const mu_text* szButton, DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertDateTime(HANDLE hParent, const mu_text* szLabel, DWORD dwDateTime, const mu_text* szFormat = muT("%Y-%m-%d"), DWORD dwFlags = 0, DWORD dwData = 0);
	HANDLE insertColor(HANDLE hParent, const mu_text* szLabel, COLORREF crColor = 0, DWORD dwFlags = 0, DWORD dwData = 0);
	const mu_text* getItemLabel(HANDLE hItem);
	void setItemLabel(HANDLE hItem, const mu_text* szLabel);
	bool isItemEnabled(HANDLE hItem);
	void enableItem(HANDLE hItem, bool bEnable);
    DWORD getItemData(HANDLE hItem);
	void setItemData(HANDLE hItem, DWORD dwData);
	bool isItemChecked(HANDLE hItem);
	void checkItem(HANDLE hItem, bool bCheck);
	int getRadioChecked(HANDLE hRadio);
	void setRadioChecked(HANDLE hRadio, int nCheck);
    int getEditNumber(HANDLE hEdit);
	void setEditNumber(HANDLE hEdit, int nNumber);
	const mu_text* getEditString(HANDLE hEdit);
	void setEditString(HANDLE hEdit, const mu_text* szString);
    void addComboItem(HANDLE hCombo, const mu_text* szItem);
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
	DWORD getDateTime(HANDLE hDateTime, bool* pbNone = NULL);
	void setDateTime(HANDLE hDateTime, DWORD dwDateTime);
	ext::string getDateTimeStr(HANDLE hDateTime);
	void setDateTimeStr(HANDLE hDateTime, const ext::string& strDateTime);
	COLORREF getColor(HANDLE hColor);
	void setColor(HANDLE hColor, COLORREF crColor);
};

#endif // HISTORYSTATS_GUARD_OPTIONSCTRL_H
