/**
  From the author (Jeff Stuart)
  "
  Let me start by saying this file is pretty big. If you feel up to it, you can
  try making changes yourself, but you would be better off to just email me at
  stuart@cs.ucdavis.edu if you think there is a bug, or have something useful you
  would like added. This project is very "near and dear" to me, so I am fairly quick
  to make bug fixes. The header files for WCPattern and WCMatcher are fairly well
  documented and the function names are pretty self-explanatory, but if you are having
  any trouble, feel free to email me at stuart@cs.ucdavis.edu.

  If you email me, make sure you put something like C++RE in the subject because
  I tend to delete email if I don't recognize the name and the subject is
  something like "I Need Your Help" or "Got A Second" or "I Found It".
  "
 */

/*
  Detailed documentation is provided in this class' header file

  @author   Jeffery Stuart
  @since    November 2004
  @version  1.07.00
*/

#ifdef _WIN32
  #pragma warning(push)
  #pragma warning(disable:4996)
#endif

#include <WCPattern.h>
#include <WCMatcher.h>
#include <wchar.h>
#include <algorithm>
#ifndef _WIN32
  #include <wctype.h>
#endif

std::map<bkstring, WCPattern *> WCPattern::compiledWCPatterns;
std::map<bkstring, std::pair<bkstring, unsigned long> > WCPattern::registeredWCPatterns;

const int WCPattern::MIN_QMATCH = 0x00000000;
const int WCPattern::MAX_QMATCH = 0x7FFFFFFF;

const unsigned long WCPattern::CASE_INSENSITIVE       = 0x01;
const unsigned long WCPattern::LITERAL                = 0x02;
const unsigned long WCPattern::DOT_MATCHES_ALL        = 0x04;
const unsigned long WCPattern::MULTILINE_MATCHING     = 0x08;
const unsigned long WCPattern::UNIX_LINE_MODE         = 0x10;

#define to_lower(a) (wchar_t)(UINT_PTR)CharLowerW((LPWSTR)(unsigned)a)
#define is_alpha IsCharAlphaW

#if defined(_WIN32)
  #define str_icmp lstrcmpiW
#elif defined(__CYGWIN__) || defined(__APPLE__)
  #include <wctype.h>
  static inline int str_icmp(const wchar_t * a, const wchar_t * b)
  {
    while (*a && *b)
    {
      const int t = (int)towlower(*a) - (int)tolower(*b);
      if (t) return t;
      ++a; ++b;
    }
    if (*a)
    {
      if (*b) return (int)towlower(*a) - (int)tolower(*b);
      return 1;
    }
    else if (*b) return 1;
    return 0;
  }
#else
  #define str_icmp wcscasecmp
#endif

WCPattern::WCPattern(const bkstring & rhs)
{
  matcher = NULL;
  pattern = rhs;
  curInd = 0;
  groupCount = 0;
  nonCapGroupCount = 0;
  error = 0;
  head = NULL;
}
// convenience function in case we want to add any extra debugging output
void WCPattern::raiseError()
{
/*  switch (pattern[curInd - 1])
  {
  case '*':
  case ')':
  case '+':
  case '?':
  case ']':
  case '}':
    fwprintf(stderr, L"%s\n%*c^\n", pattern.c_str(), curInd - 1, ' ');
    fwprintf(stderr, L"Syntax Error near here. Possible unescaped meta character.\n");
    break;
  default:
    fwprintf(stderr, L"%s\n%*c^\n", pattern.c_str(), curInd - 1, ' ');
    fwprintf(stderr, L"Syntax Error near here. \n");
    break;
  }*/
  error = 1;
}
NFAUNode * WCPattern::registerNode(NFAUNode * node)
{
  nodes[node] = 1;
  return node;
}

bkstring WCPattern::classUnion      (bkstring s1, bkstring s2)  const
{
  wchar_t * out = new wchar_t[66000];
  std::sort(s1.begin(), s1.end());
  std::sort(s2.begin(), s2.end());
  *std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), out) = 0;
  bkstring ret = out;
  delete [] out;
  return ret;
}
bkstring WCPattern::classIntersect  (bkstring s1, bkstring s2)  const
{
  wchar_t * out = new wchar_t[66000];
  std::sort(s1.begin(), s1.end());
  std::sort(s2.begin(), s2.end());
  *std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), out) = 0;
  bkstring ret = out;
  delete [] out;
  return ret;
}
bkstring WCPattern::classNegate     (bkstring s1)                  const
{
  wchar_t * out = new wchar_t[66000];
  int i, ind = 0;
  std::map<wchar_t, bool> m;

  for (i = 0; i < (int)s1.size(); ++i) m[s1[i]] = 1;
  for (i = 0xFF; i >= 0; --i) if (m.find((wchar_t)i) == m.end()) out[ind++] = (wchar_t)i;
  out[ind] = 0;
  bkstring ret(out, ind);
  delete [] out;
  return ret;
}
bkstring WCPattern::classCreateRange(wchar_t low, wchar_t hi)    const
{
  wchar_t out[300];
  int ind = 0;
  while (low != hi) out[ind++] = low++;
  out[ind++] = low;
  return bkstring(out, ind);
}

int WCPattern::getInt(int start, int end)
{
  int ret = 0;
  for (; start <= end; ++start) ret = ret * 10 + (pattern[start] - (wchar_t)'0');
  return ret;
}
bool WCPattern::quantifyCurly(int & sNum, int & eNum)
{
  bool good = 1;
  int i, ci = curInd + 1;
  int commaInd = ci, endInd = ci, len = pattern.size();
  sNum = eNum = 0;

  while (endInd   < len     && pattern[endInd  ] != (wchar_t)'}') ++endInd;
  while (commaInd < endInd  && pattern[commaInd] != (wchar_t)',') ++commaInd;
  if (endInd >= len) { raiseError(); return 0; }
  for (i = ci; good && i < endInd; ++i) if (i != commaInd && !isdigit(pattern[i])) good = 0;
  if (!good && commaInd < endInd) { raiseError(); return 0; }
  if (!good) return 0;
  /* so now everything in here is either a comma (and there is at most one comma) or a digit */
  if (commaInd == ci) // {,*}
  {
    if (endInd == commaInd + 1)    { sNum = MIN_QMATCH;               eNum = MAX_QMATCH;                        } // {,} = *
    else                           { sNum = MIN_QMATCH;               eNum = getInt(commaInd + 1, endInd - 1);  } // {,+}
  }
  else if (commaInd == endInd - 1) { sNum = getInt(ci, commaInd - 1); eNum = MAX_QMATCH;                        } // {+,}
  else if (commaInd == endInd)     { sNum = getInt(ci, endInd - 1);   eNum = sNum;                              } // {+}
  else                             { sNum = getInt(ci, commaInd - 1); eNum = getInt(commaInd + 1, endInd - 1);  } // {+,+}
  curInd = endInd + 1;
  return 1;
}
NFAUNode * WCPattern::quantifyGroup(NFAUNode * start, NFAUNode * stop, const int gn)
{
  NFAUNode * newNode = NULL;
  int type = 0;

  if (curInd < (int)pattern.size())
  {
    wchar_t ch = (curInd + 1 >= (int)pattern.size()) ? USHRT_MAX : pattern[curInd + 1];
    switch (pattern[curInd])
    {
    case (wchar_t)'*':
      ++curInd;
      switch (ch)
      {
      case (wchar_t)'?': ++curInd; type = 1; break;
      case (wchar_t)'+': ++curInd; type = 2; break;
      }
      newNode = registerNode(new NFAGroupLoopPrologueUNode(gn));
      newNode->next = registerNode(new NFAGroupLoopUNode(start, MIN_QMATCH, MAX_QMATCH, gn, type));
      stop->next = newNode->next;
      return newNode;
    case (wchar_t)'?':
      ++curInd;
      switch (ch)
      {
      case (wchar_t)'?': ++curInd; type = 1; break;
      case (wchar_t)'+': ++curInd; type = 2; break;
      }
      newNode = registerNode(new NFAGroupLoopPrologueUNode(gn));
      newNode->next = registerNode(new NFAGroupLoopUNode(start, MIN_QMATCH, 1, gn, type));
      stop->next = newNode->next;
      return newNode;
    case (wchar_t)'+':
      ++curInd;
      switch (ch)
      {
      case (wchar_t)'?': ++curInd; type = 1; break;
      case (wchar_t)'+': ++curInd; type = 2; break;
      }
      newNode = registerNode(new NFAGroupLoopPrologueUNode(gn));
      newNode->next = registerNode(new NFAGroupLoopUNode(start, 1, MAX_QMATCH, gn, type));
      stop->next = newNode->next;
      return newNode;
    case (wchar_t)'{':
      {
        int s, e;
        if (quantifyCurly(s, e))
        {
          ch = (curInd < (int)pattern.size()) ? pattern[curInd] : USHRT_MAX;
          switch (ch)
          {
          case (wchar_t)'?': ++curInd; type = 1; break;
          case (wchar_t)'+': ++curInd; type = 2; break;
          }
          newNode = registerNode(new NFAGroupLoopPrologueUNode(gn));
          newNode->next = registerNode(new NFAGroupLoopUNode(start, s, e, gn, type));
          stop->next = newNode->next;
          return newNode;
        }
      }
    default:
      break;
    }
  }
  return NULL;
}

