/*

'AutoShutdown'-Plugin for
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (C) 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_SHUTDOWN_H__
#define M_SHUTDOWN_H__

/******************************************************************/
/************************* SERVICES *******************************/
/******************************************************************/

/* Show settings dialog   v1.3.0.0+
Shows the dialog by which the shutdown watcher can be started.
If the dialog is already opened this brings it to foreground.
 wParam=lParam=0
Returns 0 on success, nonzero otherwise.
*/
#define MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG  "AutoShutdown/ShowSettingsDialog"

/* Initiate shutdown process   v1.3.0.0+
Initiates the shutdown process for a given shutdown type.
If another shutdown process is already pending it will return error.
Associated events: ME_AUTOSHUTDOWN_OKTOSHUTDOWN, ME_AUTOSHUTDOWN_SHUTDOWN
 wParam=shutdownType
 lParam=(LPARAM)(BOOL)fShowConfirmDlg (whether to show the warning dialog or not)
Returns 0 on success, nonzero otherwise.
*/
#define MS_AUTOSHUTDOWN_SHUTDOWN  "AutoShutdown/Shutdown"

/* shutdown types */
#define SDSDT_CLOSEMIRANDA         1  // close miranda process
#define SDSDT_SETMIRANDAOFFLINE    2  // set all protocols to offline
#define SDSDT_LOGOFF               3  // logoff from Windows
#define SDSDT_REBOOT               4  // reboot computer
#define SDSDT_SHUTDOWN             5  // shutdown Windows and power off
#define SDSDT_STANDBY              6  // standby mode
#define SDSDT_HIBERNATE            7  // hibernate mode
#define SDSDT_LOCKWORKSTATION      8  // lock the workstation
#define SDSDT_CLOSERASCONNECTIONS  9  // close all dialup connections
#define SDSDT_MAX                  9

/* Get shutdown type description   v1.4.0.0+
Gets a textual description of the given shutdown type.
 wParam=shutdownType
 lParam=flags (see GSTDF_* below)
Returns a static buffer of the description of the given shutdown type.
It does not need to be freed in any way.
The returned text is already translated.
Returns a pointer to a string on success, NULL on error.
*/
#define MS_AUTOSHUTDOWN_GETTYPEDESCRIPTION  "AutoShutdown/GetTypeDescription"

#define GSTDF_LONGDESC      0x0001  // returns a long description
#define GSTDF_UNICODE       0x0002  // returns a Unicode string
#define GSTDF_UNTRANSLATED  0x0004  // returns an untranslated string
#if defined(_UNICODE)
   #define GSTDF_TCHAR  GSTDF_UNICODE  // will return WCHAR*
#else
   #define GSTDF_TCHAR  0              // will return char*, as usual
#endif

/* Check if shutdown type is enabled   v1.4.0.0+
Tests if a specific shutdown type is activated and its use is possible
on the system. For example hibernate and stand-by modes are not available on
older versions of Windows (needs Windows ME/2000+).
Shutdown might also be prohibited by security restrictions.
This services checks it all.
However, there is no need to call this before calling MS_AUTOSHUTDOWN_SHUTDOWN.
MS_AUTOSHUTDOWN_SHUTDOWN will fail if the given shutdown type is not enabled.
 wParam=shutdownType
 lParam=0
Returns TRUE if the given shutdown type is enabled, FALSE otherwise.
*/
#define MS_AUTOSHUTDOWN_ISTYPEENABLED  "AutoShutdown/IsTypeEnabled"

/* Start shutdown watcher   v1.4.0.0+
Starts the watcher using the last settings specified on the dialog
shown by MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG.
Call MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG instead to offer
options about what watcher to use.
Associated event: ME_AUTOSHUTDOWN_WATCHERCHANGED
 wParam=lParam=0
Returns 0 on success, nonzero otherwise.
*/
#define MS_AUTOSHUTDOWN_STARTWATCHER  "AutoShutdown/StartWatcher"

