struct ItemData;
struct NewstoryListData;

uint32_t toggleBit(uint32_t dw, uint32_t bit);
bool CheckFilter(wchar_t *buf, wchar_t *filter);

HMENU NSMenu_Build(NewstoryListData *data, ItemData *item);
bool NSMenu_Process(int iCommand, NewstoryListData *data);

int GetFontHeight(const LOGFONTA &lf);

void ReplaceSmileys(MCONTACT hContact, CMStringA &str);
void UrlAutodetect(CMStringA &str);
void RemoveBbcodes(CMStringW &pwszText);