NFAUNode * WCPattern::quantify(NFAUNode * newNode)
{
  if (curInd < (int)pattern.size())
  {
    wchar_t ch = (curInd + 1 >= (int)pattern.size()) ? USHRT_MAX : pattern[curInd + 1];
    switch (pattern[curInd])
    {
    case (wchar_t)'*':
      ++curInd;
      switch (ch)
      {
      case (wchar_t)'?': ++curInd; newNode = registerNode(new NFALazyQuantifierUNode      (this, newNode, MIN_QMATCH, MAX_QMATCH)); break;
      case (wchar_t)'+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierUNode(this, newNode, MIN_QMATCH, MAX_QMATCH)); break;
      default:            newNode = registerNode(new NFAGreedyQuantifierUNode    (this, newNode, MIN_QMATCH, MAX_QMATCH)); break;
      }
      break;
    case (wchar_t)'?':
      ++curInd;
      switch (ch)
      {
      case (wchar_t)'?': ++curInd; newNode = registerNode(new NFALazyQuantifierUNode      (this, newNode, MIN_QMATCH, 1)); break;
      case (wchar_t)'+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierUNode(this, newNode, MIN_QMATCH, 1)); break;
      default:            newNode = registerNode(new NFAGreedyQuantifierUNode    (this, newNode, MIN_QMATCH, 1)); break;
      }
      break;
    case (wchar_t)'+':
      ++curInd;
      switch (ch)
      {
      case (wchar_t)'?': ++curInd; newNode = registerNode(new NFALazyQuantifierUNode      (this, newNode, 1, MAX_QMATCH)); break;
      case (wchar_t)'+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierUNode(this, newNode, 1, MAX_QMATCH)); break;
      default:            newNode = registerNode(new NFAGreedyQuantifierUNode    (this, newNode, 1, MAX_QMATCH)); break;
      }
      break;
    case (wchar_t)'{':
      {
        int s, e;
        if (quantifyCurly(s, e))
        {
          ch = (curInd < (int)pattern.size()) ? pattern[curInd] : USHRT_MAX;
          switch (ch)
          {
          case (wchar_t)'?': ++curInd; newNode = registerNode(new NFALazyQuantifierUNode      (this, newNode, s, e)); break;
          case (wchar_t)'+': ++curInd; newNode = registerNode(new NFAPossessiveQuantifierUNode(this, newNode, s, e)); break;
          default:            newNode = registerNode(new NFAGreedyQuantifierUNode    (this, newNode, s, e)); break;
          }
        }
      }
      break;
    default:
      break;
    }
  }
  return newNode;
}
bkstring WCPattern::parseClass()
{
  bkstring t, ret = L"";
  wchar_t ch, c1, c2;
  bool inv = 0, neg = 0, quo = 0;

  if (curInd < (int)pattern.size() && pattern[curInd] == (wchar_t)'^')
  {
    ++curInd;
    neg = 1;
  }
  while (curInd < (int)pattern.size() && pattern[curInd] != (wchar_t)']')
  {
    ch = pattern[curInd++];
    if (ch == (wchar_t)'[')
    {
      t = parseClass();
      ret = classUnion(ret, t);
    }
    /*else if (ch == (wchar_t)'-')
    {
      raiseError();
      curInd = pattern.size();
    }*/
    else if (ch == (wchar_t)'&' && curInd < (int)pattern.size() && pattern[curInd] == (wchar_t)'&')
    {
      if (pattern[++curInd] != (wchar_t)'[')
      {
        raiseError();
        curInd = pattern.size();
      }
      else
      {
        ++curInd;
        t = parseClass();
        ret = classIntersect(ret, t);
      }
    }
    else if (ch == (wchar_t)'\\')
    {
      t = parseEscape(inv, quo);
      if (quo)
      {
        raiseError();
        curInd = pattern.size();
      }
      else if (inv || t.size() > 1) // cant be part of a range (a-z)
      {
        if (inv) t = classNegate(t);
        ret = classUnion(ret, t);
      }
      else if (curInd < (int)pattern.size() && pattern[curInd] == (wchar_t)'-') // part of a range (a-z)
      {
        c1 = t[0];
        ++curInd;
        if (curInd >= (int)pattern.size()) raiseError();
        else
        {
          c2 = pattern[curInd++];
          if (c2 == (wchar_t)'\\')
          {
            t = parseEscape(inv, quo);
            if (quo)
            {
              raiseError();
              curInd = pattern.size();
            }
            else if (inv || t.size() > 1) raiseError();
            else ret = classUnion(ret, classCreateRange(c1, c2));
          }
          else if (c2 == (wchar_t)'[' || c2 == (wchar_t)']' || c2 == (wchar_t)'-' || c2 == (wchar_t)'&')
          {
            raiseError();
            curInd = pattern.size();
          }
          else ret = classUnion(ret, classCreateRange(c1, c2));
        }
      }
      else
      {
        ret = classUnion(ret, t);
      }
    }
    else if (curInd < (int)pattern.size() && pattern[curInd] == (wchar_t)'-')
    {
      c1 = ch;
      ++curInd;
      if (curInd >= (int)pattern.size()) raiseError();
      else
      {
        c2 = pattern[curInd++];
        if (c2 == (wchar_t)'\\')
        {
          t = parseEscape(inv, quo);
          if (quo)
          {
            raiseError();
            curInd = pattern.size();
          }
          else if (inv || t.size() > 1) raiseError();
          else ret = classUnion(ret, classCreateRange(c1, c2));
        }
        else if (c2 == (wchar_t)'[' || c2 == (wchar_t)']' || c2 == (wchar_t)'-' || c2 == (wchar_t)'&')
        {
          raiseError();
          curInd = pattern.size();
        }
        else
        {
          ret = classUnion(ret, classCreateRange(c1, c2));
        }
      }
    }
    else
    {
      ret += L" ";
      ret[ret.size() - 1] = ch;
    }
  }
  if (curInd >= (int)pattern.size() || pattern[curInd] != (wchar_t)']')
  {
    raiseError();
    ret = L"";
  }
  else
  {
    ++curInd;
    if (neg) ret = classNegate(ret);
  }
  return ret;
}
bkstring WCPattern::parsePosix()
{
  bkstring s7 = pattern.substr(curInd, 7);
  if (s7 == L"{Lower}") { curInd += 7; return L"abcdefghijklmnopqrstuvwxyz";                                                                       }
  if (s7 == L"{Upper}") { curInd += 7; return L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";                                                                       }
  if (s7 == L"{Alpha}") { curInd += 7; return L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";                                             }
  if (s7 == L"{Digit}") { curInd += 7; return L"0123456789";                                                                                       }
  if (s7 == L"{Alnum}") { curInd += 7; return L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";                                   }
  if (s7 == L"{Punct}") { curInd += 7; return L"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";                                                               }
  if (s7 == L"{Graph}") { curInd += 7; return L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; }
  if (s7 == L"{Print}") { curInd += 7; return L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"; }
  if (s7 == L"{Blank}") { curInd += 7; return L" \t";                                                                                              }
  if (s7 == L"{Space}") { curInd += 7; return L" \t\n\x0B\f\r";                                                                                    }
  if (s7 == L"{Cntrl}")
  {
	bkstring::value_type i;
    bkstring s = L" ";

    for (i = 0; i < 5; ++i) s += s;
    s += L" ";
    for (i = 0; i <= 0x1F; ++i) s[i] = i;
    s[0x20] = 0x7F;
    curInd += 7;
    return s;
  }
  if (s7 == L"{ASCII}")
  {
    bkstring s(0x80, (wchar_t)' ');
    for (bkstring::value_type i = 0; i <= 0x7f; ++i) s[i] = i;
    curInd += 7;
    return s;
  }
  if (pattern.substr(curInd, 8) == L"{XDigit}") { curInd += 8; return L"abcdefABCDEF0123456789"; }
  raiseError();
  return L"";
}
NFAUNode * WCPattern::parseBackref()
{
  #define is_dig(x) ((x) >= (wchar_t)'0' && (x) <= (wchar_t)'9')
  #define to_int(x) ((x) - (wchar_t)'0')
  int ci = curInd;
  int oldRef = 0, ref = 0;

  while (ci < (int)pattern.size() && is_dig(pattern[ci]) && (ref < 10 || ref < groupCount))
  {
    oldRef = ref;
    ref = ref * 10 + to_int(pattern[ci++]);
  }
  if (ci == (int)pattern.size())
  {
    oldRef = ref;
    ++ci;
  }
  if (oldRef < 0 || ci <= curInd)
  {
    raiseError();
    return registerNode(new NFAReferenceUNode(-1));
  }
  curInd = ci;
  return registerNode(new NFAReferenceUNode(ref));

  #undef is_dig
  #undef to_int
}
bkstring WCPattern::parseOctal()
{
  #define islowoc(x)  ((x) >= (wchar_t)'0' && (x) <= (wchar_t)'3')
  #define isoc(x)     ((x) >= (wchar_t)'0' && (x) <= (wchar_t)'7')
  #define fromoc(x)   ((x) - (wchar_t)'0')
  int ci = curInd;
  wchar_t ch1 = (ci + 0 < (int)pattern.size()) ? pattern[ci + 0] : USHRT_MAX;
  wchar_t ch2 = (ci + 1 < (int)pattern.size()) ? pattern[ci + 1] : USHRT_MAX;
  wchar_t ch3 = (ci + 2 < (int)pattern.size()) ? pattern[ci + 2] : USHRT_MAX;
  bkstring s = L" ";

  if (islowoc(ch1) && isoc(ch2))
  {
    curInd += 2;
    s[0] = fromoc(ch1) * 8 + fromoc(ch2);
    if (isoc(ch3))
    {
      ++curInd;
      s[0] = s[0] * 8 + fromoc(ch3);
    }
  }
  else if (isoc(ch1) && isoc(ch2))
  {
    curInd += 2;
    s[0] = fromoc(ch1) * 8 + fromoc(ch2);
  }
  else raiseError();

  return s;
  #undef islowoc
  #undef isoc
  #undef fromoc
}
bkstring WCPattern::parseHex()
{
  #define to_low(x)   (((x) >= (wchar_t)'A' && (x) <= (wchar_t)'Z') ? ((x) - (wchar_t)'A' + (wchar_t)'a') : (x))
  #define is_dig(x)   ((x) >= (wchar_t)'0' && (x) <= (wchar_t)'9')
  #define is_hex(x)   (is_dig(x) || (to_low(x) >= (wchar_t)'a' && to_low(x) <= (wchar_t)'f'))
  #define to_int(x)   ((is_dig(x)) ? ((x) - (wchar_t)'0') : (to_low(x) - (wchar_t)'a' + 10))

  int ci = curInd;
  wchar_t ch1 = (ci + 0 < (int)pattern.size()) ? pattern[ci + 0] : USHRT_MAX;
  wchar_t ch2 = (ci + 1 < (int)pattern.size()) ? pattern[ci + 1] : USHRT_MAX;
  wchar_t ch3 = (ci + 2 < (int)pattern.size()) ? pattern[ci + 2] : USHRT_MAX;
  wchar_t ch4 = (ci + 3 < (int)pattern.size()) ? pattern[ci + 3] : USHRT_MAX;
  bkstring s = L" ";

  if (is_hex(ch1) && is_hex(ch2) && is_hex(ch3) && is_hex(ch4))
  {
    curInd += 2;
    s[0] = (to_int(ch1) << 12 & 0xF000) | (to_int(ch2) << 8 & 0x0F00) |
           (to_int(ch3) <<  4 & 0x0F00) | (to_int(ch4)      & 0x000F);
  }
  else if (is_hex(ch1) && is_hex(ch2))
  {
    curInd += 2;
    s[0] = (to_int(ch1) << 4 & 0xF0) | (to_int(ch2) & 0x0F);
  }

  return s;
  #undef to_low
  #undef is_dig
  #undef is_hex
  #undef to_int
}
bkstring WCPattern::parseEscape(bool & inv, bool & quo)
{
  wchar_t ch = pattern[curInd++];
  bkstring classes = L"";

  if (curInd > (int)pattern.size())
  {
    raiseError();
    return NULL;
  }

  quo = 0;
  inv = 0;
  switch (ch)
  {
  case (wchar_t)'p': classes = parsePosix();                                                         break;
  case (wchar_t)'P': classes = L"!!"; classes += parsePosix();                                        break;
  case (wchar_t)'d': classes = L"0123456789";                                                         break;
  case (wchar_t)'D': classes = L"!!0123456789";                                                       break;
  case (wchar_t)'s': classes = L" \t\r\n\f";                                                          break;
  case (wchar_t)'S': classes = L"!! \t\r\n\f";                                                        break;
  case (wchar_t)'w': classes = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";    break;
  case (wchar_t)'W': classes = L"!!abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";  break;
  case (wchar_t)'0': classes = parseOctal(); break;
  case (wchar_t)'x': classes = parseHex();   break;

  case (wchar_t)'Q': quo = 1;        break;
  case (wchar_t)'t': classes = L"\t"; break;
  case (wchar_t)'r': classes = L"\r"; break;
  case (wchar_t)'n': classes = L"\n"; break;
  case (wchar_t)'f': classes = L"\f"; break;
  case (wchar_t)'a': classes = L"\a"; break;
  case (wchar_t)'e': classes = L"\r"; break;
  default:  classes = L" "; classes[0] = ch; break;
  }
  if (classes.substr(0, 2) == L"!!")
  {
    classes = classes.substr(2);
    inv = 1;
  }
  return classes;
}
NFAUNode * WCPattern::parseRegisteredWCPattern(NFAUNode ** end)
{
  int i, j;
  bkstring s;
  NFAUNode * ret = NULL;
  for (i = curInd; i < (int)pattern.size() && pattern[i] != (wchar_t)'}'; ++i) { }
  if (pattern[i] != (wchar_t)'}') { raiseError(); return NULL; }
  if (i == curInd + 1)   { raiseError(); return NULL; } // {}
  if (
      !(
        (pattern[curInd] >= (wchar_t)'a' && pattern[curInd] <= (wchar_t)'z') ||
        (pattern[curInd] >= (wchar_t)'A' && pattern[curInd] <= (wchar_t)'Z') ||
        (pattern[curInd] == (wchar_t)'_')
       )
     )
  {
    raiseError();
    return NULL;
  }
  for (j = curInd; !error && j < i; ++j)
  {
    if (
        !(
          (pattern[j] >= (wchar_t)'a' && pattern[j] <= (wchar_t)'z') ||
          (pattern[j] >= (wchar_t)'A' && pattern[j] <= (wchar_t)'Z') ||
          (pattern[j] >= (wchar_t)'0' && pattern[j] <= (wchar_t)'9') ||
          (pattern[j] == (wchar_t)'_')
         )
        )
    {
      raiseError();
      return NULL;
    }
  }
  s = pattern.substr(curInd, i - curInd);
  if (registeredWCPatterns.find(s) == registeredWCPatterns.end()) raiseError();
  else
  {
    unsigned long oflags = flags;
    bkstring op = pattern;
    int ci = i + 1;

    pattern = registeredWCPatterns[s].first;
    curInd = 0;
    flags = registeredWCPatterns[s].second;

    --groupCount;
    ret = parse(0, 0, end);

    pattern = op;
    curInd = ci;
    flags = oflags;
  }
  if (error) { *end = ret = NULL; }
  return ret;
}

// look behind should interpret everything as a literal (except \\) since the
// pattern must have a concrete length
NFAUNode * WCPattern::parseBehind(const bool pos, NFAUNode ** end)
{
  bkstring t = L"";
  while (curInd < (int)pattern.size() && pattern[curInd] != (wchar_t)')')
  {
    wchar_t ch = pattern[curInd++];
    t += L" ";
    if (ch == (wchar_t)'\\')
    {
      if (curInd + 1 >= (int)pattern.size())
      {
        raiseError();
        return *end = registerNode(new NFACharUNode((wchar_t)' '));
      }
      ch = pattern[curInd++];
    }
    t[t.size() - 1] = ch;
  }
  if (curInd >= (int)pattern.size() || pattern[curInd] != (wchar_t)')') raiseError();
  else ++curInd;
  return *end = registerNode(new NFALookBehindUNode(t, pos));
}
NFAUNode * WCPattern::parseQuote()
{
  bool done = 0;
  bkstring s = L"";

  while (!done)
  {
    if      (curInd >= (int)pattern.size())
    {
      raiseError();
      done = 1;
    }
    else if (pattern.substr(curInd, 2) == L"\\E")
    {
      curInd += 2;
      done = 1;
    }
    else if (pattern[curInd] == (wchar_t)'\\')
    {
      s += L" ";
      s[s.size() - 1] = pattern[++curInd];
      ++curInd;
    }
    else
    {
      s += L" ";
      s[s.size() - 1] = pattern[curInd++];
    }
  }
  if ((flags & WCPattern::CASE_INSENSITIVE) != 0) return registerNode(new NFACIQuoteUNode(s));
  return registerNode(new NFAQuoteUNode(s));
}
NFAUNode * WCPattern::parse(const bool inParen, const bool inOr, NFAUNode ** end)
{
  NFAUNode * start, * cur, * next = NULL;
  bkstring t;
  int grc = groupCount++;
  bool inv, quo;
  bool ahead = 0, pos = 0, noncap = 0, indep = 0;
  unsigned long oldFlags = flags;

  if (inParen)
  {
    if (pattern[curInd] == (wchar_t)'?')
    {
      ++curInd;
      --groupCount;
      if      (pattern[curInd]           == (wchar_t)':')   { noncap = 1; ++curInd;     grc = --nonCapGroupCount; }
      else if (pattern[curInd]           == (wchar_t)'=')   { ++curInd;     ahead = 1;  pos = 1;                  }
      else if (pattern[curInd]           == (wchar_t)'!')   { ++curInd;     ahead = 1;  pos = 0;                  }
      else if (pattern.substr(curInd, 2) == L"<=")  { curInd += 2;  return parseBehind(1, end);                   }
      else if (pattern.substr(curInd, 2) == L"<!")  { curInd += 2;  return parseBehind(0, end);                   }
      else if (pattern[curInd]           == (wchar_t)'>')   { ++curInd;     indep = 1;                            }
      else
      {
        bool negate = false, done = false;
        while (!done)
        {
          if (curInd >= (int)pattern.size())
          {
            raiseError();
            return NULL;
          }
          else if (negate)
          {
            switch (pattern[curInd])
            {
            case (wchar_t)'i': flags &= ~WCPattern::CASE_INSENSITIVE;   break;
            case (wchar_t)'d': flags &= ~WCPattern::UNIX_LINE_MODE;     break;
            case (wchar_t)'m': flags &= ~WCPattern::MULTILINE_MATCHING; break;
            case (wchar_t)'s': flags &= ~WCPattern::DOT_MATCHES_ALL;    break;
            case (wchar_t)':': done = true;                             break;
            case (wchar_t)')':
              ++curInd;
              *end = registerNode(new NFALookBehindUNode(L"", true));
              return *end;
            case (wchar_t)'-':
            default: raiseError(); return NULL;
            }
          }
          else
          {
            switch (pattern[curInd])
            {
            case (wchar_t)'i': flags |= WCPattern::CASE_INSENSITIVE;    break;
            case (wchar_t)'d': flags |= WCPattern::UNIX_LINE_MODE;      break;
            case (wchar_t)'m': flags |= WCPattern::MULTILINE_MATCHING;  break;
            case (wchar_t)'s': flags |= WCPattern::DOT_MATCHES_ALL;     break;
            case (wchar_t)':': done = true;                             break;
            case (wchar_t)'-': negate = true;                           break;
            case (wchar_t)')':
              ++curInd;
              *end = registerNode(new NFALookBehindUNode(L"", true));
              return *end;
            default:  raiseError(); return NULL;
            }
          }
          ++curInd;
        }
        noncap = 1;
        grc = --nonCapGroupCount;
      }

      if (noncap) cur = start = registerNode(new NFAGroupHeadUNode(grc));
      else        cur = start = registerNode(new NFASubStartUNode);
    }
    else cur = start = registerNode(new NFAGroupHeadUNode(grc));
  }
  else cur = start = registerNode(new NFASubStartUNode);
  while (curInd < (int)pattern.size())
  {
    wchar_t ch = pattern[curInd++];

    next = NULL;
    if (error) return NULL;
    switch (ch)
    {
    case (wchar_t)'^':
      if ((flags & WCPattern::MULTILINE_MATCHING) != 0) next = registerNode(new NFAStartOfLineUNode);
      else                                            next = registerNode(new NFAStartOfInputUNode);
      break;
    case (wchar_t)'$':
      if ((flags & WCPattern::MULTILINE_MATCHING) != 0) next = registerNode(new NFAEndOfLineUNode);
      else                                            next = registerNode(new NFAEndOfInputUNode(0));
      break;
    case (wchar_t)'|':
      --groupCount;
      cur->next = registerNode(new NFAAcceptUNode);
      cur = start = registerNode(new NFAOrUNode(start, parse(inParen, 1)));
      break;
    case (wchar_t)'\\':
      if      (curInd < (int)pattern.size())
      {
        bool eoi = 0;
        switch (pattern[curInd])
        {
        case (wchar_t)'1':
        case (wchar_t)'2':
        case (wchar_t)'3':
        case (wchar_t)'4':
        case (wchar_t)'5':
        case (wchar_t)'6':
        case (wchar_t)'7':
        case (wchar_t)'8':
        case (wchar_t)'9': next = parseBackref(); break;
        case (wchar_t)'A': ++curInd; next = registerNode(new NFAStartOfInputUNode);     break;
        case (wchar_t)'B': ++curInd; next = registerNode(new NFAWordBoundaryUNode(0));  break;
        case (wchar_t)'b': ++curInd; next = registerNode(new NFAWordBoundaryUNode(1));  break;
        case (wchar_t)'G': ++curInd; next = registerNode(new NFAEndOfMatchUNode);       break;
        case (wchar_t)'Z': eoi = 1;
        case (wchar_t)'z': ++curInd; next = registerNode(new NFAEndOfInputUNode(eoi));  break;
        default:
          t = parseEscape(inv, quo);
          //printf("inv quo classes { %c %c %s }\n", inv ? (wchar_t)'t' : (wchar_t)'f', quo ? (wchar_t)'t' : (wchar_t)'f', t.c_str());
          if (!quo)
          {
            if (t.size() > 1 || inv)
            {
              if ((flags & WCPattern::CASE_INSENSITIVE) != 0) next = registerNode(new NFACIClassUNode(t, inv));
              else                                            next = registerNode(new NFAClassUNode(t, inv));
            }
            else
            {
              next = registerNode(new NFACharUNode(t[0]));
            }
          }
          else
          {
            next = parseQuote();
          }
        }
      }
      else raiseError();
      break;
    case (wchar_t)'[':
      if ((flags & WCPattern::CASE_INSENSITIVE) == 0)
      {
        NFAClassUNode * clazz = new NFAClassUNode();
        bkstring s = parseClass();
        for (int i = 0; i < (int)s.size(); ++i) clazz->vals[s[i]] = 1;
        next = registerNode(clazz);
      }
      else
      {
        NFACIClassUNode * clazz = new NFACIClassUNode();
        bkstring s = parseClass();
        for (int i = 0; i < (int)s.size(); ++i) clazz->vals[to_lower(s[i])] = 1;
        next = registerNode(clazz);
      }
      break;
    case (wchar_t)'.':
      {
        bool useN = 1, useR = 1;
        NFAClassUNode * clazz = new NFAClassUNode(1);
        if ((flags & WCPattern::UNIX_LINE_MODE)  != 0) useR = 0;
        if ((flags & WCPattern::DOT_MATCHES_ALL) != 0) useN = useR = 0;
        if (useN) clazz->vals[(wchar_t)'\n'] = 1;
        if (useR) clazz->vals[(wchar_t)'\r'] = 1;
        next = registerNode(clazz);
      }
      break;
    case (wchar_t)'(':
      {
        NFAUNode * end, * t1, * t2;
        t1 = parse(1, 0, &end);
        if (!t1) raiseError();
        else if (t1->isGroupHeadNode() && (t2 = quantifyGroup(t1, end, grc)) != NULL)
        {
          cur->next = t2;
          cur = t2->next;
        }
        else
        {
          cur->next = t1;
          cur = end;
        }
      }
      break;
    case (wchar_t)')':
      if (!inParen) raiseError();
      else if (inOr)
      {
        --curInd;
        cur = cur->next = registerNode(new NFAAcceptUNode);
        flags = oldFlags;
        return start;
      }
      else
      {
        if (ahead)
        {
          cur = cur->next = registerNode(new NFAAcceptUNode);
          flags = oldFlags;
          return *end = registerNode(new NFALookAheadUNode(start, pos));
        }
        else if (indep)
        {
          cur = cur->next = registerNode(new NFAAcceptUNode);
          flags = oldFlags;
          return *end = registerNode(new NFAPossessiveQuantifierUNode(this, start, 1, 1));
        }
        else // capping or noncapping, it doesnt matter
        {
          *end = cur = cur->next = registerNode(new NFAGroupTailUNode(grc));
          next = quantifyGroup(start, *end, grc);
          if (next)
          {
            start = next;
            *end = next->next;
          }
          flags = oldFlags;
          return start;
        }
      }
      break;
    case (wchar_t)'{': // registered pattern
      cur->next = parseRegisteredWCPattern(&next);
      if (cur->next) cur = next;
      break;
    case (wchar_t)'*':
    case (wchar_t)'+':
    case (wchar_t)'?':
//    case (wchar_t)'}':
//    case (wchar_t)']':
      raiseError();
      break;
    default:
      if ((flags & WCPattern::CASE_INSENSITIVE) != 0) next = registerNode(new NFACICharUNode(ch));
      else                                          next = registerNode(new NFACharUNode(ch));
      break;
    }
    if (next) cur = cur->next = quantify(next);
  }
  if (inParen) raiseError();
  else
  {
    if (inOr) cur = cur->next = registerNode(new NFAAcceptUNode);
    if (end) *end = cur;
  }

  flags = oldFlags;
  if (error) return NULL;

  return start;
}

WCPattern * WCPattern::compile(const bkstring & pattern, const unsigned long mode)
{
  WCPattern * p = new WCPattern(pattern);
  NFAUNode * end;

  p->flags = mode;
  if ((mode & WCPattern::LITERAL) != 0)
  {
    p->head = p->registerNode(new NFAStartUNode);
    if ((mode & WCPattern::CASE_INSENSITIVE) != 0)  p->head->next = p->registerNode(new NFACIQuoteUNode(pattern));
    else                                          p->head->next = p->registerNode(new NFAQuoteUNode(pattern));
    p->head->next->next = p->registerNode(new NFAEndUNode);
  }
  else
  {
    p->head = p->parse(0, 0, &end);
    if (!p->head)
    {
      delete p;
      p = NULL;
    }
    else
    {
      if (!(p->head && p->head->isStartOfInputNode()))
      {
        NFAUNode * n = p->registerNode(new NFAStartUNode);
        n->next = p->head;
        p->head = n;
      }
      end->next = p->registerNode(new NFAEndUNode);
    }
  }
  if (p != NULL)
  {
    p->matcher = new WCMatcher(p, L"");
  }

  return p;
}

WCPattern * WCPattern::compileAndKeep(const bkstring & pattern, const unsigned long mode)
{
  WCPattern * ret = NULL;
  std::map<bkstring, WCPattern*>::iterator it = compiledWCPatterns.find(pattern);

  if (it != compiledWCPatterns.end())
  {
    ret = it->second;
  }
  else
  {
    ret = compile(pattern, mode);
    compiledWCPatterns[pattern] = ret;
  }

  return ret;
}
bkstring WCPattern::replace(const bkstring & pattern, const bkstring & str,
                                     const bkstring & replacementText, const unsigned long mode)
{
  bkstring ret;
  WCPattern * p = WCPattern::compile(pattern, mode);
  if (p)
  {
    ret = p->replace(str, replacementText);
    delete p;
  }
  return ret;
}

std::vector<bkstring> WCPattern::split(const bkstring & pattern, const bkstring & str, const bool keepEmptys,
                              const unsigned long limit, const unsigned long mode)
{
  std::vector<bkstring> ret;
  WCPattern * p = WCPattern::compile(pattern, mode);
  if (p)
  {
    ret = p->split(str, keepEmptys, limit);
    delete p;
  }
  return ret;
}

std::vector<bkstring> WCPattern::findAll(const bkstring & pattern, const bkstring & str, const unsigned long mode)
{
  std::vector<bkstring> ret;
  WCPattern * p = WCPattern::compile(pattern, mode);
  if (p)
  {
    ret = p->findAll(str);
    delete p;
  }
  return ret;
}

bool WCPattern::matches(const bkstring & pattern, const bkstring & str, const unsigned long mode)
{
  bool ret = 0;
  WCPattern * p = compile(pattern, mode);

  if (p)
  {
    ret = p->matches(str);
    delete p;
  }

  return ret;
}

bool WCPattern::registerWCPattern(const bkstring & name, const bkstring & pattern, const unsigned long mode)
{
  WCPattern * p = WCPattern::compile(pattern, mode);
  if (!p) return 0;
  WCPattern::registeredWCPatterns[name] = std::make_pair(pattern, mode);
  delete p;
  return 1;
}

void WCPattern::unregisterWCPatterns()
{
  registeredWCPatterns.clear();
}
void WCPattern::clearWCPatternCache()
{
  std::map<bkstring, WCPattern*>::iterator it;
  for (it = compiledWCPatterns.begin(); it != compiledWCPatterns.end(); ++it)
  {
    delete it->second;
  }
  compiledWCPatterns.clear();
}

std::pair<bkstring, int>  WCPattern::findNthMatch(const bkstring & pattern, const bkstring & str,
                                         const int matchNum, const unsigned long mode)
{
  std::pair<bkstring, int> ret;
  WCPattern * p = WCPattern::compile(pattern, mode);

  ret.second = -1;
  if (p)
  {
    int i = -1;
    p->matcher->setString(str);
    while (i < matchNum && p->matcher->findNextMatch()) { ++i; }
    if (i == matchNum && p->matcher->getStartingIndex() >= 0)
    {
      ret.first = p->matcher->getGroup(0);
      ret.second = p->matcher->getStartingIndex();
    }
    delete p;
  }

  return ret;
}

WCPattern::~WCPattern()
{
  /*
  nodes.clear();
  if (head) head->findAllNodes(nodes);
  */
  if (matcher) delete matcher;
  for (std::map<NFAUNode*, bool>::iterator it = nodes.begin(); it != nodes.end(); ++it) delete it->first;
}
bkstring WCPattern::replace(const bkstring & str, const bkstring & replacementText)
{
  int li = 0;
  bkstring ret = L"";

  matcher->setString(str);
  while (matcher->findNextMatch())
  {
    ret += str.substr(li, matcher->getStartingIndex() - li);
    ret += matcher->replaceWithGroups(replacementText);
    li = matcher->getEndingIndex();
  }
  ret += str.substr(li);

  return ret;
}
std::vector<bkstring> WCPattern::split(const bkstring & str, const bool keepEmptys, const unsigned long limit)
{
  unsigned long lim = (limit == 0 ? MAX_QMATCH : limit);
  int li = 0;
  std::vector<bkstring> ret;

  matcher->setString(str);

  while (matcher->findNextMatch() && ret.size() < lim)
  {
    if (matcher->getStartingIndex() == 0 && keepEmptys) ret.push_back(L"");
    if ((matcher->getStartingIndex() != matcher->getEndingIndex()) || keepEmptys)
    {
      if (li != matcher->getStartingIndex() || keepEmptys)
      {
        ret.push_back(str.substr(li, matcher->getStartingIndex() - li));
      }
      li = matcher->getEndingIndex();
    }
  }
  if (li < (int)str.size()) ret.push_back(str.substr(li));

  return ret;
}
std::vector<bkstring> WCPattern::findAll(const bkstring & str)
{
  matcher->setString(str);
  return matcher->findAll();
}
bool WCPattern::matches(const bkstring & str)
{
  matcher->setString(str);
  return matcher->matches();
}
unsigned long WCPattern::getFlags() const
{
  return flags;
}
bkstring WCPattern::getWCPattern() const
{
  return pattern;
}
WCMatcher * WCPattern::createWCMatcher(const bkstring & str)
{
  return new WCMatcher(this, str);
}

// NFAUNode

NFAUNode::NFAUNode() { next = NULL; }
NFAUNode::~NFAUNode() { }
void NFAUNode::findAllNodes(std::map<NFAUNode*, bool> & soFar)
{
  if (soFar.find(this) == soFar.end()) return;
  soFar[this] = 1;
  if (next) next->findAllNodes(soFar);
}

// NFACharUNode

NFACharUNode::NFACharUNode(const wchar_t c) { ch = c; }
int NFACharUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd < (int)str.size() && str[curInd] == ch) return next->match(str, matcher, curInd + 1);
  return -1;
}

// NFACICharUNode

NFACICharUNode::NFACICharUNode(const wchar_t c) { ch = to_lower(c); }
int NFACICharUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd < (int)str.size() && to_lower(str[curInd]) == ch) return next->match(str, matcher, curInd + 1);
  return -1;
}

