{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright © 2003-2004 Heiko Herkenrath

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
}

{$IFNDEF M_SPAMFILTER}
{$DEFINE M_SPAMFILTER}

const
// Spam Filter Version: 2.5.2.1
// See "spamcheck.c" in the sources for an example on how to use the spam checking services.

// The functionality and the API of Spam Filter is quite complex
// and not easy to describe or explain.
// If you have any question or extension whishes please send me an e-mail:
// hrathh at users.sourceforge.net

// Notes:
// * All services are fully thread-safe, you can call everything from any thread
//   (If there are problems with it it is a bug :-) )
// * Unicode parameters can also be passed to the ANSI version of the plugin
//   and the other way round. The strings get converted as they are needed
//   using automatic two-way translation.
//   Unicode support was added in v2.5.0.0.

// -------------------------------------------
// -- Getting handles ------------------------
// -------------------------------------------

  MS_SPAMFILTER_GETHANDLE = 'SpamFilter/GetHandle';

// Description:
// -------------
// Gets a specified icon/window handle.

// Parameters:
// -------------
// wParam = (uint)uHandleType (see below);
// lParam = 0

// Possible values for wParam:
  SFHT_HWND_PLUGIN_OPTIONS          = 2; // HWND of the "Spam Filter" options dialog (if it is loaded; else NULL)
  SFHT_HICON_SPAM                   = 3; // HICON of the spam icon (small): needs to be destroyed! (DestroyIcon())
  // Added in v2.0.2.0:
  SFHT_HICON_SPAM_LARGE             = 4; // HICON of the spam icon (large): needs to be destroyed! (DestroyIcon())
  SFHT_HICON_SPAM_LAYER             = 5; // HICON of the spam layer icon (small): needs to be destroyed! (DestroyIcon())
                                         // Icons are taken directly from resources, not skinned by user
  // Added in v2.0.3.0:
  SFHT_HWND_ADVERTISMENT_FILTER     = 6; // HWND of the "Advertisment Filter" settings dialog (if it is opened; else NULL)
  SFHT_HWND_ROBOT_FILTER            = 7; // HWND of the "Robot Filter" settings dialog (if it is opened; else NULL)
  SFHT_HWND_DISLIKEDMESSAGES_FILTER = 8; // HWND of the "Disliked Messages Filter" settings dialog (if it is opened; else NULL)
  // Added in v2.5.0.0:
  SFHT_HWND_SPAMDEFINITIONS_INFO    = 9;  // HWND of the "Spam Definitions" info dialog (if it is opened; else NULL)
  SFHT_HWND_SPAMMERS_INFO           = 10; // HWND of the Spammers info dialog (if it is opened; else NULL)

  SFHT_BOOL_IS_UNICODE              = 100; // No handle, returns TRUE if Spam Filter is installed as Unicode version, FALSE otherwise
            // (You probably never need to call this since Spam Filter performs automatic two-way translation
            // when a service is called)

// Note: Icons are taken directly from resources, they are not skinned by user.


// Return Values:
// --------------
// Returns the specified handle value: HINSTANCE, HWND, ICON
//
// Note:
// If a handle is not available (or if it was removed) the function will
// return NULL.
// When you are using this service you need to check always for the
// NULL return value.

// -------------------------------------------
// -- Activating/Deactivating the filters ----
// -------------------------------------------

  MS_SPAMFILTER_CHANGEFILTERACTIVATION = 'SpamFilter/ChangeFilterActivation';

// Description:
// -------------
// Provides the possibility to enable or disable the spam filters.
// Also updates the checkboxes in the spam filter options if the option dialog is opened.

// Parameters:
// -------------
// wParam = (uint)uFilterType (For possible values see below)
// lParam = (bool)bNewState (new activation state of filter)

// Possible filter type values (uFilterType):
  SFT_ADVERTISMENT_FILTER     = 1;
  SFT_DISLIKEDMESSAGES_FILTER = 2;
  SFT_ROBOT_FILTER            = 3;

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.


