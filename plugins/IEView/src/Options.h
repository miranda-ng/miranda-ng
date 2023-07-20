/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#define DBS_BASICFLAGS         "GeneralFlags"

#define DBS_SRMM_ENABLE        "SRMMEnable"
#define DBS_SRMM_MODE          "SRMMMode"
#define DBS_SRMM_FLAGS         "SRMMFlags"
#define DBS_SRMM_BACKGROUND    "SRMMBackgroundFile"
#define DBS_SRMM_CSS           "SRMMCSSFile"
#define DBS_SRMM_TEMPLATE      "SRMMTemplateFile"

#define DBS_CHAT_ENABLE        "ChatEnable"
#define DBS_CHAT_MODE          "ChatMode"
#define DBS_CHAT_FLAGS         "ChatFlags"
#define DBS_CHAT_BACKGROUND    "ChatBackgroundFile"
#define DBS_CHAT_CSS         	 "ChatCSSFile"
#define DBS_CHAT_TEMPLATE      "ChatTemplateFile"

#define DBS_HISTORY_ENABLE     "HistoryEnable"
#define DBS_HISTORY_MODE       "HistoryMode"
#define DBS_HISTORY_FLAGS      "HistoryFlags"
#define DBS_HISTORY_BACKGROUND "HistoryBackgroundFile"
#define DBS_HISTORY_CSS        "HistoryCSSFile"
#define DBS_HISTORY_TEMPLATE   "HistoryTemplateFile"

int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class ProtocolSettings
{
	char *protocolName;

	bool srmmEnable;
	int  srmmMode;
	int  srmmFlags;
	CMStringW srmmBackgroundFilename;
	CMStringW srmmCssFilename;
	CMStringW srmmTemplateFilename;

	bool chatEnable;
	int  chatMode;
	int  chatFlags;
	CMStringW chatBackgroundFilename;
	CMStringW chatCssFilename;
	CMStringW chatTemplateFilename;

	bool historyEnable;
	int historyMode;
	int historyFlags;
	CMStringW historyBackgroundFilename;
	CMStringW historyCssFilename;
	CMStringW historyCssFilenameRtl;
	CMStringW historyTemplateFilename;

public:
	ProtocolSettings(const char *protocolName);
	~ProtocolSettings();

	void readFromDb();

	__forceinline const char* getProtocolName() { return protocolName; }

	__forceinline void setSRMMEnable(bool enable) { this->srmmEnable = enable; }
	__forceinline bool isSRMMEnable() { return srmmEnable; }

	__forceinline void setSRMMMode(int mode) { this->srmmMode = mode; }
	__forceinline int getSRMMMode() { return srmmMode; }

	__forceinline void setSRMMFlags(int flags) { this->srmmFlags = flags; }
	__forceinline int getSRMMFlags() { return srmmFlags; }
	
	__forceinline void setSRMMBackgroundFilename(const wchar_t *filename) { srmmBackgroundFilename = filename; }
	__forceinline const wchar_t* getSRMMBackgroundFilename() { return srmmBackgroundFilename; }

	__forceinline void setSRMMCssFilename(const wchar_t *filename) { srmmCssFilename = filename; }
	__forceinline const wchar_t* getSRMMCssFilename() { return srmmCssFilename; }
	
	              void setSRMMTemplateFilename(const wchar_t *filename);
	__forceinline const wchar_t* getSRMMTemplateFilename() { return srmmTemplateFilename; }

	__forceinline void setChatEnable(bool enable) { this->chatEnable = enable; }
	__forceinline bool isChatEnable() { return chatEnable; }
	
	__forceinline void setChatMode(int mode) { this->chatMode = mode; }
	__forceinline int getChatMode() { return chatMode; }
	
	__forceinline void setChatFlags(int flags) { this->chatFlags = flags; }
	__forceinline int getChatFlags() { return chatFlags; }
	
	__forceinline void setChatBackgroundFilename(const wchar_t *filename) { chatBackgroundFilename = filename; }
	__forceinline const wchar_t* getChatBackgroundFilename() { return chatBackgroundFilename; }
	
	__forceinline void setChatCssFilename(const wchar_t *filename) { chatCssFilename = filename; }
	__forceinline const wchar_t* getChatCssFilename() { return chatCssFilename; }

	              void setChatTemplateFilename(const wchar_t *filename);
	__forceinline const wchar_t* getChatTemplateFilename() { return chatTemplateFilename; }

	__forceinline void setHistoryEnable(bool enable) { this->historyEnable = enable; }
	__forceinline bool isHistoryEnable() { return historyEnable; }
	
	__forceinline void setHistoryMode(int mode) { this->historyMode = mode; }
	__forceinline int getHistoryMode() { return historyMode; }
	
	__forceinline void setHistoryFlags(int flags) { this->historyFlags = flags; }
	__forceinline int getHistoryFlags() { return historyFlags; }
	
	__forceinline void setHistoryBackgroundFilename(const wchar_t *filename) { historyBackgroundFilename = filename; }
	__forceinline const wchar_t* getHistoryBackgroundFilename() { return historyBackgroundFilename; }
	
	__forceinline void setHistoryCssFilename(const wchar_t *filename) { historyCssFilename = filename; }
	__forceinline const wchar_t* getHistoryCssFilename() { return historyCssFilename; }
	
	              void setHistoryTemplateFilename(const wchar_t *filename);
	__forceinline const wchar_t* getHistoryTemplateFilename() { return historyTemplateFilename; }
};

namespace Options
{
	enum
	{
		MODE_COMPATIBLE = 0,
		MODE_CSS = 1,
		MODE_TEMPLATE = 2
	};

	enum OPTIONS
	{
		GENERAL_ENABLE_BBCODES = 0x000001,
		GENERAL_ENABLE_FLASH = 0x000004,
		GENERAL_SMILEYINNAMES = 0x000010,
		GENERAL_NO_BORDER = 0x000020,
		GENERAL_ENABLE_EMBED = 0x000040,

		LOG_SHOW_NICKNAMES = 0x000100,
		LOG_SHOW_TIME = 0x000200,
		LOG_SHOW_DATE = 0x000400,
		LOG_SHOW_SECONDS = 0x000800,
		LOG_LONG_DATE = 0x001000,
		LOG_RELATIVE_DATE = 0x002000,
		LOG_GROUP_MESSAGES = 0x004000,

		LOG_IMAGE_ENABLED = 0x010000,
		LOG_IMAGE_SCROLL = 0x020000

	};
	
	extern int generalFlags;
	extern bool isInited, bHasSmileyAdd, bHasAvs;

	int getEmbedSize();
	void setEmbedSize(int);

	ProtocolSettings* getProtocolSettings(const char *protocolName);
	ProtocolSettings* getDefaultSettings();

	void init();
	void uninit();
	void saveProtocolSettings();
	void reload();
};

#endif
