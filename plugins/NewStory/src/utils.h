struct ItemData;
struct NewstoryListData;

uint32_t toggleBit(uint32_t dw, uint32_t bit);
bool CheckFilter(wchar_t *buf, wchar_t *filter);

HMENU NSMenu_Build(NewstoryListData *data, ItemData *item);
bool NSMenu_Process(int iCommand, NewstoryListData *data);

int GetFontHeight(const LOGFONTA &lf);

void UrlAutodetect(CMStringW &str);
void RemoveBbcodes(CMStringW &pwszText);

Bitmap* LoadImageFromResource(HINSTANCE, int, const wchar_t *);