// -------------------------------------------
// -- Showing configure dialogs --------------
// -------------------------------------------

  MS_SPAMFILTER_SHOWFILTERDIALOG = 'SpamFilter/ShowFilterDialog';

// Description:
// -------------
// Shows the configure dialog of the specified filter.

// Parameters:
// -------------
// wParam = (HWND)hwndParent (Can be NULL)
// lParam = (uint)uFilterType (For possible values see above)
//          -> since 2.0.2.0: If this is zero then the options dialog will be opened pointing to the Spam Filter options.
// Note:
// If hwndParent is not NULL the function will not return
// until the configure dialog is closed.
// If hwndParent is NULL the Miranda window will be used as parent

// Since 2.0.2.0:
  SFTEX_OPTIONS_PAGE = 0;

// Since 2.5.0.0:
// Combine the following flag with the lParam/uFilterType parameter
// to show the corresponding Spam Definitions info dialog instead
// of the settings dialog:
  SFTEXF_SPAMDEFINITIONS_INFO = $2000;
// Note: SFTEXF_SPAMDEFINITIONS_INFO can't be combined with SFT_ROBOT_FILTER

// Combine the following flag with the lParam/uFilterType of 0 parameter
// to show the Spammers dialog instead of the settings dialog:
  SFTEXF_SPAMMERS_INFO = $4000;
// Note: SFTEXF_SPAMMERS_INFO can't be combined with SFT_*



// Return Values:
// --------------
// Returns 0 on success, nonzero on error.

// Since v2.0.3.0:
// If the specified settings window is already opened it will be
// brought to front (reactivated) instead (returns success though).
// This might be changed however in the future.

// Changed behaviour in v2.0.3.0:
// If hwndParent was not set then the function returned the handle to the new window.
// Now the function returns 0 to indicate success instead.


// -------------------------------------------
// -- Adding message types -------------------
// -------------------------------------------

  MS_SPAMFILTER_REGISTERMESSAGETYPE = 'SpamFilter/MessageTypes/Register';

// Description:
// -------------
// Adds a new message type which can be checked for spam.
// Associated event: ME_SPAMFILTER_PREREGISTERMESSAGETYPE

// Parameters:
// -------------
// wParam = 0
// lParam = (MESSAGETYPEDESC*)mtd (see below)

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.


// Already registered message types are:
// (With pszSection set to the installed network protocol names, e.g. "ICQ")
  SFMT_AUTHREQUEST = 'Auth';
  SFMT_ADDED       = 'Added'; // since v2.1.0.0
  SFMT_MESSAGE     = 'Msg';
  SFMT_URL         = 'URL';
  SFMT_FILE        = 'File';
  SFMT_CONTACTS    = 'Contacts';


// MESSAGETYPEDESC structure:
type
  PMESSAGETYPEDESC = ^TMESSAGETYPEDESC;
  TMESSAGETYPEDESC = record
    cbSize            :integer; // Set to sizeof(MESSAGETYPEDESC)
    pszSection        :PAnsiChar;   // Section of message name for database
                                // e.g. protocol name (Can not be NULL)
    SectionDescription:TChar;   // Readable name of section e.g. protocol name (can be NULL)
                                // Ignored if section existed previously
    hSectionIcon      :THANDLE; // Icon to show for section e.g. protocol icon (can be NULL)
                                // Ignored if section existed previously
    pszName           :PAnsiChar;   // Name to refer to message type when spam
                                // checking and in db (Can not be NULL)
    Description       :TChar;   // Description for options dialog (can be NULL)
    hIcon             :THANDLE; // EITHER: icon handle representing the message
                                // type OR: one of mirandas skin constants with
                                // a negative sign, e.g (HICON)-SKINICON_EVENT_MESSAGE,
                                //  same as for LoadSkinnedIcon() -> can be 0
                                // and can also be a normal icon handle (HICON)
    bDefaultStatus    :bool;    // Whether checking for this type should be
                                // activated or not by default
    dwFlags           :dword;   // Flags for the message type (for possible flags see below)
    iSectionPosition  :integer; // Approx position number for the section in the
                                // options list, lower numbers are nearer to the
                                // top (Can be 0 to be sorted alphabetically)
                                // Ignored if section existed previously (or
                                // if pszSection is NULL) Added in v2.1.1.0
    iPosition         :integer; // Approx position number for the item in the
                                // options list, lower numbers are nearer to the
                                // top (Can be 0 to be sorted alphabetically)
                                // Added in v2.1.1.0
   end;

