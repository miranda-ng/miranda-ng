#pragma once

std::wstring DBGetContactSettingStringPAN(MCONTACT hContact, char const *szModule, char const *szSetting, std::wstring errorValue);
std::string DBGetContactSettingStringPAN_A(MCONTACT hContact, char const *szModule, char const *szSetting, std::string errorValue);
std::wstring &GetDlgItemString(HWND hwnd, int id);
std::string &GetProtoList();
bool ProtoInList(const char *szProto);
std::wstring variables_parse(std::wstring const &tstrFormat, MCONTACT hContact);
const int Stricmp(const wchar_t *str, const wchar_t *substr);

bool IsUrlContains(const wchar_t *Str);
void DeleteCListGroupsByName(wchar_t *szGroupName);
void LogSpamToFile(MCONTACT hContact, std::wstring message);
std::string toUTF8(std::wstring str);
std::string toUTF8(std::string str);
std::wstring toUTF16(std::string str);
void HistoryLogFunc(MCONTACT hContact, std::string message);
std::string get_random_num(int length);