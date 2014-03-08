// Copyright Scott Ellis (mail@scottellis.com.au) 2005
// This software is licenced under the GPL (General Public Licence)
// available at http://www.gnu.org/copyleft/gpl.html
{$IFNDEF FONT_SERVICE_API}
{$DEFINE FONT_SERVICE_API}

const
// style flags
  DBFONTF_BOLD      = 1;
  DBFONTF_ITALIC    = 2;
  DBFONTF_UNDERLINE = 4;
  DBFONTF_STRIKEOUT = 8;
// flags for compatibility
  FIDF_APPENDNAME       = 1;   // append 'Name' to the setting used to store
                               // font face (as CLC settings require)
  FIDF_NOAS             = 2;   // disable the <prefix>As setting to prevent
                               // 'same as' behaviour
  FIDF_SAVEACTUALHEIGHT = 4;   // write the actual height of a test string to the db
  FIDF_SAVEPOINTSIZE    = 8;   // write the font point size to the db
// additional flags
  FIDF_DEFAULTVALID     = 32;  // the default font settings are valid - else,
                               // just use generic default
  FIDF_NEEDRESTART      = 64;  // setting changes will not take effect until
                               // miranda is restarted
  FIDF_ALLOWREREGISTER  = 128; // allow plugins to register this font again
                               // (i.e. override already registered settings
                               // such as flags)
  FIDF_ALLOWEFFECTS     = 256; // allow setting of font effects
                               // (i.e. underline and strikeout)
  FIDF_DISABLESTYLES    = 512; // don't allow to select font attributes (bold/underline/italics)
                               // FIDF_ALLOWEFFECTS has priority and will override this flag!

const
// font class
  FIDF_CLASSMASK    = $70000000;
  FIDF_CLASSHEADER  = $10000000;
  FIDF_CLASSGENERAL = $20000000;
  FIDF_CLASSSMALL   = $30000000;

type
// settings to be used for the value of 'deffontsettings' in the FontID
// structure below - i.e. defaults
  PFontSettings_tag = ^TFontSettings_tag;
  TFontSettings_tag = record
    colour  : TCOLORREF;
    size    : ShortInt;
    style   : byte; // see the DBFONTF_* flags above
    charset : byte;
    szFace  : array[0..(LF_FACESIZE)-1] of AnsiChar;
  end;
  TFontSettings = TFontSettings_tag;
  PFontSettings = ^TFontSettings;

  PFontSettingsW_tag = ^TFontSettingsW_tag;
  TFontSettingsW_tag = record
    colour  : TCOLORREF;
    size    : ShortInt;
    style   : byte; // see the DBFONTF_* flags above
    charset : byte;
    szFace  : array[0..(LF_FACESIZE)-1] of WideChar;
  end;
  TFontSettingsW = TFontSettingsW_tag;
  PFontSettingsW = ^TFontSettingsW;

type
// a font identifier structire - used for registering a font,
// and getting one out again
  PFontID_tag = ^TFontID_tag;
  TFontID_tag = record
    cbSize         : int;
    // [TRANSLATED-BY-CORE] group the font belongs to - this is the 'Font Group' list in the options page
    group          : array[0..63] of AnsiChar;
    // [TRANSLATED-BY-CORE] the name of the font setting e.g. 'contacts' in the 'contact list' group
    name           : array[0..63] of AnsiChar;
    // the 'module' in the database where the font data is stored
    dbSettingsGroup: array[0..31] of AnsiChar;
    // prepended to the settings used to store this font's data in the db
    prefix         : array[0..31] of AnsiChar;
    // bitwise OR of the FIDF_* flags above
    flags          : dword;
    deffontsettings: TFontSettings; // defaults, if flags & FIDF_DEFAULTVALID
    // controls the order in the font group in which the fonts are listed in the
    // UI (if order fields are equal, they will be ordered alphabetically by name)
    order          : int;
    backgroundGroup:array [0..63] of AnsiChar;
    backgroundName: array [0..63] of AnsiChar;
  end;
  TFontID = TFontID_tag;
  pFontID = ^TFontID;

  PFontIDW_tag = ^TFontIDW_tag;
  TFontIDW_tag = record
    cbSize         : int;
    // [TRANSLATED-BY-CORE] group the font belongs to - this is the 'Font Group' list in the options page
    group          : array[0..63] of WideChar;
    // [TRANSLATED-BY-CORE] the name of the font setting e.g. 'contacts' in the 'contact list' group
    name           : array[0..63] of WideChar;
    // the 'module' in the database where the font data is stored
    dbSettingsGroup: array[0..31] of AnsiChar;
    // prepended to the settings used to store this font's data in the db
    prefix         : array[0..31] of AnsiChar;
    // bitwise OR of the FIDF_* flags above
    flags          : dword;
    deffontsettings: TFontSettingsW; // defaults, if flags & FIDF_DEFAULTVALID
    // controls the order in the font group in which the fonts are listed in the
    // UI (if order fields are equal, they will be ordered alphabetically by name)
    order          : int;
    backgroundGroup:array [0..63] of WideChar;
    backgroundName: array [0..63] of WideChar;
  end;
  TFontIDW = TFontIDW_tag;
  PFontIDW = ^TFontIDW;

