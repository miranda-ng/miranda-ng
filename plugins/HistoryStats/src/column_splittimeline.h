#if !defined(HISTORYSTATS_GUARD_COLUMN_SPLITTIMELINE_H)
#define HISTORYSTATS_GUARD_COLUMN_SPLITTIMELINE_H

#include "column.h"

/*
 * ColSplitTimeline
 */

class ColSplitTimeline
	: public Column
{
public:
	typedef std::map<int, InOut> TimelineMap;

	struct SplitParams {
		int alignment; // 0 = day, 1 = week
		int columns_to_group;
		int hours_in_block;
		int blocks_in_column;
		int effective_vis_mode;
	};

private:
	int m_nSource; // 0 = characters, 1 = messages, 2 = chats
	int m_nSourceType; // 0 = in, 1 = out, 2 = total, 3 = ratio
	int m_nIgnoreOld;
	int m_nVisMode; // 0 = hours of day, 1 = days of week, 2 = custom
	int m_nHODGroup;
	int m_nDOWGroup;
	int m_nBlockUnit; // 0 = hours, 1 = days, 2 = weeks
	int m_nUnitsPerBlock;
	int m_nBlocks;
	int m_nGraphAlign; // 0 = day boundary, 1 = week boundary
	int m_nCustomGroup;
	bool m_bTopPerColumn;

	OptionsCtrl::Combo m_hSource;
	OptionsCtrl::Edit  m_hIgnoreOld;
	OptionsCtrl::Radio m_hVisMode;
	OptionsCtrl::Edit  m_hHODGroup;
	OptionsCtrl::Edit  m_hDOWGroup;
	OptionsCtrl::Combo m_hBlockUnit;
	OptionsCtrl::Edit  m_hUnitsPerBlock;
	OptionsCtrl::Edit  m_hBlocks;
	OptionsCtrl::Radio m_hGraphAlign;
	OptionsCtrl::Edit  m_hCustomGroup;
	OptionsCtrl::Check m_hTopPerColumn;

	uint32_t m_nTimeDiv;
	uint32_t m_nTimeOffset;
	int m_nTimelineWidth;
	int m_nBlockOffset;
	int m_nNumBlocks;

private:
	int getValue(const InOut& value)
	{
		switch (m_nSourceType)
		{
			case 0: return value.in;
			case 1: return value.out;
			case 2: return value.total();
		}

		return 0;
	}

	SplitParams getParams() const;
	void addToSlot(CContact& contact, bool bOutgoing, uint32_t localTimestamp, int toAdd);

	void outputRenderRowInOut(ext::ostream& tos, const CContact& contact, DisplayType display);
	void outputRenderRowRatio(ext::ostream& tos, const CContact& contact, DisplayType display);

public:
	explicit ColSplitTimeline();

protected:
	virtual const wchar_t* impl_getUID() const { return con::ColSplitTimeline; }
	virtual const wchar_t* impl_getTitle() const { return LPGENW("\"Split\" timeline"); }
	virtual const wchar_t* impl_getDescription() const { return LPGENW("Column holding a graphical overview of your chatting behavior (out, in, total, in/out ratio) from the first to the last day of your history. The information is spread along x- and y-axis and the values are encoded as color values. Different chatting behavior measures are available."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig | cfAcquiresData; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string*) const { return crPNGFull; }
	virtual ext::string impl_contactDataGetUID() const;
	virtual void impl_contactDataBeginAcquire();
	virtual void impl_contactDataPrepare(CContact& contact) const;
	virtual void impl_contactDataFree(CContact& contact) const;
	virtual void impl_contactDataAcquireMessage(CContact& contact, Message& msg);
	virtual void impl_contactDataAcquireChat(CContact& contact, bool bOutgoing, uint32_t localTimestampStarted, uint32_t duration);
	virtual void impl_contactDataMerge(CContact& contact, const CContact& include) const;
	virtual void impl_columnDataAfterOmit();
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_SPLITTIMELINE_H