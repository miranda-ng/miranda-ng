#include <Matcher.h>
#include <Pattern.h>

const int Matcher::MATCH_ENTIRE_STRING = 0x01;

/*
  Detailed documentation is provided in this class' header file

  @author   Jeffery Stuart
  @since    November 2004
  @version  1.07.00
*/

Matcher::Matcher(Pattern * pattern, const bkstring & text)
{
  pat = pattern;
  str = &text;
  gc = pattern->groupCount;
  ncgc = -pattern->nonCapGroupCount;
  flags = 0;
  matchedSomething = false;
  starts        = new int[gc + ncgc];
  ends          = new int[gc + ncgc];
  groups        = new int[gc + ncgc];
  groupPos      = new int[gc + ncgc];
  groupIndeces  = new int[gc + ncgc];
  starts        = starts        + ncgc;
  ends          = ends          + ncgc;
  groups        = groups        + ncgc;
  groupPos      = groupPos      + ncgc;
  groupIndeces  = groupIndeces  + ncgc;
  for (int i = 0; i < gc; ++i) starts[i] = ends[i] = 0;
}
Matcher::~Matcher()
{
  delete [] (starts       - ncgc);
  delete [] (ends         - ncgc);
  delete [] (groups       - ncgc);
  delete [] (groupIndeces - ncgc);
  delete [] (groupPos     - ncgc);
}
void Matcher::clearGroups()
{
  int i;
  lm = 0;
  for (i = 0; i < gc; ++i)    groups[i] = starts[i] = ends[i] = -1;
  for (i = 1; i <= ncgc; ++i) groups[0 - i] = -1;
}
bkstring Matcher::replaceWithGroups(const bkstring & str)
{
  bkstring ret = "";

  bkstring t = str;
  while (t.size() > 0)
  {
    if (t[0] == '\\')
    {
      t.erase(0, 1);
      if (t.size() == 0)
      {
        ret += "\\";
      }
      else if (t[0] < '0' || t[0] > '9')
      {
        ret += t[0];
        t.erase(0, 1);
      }
      else
      {
        int gn = 0;
        while (t.size() > 0 && t[0] >= '0' && t[0] <= '9')
        {
          gn = gn * 10 + (t[0] - '0');
          t.erase(0, 1);
        }
        ret += getGroup(gn);
      }
    }
    else
    {
        ret += t[0];
        t.erase(0, 1);
    }
  }

  return ret;
}
unsigned long Matcher::getFlags() const
{
  return flags;
}
const bkstring& Matcher::getText() const
{
  return *str;
}

bool Matcher::matches()
{
  flags = MATCH_ENTIRE_STRING;
  matchedSomething = false;
  clearGroups();
  lm = 0;
  return pat->head->match(*str, this, 0) == (int)str->size();
}
bool Matcher::findFirstMatch()
{
  starts[0] = 0;
  flags = 0;
  clearGroups();
  start = 0;
  lm = 0;
  ends[0] = pat->head->match(*str, this, 0);
  if (ends[0] >= 0)
  {
    matchedSomething = true;
    return 1;
  }
  return 0;
}
bool Matcher::findNextMatch()
{
  int s = starts[0], e = ends[0];

  if (!matchedSomething) return findFirstMatch();
  if (s == e) ++e;
  flags = 0;
  clearGroups();

  starts[0] = e;
  if (e >= (int)str->size()) return 0;
  start = e;
  lm = e;
  ends[0] = pat->head->match(*str, this, e);
  return ends[0] >= 0;
}
std::vector<bkstring> Matcher::findAll()
{
  std::vector<bkstring> ret;
  reset();
  while (findNextMatch())
  {
    ret.push_back(getGroup());
  }
  return ret;
}

void Matcher::reset()
{
  lm = 0;
  clearGroups();
  matchedSomething = false;
}

int Matcher::getStartingIndex(const int groupNum) const
{
  if (groupNum < 0 || groupNum >= gc) return -1;
  return starts[groupNum];
}
int Matcher::getEndingIndex(const int groupNum) const
{
  if (groupNum < 0 || groupNum >= gc) return -1;
  return ends[groupNum];
}
bkstring Matcher::getGroup(const int groupNum) const
{
  if (groupNum < 0 || groupNum >= gc) return "";
  if (starts[groupNum] < 0 || ends[groupNum] < 0) return "";
  return str->substr(starts[groupNum], ends[groupNum] - starts[groupNum]);
}
std::vector<bkstring> Matcher::getGroups(const bool includeGroupZero) const
{
  int i, start = (includeGroupZero ? 0 : 1);
  std::vector<bkstring> ret;

  for (i = start; i < gc; ++i) ret.push_back(getGroup(i));
  return ret;
}

