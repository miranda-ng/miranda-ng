{
  Variables Plugin for Miranda-IM (www.miranda-im.org)
  Copyright 2003-2006 P. Boon

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
}

{$IFNDEF M_VARS}
{$DEFINE M_VARS}

const MIID_VARIABLES:MUUID = '{630756DE-3681-440B-991E-77A4742DA595}';

// --------------------------------------------------------------------------
// String formatting
// --------------------------------------------------------------------------

  MS_VARS_FORMATSTRING:PAnsiChar = 'Vars/FormatString';
{
  This service can be used to parse tokens in a text. The tokens will be
  replaced by their resolved values. A token can either be a field or a
  function. A field takes no arguments and is represented between
  %-characters, e.g. "%winampsong%". A function can take any number of
  arguments and is represented by a ? or !-character followed by the name
  of the function and a list of arguments, e.g. "?add(1,2)".

  Parameters:
  ------------------------
  wParam = (WPARAM)(FORMATINFO *)&fi
    See below.
  lParam = 0

  Return Value:
  ------------------------
  Returns a pointer to the resolved string or NULL in case of an error.

  Note: The returned pointer needs to be freed using mir_free.
}

type
  TFORMATINFO = record
    cbSize     :int;         // Set this to sizeof(FORMATINFO).
    flags      :int;         // Flags to use (see FIF_* below).
    szFormat   :TCHAR;       // Text in which the tokens will be replaced (can't be NULL).
    szExtraText:TCHAR;       // Extra, context-specific string (can be NULL) ->
                             // The field "extratext" will be replaced by this
                             // string. (Previously szSource).
    hContact   :TMCONTACT;   // Handle to contact (can be NULL) -> The field "subject"
                             // represents this contact.
    pCount     :int;         // (output) Number of succesful parsed tokens, needs to
                             // be set to 0 before the call
    eCount     :int;         // (output) Number of failed tokens, needs to be set to
                             // 0 before the call
    szaTemporaryVars:^TChar; // Temporary variables valid only in the duration of the format call
                             // By pos: [i] is var name, [i + 1] is var value
    cbTemporaryVarsSize:int; // Number of elements in szaTemporaryVars array
  end;


const
// Possible flags:
  FIF_UNICODE = 1;  // Expects and returns unicode text (WCHAR*).
  FIF_TCHAR   = FIF_UNICODE; // Strings in structure are TCHAR*.

// --------------------------------------------------------------------------
// Register tokens
// --------------------------------------------------------------------------

// Plugins can define tokens which will be parsed by the Variables plugin.

  MS_VARS_REGISTERTOKEN:PAnsiChar = 'Vars/RegisterToken';
{
  With this service you can define your own token. The newly added tokens
  using this service are taken into account on every call to
  MS_VARS_FORMATSTRING.

  Parameters:
  ------------------------
  wParam = 0
  lParam = (LPARAM)(TOKENREGISTER*)&tr
    See below.

  Return Value:
  ------------------------
  Returns 0 on success, nonzero otherwise. Existing tokens will be
  'overwritten' if registered twice.
}

// Needed for szService and parseFunction:
type
  PARGUMENTSINFO = ^TARGUMENTSINFO;
  TARGUMENTSINFO = record
    cbSize:int;          // You need to check if this is >=sizeof(ARGUMENTSINFO)
                         // (already filled in).
    fi    :^TFORMATINFO; // Arguments passed to MS_VARS_FORMATSTRING.
    argc  :uint;         // Number of elements in the argv array.
    argv  :^TCHAR;       // Argv[0] will be the token name, the following elements
                         // are the additional arguments.
    flags :int;          // (output) You can set flags here (initially 0), use the
                         // AIF_* flags (see below).
  end;

// Available flags for ARGUMENTSINFO:
// Set the flags of the ARGUMENTSINFO struct to any of these to influence
// further parsing.
const
  AIF_DONTPARSE = 1; // Don't parse the result of this function,
                     // usually the result of a token is parsed
                     // again, if the `?` is used as a function character.
  AIF_FALSE = 2;     // The function returned logical false.

