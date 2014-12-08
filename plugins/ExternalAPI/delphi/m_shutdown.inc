{

'AutoShutdown'-Plugin for
Miranda IM: the free IM client for Microsoft* Windows*

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
}

{$IFNDEF M_SHUTDOWN}
{$DEFINE M_SHUTDOWN}

const
  MIID_SHUTDOWN:TGUID = '{142982A8-88CF-4C65-8721-6DE27D5CE7B5}';

//******************************************************************/
//************************* SERVICES *******************************/
//******************************************************************/

{ Show settings dialog   v1.3.0.0+
Shows the dialog by which the shutdown watcher can be started.
If the dialog is already opened this brings it to foreground.
 wParam=lParam=0
Returns 0 on success, nonzero otherwise.
}
  MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG:PAnsiChar = 'AutoShutdown/ShowSettingsDialog';

{ Initiate shutdown process   v1.3.0.0+
Initiates the shutdown process for a given shutdown type.
If another shutdown process is already pending it will return error.
Associated events: ME_AUTOSHUTDOWN_OKTOSHUTDOWN, ME_AUTOSHUTDOWN_SHUTDOWN
 wParam=shutdownType
 lParam=(LPARAM)(bool)fShowConfirmDlg (whether to show the warning dialog or not)
Returns 0 on success, nonzero otherwise.
}
  MS_AUTOSHUTDOWN_SHUTDOWN:PAnsiChar = 'AutoShutdown/Shutdown';

//* shutdown types */
  SDSDT_CLOSEMIRANDA        = 1;  // close miranda process 
  SDSDT_SETMIRANDAOFFLINE   = 2;  // set all protocols to offline
  SDSDT_LOGOFF              = 3;  // logoff from Windows
  SDSDT_REBOOT              = 4;  // reboot computer
  SDSDT_SHUTDOWN            = 5;  // shutdown Windows and power off
  SDSDT_STANDBY             = 6;  // standby mode
  SDSDT_HIBERNATE           = 7;  // hibernate mode
  SDSDT_LOCKWORKSTATION     = 8;  // lock the workstation
  SDSDT_CLOSERASCONNECTIONS = 9;  // close all dialup connections
  SDSDT_MAX                 = 9;

{ Get shutdown type description   v1.4.0.0+
Gets a textual description of the given shutdown type.
 wParam=shutdownType
 lParam=flags (see GSTDF_* below)
Returns a static buffer of the description of the given shutdown type.
It does not need to be freed in any way.
The returned text is already translated.
Returns a pointer to a string on success, NULL on error.
}
  MS_AUTOSHUTDOWN_GETTYPEDESCRIPTION:PAnsiChar = 'AutoShutdown/GetTypeDescription';

  GSTDF_LONGDESC     = $0001;  // returns a long description
  GSTDF_UNICODE      = $0002;  // returns a Unicode string
  GSTDF_UNTRANSLATED = $0004;  // returns an untranslated string

{ Check if shutdown type is enabled   v1.4.0.0+
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
}
  MS_AUTOSHUTDOWN_ISTYPEENABLED:PAnsiChar = 'AutoShutdown/IsTypeEnabled';

{ Start shutdown watcher   v1.4.0.0+
Starts the watcher using the last settings specified on the dialog
shown by MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG.
Call MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG instead to offer
options about what watcher to use.
Associated event: ME_AUTOSHUTDOWN_WATCHERCHANGED
 wParam=lParam=0
Returns 0 on success, nonzero otherwise.
}
  MS_AUTOSHUTDOWN_STARTWATCHER:PAnsiChar = 'AutoShutdown/StartWatcher';

{ Stop shutdown watcher   v1.4.0.0+
Stops the currently running watcher.
If the watcher is not running, it returns error.
Associated event: ME_AUTOSHUTDOWN_WATCHERCHANGED
 wParam=lParam=0
Returns 0 on success, nonzero otherwise.
}
  MS_AUTOSHUTDOWN_STOPWATCHER:PAnsiChar = 'AutoShutdown/StopWatcher';

