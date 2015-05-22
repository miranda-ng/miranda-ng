/*
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
*/

#ifndef __M_VARS
#define __M_VARS

#if !defined(_TCHAR_DEFINED)
	#include <tchar.h>
#endif

#if !defined(M_CORE_H__)
	#include <m_core.h>
#endif

#ifndef VARIABLES_NOHELPER
	#include <m_button.h>
#endif

#ifndef SIZEOF
	#include <win2k.h>
#endif

// --------------------------------------------------------------------------
// String formatting
// --------------------------------------------------------------------------

#define MS_VARS_FORMATSTRING    "Vars/FormatString"

// This service can be used to parse tokens in a text. The tokens will be
// replaced by their resolved values. A token can either be a field or a
// function. A field takes no arguments and is represented between
// %-characters, e.g. "%winampsong%". A function can take any number of
// arguments and is represented by a ? or !-character followed by the name
// of the function and a list of arguments, e.g. "?add(1,2)".

// Parameters:
// ------------------------
// wParam = (WPARAM)(FORMATINFO *)&fi
//   See below.
// lParam = 0

// Return Value:
// ------------------------
// Returns a pointer to the resolved string or NULL in case of an error.

// Note: The returned pointer needs to be freed using mir_free().

typedef struct {
  int cbSize;  // Set this to sizeof(FORMATINFO).
  int flags;  // Flags to use (see FIF_* below).
  union {
    char *szFormat;  // Text in which the tokens will be replaced (can't be
                     // NULL).
    WCHAR *wszFormat;
    TCHAR *tszFormat;
  };
  union {
    char *szExtraText;  // Extra, context-specific string (can be NULL) ->
                        // The field "extratext" will be replaced by this
                        // string. (Previously szSource).
    WCHAR *wszExtraText;
    TCHAR *tszExtraText;
  };
  MCONTACT hContact;  // Handle to contact (can be NULL) -> The field "subject"
                    // represents this contact.
  int pCount;  // (output) Number of succesful parsed tokens, needs to be set
               // to 0 before the call
  int eCount;  // (output) Number of failed tokens, needs to be set to 0
               // before the call
  union {
     char **szaTemporaryVars;	// Temporary variables valid only in the duration of the format call
     TCHAR **tszaTemporaryVars;	// By pos: [i] is var name, [i + 1] is var value
     WCHAR **wszaTemporaryVars;
  };
  int cbTemporaryVarsSize;		// Number of elements in szaTemporaryVars array

} FORMATINFO;

#define FORMATINFOV2_SIZE (sizeof(int)*4+sizeof(void*)*2 + sizeof(HANDLE))

// Possible flags:
#define FIF_UNICODE 0x01  // Expects and returns unicode text (WCHAR*).

#if defined(UNICODE) || defined(_UNICODE)
#define FIF_TCHAR   FIF_UNICODE // Strings in structure are TCHAR*.
#else
#define FIF_TCHAR   0
#endif

// Helper functions for easy using:

// Helper #1: variables_parse
// ------------------------
// The returned string needs to be freed using mir_free.

