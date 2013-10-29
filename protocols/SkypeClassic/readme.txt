Skype Protocol - Maybe we can call it beta now? ;)

As so many people requested it, here is now a implementation of the Skype
protocol for Miranda IM.
Note, that this is just a wrapper for the Skype-API, which means that Skype
has to be running while you use this plugin.
The plugin should be able to launch Skype, if it is not running on startup.
Please note, that I never coded a protocol-plugin for Miranda before, so
expect it to be buggy and unstable, I hope I have some time to correct the
severest bugs and to add more features soon.
No warranty, whatsoever! I suggest that you back up your existing Miranda
Database before using this plugin, just to be sure you have a backup if
it runs amok ;-) Feel free to improve the ugly sourcecode.
Please give me feedback, if it works for you, it would be interesting.

NOTES - READ THEM CAREFULLY!
----------------------------

  * You need Skype 1.0.0.97 or above in order to have access to the Skype API 
  * You have to manually disable popup of messages in SKYPE, as there is
    currently no function in the API to do this
    Got to File/Options/Short messages and disable the checkboxes there
    Otherwise you would get all Skype-Messages twice (in Skype AND Miranda)
  * If you always get "Unknown event" when a call is incoming and you are using
    the NewEventNotify-plugin or Tabsrmm then go to the plugin's options 
    (Sessions / Event Notifications / Announce events of type) and disable 
    "other events" there.
  * Importing history for a contact can be launched by clickting "import History"
    in the context menu of a contact. The importing takes place in the background.
    AS soon, as it's finished, you will be notified by a messagebox.
    This feature is still buggy and quite a bit unpredictable.
  * There is a nice Iconset with the original Skype-Icons created by X-Byte
    Thanks for that. Grab it at http://dose.0wnz.at/Skype/Skype_icons.zip 
  * Skype API bug: When you set Skype offline via API, the contacts stay online
    I made a Miranda-sie workaround for this now, but it's a Skype-API bug,
    not a plugin-bug
  * Skype API bug: when you rename Skypeout-contacts in Skype, their new nicks
    aren't sent correctly via the API, instead, the previous nick is sent and
    therefore the Miranda-name is not up-to-date after nick change.
    This is not a plugin bug either.
BUGS
----

 * To track bugs, you have to make a Debug-Build out from the source.
   The plugin then will log the Skype-API communication to a logfile
   called skype_log.txt so that we may se what caused the crash.