/* Stop shutdown watcher   v1.4.0.0+
Stops the currently running watcher.
If the watcher is not running, it returns error.
Associated event: ME_AUTOSHUTDOWN_WATCHERCHANGED
 wParam=lParam=0
Returns 0 on success, nonzero otherwise.
*/
#define MS_AUTOSHUTDOWN_STOPWATCHER  "AutoShutdown/StopWatcher"

/* Check if watcher is running   v1.4.0.0+
Checks if the watcher is currently active or not.
 wParam=lParam=0
Returns TRUE if the watcher is enabled, FALSE otherwise.
*/
#define MS_AUTOSHUTDOWN_ISWATCHERENABLED  "AutoShutdown/IsWatcherEnabled"

/******************************************************************/
/************************** EVENTS ********************************/
/******************************************************************/

/* Disallow shutdown process (event)   v1.3.0.0+
Gets fired when MS_AUTOSHUTDOWN_SHUTDOWN is called.
Parmeters are the same as specified at the call to MS_AUTOSHUTDOWN_SHUTDOWN.
 wParam=shutdownType
 lParam=(LPARAM)(BOOL)fShowConfirmDlg
Return 0 to allow shutdown, 1 to disallow.
*/
#define ME_AUTOSHUTDOWN_OKTOSHUTDOWN  "AutoShutdown/OkToShutdown"

/* Shutdown process started (event)   v1.3.0.1+
Gets fired when ME_AUTOSHUTDOWN_OKTOSHUTDOWN was confirmed.
Parmeters are the same as specified at the call
to MS_AUTOSHUTDOWN_SHUTDOWN.
 wParam=shutdownType
 lParam=(LPARAM)(BOOL)fShowConfirmDlg
Unused, return always 0 here.
*/
#define ME_AUTOSHUTDOWN_SHUTDOWN  "AutoShutdown/ShutdownEvent"

/* Watcher changed (event)   v1.3.0.0+
Fired when MS_AUTOSHUTDOWN_STARTWATCHER or MS_AUTOSHUTDOWN_STOPWATCHER
is called.
 wParam=(WPARAM)(BOOL)fIsStarted (whether whe watcher is running now)
 lParam=0
Unused, return always 0 here.
*/
#define ME_AUTOSHUTDOWN_WATCHERCHANGED  "AutoShutdown/Watcher/Changed"


#ifndef SHUTDOWN_NOSETTINGS
#define SETTING_REMEMBERONRESTART_DEFAULT    0  // SDROR_RUNNING
#define SETTING_SHOWCONFIRMDLG_DEFAULT       1
#define SETTING_CONFIRMDLGCOUNTDOWN_DEFAULT  30 // seconds
#define SETTING_WEATHERSHUTDOWN_DEFAULT      0
#define SETTING_SMARTOFFLINECHECK_DEFAULT    1

#define SETTING_SHUTDOWNTYPE_DEFAULT         SDSDT_SHUTDOWN
#define SETTING_WATCHERFLAGS_DEFAULT         (SDWTF_SPECIFICTIME|SDWTF_ST_COUNTDOWN)
#define SETTING_COUNTDOWN_DEFAULT            30
#define SETTING_COUNTDOWNUNIT_DEFAULT        60 // x times countdown seconds
#define SETTING_TIMESTAMP_DEFAULT            (DWORD)(time(NULL)+(SETTING_COUNTDOWN_DEFAULT*SETTING_COUNTDOWNUNIT_DEFAULT))
#define SETTING_CPUUSAGETHRESHOLD_DEFAULT    90 // percent

#define SDWTF_SPECIFICTIME  0x0001
#define SDWTF_ST_TIME       0x0002
#define SDWTF_ST_COUNTDOWN  0x0004
#define SDWTF_ST_MASK       0x0006 /* bitmask for SDWTF_ST_* bits */
#define SDWTF_MESSAGE       0x0008
#define SDWTF_FILETRANSFER  0x0010
#define SDWTF_IDLE          0x0020
#define SDWTF_STATUS        0x0040
#define SDWTF_CPUUSAGE      0x0080
#define SDWTF_MASK          0x00FF /* bitmask for all SDWTF_* bits */

#define SDROR_RUNNING       3
#endif

#endif // M_SHUTDOWN_H