// NFAStartUNode

NFAStartUNode::NFAStartUNode() { }
int NFAStartUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int ret = -1, ci = curInd;

  matcher->starts[0] = curInd;
  if ((matcher->getFlags() & WCMatcher::MATCH_ENTIRE_STRING) == (unsigned int)WCMatcher::MATCH_ENTIRE_STRING)
  {
    if (curInd != 0)
    {
      matcher->starts[0] = -1;
      return -1;
    }
    return next->match(str, matcher, 0);
  }
  while ((ret = next->match(str, matcher, ci)) == -1 && ci < (int)str.size())
  {
    matcher->clearGroups();
    matcher->starts[0] = ++ci;
  }
  if (ret < 0) matcher->starts[0] = -1;
  return ret;
}

// NFAEndUNode

NFAEndUNode::NFAEndUNode() { }
int NFAEndUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  matcher->ends[0] = curInd;
  if ((matcher->getFlags() & WCMatcher::MATCH_ENTIRE_STRING) != 0)
  {
    if (curInd == (int)str.size()) return curInd;
    matcher->ends[0] = -1;
    return -1;
  }
  return curInd;
}

// NFAQuantifierUNode

void NFAQuantifierUNode::findAllNodes(std::map<NFAUNode*, bool> & soFar)
{
  inner->findAllNodes(soFar);
  NFAUNode::findAllNodes(soFar);
}
NFAQuantifierUNode::NFAQuantifierUNode(WCPattern * pat, NFAUNode * internal, const int minMatch, const int maxMatch)
{
  inner = internal;
  inner->next = pat->registerNode(new NFAAcceptUNode);
  min = (minMatch < WCPattern::MIN_QMATCH) ? WCPattern::MIN_QMATCH : minMatch;
  max = (maxMatch > WCPattern::MAX_QMATCH) ? WCPattern::MAX_QMATCH : maxMatch;
}

int NFAQuantifierUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int i0, i1, i2 = 0;

  i0 = i1 = curInd;
  while (i2 < min)
  {

    ++i2;
    i1 = inner->match(str, matcher, i0);
    if (i1 <= i0) return i1; // i1 < i0 means i1 is -1
    i0 = i1;
  }

  return i1;
}
// NFAGreedyQuantifierUNode

NFAGreedyQuantifierUNode::NFAGreedyQuantifierUNode(WCPattern * pat, NFAUNode * internal, const int minMatch, const int maxMatch)
                        : NFAQuantifierUNode(pat, internal, minMatch, maxMatch) { }
int NFAGreedyQuantifierUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int t = NFAQuantifierUNode::match(str, matcher, curInd);
  if (t != -1) return matchInternal(str, matcher, t, min);
  return t;
}
int NFAGreedyQuantifierUNode::matchInternal(const bkstring & str, WCMatcher * matcher, const int curInd, const int soFar) const
{
  if (soFar >= max) return next->match(str, matcher, curInd);

  int i, j;

  i = inner->match(str, matcher, curInd);
  if (i != -1)
  {
    j = matchInternal(str, matcher, i, soFar + 1);
    if (j != -1) return j;
  }
  return next->match(str, matcher, curInd);
}

// NFALazyQuantifierUNode

