#if !defined(HISTORYSTATS_GUARD_COLBASE_WORDS_H)
#define HISTORYSTATS_GUARD_COLBASE_WORDS_H

#include "column.h"

class ColBaseWords
	: public Column
{
public:
	typedef std::map<ext::string, InOut> WordMap;
	typedef Settings::WordSet WordSet;
	typedef Settings::ColFilterSet ColFilterSet;
	typedef Settings::Filter Filter;
	typedef std::vector<const Filter*> FilterVecC;

protected:
	int m_nSource;
	int m_nMinLength;
	int m_nMaxLength;
	bool m_bFilterLinks;
	ColFilterSet m_FilterWords;

	OptionsCtrl::Radio m_hSource;
	OptionsCtrl::Edit  m_hMinLength;
	OptionsCtrl::Edit  m_hMaxLength;
	OptionsCtrl::Check m_hFilterLinks;

	bool m_bFilterMessages;
	bool m_bFilterWords;
	FilterVecC m_ActiveMessageFilter;
	FilterVecC m_ActiveWordFilter;

private:
	void addWord(WordMap* pWords, const ext::string& word, bool bOutgoing) const;
	void parseMsg(WordMap* pWords, const ext::string& msg, bool bOutgoing) const;

protected:
	explicit ColBaseWords();

protected:
	virtual int impl_getFeatures() const { return cfHasConfig | cfAcquiresData | cfTransformsData | cfIsColBaseWords; }
	virtual void impl_copyConfig(const Column* pSource);
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string* pDetails) const { return crHTMLFull; }
	virtual ext::string impl_contactDataGetUID() const;
	virtual void impl_contactDataBeginAcquire();
	virtual void impl_contactDataPrepare(Contact& contact) const;
	virtual void impl_contactDataFree(Contact& contact) const;
	virtual void impl_contactDataAcquireMessage(Contact& contact, Message& msg);
	virtual void impl_contactDataMerge(Contact& contact, const Contact& include) const;

public:
	const ColFilterSet& getFilterWords() const { return m_FilterWords; }
	void setFilterWords(const ColFilterSet& FilterWords) { m_FilterWords = FilterWords; }
};

#endif // HISTORYSTATS_GUARD_COLBASE_WORDS_H