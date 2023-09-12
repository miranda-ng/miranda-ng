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

void RemoveBbcodes(CMStringW &wszText)
{
	if (wszText.IsEmpty())
		return;

	if (bbcodes[0].cbStart == 0)
		for (auto &it : bbcodes) {
			it.cbStart = wcslen(it.pStart);
			if (it.pEnd)
				it.cbEnd = wcslen(it.pEnd);
		}

	for (int idx = wszText.Find('[', 0); idx != -1; idx = wszText.Find('[', idx)) {
		for (auto &it : bbcodes) {
			if (wcsncmp(wszText.c_str() + idx, it.pStart, it.cbStart))
				continue;

			wszText.Delete(idx, (int)it.cbStart);

			if (it.pEnd) {
				int idx2 = wszText.Find(it.pEnd, idx);
				if (idx2 != -1) {
					wszText.Delete(idx2 - idx);
					wszText.Delete((int)it.cbEnd);
				}
			}

			break;
		}
	}
}
