#pragma once

const char *getName(const TD::usernames *pName);

TD::object_ptr<TD::inputFileLocal> makeFile(const wchar_t *pwszFilename);

TD::object_ptr<TD::formattedText> formatBbcodes(const char *pszText);

TG_FILE_REQUEST::Type AutoDetectType(const wchar_t *pwszFilename);