const

// Possible MESSAGETYPEDESC flags:
  MTDF_DEFAULTDISABLED = $00000001; // Is disabled by default
  MTDF_HIDDEN          = $00000002; // Does not get shown on the options page
// Since v2.5.0.0
  MTDF_UNICODE         = $00000004; // pszSectionDescription and pszDescription are Unicode strings


// -------------------------------------------
// -- Modify new added message type (Event) --
// -------------------------------------------

  ME_SPAMFILTER_PREREGISTERMESSAGETYPE = 'SpamFilter/MessageTypes/PreRegister';

// Description:
// -------------
// Gets fired before a new message type is added to be checked for spam.
//
// This is meant for asking other plugins if the specified default data is ok.
// Other plugins (especially protocols) can disable a message type if either sending spam
// is not possible for those protocols or if spam checking would cause unwanted side-effects.

// Warning: Most elements of the MESSAGETYPEDESC structure can also be NULL.
// However, the structure was checked for errors before this events gets called.

// Parameters:
// -----------------
// wParam = 0 (same wParam as used for MS_SPAMFILTER_REGISTERMESSAGETYPE)
// lParam = (MESSAGETYPEDESC*)mtd (see above, contents can be modified)

// Return Values:
// -----------------
// Returning 0 on this event will accept the message type to be added.
// Returning 1 will prevent the the message type to be added.

// Note: Works as it does now since v2.1.1.0


// -------------------------------------------
// -- Uninstalling a message type ------------
// -------------------------------------------

  MS_SPAMFILTER_REMOVEMESSAGETYPE = 'SpamFilter/RemoveMessageType';

// Description:
// -------------
// Uninstalls a specifed message type. It removes the database setting related to the given message type.
// Should only be used for uninstalling purposes (e.g in uninstall function of PluginUninstaller)

// Parameters:
// -------------
// wParam = (AnsiChar*)pszMsgTypeName (Message type name; You need to use MS_SPAMFILTER_REGISTERMESSAGETYPE before)
// lParam = (AnsiChar*)pszMsgTypeSection (The section of the message type string; You need to use MS_SPAMFILTER_REGISTERMESSAGETYPE before)

// Return Values:
// --------------
// Returns 0 on success, nonzero on error..


// -------------------------------------------
// -- Message type activation ----------------
// -------------------------------------------

  MS_SPAMFILTER_ISMESSAGETYPEACTIVATED = 'SpamFilter/MessageTypes/IsActivated';

// Description:
// -------------
// Returns if a specified message type is activated or not.

// Parameters:
// -------------
// wParam = (AnsiChar*)pszMsgTypeName (Message type name; You need to use MS_SPAMFILTER_REGISTERMESSAGETYPE before)
// lParam = (AnsiChar*)pszMsgTypeSection (The section of the message type string; You need to use MS_SPAMFILTER_REGISTERMESSAGETYPE before)

// Return Values:
// --------------
// Returns TRUE if the message type is activated and FALSE otherwise.


// -------------------------------------------
// -- Reset AutoIgnore counters --------------
// -------------------------------------------

  MS_SPAMFILTER_RESETAUTOIGNORE = 'SpamFilter/AutoIgnore/Reset';

// Description:
// -------------
// Resets the list in which the total number of spam sent by
// each user is kept.
// The list is necessary to determine when a particular user reached the
// allowed spam messages limit.

// Parameters:
// -------------
// wParam = (bool)bOnlyCount (if TRUE the service will only return the user count without doing reset)
// lParam = 0

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.


