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
class ProtocolSettings;
class Options;
#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
//#include "FontList.h"
#include "stdafx.h"

#define DBS_BASICFLAGS  		  	"GeneralFlags"

#define DBS_SRMM_ENABLE          	"SRMMEnable"
#define DBS_SRMM_MODE          		"SRMMMode"
#define DBS_SRMM_FLAGS          	"SRMMFlags"
#define DBS_SRMM_BACKGROUND    		"SRMMBackgroundFile"
#define DBS_SRMM_CSS         		"SRMMCSSFile"
#define DBS_SRMM_TEMPLATE         	"SRMMTemplateFile"

#define DBS_CHAT_ENABLE          	"ChatEnable"
#define DBS_CHAT_MODE          		"ChatMode"
#define DBS_CHAT_FLAGS          	"ChatFlags"
#define DBS_CHAT_BACKGROUND    		"ChatBackgroundFile"
#define DBS_CHAT_CSS         		"ChatCSSFile"
#define DBS_CHAT_TEMPLATE         	"ChatTemplateFile"

#define DBS_HISTORY_ENABLE          "HistoryEnable"
#define DBS_HISTORY_MODE          	"HistoryMode"
#define DBS_HISTORY_FLAGS          	"HistoryFlags"
#define DBS_HISTORY_BACKGROUND    	"HistoryBackgroundFile"
#define DBS_HISTORY_CSS         	"HistoryCSSFile"
#define DBS_HISTORY_TEMPLATE        "HistoryTemplateFile"

extern int IEViewOptInit(WPARAM wParam, LPARAM lParam);

class ProtocolSettings
{
	char *protocolName;
	ProtocolSettings *next;

	bool srmmEnable;
	int srmmMode;
	int srmmFlags;
	char *srmmBackgroundFilename;
	char *srmmCssFilename;
	char *srmmTemplateFilename;

	bool srmmEnableTemp;
	int srmmModeTemp;
	int srmmFlagsTemp;
	char *srmmBackgroundFilenameTemp;
	char *srmmCssFilenameTemp;
	char *srmmTemplateFilenameTemp;

	bool chatEnable;
	int chatMode;
	int chatFlags;
	char *chatBackgroundFilename;
	char *chatCssFilename;
	char *chatTemplateFilename;

	bool chatEnableTemp;
	int chatModeTemp;
	int chatFlagsTemp;
	char *chatBackgroundFilenameTemp;
	char *chatCssFilenameTemp;
	char *chatCssFilenameRtlTemp;
	char *chatTemplateFilenameTemp;

	bool historyEnable;
	int historyMode;
	int historyFlags;
	char *historyBackgroundFilename;
	char *historyCssFilename;
	char *historyCssFilenameRtl;
	char *historyTemplateFilename;

	bool historyEnableTemp;
	int historyModeTemp;
	int historyFlagsTemp;
	char *historyBackgroundFilenameTemp;
	char *historyCssFilenameTemp;
	char *historyCssFilenameRtlTemp;
	char *historyTemplateFilenameTemp;

public:
	ProtocolSettings(const char *protocolName);
	~ProtocolSettings();

	__forceinline const char* getProtocolName() { return protocolName; }

	__forceinline void setNext(ProtocolSettings *_next) {	next = _next; }
	__forceinline ProtocolSettings* getNext() {	return next; }

	__forceinline void setSRMMEnable(bool enable) { this->srmmEnable = enable; }
	__forceinline bool isSRMMEnable() { return srmmEnable; }

	__forceinline void setSRMMMode(int mode) { this->srmmMode = mode; }
	__forceinline int getSRMMMode() { return srmmMode; }

	__forceinline void setSRMMFlags(int flags) { this->srmmFlags = flags; }
	__forceinline int getSRMMFlags() { return srmmFlags; }
	
	__forceinline void setSRMMBackgroundFilename(const char *filename) { replaceStr(srmmBackgroundFilename, filename); }
	__forceinline const char* getSRMMBackgroundFilename() { return srmmBackgroundFilename; }

	__forceinline void setSRMMCssFilename(const char *filename) { replaceStr(srmmCssFilename, filename); }
	__forceinline const char* getSRMMCssFilename() { return srmmCssFilename; }
	
	              void setSRMMTemplateFilename(const char *filename);
	__forceinline const char* getSRMMTemplateFilename() { return srmmTemplateFilename; }

	__forceinline void setSRMMEnableTemp(bool enable) { this->srmmEnableTemp = enable; }
	__forceinline bool isSRMMEnableTemp() { return srmmEnableTemp; }

	__forceinline void setSRMMModeTemp(int mode) { this->srmmModeTemp = mode; }
	__forceinline int getSRMMModeTemp() { return srmmModeTemp; }

	__forceinline void setSRMMFlagsTemp(int flags) { this->srmmFlagsTemp = flags; }
	__forceinline int getSRMMFlagsTemp() { return srmmFlagsTemp; }
	
	__forceinline void setSRMMBackgroundFilenameTemp(const char *filename) { replaceStr(srmmBackgroundFilenameTemp, filename); }
	__forceinline const char* getSRMMBackgroundFilenameTemp() { return srmmBackgroundFilenameTemp; }

	__forceinline void setSRMMCssFilenameTemp(const char *filename) { replaceStr(srmmCssFilenameTemp, filename); }
	__forceinline const char* getSRMMCssFilenameTemp() { return srmmCssFilenameTemp; }

	__forceinline void setSRMMTemplateFilenameTemp(const char *filename) { replaceStr(srmmTemplateFilenameTemp, filename); }
	__forceinline const char* getSRMMTemplateFilenameTemp() { return srmmTemplateFilenameTemp; }