NFALazyQuantifierUNode::NFALazyQuantifierUNode(WCPattern * pat, NFAUNode * internal, const int minMatch, const int maxMatch)
                      : NFAQuantifierUNode(pat, internal, minMatch, maxMatch) { }
int NFALazyQuantifierUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int i, j, m = NFAQuantifierUNode::match(str, matcher, curInd);

  if (m == -1) return -1;

  for (i = min; i < max; ++i)
  {
    j = next->match(str, matcher, m);
    if (j == -1)
    {
      j = inner->match(str, matcher, m);
      // if j < m, then j is -1, so we bail.
      // if j == m, then we would just go and call next->match on the same index,
      // but it already failed trying to match right there, so we know we can
      // just bail
      if (j <= m) return -1;
      m = j;
    }
    else return j;
  }
  return next->match(str, matcher, m);
}

// NFAPossessiveQuantifierUNode

NFAPossessiveQuantifierUNode::NFAPossessiveQuantifierUNode(WCPattern * pat, NFAUNode * internal, const int minMatch, const int maxMatch)
                            : NFAQuantifierUNode(pat, internal, minMatch, maxMatch) { }
int NFAPossessiveQuantifierUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int i, j, m = NFAQuantifierUNode::match(str, matcher, curInd);

  if (m == -1) return -1;
  for (i = min; i < max; ++i)
  {
    j = inner->match(str, matcher, m);
    if (j <= m) return next->match(str, matcher, m);
    m = j;
  }
  return next->match(str, matcher, m);
}