// -----------------------------------------
// -- Spam Checking ------------------------
// -----------------------------------------

  MS_SPAMFILTER_ADVERTISMENTCHECK = 'SpamFilter/AdvertismentCheck';
  MS_SPAMFILTER_DISLIKEDMESSAGESCHECK = 'SpamFilter/DislikedMessagesCheck';
  MS_SPAMFILTER_ROBOTCHECK = 'SpamFilter/RobotCheck';

// Description:
// -------------
// These services provide the possibility to check a specified text for contained spam.
// The function checks for everything that is activated in the plugin's options.

// Note: These functions work as they do now since v2.1.0.0

// Parameters:
// -------------
// wParam = (SPAMCHECKDATA*)scd
// lParam = 0

// SPAMCHECKDATA structure:
type
  PSPAMCHECKDATA = ^TSPAMCHECKDATA;
  TSPAMCHECKDATA = record
    cbSize           :integer; // sizeof(SPAMCHECKDATA)
    pszMsgTypeSection:PAnsiChar;   // The section of the message type string: You need
                               // to use MS_SPAMFILTER_REGISTERMESSAGETYPE before
    pszMsgTypeName   :PAnsiChar;   // Description of the message: You need to use
                               // MS_SPAMFILTER_REGISTERMESSAGETYPE before
    MsgText          :TChar;   // Pointer to the text of a message which is
                               // checked for spam. Can be NULL.
    dwFlags          :dword;   // Flags for the spam checking.  (For more details see below.)
    case LongInt of
     0: (hContact: TMCONTACT); // Handle to the sending contact (use pszUserName
                               // instead if no hContact is available)
     1: (UserName: TChar);     // Name of the user, e.g. nick (set SCDF_NO_CONTACT
                               // in the flags to use this parameter) -> should
                               // be as unique as possible
  end;

const
// Possible flags in the SPAMCHECKDATA structure:
  SCDF_NO_NOTIFY     = $00000001; // Don't show a popup, play sound or log to file if the message is spam.
  SCDF_NO_CANCEL     = $00000002; // Do check even if user pressed control key to avoid the spam check manually
  SCDF_NO_AUTOIGNORE = $00000004; // Do not use the AutoIgnore feature for this check
  SCDF_NO_CONTACT    = $00000008; // Use pszUserName instead of hContact of union
// Since v2.5.0.0:
  SCDF_UNICODE       = $00000010; // Specify this flag if associated message text and/or user name is Unicode


// Return Values:
// --------------
// Possible return flags:
  SFF_MARKREAD             = $00000010; // Flag
  SFF_DELETE               = $00000020; // Flag
  SFF_IGNORE               = $00000040; // Flag
  SFF_SENDMSG_NOTIFY       = $00000100; // Flag, Only MS_SPAMFILTER_DISLIKEDMESSAGESCHECK
  SFF_SENDMSG_INSTRUCTION  = $00000200; // Flag, Only MS_SPAMFILTER_ROBOTCHECK
  SFF_SENDMSG_CONFIRMATION = $00000400; // Flag, Only MS_SPAMFILTER_ROBOTCHECK
  SFF_TESTPASSED           = $00000080; // Flag, Only MS_SPAMFILTER_ROBOTCHECK
  SFF_ISSPAM               = $00000002; // Flag
  SFF_ISNORMAL             = $00000001; // Flag
// Since 2.5.2.0:
  SFF_HIDE                 = $00001000; // Flag, Only MS_SPAMFILTER_ROBOTCHECK

// -------------------------------------------
// -- Getting additional Check Info ----------
// -------------------------------------------

  MS_SPAMFILTER_GETSPAMCHECKINFO = 'SpamFilter/GetSpamCheckInfo';

// Description:
// -------------
// Gets data needed for the processing of spam mesages.

// Parameters:
// -------------
// wParam = (dword)dwDataType (For possible values see below)
// lParam = depends on wParam (see below)

