NAME
    proxySwith - Automatic proxy settings manager

FEATURES
    proxySwitch can enable/disable proxy in Miranda IM, Internet Explorer,
    and Firefox based on the current IP address and defined rules. This
    plugin is useful mainly for laptop/notebook owners.

    * enables/disables proxy for all Miranda IM plugins on startup and later
    on any IP address change
    * reconnects all Miranda IM protocols if necessary
    * notifies user on every network interface IP address change
    * recognizes which network interface is used by Miranda IM and
    reconnects if that interface loses IP address
    * can display current IP addresses (per interface)
    * can copy current IP address to clipboard (per interface)
    * enables/disables proxy for Internet Explorer (no need to restart IE)
    * enables/disables proxy for Firefox (you must close and rerun Firefox)

    Why I wrote this plugin? Because I don't want to do many things
    manually:

    * I have to use a proxy server at work but I don't have one at home this
    plugin will change the settings for me when I change the place
    * I'm connected to LAN and Wifi at the same time and when I unplug the
    ethernet cable (rushing to the meeting) I want Miranda to reconnect
    using Wifi but only if it was connected via ethernet cable

    And what the plugin can't do?

    * If you need other software's proxy usage to be managed by this plugin
    feel free to modify the source and build your version of proxySwitch.
    * It's not intended for changing the proxy setup. If you need proxy_1 at
    work and proxy_2 at home (or other location) this plugin will be useless
    for you. Well, this functionality could be added... Any volunteers?

  Screenshot
        http://trooper.tricube.cz/screenshot.gif

REQUIREMENTS
    This plugin requires modified Netlib. It is not possible to change the
    proxy settings run-time and it is not possible to determine which
    network interface is used by Miranda IM without this modification. It
    means that you need new modified miranda32.exe!

    If you compile miranda for yourself you can download a patch file
    otherwise pre-compiled binary is ready for you.

    The updated Miranda should have "(Netlib modf)" tag on the About dialog.

    I hope my modification will be soon incorporated into Miranda IM v0.4
    official source tree.

DOWNLOAD
  Plugin
    Download or install the plugin using Miranda Installer. The source is
    available also. Please note the requirements above.

        http://miranda-im.org/download/feed.php?dlfile=1682
        http://miranda-im.org/download/feed.php?mirinst=1&dlfile=1682
        http://miranda-im.org/download/feed.php?dlsource=1682

  Miranda
    Replace your miranda32.exe with the respective version downloaded from
    this page. Or you can download a patch file to modify the source and you
    can compile it for yourself.

        http://trooper.tricube.cz/miranda-0.3.3.1.zip
        http://trooper.tricube.cz/miranda-0.3.4.zip
        http://trooper.tricube.cz/miranda-0.4.zip

        http://trooper.tricube.cz/miranda_modif_netlib_v0331.diff
        http://trooper.tricube.cz/miranda_modif_netlib_v034x.diff
        http://trooper.tricube.cz/miranda_modif_netlib_v04xx.diff

    Apply the diff to the Miranda source manually or using Cygwin GNU patch
    program:

            patch -l -u -d miranda_src331 -p 1 < miranda_modif_netlib_v03xx.diff

CHANGE HISTORY
    v1.0.0.1 (2005-03-11)

            * Plugin can now handle Firefox proxy settings but unlike IE you
            have to close and reopen Firefox windows to take the change
            effect
            * Reconnects Miranda on any IP address change if offline and
            Always Reconnect is selected
            * Fixed detection of network interfaces used by Miranda IM
            * Fixed options UI layout

    v0.0.2 (2005-01-18)

            * Improved detection of network interface used by Miranda IM now
            does not require Win XP+
            * Fixed (enlarged) UI to enable translation to other languages
            * Fixed crash when new network interface is discovered or
            existing one is disabled

    v0.0.1 (2005-01-15)

            * First public release

AUTHOR
    Petr Smejkal <petr.smejkal@seznam.cz>, 11 March 2005.