// NFAAcceptUNode

NFAAcceptUNode::NFAAcceptUNode() { }
int NFAAcceptUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (!next) return curInd;
  else return next->match(str, matcher, curInd);
}

// NFAClassUNode

NFAClassUNode::NFAClassUNode(const bool invert)
{
  inv = invert;
}
NFAClassUNode::NFAClassUNode(const bkstring & clazz, const bool invert)
{
  inv = invert;
  for (int i = 0; i < (int)clazz.size(); ++i) vals[clazz[i]] = 1;
}
int NFAClassUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd < (int)str.size() && ((vals.find(str[curInd]) != vals.end()) ^ inv))
  {
    return next->match(str, matcher, curInd + 1);
  }
  return -1;
}

// NFACIClassUNode

NFACIClassUNode::NFACIClassUNode(const bool invert)
{
  inv = invert;
}
NFACIClassUNode::NFACIClassUNode(const bkstring & clazz, const bool invert)
{
  inv = invert;
  for (int i = 0; i < (int)clazz.size(); ++i) vals[to_lower(clazz[i])] = 1;
}
int NFACIClassUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd < (int)str.size() && ((vals.find(to_lower(str[curInd])) != vals.end()) ^ inv))
  {
    return next->match(str, matcher, curInd + 1);
  }
  return -1;
}