// Possible values for lParam:

// TRUE/FALSE: if the specified filter is enabled
  SFSCI_ACTIVATION_FILTER     = 1; // lParam = uFilterType (For possible values see above)

// Strings: Get the message texts specified by the user
// ANSI
  SFSCI_MSGTEXT_NOTIFYA       = 2; // lParam = (SPAMCHECKDATA*)scd, scd->hContact (or scd->pszUserName) and scd->pszMsgText (and scd->cbSize) should be filled in.
  SFSCI_MSGTEXT_INSTRUCTIONA  = 3; // lParam: see SFSCI_MSGTEXT_NOTIFYA
  SFSCI_MSGTEXT_CONFIRMATIONA = 4; // lParam: see SFSCI_MSGTEXT_NOTIFYA

// Unicode (since v2.5.0.0)
  SFSCI_MSGTEXT_NOTIFYW       = 6; // lParam = (SPAMCHECKDATA*)scd, scd->hContact (or scd->pwszUserName) and scd->pwszMsgText (and scd->cbSize) should be filled in.
  SFSCI_MSGTEXT_INSTRUCTIONW  = 7; // lParam: see SFSCI_MSGTEXT_NOTIFYW
  SFSCI_MSGTEXT_CONFIRMATIONW = 8; // lParam: see SFSCI_MSGTEXT_NOTIFYW

// TRUE/FALSE: if the above message text should be added to history.
  SFSCI_ADD_TO_HISTORY        = 5; // lParam = SFSCI_MSGTEXT_NOTIFY(A/W), SFSCI_MSGTEXT_INSTRUCTION(A/W) or SFSCI_MSGTEXT_CONFIRMATION(A/W)

// Since 2.5.2.0:
// Counts of milliseconds to delay the above message texts sending.
  SFSCI_DELAY_TIME_REPLY      = 9; // lParam=0


// Return Values:
// --------------
// SFSCI_ACTIVATION_FILTER    -> (bool)TRUE/FALSE
// SFSCI_DELAY_TIME_REPLY    -> (bool)TRUE/FALSE

// SFSCI_MSGTEXT_NOTIFYA    -> (AnsiChar*) free with miranda_sys_free() of "m_system.h", (all %vars% in the text are already resolved)
// SFSCI_MSGTEXT_INSTRUCTIONA  -> see SFSCI_MSGTEXT_NOTIFYA
// SFSCI_MSGTEXT_CONFIRMATIONA  -> see SFSCI_MSGTEXT_NOTIFYA

// SFSCI_MSGTEXT_NOTIFYW    -> (WCHAR*) free with miranda_sys_free() of "m_system.h", (all %vars% in the text are already resolved)
// SFSCI_MSGTEXT_INSTRUCTIONW  -> see SFSCI_MSGTEXT_NOTIFYW
// SFSCI_MSGTEXT_CONFIRMATIONW  -> see SFSCI_MSGTEXT_NOTIFYW

// SFSCI_ADD_TO_HISTORY      -> (bool)TRUE/FALSE

// returns NULL on error


// -------------------------------------------
// -- Confirm spam check (Event) -------------
// -------------------------------------------

  ME_SPAMFILTER_OKTOSPAMCHECK = 'SpamFilter/OkToSpamCheck';

// Description:
// -------------
// Gets fired before the spam check for a message starts.

// Parameters:
// -----------------
// wParam = (SPAMCHECKDATA*)scd (Pointer to SPAMCHECKDATA struct)
// lParam = (uint)uFilterType (For possible values see above)

// Return Values:
// -----------------
// Returning 0 on this event will accept spam checking for the message,
// Returning 1 will prevent the spam check.


// -------------------------------------------
// -- Confirm spam detection (Event) ---------
// -------------------------------------------

  ME_SPAMFILTER_OKTOSPAMDETECTION = 'SpamFilter/OkToSpamDetection';

// Description:
// -------------
// Gets fired after a spam message is detected.

