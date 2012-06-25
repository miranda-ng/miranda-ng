// Copyright Scott Ellis (mail@scottellis.com.au) 2005
// This software is licenced under the GPL (General Public Licence)
// available at http://www.gnu.org/copyleft/gpl.html

#ifndef _FONT_SERVICE_API_INC
#define _FONT_SERVICE_API_INC

//////////////////////////////////////////////////////////////////////////
//
//  FONTS
//

// style flags
#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4
#define DBFONTF_STRIKEOUT  8

// flags for compatibility
#define FIDF_APPENDNAME			1		// append 'Name' to the setting used to store font face (as CLC settings require)
#define FIDF_NOAS				2		// disable the <prefix>As setting to prevent 'same as' behaviour
#define FIDF_SAVEACTUALHEIGHT	4		// write the actual height of a test string to the db
#define FIDF_SAVEPOINTSIZE		8		// write the font point size to the db

// additional flags
#define FIDF_DEFAULTVALID		32		// the default font settings are valid - else, just use generic default
#define FIDF_NEEDRESTART		64		// setting changes will not take effect until miranda is restarted
#define FIDF_ALLOWREREGISTER	128		// allow plugins to register this font again (i.e. override already registered settings such as flags)
#define FIDF_ALLOWEFFECTS		256		// allow setting of font effects (i.e. underline and strikeout)
#define FIDF_DISABLESTYLES		512		// don't allow to select font attributes (bold/underline/italics)
										// FIDF_ALLOWEFFECTS has priority and will override this flag!

// font class
#define FIDF_CLASSMASK			0x70000000
#define FIDF_CLASSHEADER		0x10000000
#define FIDF_CLASSGENERAL		0x20000000
#define FIDF_CLASSSMALL			0x30000000

// settings to be used for the value of 'deffontsettings' in the FontID structure below - i.e. defaults
typedef struct FontSettings_tag
{
	COLORREF colour;
	char  size;
	BYTE  style;					// see the DBFONTF_* flags above
	BYTE  charset;
	char  szFace[LF_FACESIZE];
}
	FontSettings;

typedef struct FontSettingsW_tag
{
	COLORREF colour;
	char size;
	BYTE style;					// see the DBFONTF_* flags above
	BYTE charset;
	wchar_t szFace[LF_FACESIZE];
}
	FontSettingsW;

#if defined( _UNICODE )
	#define FontSettingsT FontSettingsW
#else
	#define FontSettingsT FontSettings
#endif

// a font identifier structure - used for registering a font, and getting one out again

// WARNING: do not use Translate(TS) for name or group as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
typedef struct FontID_tag
{
	int   cbSize;
	char  group[64];			// [TRANSLATED-BY-CORE] group the font belongs to - this is the 'Font Group' list in the options page
	char  name[64];				// [TRANSLATED-BY-CORE] this is the name of the font setting - e.g. 'contacts' in the 'contact list' group
	char  dbSettingsGroup[32];	// the 'module' in the database where the font data is stored
	char  prefix[32];			// this is prepended to the settings used to store this font's data in the db
	DWORD flags;				// bitwise OR of the FIDF_* flags above
	FontSettings deffontsettings; // defaults, valid if flags & FIDF_DEFAULTVALID
	int   order;					// controls the order in the font group in which the fonts are listed in the UI (if order fields are equal,
								// they will be ordered alphabetically by name)
	char backgroundGroup[64];
	char backgroundName[64];
}
	FontID;

typedef struct FontIDW_tag
{
	int cbSize;
	wchar_t group[64];			// [TRANSLATED-BY-CORE] group the font belongs to - this is the 'Font Group' list in the options page
	wchar_t name[64];			// [TRANSLATED-BY-CORE] this is the name of the font setting - e.g. 'contacts' in the 'contact list' group
	char dbSettingsGroup[32];	// the 'module' in the database where the font data is stored
	char prefix[32];			// this is prepended to the settings used to store this font's data in the db
	DWORD flags;				// bitwise OR of the FIDF_* flags above
	FontSettingsW deffontsettings; // defaults, valid if flags & FIDF_DEFAULTVALID
	int order;					// controls the order in the font group in which the fonts are listed in the UI (if order fields are equal,
								// they will be ordered alphabetically by name)
	wchar_t backgroundGroup[64];
	wchar_t backgroundName[64];
}
	FontIDW;

#if defined( _UNICODE )
	#define FontIDT FontIDW
#else
	#define FontIDT FontID
#endif

// register a font
// wparam = (FontID *)&font_id
// lparam = hLangpack

extern int hLangpack;

__forceinline void FontRegister( FontID* pFontID )
{	CallService("Font/Register", (WPARAM)pFontID, hLangpack);
}

__forceinline void FontRegisterW( FontIDW* pFontID )
{	CallService("Font/RegisterW", (WPARAM)pFontID, hLangpack);
}

#if defined( _UNICODE )
	#define MS_FONT_REGISTERT MS_FONT_REGISTERW
	#define FontRegisterT FontRegisterW
#else
	#define MS_FONT_REGISTERT MS_FONT_REGISTER
	#define FontRegisterT FontRegister
#endif

// get a font
// will fill the logfont structure passed in with the user's choices, or the default if it was set and the user has not chosen a font yet,
// or the global default font settings otherwise (i.e. no user choice and default set, or font not registered)
// global default font is gotten using SPI_GETICONTITLELOGFONT, color COLOR_WINDOWTEXT, size 8.
// wparam = (FontID *)&font_id (only name and group matter)
// lParam = (LOGFONT *)&logfont
// returns the font's colour
#define MS_FONT_GET           "Font/Get"
#define MS_FONT_GETW          "Font/GetW"

