#include <regexp/Matcher.h>
#include <regexp/Pattern.h>
#include <regexp/UnicodeMatcher.h>
#include <regexp/UnicodePattern.h>

int main()
{
  {
    Pattern * p = Pattern::compile("^([^:]*)://([^/:]*)((?::[0-9]+)?)/?(([^?]*)((?:\\?.*)?))$");
    Matcher * m0 = p->createMatcher("http://www.example.com:80/test.php?a=1&a=1&a=1");

    if (m0->matches())
    {
      std::vector<std::string> groups = m0->getGroups(true);
      for (int i = 0; i < (int)groups.size(); ++i)
      {
        printf("m->group(%d): %s\n", i, groups[i].c_str());
      }
    }
  }
  {
    std::wstring pat = L"^([^:]*)://([^/:]*)((?::[0-9]+)?)/?(([^?]*)((?:\\?.*)?))$";
    std::wstring mat = L"http://www.example.com:80/test.php?a=1&a=1&a=1";
    UnicodePattern * p = UnicodePattern::compile(pat);
    UnicodeMatcher * m0 = p->createUnicodeMatcher(mat);

    if (m0->matches())
    {
      std::vector<std::wstring> groups = m0->getGroups(true);
      for (int i = 0; i < (int)groups.size(); ++i)
      {
        wprintf(L"m->group(%d): %s\n", i, groups[i].c_str());
      }
    }
  }

  return 0;
}
