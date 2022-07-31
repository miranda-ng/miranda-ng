#if !defined(HISTORYSTATS_GUARD_COLUMN_TIMELINE_H)
#define HISTORYSTATS_GUARD_COLUMN_TIMELINE_H

#include "column.h"

/*
 * ColTimeline
 */

class ColTimeline
	: public Column
{
public:
	typedef std::map<int, InOut> TimelineMap;

private:
	int m_nSource;
	int m_nSourceType;
	int m_nIgnoreOld;
	bool m_bDetail;
	int m_nDays;

	OptionsCtrl::Combo m_hSource;
	OptionsCtrl::Edit  m_hIgnoreOld;
	OptionsCtrl::Check m_hDetail;
	OptionsCtrl::Edit  m_hDays;

	ext::string m_CSS;
	int m_nTimelineWidth;
	int m_nFirstDay;
	int m_nLastDay;

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

	void addToSlot(CContact& contact, bool bOutgoing, uint32_t localTimestamp, int toAdd);

	void outputRenderRowInOut(ext::ostream& tos, const CContact& contact, DisplayType display);
	void outputRenderRowRatio(ext::ostream& tos, const CContact& contact, DisplayType display);

public:
	explicit ColTimeline();

protected:
	virtual const wchar_t* impl_getUID() const { return con::ColTimeline; }
	virtual const wchar_t* impl_getTitle() const { return LPGENW("Timeline"); }
	virtual const wchar_t* impl_getDescription() const { return LPGENW("Column holding a graphical overview of your chatting behavior (out, in, total, in/out ratio) from the first to the last day of your history on a daily basis. Multiple days can be grouped. Different chatting behavior measures are available."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig | cfAcquiresData; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string* pDetails) const;
	virtual ext::string impl_contactDataGetUID() const;
	virtual void impl_contactDataPrepare(CContact& contact) const;
	virtual void impl_contactDataFree(CContact& contact) const;
	virtual void impl_contactDataAcquireMessage(CContact& contact, Message& msg);
	virtual void impl_contactDataAcquireChat(CContact& contact, bool bOutgoing, uint32_t localTimestampStarted, uint32_t duration);
	virtual void impl_contactDataMerge(CContact& contact, const CContact& include) const;
	virtual void impl_columnDataAfterOmit();
	virtual StyleList impl_outputGetAdditionalStyles(IDProvider& idp);
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_TIMELINE_H