#if defined( _UNICODE )
	#define MS_FONT_GETT MS_FONT_GETW
#else
	#define MS_FONT_GETT MS_FONT_GET
#endif

// fired when a user modifies font settings, so reload your fonts
// wparam = lparam = 0
#define ME_FONT_RELOAD        "Font/Reload"

//////////////////////////////////////////////////////////////////////////
//
//  COLOURS
//

// WARNING: do not use Translate(TS) for name or group as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
typedef struct ColourID_tag
{
	int      cbSize;
	char     group[64];	// [TRANSLATED-BY-CORE]
	char     name[64];	// [TRANSLATED-BY-CORE]
	char     dbSettingsGroup[32];
	char     setting[32];
	DWORD    flags;		// not used
	COLORREF defcolour; // default value
	int      order;
}
	ColourID;

// a font identifier structure - used for registering a font, and getting one out again
typedef struct ColourIDW_tag
{
	int cbSize;
	wchar_t group[64];	// [TRANSLATED-BY-CORE]
	wchar_t name[64];	// [TRANSLATED-BY-CORE]
	char dbSettingsGroup[32];
	char setting[32];
	DWORD flags;		// not used
	COLORREF defcolour; // default value
	int order;
}
	ColourIDW;

#if defined( _UNICODE )
	#define ColourIDT ColourIDW
#else
	#define ColourIDT ColourID
#endif

// register a colour (this should be used for everything except actual text colour for registered fonts)
// [note - a colour with name 'Background' [translated!] has special meaning and will be used as the background colour of
// the font list box in the options, for the given group]
// wparam = (ColourID *)&colour_id
// lparam = hLangpack

__forceinline void ColourRegister(ColourID* pColorID)
{	CallService("Colour/Register", (WPARAM)pColorID, hLangpack);
}

__forceinline void ColourRegisterW(ColourIDW* pColorID)
{	CallService("Colour/RegisterW", (WPARAM)pColorID, hLangpack);
}

#if defined( _UNICODE )
	#define MS_COLOUR_REGISTERT MS_COLOUR_REGISTERW
	#define ColourRegisterT ColourRegisterW
#else
	#define MS_COLOUR_REGISTERT MS_COLOUR_REGISTER
	#define ColourRegisterT ColourRegister
#endif

// get a colour
// wparam = (ColourID *)&colour_id (only name and group matter)
// rerturns the colour (as COLORREF), or -1 if not found
#define MS_COLOUR_GET         "Colour/Get"
#define MS_COLOUR_GETW        "Colour/GetW"

#if defined( _UNICODE )
	#define MS_COLOUR_GETT MS_COLOUR_GETW
#else
	#define MS_COLOUR_GETT MS_COLOUR_GET
#endif

// fired when a user modifies font settings, so reget your fonts and colours
// wparam = lparam = 0
#define ME_COLOUR_RELOAD      "Colour/Reload"

//////////////////////////////////////////////////////////////////////////
//
//  EFFECTS
//
typedef struct FONTEFFECT_tag
{
	BYTE     effectIndex;
	DWORD    baseColour;        // ARGB
	DWORD    secondaryColour;   // ARGB
}
	FONTEFFECT;

typedef struct EffectID_tag
{
	int      cbSize;
	char     group[64];
	char     name[64];
	char     dbSettingsGroup[32];
	char     setting[32];
	DWORD    flags;
	FONTEFFECT defeffect;
	int      order;

	FONTEFFECT value;
}
	EffectID;

typedef struct EffectIDW_tag
{
	int      cbSize;
	wchar_t  group[64];
	wchar_t  name[64];
	char     dbSettingsGroup[32];
	char     setting[32];
	DWORD    flags;
	FONTEFFECT defeffect;
	int      order;

	FONTEFFECT value;
}
	EffectIDW;

#if defined( _UNICODE )
	#define EffectIDT EffectIDW
#else
	#define EffectIDT EffectID
#endif

// register an effect
// wparam = (EffectID *)&effect_id
// lparam = 0

__forceinline void EffectRegister(EffectID* pEffectID)
{	CallService("Effect/Register", (WPARAM)pEffectID, hLangpack);
}

__forceinline void EffectRegisterW(EffectIDW* pEffectID)
{	CallService("Effect/RegisterW", (WPARAM)pEffectID, hLangpack);
}

#if defined( _UNICODE )
	#define MS_EFFECT_REGISTERT MS_EFFECT_REGISTERW
	#define EffectRegisterT EffectRegisterW
#else
	#define MS_EFFECT_REGISTERT MS_EFFECT_REGISTER
	#define EffectRegisterT EffectRegister
#endif

// get a effect
// wparam = (EffectID *)&effect_id (only name and group matter)
// lparam = (FONTEFFECT *)&effect
// rerturns 0, or -1 if not found
#define MS_EFFECT_GET         "Effect/Get"
#define MS_EFFECT_GETW        "Effect/GetW"

#if defined( _UNICODE )
	#define MS_EFFECT_GETT MS_EFFECT_GETW
#else
	#define MS_EFFECT_GETT MS_EFFECT_GET
#endif

// fired when a user modifies font settings, so reget your fonts and colours
// wparam = lparam = 0
#define ME_EFFECT_RELOAD      "Effect/Reload"

#endif // _FONT_SERVICE_API_INC
