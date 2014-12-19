#if !defined(HISTORYSTATS_GUARD_COLUMN_SPLIT_H)
#define HISTORYSTATS_GUARD_COLUMN_SPLIT_H

#include "column.h"

/*
 * ColSplit
 */

class ColSplit
	: public Column
{
public:
	struct SplitParams {
		int alignment; // 0 = day, 1 = week
		int hours_in_block;
		int blocks_in_column;
		int effective_vis_mode;
	};

private:
	int m_nSource; // 0 = characters, 1 = messages, 2 = chats
	int m_nSourceType; // 0 = in, 1 = out, 2 = total
	int m_nVisMode; // 0 = hours of day, 1 = days of week, 2 = custom
	bool m_bDetail;
	int m_nBlockUnit; // 0 = hours, 1 = days, 2 = weeks
	int m_nUnitsPerBlock;
	int m_nBlocks;
	int m_nGraphAlign; // 0 = day boundary, 1 = week boundary

	OptionsCtrl::Combo m_hSource;
	OptionsCtrl::Radio m_hVisMode;
	OptionsCtrl::Check m_hDetail;
	OptionsCtrl::Combo m_hBlockUnit;
	OptionsCtrl::Edit  m_hUnitsPerBlock;
	OptionsCtrl::Edit  m_hBlocks;
	OptionsCtrl::Radio m_hGraphAlign;

	ext::string m_CSS;
	DWORD m_nTimeDiv;
	DWORD m_nTimeMod;
	DWORD m_nTimeOffset;

private:
	SplitParams getParams() const;
	void addToSlot(Contact& contact, DWORD localTimestamp, int toAdd);

public:
	explicit ColSplit();

protected:
	virtual const TCHAR* impl_getUID() const { return con::ColSplit; }
	virtual const TCHAR* impl_getTitle() const { return LPGENT("\"Split\""); }
	virtual const TCHAR* impl_getDescription() const { return LPGENT("Column holding a graphical overview of your chatting amount split by day of week or by hour of day. Different chatting amount measures are available."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig | cfAcquiresData; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string* pDetails) const;
	virtual ext::string impl_contactDataGetUID() const;
	virtual void impl_contactDataBeginAcquire();
	virtual void impl_contactDataPrepare(Contact& contact) const;
	virtual void impl_contactDataFree(Contact& contact) const;
	virtual void impl_contactDataAcquireMessage(Contact& contact, Message& msg);
	virtual void impl_contactDataAcquireChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration);
	virtual void impl_contactDataMerge(Contact& contact, const Contact& include) const;
	virtual StyleList impl_outputGetAdditionalStyles(IDProvider& idp);
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const Contact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_SPLIT_H