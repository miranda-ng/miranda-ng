KeepStatus plugin for Miranda IM v0.0.2.x
unregistered@users.sourceforge.net

--- About ---

KeepStatus, a connection checker, for use with (or without)
StartupStatus and/or AdvancedAutoAway.

--- Usage ---

There are two tabs in the options screen, one with basic options, and
one with advanced options.

The Basic Tab:

[] Show popups, this will make use of the popup plugin to notify you
in case of a connection loss or reconnect attempt.

[] Check connection, with this option you can enable or disable the
plugin.

[count] Max. retries, in case connection is lost, this number of
reconnect attempts will be made to restore the connection.

[secs] Delay between retries, the number of seconds to wait before a
new attempt is made.

[] Don't reconnect if no internet connection seems available, if
enabled, KS will not reconnect if Windows reports that no internet
connection is available.

[] Continuesly check for internet connection, if enables, KS will keep
looking for a connection, either by asking Windows, or by pinging a
host. You can specify multiple hosts by seperating them with
spaces. If one host replies, a connection is assumed to be available.

Note: All dial-up options are experimental. I cannot test these
myself. Please let me know if you encounter problems with it (and you
are willing to run some tests ;) )

The Advanced Tab:

[] Increase delay exponential, if enabled, the delay is multiplied by
2 after each check. So checks are made  at t=10, t=20, t=40, t=40,
t=40, etc. if "Max. delay" = 40, "Initial  delay" = 10, and connection
was lost at t=0.

[secs] Max. delay, the maximum delay allowed in case "increase delay
exponential" is enabled.

[secs] Max. protocol connecting time, -experimental-, this is the
maximum number of seconds a protocol is allowed to be in the
"connecting state" during a reconnect attempt. After this time, KS
will force the protocol offline and make a new connection attempt.

[] Ignore locked status of protocols, if enabled, KS will reconnect
protocols in case of a global status change, even if they are locked
by the clist (nicer/modern). You can enable this if protocols don't
reconnect properly after a global status change using a clist that
doesn't support locking.

[] Set protocol offline before a connection attempt, this will force a
protocol offline, before KS tries to restore its connection.

[] Cancel all if a protocol connects from another location, this will
not reconnect any of the protocols if one of them is connected from
another location. The reason for this option is because not all
protocols can detect a second login.

[] Reconnect on APM resume, will reconnect you when you come back from
standby or hibernation. If it for some reason fails, please try
another clist.

[] React on login errors, influences the way KS deals with login
errors (default is to keep retrying). A login error can be "wrong
password" or "server busy" for example.

() Stop trying to reconnect, cancels reconnecting for the protocol.

() Set delay to [secs], this will (possibly) increase the delay before
a new attempt is made to restore the connection.

Consider connection lost after [count] failed pings, tells KS how many
ping attempts must fail before the connection should be considered
"lost" and need to be restored.

[secs] Continuously check interval, the delay between two pings.

--- Notes ---

All options regarding dial-up connections are experimental.

--- Changelog ---

0.0.2.92: - fixes for Miranda 0.8 (added MUID)
          - some minor fixes.

0.0.2.85: - Different handling of timers
		- Partitial locked status support (full support requires changes in core)
		- Fix 'random' crash

0.0.2.74: - A few more fixes

0.0.2.73: - Fix when setting no status message

0.0.2.72: - Fix options dialog glitch

0.0.2.71: - Several bug fixes
		- Added options for continuously checking
		- Support for "protocol locking" by clist
		- Better NAS support (requires NAS  v0.3.7.2 (build 2338; April 30, 2006))
		- Added more convenient service for developers MS_KS_ANNOUNCESTATUSCHANGE
		- Popups stay until next retry if delay is set to 0
		- Added option "Max. connecting time", see documentation
		- Added option to set protocol offline before reconnecting
		- Set global status when possible
		- Ping using ICMP instead of using InternetCheckConnection.

0.0.2.4: - popups now show the icon of the first protocol that is about to be reconnected
	 - added popup option to show additional info in popup

0.0.2.3: - you can now change the delay when a login error occurs

0.0.2.2: - many bugfixes (with great help of weinstock and ghazan)
	 - added option to check internet connection by pinging
	 - added more options for popups
	 - added option to reconnect on APM resume (standby, hibernate)

0.0.1.0: - doesn't reconnect when logged in from another location
	 - Added: option reconnect on login failures

0.0.0.4: - fixed: now also works with popups disabled. (thanks to Stefan Waldmann)

0.0.0.3: - Added support for PluginUninstaller
	 - bug fixes

0.0.0.2: - Several bug fixes
	 - Better menu handling
	 - Fixed: Jabber/Tlen support (not in combination with (current) awaysys!)

0.0.0.1: - First release without StartupStatus

--- Disclaimer ---

If something terrible happens, don't blame me.