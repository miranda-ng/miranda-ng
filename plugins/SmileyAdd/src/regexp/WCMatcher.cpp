#include "..\general.h"

const int WCMatcher::MATCH_ENTIRE_STRING = 0x01;

/*
  Detailed documentation is provided in this class' header file

  @author   Jeffery Stuart
  @since    November 2004
  @version  1.07.00
  */

WCMatcher::WCMatcher(WCPattern * pattern, const CMString & text)
{
	pat = pattern;
	str = &text;
	gc = pattern->groupCount;
	ncgc = -pattern->nonCapGroupCount;
	flags = 0;
	matchedSomething = false;
	starts = new int[gc + ncgc];
	ends = new int[gc + ncgc];
	groups = new int[gc + ncgc];
	groupPos = new int[gc + ncgc];
	groupIndeces = new int[gc + ncgc];
	starts = starts + ncgc;
	ends = ends + ncgc;
	groups = groups + ncgc;
	groupPos = groupPos + ncgc;
	groupIndeces = groupIndeces + ncgc;
	for (int i = 0; i < gc; ++i) starts[i] = ends[i] = 0;
}
WCMatcher::~WCMatcher()
{
	delete[](starts - ncgc);
	delete[](ends - ncgc);
	delete[](groups - ncgc);
	delete[](groupIndeces - ncgc);
	delete[](groupPos - ncgc);
}
void WCMatcher::clearGroups()
{
	int i;
	lm = 0;
	for (i = 0; i < gc; ++i)    groups[i] = starts[i] = ends[i] = -1;
	for (i = 1; i <= ncgc; ++i) groups[0 - i] = starts[0 - i] = ends[0 - i] = -1;
}
CMString WCMatcher::replaceWithGroups(const CMString & str)
{
	CMString ret;

	CMString t = str;
	while (t.GetLength() > 0) {
		if (t[0] == '\\') {
			t.Delete(0);
			if (t.GetLength() == 0)
				ret += L"\\";
			else if (t[0] < '0' || t[0] > '9') {
				ret += t[0];
				t.Delete(0);
			}
			else {
				int gn = 0;
				while (t.GetLength() > 0 && t[0] >= '0' && t[0] <= '9')
				{
					gn = gn * 10 + (t[0] - '0');
					t.Delete(0);
				}
				ret += getGroup(gn);
			}
		}
		else {
			ret += t[0];
			t.Delete(0);
		}
	}

	return ret;
}
unsigned long WCMatcher::getFlags() const
{
	return flags;
}
const CMString& WCMatcher::getText() const
{
	return *str;
}

bool WCMatcher::matches()
{
	flags = MATCH_ENTIRE_STRING;
	matchedSomething = false;
	clearGroups();
	lm = 0;
	return pat->head->match(*str, this, 0) == str->GetLength();
}

bool WCMatcher::findFirstMatch()
{
	starts[0] = 0;
	flags = 0;
	clearGroups();
	start = 0;
	lm = 0;
	ends[0] = pat->head->match(*str, this, 0);
	if (ends[0] >= 0) {
		matchedSomething = true;
		return 1;
	}
	return 0;
}

bool WCMatcher::findNextMatch()
{
	int s = starts[0], e = ends[0];

	if (!matchedSomething) return findFirstMatch();
	if (s == e) ++e;
	flags = 0;
	clearGroups();

	starts[0] = e;
	if (e >= str->GetLength()) return 0;
	start = e;
	lm = e;
	ends[0] = pat->head->match(*str, this, e);
	return ends[0] >= 0;
}

std::vector<CMString> WCMatcher::findAll()
{
	std::vector<CMString> ret;
	reset();
	while (findNextMatch())
		ret.push_back(getGroup());

	return ret;
}

void WCMatcher::reset()
{
	lm = 0;
	clearGroups();
	matchedSomething = false;
}

int WCMatcher::getStartingIndex(const int groupNum) const
{
	if (groupNum < 0 || groupNum >= gc)
		return -1;
	return starts[groupNum];
}

int WCMatcher::getEndingIndex(const int groupNum) const
{
	if (groupNum < 0 || groupNum >= gc)
		return -1;
	return ends[groupNum];
}

CMString WCMatcher::getGroup(const int groupNum) const
{
	if (groupNum < 0 || groupNum >= gc) return L"";
	if (starts[groupNum] < 0 || ends[groupNum] < 0) return L"";
	return str->Mid(starts[groupNum], ends[groupNum] - starts[groupNum]);
}

std::vector<CMString> WCMatcher::getGroups(const bool includeGroupZero) const
{
	std::vector<CMString> ret;

	for (int i = (includeGroupZero ? 0 : 1); i < gc; ++i)
		ret.push_back(getGroup(i));

	return ret;
}
