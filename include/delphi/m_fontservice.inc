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
    group          : array[0..63] of AnsiChar;
    name           : array[0..63] of AnsiChar;
    dbSettingsGroup: array[0..31] of AnsiChar;
    prefix         : array[0..31] of AnsiChar;
    deffontsettings: TFontSettings; // defaults, if flags & FIDF_DEFAULTVALID
    flags          : dword;
    order          : int;
    backgroundGroup:array [0..63] of AnsiChar;
    backgroundName: array [0..63] of AnsiChar;
  end;
  TFontID = TFontID_tag;
  pFontID = ^TFontID;

  PFontIDW_tag = ^TFontIDW_tag;
  TFontIDW_tag = record
    group          : array[0..63] of WideChar;
    name           : array[0..63] of WideChar;
    dbSettingsGroup: array[0..31] of AnsiChar;
    prefix         : array[0..31] of AnsiChar;
    deffontsettings: TFontSettingsW; // defaults, if flags & FIDF_DEFAULTVALID
    flags          : dword;
    order          : int;
    backgroundGroup:array [0..63] of WideChar;
    backgroundName: array [0..63] of WideChar;
  end;
  TFontIDW = TFontIDW_tag;
  PFontIDW = ^TFontIDW;

const
  { fired when a user modifies font settings, so reget your fonts }
  ME_FONT_RELOAD:PAnsiChar = 'Font/Reload';

function Font_Register(pFont:pFontID; pPlugin:Pointer) : int; stdcall; external AppDll;
function Font_Get(pGroup,pName:pAnsiChar; pFont:pLogFontA) : longint; stdcall; external AppDll;

type
  PColourID_tag = ^TColourID_tag;
  TColourID_tag = record
    group           : array[0..63] of AnsiChar;
    name            : array[0..63] of AnsiChar;
    dbSettingsGroup : array[0..31] of AnsiChar;
    setting         : array[0..31] of AnsiChar;
    defcolour       : TCOLORREF; // default value
    order           : int;
  end;
  TColourID = TColourID_tag;
  PColourID = ^TColourID;

  PColourIDW_tag = ^TColourIDW_tag;
  TColourIDW_tag = record
    group           : array[0..63] of WideChar;
    name            : array[0..63] of WideChar;
    dbSettingsGroup : array[0..31] of AnsiChar;
    setting         : array[0..31] of AnsiChar;
    defcolour       : TCOLORREF; // default value
    order           : int;
  end;
  TColourIDW = TColourIDW_tag;
  PColourIDW = ^TColourIDW;

const
  { fired when a user modifies font settings, so reget your fonts and colours }
  ME_COLOUR_RELOAD:PAnsiChar = 'Colour/Reload';

function Colour_Register(pColour:pColourID; pPlugin:Pointer) : int; stdcall; external AppDll;
function Colour_Get(pGroup,pName:pAnsiChar) : longint; stdcall; external AppDll;

{$ENDIF}