// Parameters:
// -----------------
// wParam = (SPAMCHECKDATA*)scd (Pointer to SPAMCHECKDATA struct)
// lParam = (uint)uFilterType (For possible values see above)

// Return Values:
// -----------------
// Returning 0 on this event will accept the spam detection
// Returning 1 cause the message not to be seen as spam.

// Note: If you only would like to get the final result hook
// ME_SPAMFILTER_SPAMRECEIVED instead


// -------------------------------------------
// -- Spam message received (Event) ----------
// -------------------------------------------

  ME_SPAMFILTER_SPAMRECEIVED = 'SpamFilter/SpamReceived';

// Description:
// -------------
// Gets fired when a message if recognized as spam.

// Parameters:
// -----------------
// wParam = (SPAMCHECKDATA*)scd (Pointer to SPAMCHECKDATA struct)
// lParam = (uint)uFilterType (For possible values see above)

// Return Values:
// -----------------
// The return value should be 0.


// -------------------------------------------
// -- Spam Filter loaded (Event) -------------
// -------------------------------------------

  ME_SPAMFILTER_MODULELOADED = 'SpamFilter/ModuleLoaded';

// Description:
// -------------
// Gets fired when a all parts of the "Spam Filter" were loaded and available
// to other plugins.
// Past this event all functionality of the plugin is fully available.
// The event works quite similar as ME_SYSTEM_MODULESLOADED.
// You can use ME_SYSTEM_MODULESLOADED in most cases instead. Please do so!

// Parameters:
// -----------------
// wParam = 0
// lParam = 0

// Return Values:
// -----------------
// The return value should be 0.



// -------------------------------------------
// -- Helper: Duplicate SPAMCHECKDATA --------
// -------------------------------------------

  MS_SPAMFILTER_COPYSPAMCHECKDATA = 'SpamFilter/SpamCheckData/Copy';

// Description:
// -------------
// Copys all contents of a SPAMCHECKDATA struct (especially strings) to another one
// using Miranda's Memory Manager Interface (see m_system.h).
// Use MS_SPAMFILTER_FREESPAMCHECKDATA to free the result when no longer needed.

// Available since 2.5.2.0.

// Parameters:
// -------------
// wParam = (SPAMCHECKDATA*)pscdTo
// lParam = (SPAMCHECKDATA*)pscdFrom

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.



// -------------------------------------------
// -- Helper: Free SPAMCHECKDATA -------------
// -------------------------------------------

  MS_SPAMFILTER_FREESPAMCHECKDATA = 'SpamFilter/SpamCheckData/Free';

// Description:
// -------------
// Frees the memory allocated by MS_SPAMFILTER_COPYSPAMCHECKDATA.
// using Miranda's Memory Manager Interface (see m_system.h).

// Available since 2.5.2.0.

// Parameters:
// -------------
// wParam = (SPAMCHECKDATA*)pscd
// lParam = 0

// Return Values:
// --------------
// Returns 0 on success, nonzero on failure.



// -------------------------------------------
// -- Showing error messages -----------------
// -------------------------------------------

  MS_SPAMFILTER_SHOWERROR = 'SpamFilter/ShowError';

// Description:
// -------------
// Shows a Miranda try balloon tip, popup or message box
// with the specified error message.

// Parameters:
// -------------
// wParam = (uint)uErrorType (For possible values see below)
// lParam = 0

// Possible values for wParam:
  SFSE_CRITICAL_ERROR = 1; // Unspecified critical error occured related to spam checking.
             // Only use this for situations that may never ever occur.
             // Please do avoid this if possible.

  SFSE_SEND_FAILED = 2; // Show this error when a sending of SFSCI_MSGTEXT_NOTIFY,
            // SFSCI_MSGTEXT_INSTRUCTION, or SFSCI_MSGTEXT_CONFIRMATION failed

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.



// -------------------------------------------
// -- Set contact as spammer (Contact) -------
// -------------------------------------------

  MS_SPAMFILTER_CONTACT_SETASSPAMMER = 'SpamFilter/Contact/SetAsSpammer';

