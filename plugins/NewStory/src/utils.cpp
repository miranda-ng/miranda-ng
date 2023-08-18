#include "stdafx.h"

uint32_t toggleBit(uint32_t dw, uint32_t bit)
{
	if (dw & bit)
		return dw & ~bit;
	return dw | bit;
}

bool CheckFilter(wchar_t *buf, wchar_t *filter)
{
	//	MessageBox(0, buf, filter, MB_OK);
	int l1 = (int)mir_wstrlen(buf);
	int l2 = (int)mir_wstrlen(filter);
	for (int i = 0; i < l1 - l2 + 1; i++)
		if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, buf + i, l2, filter, l2) == CSTR_EQUAL)
			return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	wchar_t *pStart, *pEnd;
	size_t cbStart, cbEnd;
}
static bbcodes[] = 
{
	{ L"[b]",      nullptr },
	{ L"[/b]",     nullptr },
	{ L"[i]",      nullptr },
	{ L"[/i]",     nullptr },
	{ L"[u]",      nullptr },
	{ L"[/u]",     nullptr },
	{ L"[s]",      nullptr },
	{ L"[/s]",     nullptr },

	{ L"[color=", L"]"     },
	{ L"[/color]", nullptr },

	{ L"[$hicon=", L"$]"   },

	{ L"[url]", L"[/url]"  },
	{ L"[url=", L"]",      },
	{ L"[img]", L"[/img]"  },
	{ L"[img=", L"]"       },
};

void RemoveBbcodes(wchar_t *pwszText)
{
	if (!pwszText)
		return;

	if (bbcodes[0].cbStart == 0)
		for (auto &it : bbcodes) {
			it.cbStart = wcslen(it.pStart);
			if (it.pEnd)
				it.cbEnd = wcslen(it.pEnd);
		}

	for (auto *p = wcschr(pwszText, '['); p != 0; p = wcschr(p, '[')) {
		for (auto &it : bbcodes) {
			if (wcsncmp(p, it.pStart, it.cbStart))
				continue;

			strdelw(p, it.cbStart);

			if (it.pEnd)
				if (auto *pp = wcsstr(p, it.pEnd)) {
					strdelw(p, size_t(pp - p));
					strdelw(p, it.cbEnd);
				}

			break;
		}
	}
}