// NFASubStartUNode

NFASubStartUNode::NFASubStartUNode() { }
int NFASubStartUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  return next->match(str, matcher, curInd);
}

// NFAOrUNode

NFAOrUNode::NFAOrUNode(NFAUNode * first, NFAUNode * second) : one(first), two(second) { }
void NFAOrUNode::findAllNodes(std::map<NFAUNode*, bool> & soFar)
{
  if (one) one->findAllNodes(soFar);
  if (two) two->findAllNodes(soFar);
  NFAUNode::findAllNodes(soFar);
}
int NFAOrUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int ci = one->match(str, matcher, curInd);

  if (ci != -1) ci = next->match(str, matcher, ci);
  if (ci != -1) return ci;
  if (ci == -1) ci = two->match(str, matcher, curInd);
  if (ci != -1) ci = next->match(str, matcher, ci);
  return ci;
}

// NFAQuoteUNode

NFAQuoteUNode::NFAQuoteUNode(const bkstring & quoted) : qStr(quoted) { }
int NFAQuoteUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd + qStr.size() > str.size())       return -1;
  if (str.substr(curInd, qStr.size()) != qStr) return -1;
  return next->match(str, matcher, curInd + (int)qStr.size());
}

// NFACIQuoteUNode

NFACIQuoteUNode::NFACIQuoteUNode(const bkstring & quoted) : qStr(quoted) { }
int NFACIQuoteUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd + qStr.size() > str.size()) return -1;
  if (str_icmp(str.substr(curInd, qStr.size()).c_str(),  qStr.c_str())) return -1;
  return next->match(str, matcher, (int)qStr.size());
}

// NFALookAheadUNode

NFALookAheadUNode::NFALookAheadUNode(NFAUNode * internal, const bool positive) : NFAUNode(), pos(positive), inner(internal) { }
void NFALookAheadUNode::findAllNodes(std::map<NFAUNode*, bool> & soFar)
{
  if (inner) inner->findAllNodes(soFar);
  NFAUNode::findAllNodes(soFar);
}
int NFALookAheadUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  return ((inner->match(str, matcher, curInd) == -1) ^ pos) ? next->match(str, matcher, curInd) : -1;
}

// NFALookBehindUNode

NFALookBehindUNode::NFALookBehindUNode(const bkstring & str, const bool positive) : pos(positive), mStr(str) { }
int NFALookBehindUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (pos)
  {
    if (curInd < (int)mStr.size()) return -1;
    if (str.substr(curInd - mStr.size(), mStr.size()) == mStr) return next->match(str, matcher, curInd);
  }
  else
  {
    if (curInd < (int)mStr.size()) return next->match(str, matcher, curInd);
    if (str.substr(curInd - mStr.size(), mStr.size()) == mStr) return -1;
     return next->match(str, matcher, curInd);
  }
  return -1;
}

// NFAStartOfLineUNode

NFAStartOfLineUNode::NFAStartOfLineUNode() { }
int NFAStartOfLineUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd == 0 || str[curInd - 1] == (wchar_t)'\n' || str[curInd - 1] == (wchar_t)'\r')
  {
    return next->match(str, matcher, curInd);
  }
  return -1;
}