	__forceinline void setChatEnable(bool enable) { this->chatEnable = enable; }
	__forceinline bool isChatEnable() { return chatEnable; }
	
	__forceinline void setChatMode(int mode) { this->chatMode = mode; }
	__forceinline int getChatMode() { return chatMode; }
	
	__forceinline void setChatFlags(int flags) { this->chatFlags = flags; }
	__forceinline int getChatFlags() { return chatFlags; }
	
	__forceinline void setChatBackgroundFilename(const char *filename) { replaceStr(chatBackgroundFilename, filename); }
	__forceinline const char* getChatBackgroundFilename() { return chatBackgroundFilename; }
	
	__forceinline void setChatCssFilename(const char *filename) { replaceStr(chatCssFilename, filename); }
	__forceinline const char* getChatCssFilename() { return chatCssFilename; }

	              void setChatTemplateFilename(const char *filename);
	__forceinline const char* getChatTemplateFilename() { return chatTemplateFilename; }

	__forceinline void setChatEnableTemp(bool enable) { this->chatEnableTemp = enable; }
	__forceinline bool isChatEnableTemp() { return chatEnableTemp; }
	
	__forceinline void setChatModeTemp(int mode) { this->chatModeTemp = mode; }
	__forceinline int getChatModeTemp() { return chatModeTemp; }
	
	__forceinline void setChatFlagsTemp(int flags) { this->chatFlagsTemp = flags; }
	__forceinline int getChatFlagsTemp() { return chatFlagsTemp; }
	
	__forceinline void setChatBackgroundFilenameTemp(const char *filename) { replaceStr(chatBackgroundFilenameTemp, filename); }
	__forceinline const char* getChatBackgroundFilenameTemp() { return chatBackgroundFilenameTemp; }

	__forceinline void setChatCssFilenameTemp(const char *filename) { replaceStr(chatCssFilenameTemp, filename); }
	__forceinline const char* getChatCssFilenameTemp() { return chatCssFilenameTemp; }
	
	__forceinline void setChatTemplateFilenameTemp(const char *filename) { replaceStr(chatTemplateFilenameTemp, filename); }
	__forceinline const char* getChatTemplateFilenameTemp() { return chatTemplateFilenameTemp; }

	__forceinline void setHistoryEnable(bool enable) { this->historyEnable = enable; }
	__forceinline bool isHistoryEnable() { return historyEnable; }
	
	__forceinline void setHistoryMode(int mode) { this->historyMode = mode; }
	__forceinline int getHistoryMode() { return historyMode; }
	
	__forceinline void setHistoryFlags(int flags) { this->historyFlags = flags; }
	__forceinline int getHistoryFlags() { return historyFlags; }
	
	__forceinline void setHistoryBackgroundFilename(const char *filename) { replaceStr(historyBackgroundFilename, filename); }
	__forceinline const char* getHistoryBackgroundFilename() { return historyBackgroundFilename; }
	
	__forceinline void setHistoryCssFilename(const char *filename) { replaceStr(historyCssFilename, filename); }
	__forceinline const char* getHistoryCssFilename() { return historyCssFilename; }
	
	              void setHistoryTemplateFilename(const char *filename);
	__forceinline const char* getHistoryTemplateFilename() { return historyTemplateFilename; }

	__forceinline void setHistoryEnableTemp(bool enable) { this->historyEnableTemp = enable; }
	__forceinline bool isHistoryEnableTemp() { return historyEnableTemp; }
	
	__forceinline void setHistoryModeTemp(int mode) { this->historyModeTemp = mode; }
	__forceinline int getHistoryModeTemp() { return historyModeTemp; }
	
	__forceinline void setHistoryFlagsTemp(int flags) { this->historyFlagsTemp = flags; }
	__forceinline int getHistoryFlagsTemp() { return historyFlagsTemp; }
	
	__forceinline void setHistoryBackgroundFilenameTemp(const char *filename) { replaceStr(historyBackgroundFilenameTemp, filename); }
	__forceinline const char* getHistoryBackgroundFilenameTemp() { return historyBackgroundFilenameTemp; }
	
	__forceinline void setHistoryCssFilenameTemp(const char *filename) { replaceStr(historyCssFilenameTemp, filename); }
	__forceinline const char* getHistoryCssFilenameTemp() { return historyCssFilenameTemp; }
	
	__forceinline void setHistoryTemplateFilenameTemp(const char *filename) { replaceStr(historyTemplateFilenameTemp, filename); }
	__forceinline const char* getHistoryTemplateFilenameTemp() { return historyTemplateFilenameTemp; }

	void 	copyToTemp();
	void	copyFromTemp();
};

class Options {
private:
	static int 		generalFlags;
	static bool     isInited;
	static bool     bSmileyAdd;
	static int      avatarServiceFlags;
	static ProtocolSettings* protocolList;
public:
	enum MODES {
		MODE_COMPATIBLE = 0,
		MODE_CSS = 1,
		MODE_TEMPLATE = 2
	};
	enum OPTIONS {
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
	enum AVATARSERVICEFLAGS {
		AVATARSERVICE_PRESENT = 0x0001,
	};

	static void     		setGeneralFlags(int flags);
	static int				getGeneralFlags();
	static void     		setEmbedsize(int size);
	static int				getEmbedsize();

	static bool             isSmileyAdd();
	static int				getAvatarServiceFlags();
	static void      		init();
	static void      		uninit();
	static void				saveProtocolSettings();
	static void				resetProtocolSettings();
	static ProtocolSettings*getProtocolSettings();
	static ProtocolSettings*getProtocolSettings(const char *protocolName);
	static void             Reload();
};

#endif
