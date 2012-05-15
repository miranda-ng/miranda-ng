// Copyright Scott Ellis (mail@scottellis.com.au) 2005
// This software is licenced under the GPL (General Public Licence)
// available at http://www.gnu.org/copyleft/gpl.html

#ifndef _FONT_SERVICE_API_INC
#define _FONT_SERVICE_API_INC

typedef struct FontID_tag {
	int cbSize;
	char group[64];
	char name[64];
	char dbSettingsGroup[32];
	char prefix[32];
	int order;
} FontID;

// register a font
// wparam = (FontID *)&font_id
// lparam = 0
#define MS_FONT_REGISTER		"Font/Register"

// get a font
// wparam = (FontID *)&font_id (only name and group matter)
// lParam = (LOGFONT *)&logfont
#define MS_FONT_GET				"Font/Get"

// fired when a user modifies font settings, so reget your fonts
#define ME_FONT_RELOAD			"Font/Reload"

#endif