// Definition of parse/cleanup functions:
{
typedef AnsiChar* (*VARPARSEFUNCA)(ARGUMENTSINFO *ai);
typedef WCHAR* (*VARPARSEFUNCW)(ARGUMENTSINFO *ai);
typedef void (*VARCLEANUPFUNCA)(AnsiChar *szReturn);
typedef void (*VARCLEANUPFUNCW)(WCHAR *wszReturn);

#define VARPARSEFUNC    VARPARSEFUNCW
#define VARCLEANUPFUNC  VARCLEANUPFUNCW
}
type
  TTOKENREGISTER = record
    cbSize:int;          // Set this to sizeof(TOKENREGISTER).
    szTokenString:TCHAR; // Name of the new token to be created, without %,
                         // ?, ! etc. signs (can't be NULL).
    szService:PAnsiChar; // Name of a service that is used to request the
                     // token's value, if no service is used, a function
                     // and TRF_PARSEFUNC must be used.
                     // [VARPARSEFUNC];
    szCleanupService:PAnsiChar; // Name of a service to be called when the
                            // memory allocated in szService can be freed
                            // (only used when flag VRF_CLEANUP is set,
                            // else set this to NULL).
                            // [VARCLEANUPFUNC]
    szHelpText:PAnsiChar;// Help info shown in help dialog (can be NULL). Has to
                     // be in the following format:
                     // "subject\targuments\tdescription"
                     // (Example: "math\t(x, y ,...)\tx + y + ..."), or:
                     // "subject\tdescription"
                     // (Example: "miranda\tPath to the Miranda-IM
                     // executable").
                     // Note: subject and description are translated by Variables.
    memType:int;     // Describes which method Varibale's plugin needs to use to
                     // free the returned buffer, use one of the VR_MEM_* values
                     // (see below). Only valid if the flag VRF_FREEMEM is set,
                     // use TR_MEM_OWNER otherwise).
    flags  :int;     // Flags to use (see below), one of TRF_* (see below).
  end;

const
// Available Memory Storage Types:
// These values describe which method Variables Plugin will use to free the
// buffer returned by the parse function or service
  TR_MEM_MIRANDA   = 2;  // Memory is allocated using Miranda's Memory
                         // Manager Interface (using the functions
                         // returned by MS_SYSTEM_GET_MMI), if
                         // VRF_FREEMEM is set, the memory will be
                         // freed by Variables.
  TR_MEM_OWNER     = 3;  // Memory is owned by the calling plugin
                         // (can't be freed by Variables Plugin
                         // automatically). This should be used if
                         // VRF_FREEMEM is not specified in the flags.

// Available Flags for TOKENREGISTER:
  TRF_FREEMEM      = $01; // Variables Plugin will automatically free the
                          // pointer returned by the parse function or
                          // service (which method it will us is
                          // specified in memType -> see above).
  TRF_CLEANUP      = $02; // Call cleanup service or function, notifying
                          // that the returned buffer can be freed.
                          // Normally you should use either TRF_FREEMEM
                          // or TRF_CLEANUP.
  TRF_PARSEFUNC    = $40; // parseFunction will be used instead of a service.
  TRF_CLEANUPFUNC  = $80; // cleanupFunction will be used instead of a service.
  TRF_USEFUNCS = TRF_PARSEFUNC or TRF_CLEANUPFUNC;
  TRF_UNPARSEDARGS = $04; // Provide the arguments for the parse
                          // function in their raw (unparsed) form.
                          // By default, arguments are parsed before
                          // presenting them to the parse function.
  TRF_FIELD        = $08; // The token can be used as a %field%.
  TRF_FUNCTION     = $10; // The token can be used as a ?function().
                          // Normally you should use either TRF_FIELD or
                          // TRF_FUNCTION.
  TRF_UNICODE      = $20; // Strings in structure are unicode (WCHAR*).
                          // In this case, the strings pointing to the
                          // arguments in the ARGUMENTS struct are
                          // unicode also. The returned buffer is
                          // expected to be unicode also, and the
                          // unicode parse and cleanup functions are called.

  TRF_TCHAR = TRF_UNICODE; // Strings in structure are TCHAR*.

// Deprecated:
  TRF_CALLSVC = TRF_CLEANUP;

// Callback Service (szService) / parseFunction:
// ------------------------
// Service that is called automatically by the Variable's Plugin to resolve a
// registered variable.

// Parameters:
// wParam = 0
// lParam = (LPARAM)(ARGUMENTSINFO *)&ai
//   see above

// Return Value:
// Needs to return the pointer to a dynamically allocacated string or NULL.
// A return value of NULL is regarded as an error (eCount will be increaded).
// Flags in the ARGUMENTSINFO struct can be set (see above).

// Callback Service (szCallbackService) / cleanupFunction:
// ------------------------
// This service is called when the memory that was allocated by the parse
// function or service can be freed. Note: It will only be called when the
// flag VRF_CLEANUP of TOKENREGISTER is set.

// Parameters:
// wParam = 0
// lParam = (LPARAM)(AnsiChar *)&res
//   Result from parse function or service (pointer to a string).

// Return Value:
// Should return 0 on success.



// --------------------------------------------------------------------------
// Show the help dialog
// --------------------------------------------------------------------------

// Plugins can invoke Variables' help dialog which can be used for easy input
// by users.

  MS_VARS_SHOWHELPEX:PAnsiChar = 'Vars/ShowHelpEx';

// This service can be used to open the help dialog of Variables. This dialog
// provides easy input for the user and/or information about the available
// tokens.

// Parameters:
// ------------------------
// wParam = (WPARAM)(HWND)hwndParent
// lParam = (LPARAM)(VARHELPINFO)&vhi
//   See below.

// Return Value:
// ------------------------
// Returns 0 on succes, any other value on error.

