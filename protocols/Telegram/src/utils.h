#pragma once

const char *getName(const TD::usernames *pName);

TD::object_ptr<TD::inputFileLocal> makeFile(const CMStringW &wszFile);

TD::object_ptr<TD::formattedText> formatBbcodes(const char *pszText);