const
  { register a font }
  { wparam = pointer to FontID structure}
  { lparam = hLangpack }
  MS_FONT_REGISTERA:PAnsiChar = 'Font/Register';
  MS_FONT_REGISTERW:PAnsiChar = 'Font/RegisterW';

  { get a font }
  { wparam = pointer to FontID structure (only name and group matter) }
  { lParam = pointer to LOGFONT structure }
  { returns the font's colour }
  MS_FONT_GETA:PAnsiChar = 'Font/Get';
  MS_FONT_GETW:PAnsiChar = 'Font/GetW';

  { fired when a user modifies font settings, so reget your fonts }
  ME_FONT_RELOAD:PAnsiChar = 'Font/Reload';

type
  PColourID_tag = ^TColourID_tag;
  TColourID_tag = record
    cbSize          : int;
    group           : array[0..63] of AnsiChar;
    name            : array[0..63] of AnsiChar;
    dbSettingsGroup : array[0..31] of AnsiChar;
    setting         : array[0..31] of AnsiChar;
    flags           : dword;     // not used
    defcolour       : TCOLORREF; // default value
    order           : int;
  end;
  TColourID = TColourID_tag;
  PColourID = ^TColourID;

  PColourIDW_tag = ^TColourIDW_tag;
  TColourIDW_tag = record
    cbSize          : int;
    group           : array[0..63] of WideChar;
    name            : array[0..63] of WideChar;
    dbSettingsGroup : array[0..31] of AnsiChar;
    setting         : array[0..31] of AnsiChar;
    flags           : dword;     // not used
    defcolour       : TCOLORREF; // default value
    order           : int;
  end;
  TColourIDW = TColourIDW_tag;
  PColourIDW = ^TColourIDW;

const
  { register a colour (this should be used for everything except actual text colour for registered fonts) }
  { [note - a colour with name 'Background' has special meaning and will be used as the background colour of  }
  { the font list box in the options, for the given group] }
  { wparam = pointer to ColourID structure }
  { lparam = hLangpack }
  MS_COLOUR_REGISTERA:PAnsiChar = 'Colour/Register';
  MS_COLOUR_REGISTERW:PAnsiChar = 'Colour/RegisterW';

  { get a colour }
  { wparam = pointer to ColourID structure (only name and group matter) }
  { returns the colour (as COLORREF), or -1 if not found }
  MS_COLOUR_GETA:PAnsiChar = 'Colour/Get';
  MS_COLOUR_GETW:PAnsiChar = 'Colour/GetW';

  { fired when a user modifies font settings, so reget your fonts and colours }
  ME_COLOUR_RELOAD:PAnsiChar = 'Colour/Reload';

var
  MS_FONT_REGISTER  :PAnsiChar absolute MS_FONT_REGISTERW;
  MS_FONT_GET       :PAnsiChar absolute MS_FONT_GETW;
  MS_COLOUR_REGISTER:PAnsiChar absolute MS_COLOUR_REGISTERW;
  MS_COLOUR_GET     :PAnsiChar absolute MS_COLOUR_GETW;


//////////////////////////////////////////////////////////////////////////
//
//  EFFECTS
//
type
  FONTEFFECT_tag = record
    effectIndex    :byte;
    baseColour     :dword; // ARGB
    secondaryColour:dword; // ARGB
  end;
  TFONTEFFECT = FONTEFFECT_tag;

type
  EffectID_tag = record
    cbSize         :int;
    group          :array [0..63] of AnsiChar;
    name           :array [0..63] of AnsiChar;
    dbSettingsGroup:array [0..31] of AnsiChar;
    setting        :array [0..31] of AnsiChar;
    flags          :dword;
    defeffect      :TFONTEFFECT;
    order          :int;
    value          :TFONTEFFECT;
  end;
  PEffectID = ^TEffectID;
  TEffectID = EffectID_tag;

type
  EffectIDW_tag = record
    cbSize         :int;
    group          :array [0..63] of WideChar;
    name           :array [0..63] of WideChar;
    dbSettingsGroup:array [0..31] of AnsiChar;
    setting        :array [0..31] of AnsiChar;
    flags          :dword;
    defeffect      :TFONTEFFECT;
    order          :int;
    value          :TFONTEFFECT;
  end;
  PEffectIDW = ^TEffectIDW;
  TEffectIDW = EffectIDW_tag;

const
// register an effect
// wparam = (EffectID *)&effect_id
// lparam = hLangpack
  MS_EFFECT_REGISTERA:PAnsiChar = 'Effect/Register';
  MS_EFFECT_REGISTERW:PAnsiChar = 'Effect/RegisterW';

// get a effect
// wparam = (EffectID *)&effect_id (only name and group matter)
// lparam = (FONTEFFECT *)&effect
// rerturns 0, or -1 if not found
  MS_EFFECT_GET :PAnsiChar = 'Effect/Get';
  MS_EFFECT_GETW:PAnsiChar = 'Effect/GetW';

// fired when a user modifies font settings, so reget your fonts and colours
// wparam = lparam = 0
  ME_EFFECT_RELOAD:PAnsiChar = 'Effect/Reload';

{$ENDIF}