{ Check if watcher is running   v1.4.0.0+
Checks if the watcher is currently active or not.
 wParam=lParam=0
Returns TRUE if the watcher is enabled, FALSE otherwise.
}
  MS_AUTOSHUTDOWN_ISWATCHERENABLED:PAnsiChar = 'AutoShutdown/IsWatcherEnabled';

//******************************************************************/
//************************** EVENTS ********************************/
//******************************************************************/

{ Disallow shutdown process (event)   v1.3.0.0+
Gets fired when MS_AUTOSHUTDOWN_SHUTDOWN is called.
Parmeters are the same as specified at the call to MS_AUTOSHUTDOWN_SHUTDOWN.
 wParam=shutdownType
 lParam=(LPARAM)(bool)fShowConfirmDlg
Return 0 to allow shutdown, 1 to disallow.
}
  ME_AUTOSHUTDOWN_OKTOSHUTDOWN:PAnsiChar = 'AutoShutdown/OkToShutdown';

{ Shutdown process started (event)   v1.3.0.1+
Gets fired when ME_AUTOSHUTDOWN_OKTOSHUTDOWN was confirmed.
Parmeters are the same as specified at the call
to MS_AUTOSHUTDOWN_SHUTDOWN.
 wParam=shutdownType
 lParam=(LPARAM)(bool)fShowConfirmDlg
Unused, return always 0 here.
}
  ME_AUTOSHUTDOWN_SHUTDOWN:PAnsiChar = 'AutoShutdown/ShutdownEvent';

{ Watcher changed (event)   v1.3.0.0+
Fired when MS_AUTOSHUTDOWN_STARTWATCHER or MS_AUTOSHUTDOWN_STOPWATCHER
is called.
 wParam=(WPARAM)(bool)fIsStarted (whether whe watcher is running now)
 lParam=0
Unused, return always 0 here.
}
  ME_AUTOSHUTDOWN_WATCHERCHANGED:PAnsiChar = 'AutoShutdown/Watcher/Changed';

  SDWTF_SPECIFICTIME = $0001;
  SDWTF_ST_TIME      = $0002;
  SDWTF_ST_COUNTDOWN = $0004;
  SDWTF_ST_MASK      = $0006; // bitmask for SDWTF_ST_* bits
  SDWTF_MESSAGE      = $0008;
  SDWTF_FILETRANSFER = $0010;
  SDWTF_IDLE         = $0020;
  SDWTF_STATUS       = $0040;
  SDWTF_CPUUSAGE     = $0080;
  SDWTF_MASK         = $00FF; // bitmask for all SDWTF_* bits

  SETTING_REMEMBERONRESTART_DEFAULT   = 0;  // SDROR_RUNNING
  SETTING_SHOWCONFIRMDLG_DEFAULT      = 1;
  SETTING_CONFIRMDLGCOUNTDOWN_DEFAULT = 30; // seconds
  SETTING_WEATHERSHUTDOWN_DEFAULT     = 0;
  SETTING_HDDOVERHEATSHUTDOWN_DEFAULT = 1;
  SETTING_SMARTOFFLINECHECK_DEFAULT   = 1;

  SETTING_SHUTDOWNTYPE_DEFAULT        = SDSDT_SHUTDOWN;
  SETTING_WATCHERFLAGS_DEFAULT        = SDWTF_SPECIFICTIME or SDWTF_ST_COUNTDOWN;
  SETTING_COUNTDOWN_DEFAULT           = 30;
  SETTING_COUNTDOWNUNIT_DEFAULT       = 60; // x times countdown seconds
  SETTING_TIMESTAMP_DEFAULT           = SETTING_COUNTDOWN_DEFAULT*SETTING_COUNTDOWNUNIT_DEFAULT;
  SETTING_CPUUSAGETHRESHOLD_DEFAULT   = 90; // percent

  SDROR_RUNNING = 3;

{$ENDIF}