// Description:
// -------------
// This service adds a specified user to the spammer list (fully ignored).

// Note: works same as MS_SPAMFILTER_CONTACT_SHOWSETASSPAMMERDIALOG
// but does not prompt the user.

// Available since v2.5.0.0

// Parameters:
// -------------
// wParam = (HANDLE)hContact (Handle to a contact in database. Can't be NULL.)
// lParam = (dword)dwFlags (flags about what should be done, see below)

// Available Flags:
  SCASF_NO_NOTIFY            = $00000001; // Prevents logging and playing of sounds
  SCASF_USE_ROBOT_SOUND      = $00000002; // play robot sound instead of advertisment sound (only if SCASF_NO_NOTIFY is not set)
  SCASF_NO_REMOVE_HISTORY    = $00000004; // history is marked read instead of fully removed
  SCASF_NO_DENY_AUTHREQUESTS = $00000008; // do not deny pending auth requests

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.


// -------------------------------------------
// -- Show mark spammer dialog (Contact) -----
// -------------------------------------------

  MS_SPAMFILTER_CONTACT_SHOWSETASSPAMMERDIALOG = 'SpamFilter/Contact/ShowSetAsSpammerDialog';

// Description:
// -------------
// This service adds a specified user to the spammer list.
// Before it shows a dialog to let the user confirm the action.
// For use for example as action for a button on the message dialog.

// Note: This service performs the same tasks as MS_SPAMFILTER_SETCONTACTASSPAMMER
// to execute the action the user chooses.

// Available since v2.5.0.0

// Parameters:
// -------------
// wParam = (HANDLE)hContact (Handle to a contact in database. Can't be NULL.)
// lParam = (HWND)hwndParent (Handle to the parent window. Can be NULL.)

// Return Values:
// --------------
// Returns FALSE if user clicked cancel on the dialog.
// TRUE if the contact was handled as spammer.


// Changed in v2.5.0.0:
// The service MS_SPAMFILTER_SETSPAMMANUALLY has been deprecated.
// New plugins should use MS_SPAMFILTER_CONTACT_SHOWSETASSPAMMERDIALOG instead.
  MS_SPAMFILTER_SETSPAMMANUALLY = 'SpamFilter/SetSpamManually';


// -------------------------------------------
// -- Test if contact is spammer (Contact) ---
// -------------------------------------------

  MS_SPAMFILTER_CONTACT_ISSPAMMER = 'SpamFilter/Contact/IsSpammer';

// Description:
// -------------
// This service tests if a specified contact is on the spammer list (fully ignored).

// Available since v2.5.0.0

// Parameters:
// -------------
// wParam = (HANDLE)hContact (Handle to a contact in database. Can't be NULL.)
// lParam = 0

// Return Values:
// --------------
// Returns TRUE when the contact is on the spammer list, FALSE otherwise.


// -------------------------------------------
// -- Unmarks a contact as spammer (Contact) -
// -------------------------------------------

  MS_SPAMFILTER_CONTACT_UNSETSPAMMER = 'SpamFilter/Contact/UnSetSpammer';

// Description:
// -------------
// This service restores a spammer to be again a normal contact (unignored).

// Available since v2.5.0.0

// Parameters:
// -------------
// wParam = (HANDLE)hContact (Handle to a contact in database. Can't be NULL.)
// lParam = 0

// Return Values:
// --------------
// Returns 0 on success, nonzero on error.


// -------------------------------------------
// -- Spammer state changed (Event) ----------
// -------------------------------------------

  ME_SPAMFILTER_CONTACT_SPAMMERSTATECHANGED = 'SpamFilter/Contact/SpammerStateChanged';

// Description:
// -------------
// Gets fired when a a contact gets marked as spammer or unmarked.

// Parameters:
// -----------------
// wParam = (HANDLE)hContact
// lParam = (bool)bIsSpammer (new state)

// Return Values:
// -----------------
// The return value should be 0.

{$ENDIF}