#ifndef VARIABLES_NOHELPER
__inline static TCHAR *variables_parse(TCHAR *tszFormat, TCHAR *tszExtraText, MCONTACT hContact)
{
	FORMATINFO fi = { sizeof(fi) };
	fi.tszFormat = tszFormat;
	fi.tszExtraText = tszExtraText;
	fi.hContact = hContact;
	fi.flags = FIF_TCHAR;
	return (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
}
#endif

__inline static TCHAR *variables_parse_ex(TCHAR *tszFormat, TCHAR *tszExtraText, MCONTACT hContact,
 										  TCHAR **tszaTemporaryVars, int cbTemporaryVarsSize) {

	FORMATINFO fi = { 0 };

	ZeroMemory(&fi, sizeof(fi));
	fi.cbSize = sizeof(fi);
	fi.tszFormat = tszFormat;
	fi.tszExtraText = tszExtraText;
	fi.hContact = hContact;
	fi.flags = FIF_TCHAR;
	fi.tszaTemporaryVars = tszaTemporaryVars;
	fi.cbTemporaryVarsSize = cbTemporaryVarsSize;
	return (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
}

// Helper #2: variables_parsedup
// ------------------------
// Returns a _strdup()'ed copy of the unparsed string when Variables is not
// installed, returns a strdup()'ed copy of the parsed result otherwise.

// Note: The returned pointer needs to be released using your own free().

#ifndef VARIABLES_NOHELPER
__inline static TCHAR *variables_parsedup(TCHAR *tszFormat, TCHAR *tszExtraText, MCONTACT hContact)
{
	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		FORMATINFO fi = { sizeof(fi) };
		fi.tszFormat = tszFormat;
		fi.tszExtraText = tszExtraText;
		fi.hContact = hContact;
		fi.flags |= FIF_TCHAR;
		TCHAR *tszParsed = (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		if (tszParsed)
			return tszParsed;
	}
	return tszFormat ? mir_tstrdup(tszFormat) : tszFormat;
}

__inline static TCHAR *variables_parsedup_ex(TCHAR *tszFormat, TCHAR *tszExtraText, MCONTACT hContact,
										  TCHAR **tszaTemporaryVars, int cbTemporaryVarsSize)
{
	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		FORMATINFO fi = { sizeof(fi) };
		fi.tszFormat = tszFormat;
		fi.tszExtraText = tszExtraText;
		fi.hContact = hContact;
		fi.flags |= FIF_TCHAR;
		fi.tszaTemporaryVars = tszaTemporaryVars;
		fi.cbTemporaryVarsSize = cbTemporaryVarsSize;
		TCHAR *tszParsed = (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		if (tszParsed)
			return tszParsed;
	}
	return tszFormat ? mir_tstrdup(tszFormat) : tszFormat;
}
#endif



// --------------------------------------------------------------------------
// Register tokens
// --------------------------------------------------------------------------

// Plugins can define tokens which will be parsed by the Variables plugin.

#define MS_VARS_REGISTERTOKEN "Vars/RegisterToken"

// With this service you can define your own token. The newly added tokens
// using this service are taken into account on every call to
// MS_VARS_FORMATSTRING.

// Parameters:
// ------------------------
// wParam = 0
// lParam = (LPARAM)(TOKENREGISTER*)&tr
//   See below.

// Return Value:
// ------------------------
// Returns 0 on success, nonzero otherwise. Existing tokens will be
// 'overwritten' if registered twice.

// Needed for szService and parseFunction:
typedef struct {
  int cbSize;  // You need to check if this is >=sizeof(ARGUMENTSINFO)
               // (already filled in).
  FORMATINFO *fi;  // Arguments passed to MS_VARS_FORMATSTRING.
  unsigned int argc;  // Number of elements in the argv array.
  union {
    char **argv; // Argv[0] will be the token name, the following elements
                 // are the additional arguments.
    WCHAR **wargv;  // If the registered token was registered as a unicode
                    // token, wargv should be accessed.
    TCHAR **targv;
  };
  int flags;  // (output) You can set flags here (initially 0), use the
              // AIF_* flags (see below).
} ARGUMENTSINFO;

// Available flags for ARGUMENTSINFO:
// Set the flags of the ARGUMENTSINFO struct to any of these to influence
// further parsing.
#define AIF_DONTPARSE   0x01    // Don't parse the result of this function,
                                // usually the result of a token is parsed
                                // again, if the `?` is used as a function
                                // character.
#define AIF_FALSE       0x02    // The function returned logical false.

// Definition of parse/cleanup functions:
typedef char* (*VARPARSEFUNCA)(ARGUMENTSINFO *ai);
typedef WCHAR* (*VARPARSEFUNCW)(ARGUMENTSINFO *ai);
typedef void (*VARCLEANUPFUNCA)(char *szReturn);
typedef void (*VARCLEANUPFUNCW)(WCHAR *wszReturn);

#if defined(UNICODE) || defined(_UNICODE)
#define VARPARSEFUNC    VARPARSEFUNCW
#define VARCLEANUPFUNC  VARCLEANUPFUNCW
#else
#define VARPARSEFUNC    VARPARSEFUNCA
#define VARCLEANUPFUNC  VARCLEANUPFUNCA
#endif

typedef struct {
  int cbSize;  // Set this to sizeof(TOKENREGISTER).
  union {
    char *szTokenString;  // Name of the new token to be created, without %,
                          // ?, ! etc. signs (can't be NULL).
    WCHAR *wszTokenString;
    TCHAR *tszTokenString;
  };
  union {
    char *szService;  // Name of a service that is used to request the
                      // token's value, if no service is used, a function
                      // and TRF_PARSEFUNC must be used.
    VARPARSEFUNCA parseFunction;  // See above, use with TRF_PARSEFUNC.
    VARPARSEFUNCW parseFunctionW;
    VARPARSEFUNC parseFunctionT;
  };
  union {
    char *szCleanupService;  // Name of a service to be called when the
                             // memory allocated in szService can be freed
                             // (only used when flag VRF_CLEANUP is set,
                             // else set this to NULL).
    VARCLEANUPFUNCA cleanupFunction;  // See above, use with TRF_CLEANUPFUNC.
    VARCLEANUPFUNCW cleanupFunctionW;
    VARCLEANUPFUNC cleanupFunctionT;
  };
  char *szHelpText;  // Help info shown in help dialog (can be NULL). Has to
                     // be in the following format:
                     // "subject\targuments\tdescription"
                     // (Example: "math\t(x, y ,...)\tx + y + ..."), or:
                     // "subject\tdescription"
                     // (Example: "miranda\tPath to the Miranda-IM
                     // executable").
                     // Note: subject and description are translated by
                     // Variables.
  int memType;  // Describes which method Varibale's plugin needs to use to
                // free the returned buffer, use one of the VR_MEM_* values
                // (see below). Only valid if the flag VRF_FREEMEM is set,
                // use TR_MEM_OWNER otherwise).
  int flags; // Flags to use (see below), one of TRF_* (see below).
} TOKENREGISTER;

// Available Memory Storage Types:
// These values describe which method Variables Plugin will use to free the
// buffer returned by the parse function or service
#define TR_MEM_MIRANDA      2  // Memory is allocated using Miranda's Memory
                               // Manager Interface (using the functions
                               // returned by MS_SYSTEM_GET_MMI), if
                               // VRF_FREEMEM is set, the memory will be
                               // freed by Variables.
#define TR_MEM_OWNER        3  // Memory is owned by the calling plugin
                               // (can't be freed by Variables Plugin
                               // automatically). This should be used if
                               // VRF_FREEMEM is not specified in the flags.

// Available Flags for TOKENREGISTER:
#define TRF_FREEMEM     0x01  // Variables Plugin will automatically free the
                              // pointer returned by the parse function or
                              // service (which method it will us is
                              // specified in memType -> see above).
#define TRF_CLEANUP     0x02  // Call cleanup service or function, notifying
                              // that the returned buffer can be freed.
                              // Normally you should use either TRF_FREEMEM
                              // or TRF_CLEANUP.
#define TRF_PARSEFUNC   0x40  // parseFunction will be used instead of a
                              // service.
#define TRF_CLEANUPFUNC 0x80  // cleanupFunction will be used instead of a
                              // service.
#define TRF_USEFUNCS    TRF_PARSEFUNC|TRF_CLEANUPFUNC
#define TRF_UNPARSEDARGS    0x04  // Provide the arguments for the parse
                                  // function in their raw (unparsed) form.
                                  // By default, arguments are parsed before
                                  // presenting them to the parse function.
#define TRF_FIELD       0x08  // The token can be used as a %field%.
#define TRF_FUNCTION    0x10  // The token can be used as a ?function().
                              // Normally you should use either TRF_FIELD or
                              // TRF_FUNCTION.
#define TRF_UNICODE     0x20  // Strings in structure are unicode (WCHAR*).
                              // In this case, the strings pointing to the
                              // arguments in the ARGUMENTS struct are
                              // unicode also. The returned buffer is
                              // expected to be unicode also, and the
                              // unicode parse and cleanup functions are
                              // called.

#if defined(UNICODE) || defined(_UNICODE)
#define TRF_TCHAR   TRF_UNICODE // Strings in structure are TCHAR*.
#else
#define TRF_TCHAR   0
#endif

// Deprecated:
#define TRF_CALLSVC     TRF_CLEANUP

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
// lParam = (LPARAM)(char *)&res
//   Result from parse function or service (pointer to a string).

// Return Value:
// Should return 0 on success.



// --------------------------------------------------------------------------
// Show the help dialog
// --------------------------------------------------------------------------

// Plugins can invoke Variables' help dialog which can be used for easy input
// by users.

#define MS_VARS_SHOWHELPEX    "Vars/ShowHelpEx"

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

typedef struct {
  int cbSize;  // Set to sizeof(VARHELPINFO).
  FORMATINFO *fi;  // Used for both input and output. If this pointer is not
                   // NULL, the information is used as the initial values for
                   // the dialog.
  HWND hwndCtrl;  // Used for both input and output. The window text of this
                  // window will be read and used as the initial input of the
                  // input dialog. If the user presses the OK button the window
                  // text of this window will be set to the text of the input
                  // field and a EN_CHANGE message via WM_COMMAND is send to
                  // this window. (Can be NULL).
  char *szSubjectDesc;  // The description of the %subject% token will be set
                        // to this text, if not NULL. This is translated
                        // automatically.
  char *szExtraTextDesc;  // The description of the %extratext% token will be
                          // set to this text, if not NULL. This is translated
                          // automatically.
  int flags;  // Flags, see below.
} VARHELPINFO;


// Flags for VARHELPINFO
#define VHF_TOKENS              0x00000001  // Create a dialog with the list of
                                            // tokens
#define VHF_INPUT               0x00000002  // Create a dialog with an input
                                            // field (this contains the list of
                                            // tokens as well).
#define VHF_SUBJECT             0x00000004  // Create a dialog to select a
                                            // contact for the %subject% token.
#define VHF_EXTRATEXT           0x00000008  // Create a dialog to enter a text
                                            // for the %extratext% token.
#define VHF_HELP                0x00000010  // Create a dialog with help info.
#define VHF_HIDESUBJECTTOKEN    0x00000020  // Hide the %subject% token in the
                                            // list of tokens.
#define VHF_HIDEEXTRATEXTTOKEN  0x00000040  // Hide the %extratext% token in
                                            // the list of tokens.
#define VHF_DONTFILLSTRUCT      0x00000080  // Don't fill the struct with the
                                            // new information if OK is pressed
#define VHF_FULLFILLSTRUCT      0x00000100  // Fill all members of the struct
                                            // when OK is pressed. By default
                                            // only szFormat is set. With this
                                            // flag on, hContact and
                                            // szExtraText are also set.
#define VHF_SETLASTSUBJECT      0x00000200  // Set the last contact that was
                                            // used in the %subject% dialog in
                                            // case fi.hContact is NULL.

// Predefined flags
#define VHF_FULLDLG             VHF_INPUT|VHF_SUBJECT|VHF_EXTRATEXT|VHF_HELP
#define VHF_SIMPLEDLG           VHF_INPUT|VHF_HELP
#define VHF_NOINPUTDLG          VHF_TOKENS|VHF_HELP

// If the service fills information in the struct for szFormat or szExtraText,
// these members must be free'd using the free function of Variables.
// If wParam==NULL, the dialog is created modeless. Only one dialog can be
// shown at the time.
// If both hwndCtrl and fi are NULL, the user input will not be retrievable.
// In this case, the dialog is created with only a "Close" button, instead of
// the "OK" and "Cancel" buttons.
// In case of modeless dialog and fi != NULL, please make sure this pointer
// stays valid while the dialog is open.

// Helper function for easy use in standard case:
#ifndef VARIABLES_NOHELPER
__inline static int variables_showhelp(HWND hwndDlg, UINT uIDEdit, int flags, char *szSubjectDesc, char *szExtraDesc) {

  VARHELPINFO vhi;

  ZeroMemory(&vhi, sizeof(VARHELPINFO));
  vhi.cbSize = sizeof(VARHELPINFO);
  if (flags == 0) {
    flags = VHF_SIMPLEDLG;
  }
  vhi.flags = flags;
  vhi.hwndCtrl = GetDlgItem(hwndDlg, uIDEdit);
  vhi.szSubjectDesc = szSubjectDesc;
  vhi.szExtraTextDesc = szExtraDesc;

  return CallService(MS_VARS_SHOWHELPEX, (WPARAM)hwndDlg, (LPARAM)&vhi);
}
#endif


#define MS_VARS_GETSKINITEM	  "Vars/GetSkinItem"

// This service can be used to get the icon you can use for example on the
// Variables help button in your options screen. You can also get the tooltip
// text to use with such a button. If icon library is available the icon will
// be retrieved from icon library manager, otherwise the default is returned.

// Parameters:
// ------------------------
// wParam = 0
// lParam = (LPARAM)VSI_* (see below)

// Return Value:
// ------------------------
// Depends on the information to retrieve (see below).

// VSI_ constants
#define VSI_HELPICON    1  // Can be used on the button accessing the
                           // Variables help dialog. Returns (HICON)hIcon on
                           // success or NULL on failure;
#define VSI_HELPTIPTEXT 2  // Returns the tooltip text you can use for the
                           // help button. Returns (char *)szTipText, a
                           // static, translated buffer containing the help
                           // text or NULL on error.

// Helper to set the icon on a button accessing the help dialog.
// Preferably a 16x14 MButtonClass control, but it works on a standard
// button control as well. If no icon is availble (because of old version of
// Variables) the string "V" is shown on the button. If Variables is not
// available, the button will be hidden.
#ifndef VARIABLES_NOHELPER
__inline static int variables_skin_helpbutton(HWND hwndDlg, UINT uIDButton) {

	int res;
	HICON hIcon;
	TCHAR tszClass[32];

	hIcon = NULL;
	res = 0;
	if (ServiceExists(MS_VARS_GETSKINITEM))
		hIcon = (HICON)CallService(MS_VARS_GETSKINITEM, 0, (LPARAM)VSI_HELPICON);

	GetClassName(GetDlgItem(hwndDlg, uIDButton), tszClass, SIZEOF(tszClass));
	if (!mir_tstrcmp(tszClass, _T("Button"))) {
		if (hIcon != NULL) {
			SetWindowLongPtr(GetDlgItem(hwndDlg, uIDButton), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, uIDButton), GWL_STYLE)|BS_ICON);
			SendMessage(GetDlgItem(hwndDlg, uIDButton), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
		}
		else {
			SetWindowLongPtr(GetDlgItem(hwndDlg, uIDButton), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, uIDButton), GWL_STYLE)&~BS_ICON);
			SetDlgItemText(hwndDlg, uIDButton, _T("V"));
		}
	}
	else if (!mir_tstrcmp(tszClass, MIRANDABUTTONCLASS)) {
		if (hIcon != NULL) {
			char *szTipInfo = NULL;

			SendMessage(GetDlgItem(hwndDlg, uIDButton), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
			if (ServiceExists(MS_VARS_GETSKINITEM))
				szTipInfo = (char *)CallService(MS_VARS_GETSKINITEM, 0, (LPARAM)VSI_HELPTIPTEXT);

			if (szTipInfo == NULL)
				szTipInfo = Translate("Open String Formatting Help");

			SendMessage(GetDlgItem(hwndDlg, uIDButton), BUTTONADDTOOLTIP, (WPARAM)szTipInfo, 0);
			SendDlgItemMessage(hwndDlg, uIDButton, BUTTONSETASFLATBTN, 0, 0);
		}
		else SetDlgItemText(hwndDlg, uIDButton, _T("V"));
	}
	else res = -1;

	ShowWindow(GetDlgItem(hwndDlg, uIDButton), ServiceExists(MS_VARS_FORMATSTRING));

	return res;
}
#endif

#define MS_VARS_SHOWHELP    "Vars/ShowHelp"

// WARNING: This service is obsolete, please use MS_VARS_SHOWHELPEX

// Shows a help dialog where all possible tokens are displayed. The tokens
// are explained on the dialog, too. The user can edit the initial string and
// insert as many tokens as he likes.

// Parameters:
// ------------------------
// wParam = (HWND)hwndEdit
//   Handle to an edit control in which the modified string
//   should be inserted (When the user clicks OK in the dialog the edited
//   string will be set to hwndEdit) (can be NULL).
// lParam = (char *)pszInitialString
//   String that the user is provided with initially when
//   the dialog gets opened (If this is NULL then the current text in the
//   hwndEdit edit control will be used) (can be NULL).

// Return Value:
// ------------------------
// Returns the handle to the help dialog (HWND).

// Note: Only one help dialog can be opened at a time. When the dialog gets
// closed an EN_CHANGE of the edit controll will be triggered because the
// contents were updated. (Only when user selected OK).

// Example:
// CallService(MS_VARS_SHOWHELP, (WPARAM)hwndEdit, (LPARAM)"some initial text");


#endif //__M_VARS