// NFAEndOfLineUNode

NFAEndOfLineUNode::NFAEndOfLineUNode() { }
int NFAEndOfLineUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd >= (int)str.size() || str[curInd] == (wchar_t)'\n' || str[curInd] == (wchar_t)'\r')
  {
    return next->match(str, matcher, curInd);
  }
  return -1;
}

// NFAReferenceUNode

NFAReferenceUNode::NFAReferenceUNode(const int groupIndex) : gi(groupIndex) { }
int NFAReferenceUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int len = matcher->ends[gi] - matcher->starts[gi];
  int ni = -1;
  if      (gi < 1 || matcher->ends[gi] < matcher->starts[gi] || len == 0)             ni = curInd;
  else if (curInd + len > (int)str.size())                                            return -1;
  else if (str.substr(curInd, len) != str.substr(matcher->starts[gi], len))  return -1;
  else                                                                                ni = curInd + len;

  return next->match(str, matcher, ni);
}

// NFAStartOfInputUNode

NFAStartOfInputUNode::NFAStartOfInputUNode() { }
int NFAStartOfInputUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd == 0) return next->match(str, matcher, curInd);
  return -1;
}

// NFAEndOfInputUNode

NFAEndOfInputUNode::NFAEndOfInputUNode(const bool lookForTerm) : term(lookForTerm) { }
int NFAEndOfInputUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int len = (int)str.size();
  if      (curInd == len) return next->match(str, matcher, curInd);
  else if (term)
  {
    if      (curInd == len - 1 && (str[curInd] == (wchar_t)'\r' || str[curInd] == (wchar_t)'\n'))
    {
      return next->match(str, matcher, curInd);
    }
    else if (curInd == len - 2 && str.substr(curInd, 2) == L"\r\n")
    {
      return next->match(str, matcher, curInd);
    }
  }
  return -1;
}

// NFAWordBoundaryUNode

NFAWordBoundaryUNode::NFAWordBoundaryUNode(const bool positive) : pos(positive) { }
int NFAWordBoundaryUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int len = (int)str.size();

  wchar_t c1 = (curInd - 1 < len && curInd > 0) ? str[curInd - 1] : '\n';
  wchar_t c2 = (curInd     < len) ? str[curInd    ] : '\n';

  if      (curInd == len) return next->match(str, matcher, curInd);
  bool ok = is_alpha(c1) != is_alpha(c2);
  if (ok && pos) return next->match(str, matcher, curInd);
  return -1;
}

// NFAEndOfMatchUNode

NFAEndOfMatchUNode::NFAEndOfMatchUNode() { }
int NFAEndOfMatchUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  if (curInd == matcher->lm) return next->match(str, matcher, curInd);
  return -1;
}

// NFAGroupHeadUNode

NFAGroupHeadUNode::NFAGroupHeadUNode(const int groupIndex) : gi(groupIndex) { }
int NFAGroupHeadUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int ret, o = matcher->starts[gi];

  matcher->starts[gi] = curInd;
  ret = next->match(str, matcher, curInd);
  if (ret < 0) matcher->starts[gi] = o;

  return ret;
}

// NFAGroupTailUNode

NFAGroupTailUNode::NFAGroupTailUNode(const int groupIndex) : gi(groupIndex) { }
int NFAGroupTailUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int ret, o = matcher->ends[gi];

  matcher->ends[gi] = curInd;
  ret = next->match(str, matcher, curInd);
  if (ret < 0) matcher->ends[gi] = o;

  return ret;
}

// NFAGroupLoopPrologueUNode

NFAGroupLoopPrologueUNode::NFAGroupLoopPrologueUNode(const int groupIndex) : gi(groupIndex) { }
int NFAGroupLoopPrologueUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int ret, o1 = matcher->groups[gi], o2 = matcher->groupPos[gi], o3 = matcher->groupIndeces[gi];

  matcher->groups[gi] = 0;
  matcher->groupPos[gi] = 0;
  matcher->groupIndeces[gi] = -1;
  ret = next->match(str, matcher, curInd);
  if (ret < 0)
  {
    matcher->groups[gi] = o1;
    matcher->groupPos[gi] = o2;
    matcher->groupIndeces[gi] = o3;
  }

  return ret;
}

// NFAGroupLoopUNode

NFAGroupLoopUNode::NFAGroupLoopUNode(NFAUNode * internal, const int minMatch, const int maxMatch,
                                   const int groupIndex, const int matchType)
{
  inner = internal;
  min = minMatch;
  max = maxMatch;
  gi = groupIndex;
  type = matchType;
}
void NFAGroupLoopUNode::findAllNodes(std::map<NFAUNode*, bool> & soFar)
{
  if (inner) inner->findAllNodes(soFar);
  NFAUNode::findAllNodes(soFar);
}
int NFAGroupLoopUNode::match(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  bool b = (curInd > matcher->groupIndeces[gi]);

  if (b && matcher->groups[gi] < min)
  {
    ++matcher->groups[gi];
    int o = matcher->groupIndeces[gi];
    matcher->groupIndeces[gi] = curInd;
    int ret = inner->match(str, matcher, curInd);
    if (ret < 0)
    {
      matcher->groupIndeces[gi] = o;
      --matcher->groups[gi];
    }
    return ret;
  }
  else if (!b || matcher->groups[gi] >= max)
  {
    return next->match(str, matcher, curInd);
  }
  else
  {
    switch (type)
    {
    case 0: return matchGreedy(str, matcher, curInd);
    case 1: return matchLazy(str, matcher, curInd);
    case 2: return matchPossessive(str, matcher, curInd);
    }
  }
  return -1;
}
int NFAGroupLoopUNode::matchGreedy(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int o = matcher->groupIndeces[gi];            // save our info for backtracking
  matcher->groupIndeces[gi] = curInd;           // move along
  ++matcher->groups[gi];
  int ret = inner->match(str, matcher, curInd); // match internally
  if (ret < 0)
  {                                             // if we failed, then restore info and match next
    --matcher->groups[gi];
    matcher->groupIndeces[gi] = o;
    ret = next->match(str, matcher, curInd);
  }
  return ret;
}
int NFAGroupLoopUNode::matchLazy(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int ret = next->match(str, matcher, curInd);  // be lazy, just go on
  if (ret < 0)
  {
    int o = matcher->groupIndeces[gi];          // save info for backtracking
    matcher->groupIndeces[gi] = curInd;         // advance our position
    ++matcher->groups[gi];
    ret = inner->match(str, matcher, curInd);   // match our internal stuff
    if (ret < 0)                                // if we failed, then restore the info
    {
      --matcher->groups[gi];
      matcher->groupIndeces[gi] = o;
    }
  }
  return ret;
}
int NFAGroupLoopUNode::matchPossessive(const bkstring & str, WCMatcher * matcher, const int curInd) const
{
  int o = matcher->groupIndeces[gi];            // save info for backtracking
  matcher->groupPos[gi] = matcher->groups[gi];  // set a flag stating we have matcher at least this much
  matcher->groupIndeces[gi] = curInd;           // move along
  ++matcher->groups[gi];
  int ret = inner->match(str, matcher, curInd); // try and match again
  if (ret < 0)
  {                                             // if we fail, back off, but to an extent
    --matcher->groups[gi];
    matcher->groupIndeces[gi] = o;
    if (matcher->groups[gi] == matcher->groupPos[gi]) ret = next->match(str, matcher, curInd);
  }
  return ret;
}

#ifdef _WIN32
  #pragma warning(pop)
#endif
