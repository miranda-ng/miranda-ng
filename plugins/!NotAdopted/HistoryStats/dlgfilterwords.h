#if !defined(HISTORYSTATS_GUARD_DLGFILTERWORDS_H)
#define HISTORYSTATS_GUARD_DLGFILTERWORDS_H

#include "_globals.h"

#include "bandctrl.h"
#include "optionsctrl.h"
#include "settings.h"

class DlgFilterWords
	: private pattern::NotCopyable<DlgFilterWords>
{
private:
	enum SetAction {
		saAdd         = 0,
		saDel         = 1,
	};

	typedef Settings::WordSet WordSet;
	typedef Settings::Filter Filter;
	typedef Settings::FilterSet FilterSet;
	typedef Settings::ColFilterSet ColFilterSet;
	typedef std::vector<Filter*> FilterVec;

	class FilterCompare
	{
	public:
		bool operator ()(const Filter*& first, const Filter*& second) { return ext::strfunc::icoll(first->getName().c_str(), second->getName().c_str()) < 0; }
	};

private:
	static BOOL CALLBACK staticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWnd;
	BandCtrl m_Band;
	HANDLE m_hActionButtons[2];
	OptionsCtrl m_Sets;
	FilterVec m_Filters;
	bool m_bColProvided;
	ColFilterSet m_ColFilters;

private:
	void onWMInitDialog();
	void onWMDestroy();
	void onBandClicked(HANDLE hButton, DWORD dwData);
	void onSetAdd();
	void onSetDel();
	void onSetItemModified(HANDLE hItem, DWORD dwData);
	void onSetSelChanging(HANDLE hItem, DWORD dwData);
	void onSetSelChanged(HANDLE hItem, DWORD dwData);

	void clearFilters();

public:
    explicit DlgFilterWords();
	~DlgFilterWords();

	bool showModal(HWND hParent);
	void setFilters(const FilterSet& Filters);
	void setColFilters(const ColFilterSet& ColFilters);
	void updateFilters(FilterSet& Filters);
	const ColFilterSet& getColFilters() { return m_ColFilters; }
};

#endif // HISTORYSTATS_GUARD_DLGFILTERWORDS_H