type
  TVARHELPINFO = record
    cbSize:int;      // Set to sizeof(VARHELPINFO).
    fi:^TFORMATINFO; // Used for both input and output. If this pointer is not
                   // NULL, the information is used as the initial values for
                   // the dialog.
    hwndCtrl:HWND; // Used for both input and output. The window text of this
                   // window will be read and used as the initial input of the
                   // input dialog. If the user presses the OK button the window
                   // text of this window will be set to the text of the input
                   // field and a EN_CHANGE message via WM_COMMAND is send to
                   // this window. (Can be NULL).
    szSubjectDesc:PAnsiChar; // The description of the %subject% token will be set
                         // to this text, if not NULL. This is translated
                         // automatically.
    szExtraTextDesc:PAnsiChar; // The description of the %extratext% token will be
                           // set to this text, if not NULL. This is translated
                           // automatically.
    flags:int;  // Flags, see below.
  end;


const
// Flags for VARHELPINFO
  VHF_TOKENS             = $00000001; // Create a dialog with the list of tokens
  VHF_INPUT              = $00000002; // Create a dialog with an input
                                      // field (this contains the list of
                                      // tokens as well).
  VHF_SUBJECT            = $00000004; // Create a dialog to select a
                                      // contact for the %subject% token.
  VHF_EXTRATEXT          = $00000008; // Create a dialog to enter a text
                                      // for the %extratext% token.
  VHF_HELP               = $00000010; // Create a dialog with help info.
  VHF_HIDESUBJECTTOKEN   = $00000020; // Hide the %subject% token in the
                                      // list of tokens.
  VHF_HIDEEXTRATEXTTOKEN = $00000040; // Hide the %extratext% token in
                                      // the list of tokens.
  VHF_DONTFILLSTRUCT     = $00000080; // Don't fill the struct with the
                                      // new information if OK is pressed
  VHF_FULLFILLSTRUCT     = $00000100; // Fill all members of the struct
                                      // when OK is pressed. By default
                                      // only szFormat is set. With this
                                      // flag on, hContact and
                                      // szExtraText are also set.
  VHF_SETLASTSUBJECT     = $00000200; // Set the last contact that was
                                      // used in the %subject% dialog in
                                      // case fi.hContact is NULL.

// Predefined flags
  VHF_FULLDLG    = VHF_INPUT  or VHF_HELP or VHF_SUBJECT or VHF_EXTRATEXT;
  VHF_SIMPLEDLG  = VHF_INPUT  or VHF_HELP;
  VHF_NOINPUTDLG = VHF_TOKENS or VHF_HELP;

// If the service fills information in the struct for szFormat or szExtraText,
// these members must be free'd using the free function of Variables.
// If wParam==NULL, the dialog is created modeless. Only one dialog can be
// shown at the time.
// If both hwndCtrl and fi are NULL, the user input will not be retrievable.
// In this case, the dialog is created with only a "Close" button, instead of
// the "OK" and "Cancel" buttons.
// In case of modeless dialog and fi != NULL, please make sure this pointer
// stays valid while the dialog is open.


  MS_VARS_GETSKINITEM:PAnsiChar = 'Vars/GetSkinItem';
{
  This service can be used to get the icon you can use for example on the
  Variables help button in your options screen. You can also get the tooltip
  text to use with such a button. If icon library is available the icon will
  be retrieved from icon library manager, otherwise the default is returned.

  Parameters:
  ------------------------
  wParam = (WPARAM)0
  lParam = (LPARAM)VSI_* (see below)

  Return Value:
  ------------------------
  Depends on the information to retrieve (see below).
}
// VSI_ constants
  VSI_HELPICON    = 1; // Can be used on the button accessing the
                       // Variables help dialog. Returns (HICON)hIcon on
                       // success or NULL on failure;
  VSI_HELPTIPTEXT = 2; // Returns the tooltip text you can use for the
                       // help button. Returns (AnsiChar *)szTipText, a
                       // static, translated buffer containing the help
                       // text or NULL on error.

  MS_VARS_SHOWHELP:PAnsiChar = 'Vars/ShowHelp';
{
  WARNING: This service is obsolete, please use MS_VARS_SHOWHELPEX

  Shows a help dialog where all possible tokens are displayed. The tokens
  are explained on the dialog, too. The user can edit the initial string and
  insert as many tokens as he likes.

  Parameters:
  ------------------------
  wParam = (HWND)hwndEdit
    Handle to an edit control in which the modified string
    should be inserted (When the user clicks OK in the dialog the edited
    string will be set to hwndEdit) (can be NULL).
  lParam = (AnsiChar *)pszInitialString
    String that the user is provided with initially when
    the dialog gets opened (If this is NULL then the current text in the
    hwndEdit edit control will be used) (can be NULL).

  Return Value:
  ------------------------
  Returns the handle to the help dialog (HWND).

  Note: Only one help dialog can be opened at a time. When the dialog gets
  closed an EN_CHANGE of the edit controll will be triggered because the
  contents were updated. (Only when user selected OK).

  Example:
  CallService(MS_VARS_SHOWHELP, (WPARAM)hwndEdit, (LPARAM)"some initial text");
}

{$ENDIF}
